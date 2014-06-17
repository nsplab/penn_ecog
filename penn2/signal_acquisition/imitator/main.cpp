#include <iostream>
#include <sstream>
#include <thread>
#include <chrono>

#include <zmq.hpp>

#include <armadillo>
#include <eigen3/Eigen/Dense>

#include "../../libs/rse/reachstateequation.h"
#include "../../libs/rse/matrix.h"

using namespace std;
using namespace zmq;
using namespace Eigen;

float x=0, y=0, z=0;
float prevx=0, prevy=0, prevz=0;
float diffx=0.0, diffy=0.0, diffz=0.0;

//launcher calls the main function below to set direct = true or false based on user preference
// direct = true runs a mode where output of the imitator is the actual intended velocity, rather than
// direct = false runs a mode in which the imitator outputs sin waves at set frequencies (one for each of 3 features, roughly in the 80-90 Hz range)
bool direct = false;

float sign(float par) {
    if (par < 0.0f)
        return -1;
    return +1;
}


context_t context(2);

bool quit = false;
void GenerateSignal() {
    socket_t publisher(context, ZMQ_PUB);

    int hwm = 1;
    publisher.setsockopt(ZMQ_SNDHWM, &hwm, sizeof(hwm));
    int conflate = 1;
    publisher.setsockopt(ZMQ_CONFLATE, &conflate, sizeof(conflate));

    if (!direct)
        publisher.bind("ipc:///tmp/signal.pipe");
    else {
        cout<<"bypass feature extractor"<<endl;
        publisher.bind("ipc:///tmp/features.pipe");
    }

    float xSignalAmp = 2.0f;
    float xSignalFrq = 80.0f; // Hz

    float ySignalAmp = 2.0f;
    float ySignalFrq = 85.0f; // Hz

    float zSignalAmp = 2.0f;
    float zSignalFrq = 90.0f; // Hz

    size_t samplingRate = 1000; // Hz

    // the synthetic signals are mixed to generate the output singnals/channels
    int numberOfChannels = 4; // number of output channels
    Matrix<float, Dynamic, Dynamic> mixingMatrix(numberOfChannels,3);
    mixingMatrix << 1, 0, 0,
                    0, 1, 0,
                    0, 0, 1,
                    1, 1, 1;

    Matrix<float, Dynamic, Dynamic> signal(numberOfChannels,1); // output signal
    //Matrix<float, Dynamic, Dynamic> tsignal(60,1); // output signal
    Vector3f sample; // synthetic signal based on x,y,z from kinect

    for (size_t timeStamp=0; !quit; timeStamp++) {

        size_t i = timeStamp % samplingRate;
        float dx = diffx;
        float dy = diffy;
        float dz = diffz;

        // assume intended velocity is power modulated
        float maxSpeed = 50.0;
        float baselinePower = maxSpeed;
        if (dx >= maxSpeed)
            dx = maxSpeed;
        else if (dx <= -maxSpeed)
            dx = -maxSpeed;
        if (dy >= maxSpeed)
            dy = maxSpeed;
        else if (dy <= -maxSpeed)
            dy = -maxSpeed;
        if (dz >= maxSpeed)
            dz = maxSpeed;
        else if (dz <= -maxSpeed)
            dz = -maxSpeed;

        float noiseToSignalRatio = 2.0;
        // add noise ~ gaussian(0,1)
        arma::vec v = arma::randn<arma::vec>(3);
        //sample(0) += v(0);
        //sample(1) += v(1);
        //sample(2) += v(2);

        // change this to above baseline threshold (3 times variance + mean) : + velocity otherwise - velocity

        //cout<<"v0 "<<v(0)<<endl;
        //cout<<"v1 "<<v(1)<<endl;
        //cout<<"v2 "<<v(2)<<endl;


        sample(0) = cos(2.0*M_PI * float(i)/float(samplingRate) * xSignalFrq) * xSignalAmp * sqrt(dx+baselinePower);// + v(0) * noiseToSignalRatio);
        sample(1) = cos(2.0*M_PI * float(i)/float(samplingRate) * ySignalFrq) * ySignalAmp * sqrt(dy+baselinePower);// + v(1) * noiseToSignalRatio);
        sample(2) = cos(2.0*M_PI * float(i)/float(samplingRate) * zSignalFrq) * zSignalAmp * sqrt(dz+baselinePower);// + v(2) * noiseToSignalRatio);

        signal = mixingMatrix * sample;
        //cout<<"signal: "<<signal<<endl;
        //cout<<"numberOfChannels "<<numberOfChannels<<endl;
        //cout<<"t: "<<timeStamp<<endl;

        if (direct) {
            signal(0) = xSignalAmp * sqrt(dx+baselinePower);
            signal(1) = ySignalAmp * sqrt(dy+baselinePower);
            signal(2) = zSignalAmp * sqrt(dz+baselinePower);
        }

        //message_t zmqMessage(sizeof(float)*numberOfChannels+sizeof(size_t));
        message_t zmqMessage(sizeof(float)*numberOfChannels+sizeof(size_t));
        memcpy(zmqMessage.data(), &timeStamp, sizeof(size_t));
        memcpy(static_cast<size_t*>(zmqMessage.data())+1, signal.data(), sizeof(float)*numberOfChannels);
        //memcpy(static_cast<size_t*>(zmqMessage.data())+1, tsignal.data(), sizeof(float)*60);

        //cout<<"signal: "<<signal<<endl;
        publisher.send(zmqMessage);

        if (!direct) {
            this_thread::sleep_for(chrono::microseconds(static_cast<int>(1.0/samplingRate * 1000000.0)));
        } else {
            // direct mode imitator: introduce a delay between features issued by the imitator in order to avoid flooding the receiving filter with features.
            this_thread::sleep_for(chrono::microseconds(static_cast<int>(100000.0)));
        }
    }
}

int main(int argc, char** argv)
{
    cout<<"argc: "<<argc<<endl;
    if (argc == 2) {
        direct = true;
    }

    thread broadcast(GenerateSignal);

    double timeBin = 0.1;
    const double reachTimeSteps = 5.0/timeBin;
    const double maxTimeSteps = 10.0/timeBin;
    size_t dim = 3;
    double diagQ=1.0e-3;
    double finalPosCov=1.0e-6;
    double finalVelCov=1.0e-8;

    //reachStateEquation rseComputer(maxTimeSteps, reachTimeSteps, reachTarget, timeBin);
    RSEMatrixStruct rseParams;// = rseComputer.returnAnswer();

    socket_t supervisor(context, ZMQ_REQ);
    supervisor.connect("ipc:///tmp/supervisor.pipe");

    unsigned prevTrial = -1;
    unsigned currentTrial = -1;
    
    string sendMsg("pass");

    vector<float> target(3);
    vector<float> handPos(3);



    arma::vec handState;

    int timeStep = 0;

    // storing the old velocity values and limit the acceleration in the controler
    float diffxOld, diffyOld, diffzOld;

    // threshold on acceleration
    float accThreshold = 0.2;

    for (;;)   {

        message_t zmq_message(sendMsg.length());
        memcpy((char *) zmq_message.data(), sendMsg.c_str(), sendMsg.length());
        cout<<"before supervisor.send"<<endl;
        supervisor.send(zmq_message);
        cout<<"after supervisor.send"<<endl;

        message_t supervisor_msg;
        cout<<"before supervisor.recv"<<endl;
        supervisor.recv(&supervisor_msg);
        cout<<"after supervisor.recv"<<endl;

        string recvMsg;
        recvMsg.resize(supervisor_msg.size(),'\0');
        recvMsg.assign((char *)supervisor_msg.data(),supervisor_msg.size());
        cout<<"recvMsg "<<recvMsg<<endl;
        stringstream ss(recvMsg);
        cout<<"timeStep "<<timeStep<<endl;
        
        // extract target position, hand position, trial ID, mode (training/testing)
        // and attending value from supervisor's message
        ss >> target[0];ss >> target[1];ss >> target[2];
        ss >> handPos[0];ss >> handPos[1];ss >> handPos[2];

        ss >> currentTrial;
        // RSE control
        /*
         *if (currentTrial != prevTrial) {
            cout<<"new trial"<<endl;
            arma::mat reachTarget = arma::zeros<arma::mat>(6, 1);
            reachTarget<<target[0]<<arma::endr<<target[1]<<arma::endr<<target[2]<<arma::endr<<0<<arma::endr<<0<<arma::endr<<0<<arma::endr;

            cout<<"rse"<<endl;
            reachStateEquation rseComputer(maxTimeSteps, reachTimeSteps, reachTarget, dim, diagQ, finalPosCov, finalVelCov, timeBin);
            rseParams = rseComputer.returnAnswer();
            cout<<"rse"<<endl;
            arma::vec newHandState;
            newHandState<<handPos[0]<<handPos[1]<<handPos[2]<<0<<0<<0;
            handState = newHandState;
            prevTrial = currentTrial;
            timeStep = 0;
        } else {
            handState(0) = handPos[0];
            handState(1) = handPos[1];
            handState(2) = handPos[2];
        }

        cout<<"timeStep "<<timeStep<<endl;
        handState = rseParams.F.slice(timeStep) * handState + // randomNoise +
                    rseParams.b.slice(timeStep);
        if (timeStep < (maxTimeSteps-1)) {
            timeStep++;
        }
        x = handState(0); y = handState(1); z = handState(2);

        diffx = handState(3);//x - prevx;
        diffy = handState(4);//y - prevy;
        diffz = handState(5);//z - prevz;
        prevx = x; prevy = y; prevz = z;

        cout<<"diffx "<<diffx<<endl;
        cout<<"diffy "<<diffy<<endl;
        cout<<"diffz "<<diffz<<endl;

        cout<<"handState: "<<handState<<endl;
        cout<<"target: "<<target[0]<<" "<<target[1]<<" "<<target[2]<<endl;
        cout<<"currentTrial: "<<currentTrial<<endl;
        */ // RSE control

        // storing oild velocity values to compute the acceleration
        diffxOld = diffx;
        diffyOld = diffy;
        diffzOld = diffz;

        // time-invariant control policy
        diffx = (target[0] - handPos[0]) / 10.0;
        diffy = (target[1] - handPos[1]) / 10.0;
        diffz = (target[2] - handPos[2]) / 10.0;

        float accX, accY, accZ;
        accX = diffx - diffxOld;
        accY = diffy - diffyOld;
        accZ = diffz - diffzOld;


        if (fabs(accX) > accThreshold) {
            accX = accThreshold * sign(accX);
            diffx = diffxOld + accX;
        }
        if (fabs(accY) > accThreshold) {
            accY = accThreshold * sign(accY);
            diffy = diffyOld + accY;
        }
        if (fabs(accZ) > accThreshold) {
            accZ = accThreshold * sign(accZ);
            diffz = diffzOld + accZ;
        }



        cout<<"diffx "<<diffx<<endl;
        cout<<"diffy "<<diffy<<endl;
        cout<<"diffz "<<diffz<<endl;

        this_thread::sleep_for(chrono::microseconds(static_cast<int>(timeBin * 1000000.0)));
        fprintf(stderr, "argc: %d > 2?\n", argc);
        if (argc > 2) {
            quit = true;
            broadcast.join();
            return 0;
        }
    }

    return 0;
}
