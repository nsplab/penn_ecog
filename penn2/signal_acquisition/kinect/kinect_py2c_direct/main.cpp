#include <iostream>
#include <sstream>
#include <thread>
#include <chrono>

#include <zmq.hpp>

#include <eigen3/Eigen/Dense>

using namespace std;
using namespace zmq;
using namespace Eigen;

float x=0, y=0, z=0;
float prevx=0, prevy=0, prevz=0;
float diffx=0.0, diffy=0.0, diffz=0.0;

context_t context(2);

float RawDepthToMeters(int depthValue)
{
    if (depthValue < 2047)
    {
        return float(1.0 / (double(depthValue) * -0.0030711016 + 3.3309495161));
    }
    return 0.0f;
}

Vector3f DepthToWorld(int x, int y, int depthValue)
{
    static const double fx_d = 1.0 / 5.9421434211923247e+02;
    static const double fy_d = 1.0 / 5.9104053696870778e+02;
    static const double cx_d = 3.3930780975300314e+02;
    static const double cy_d = 2.4273913761751615e+02;

    Vector3f result;
    const double depth = RawDepthToMeters(depthValue);
    result[0] = float((x - cx_d) * depth * fx_d);
    result[1]= float((y - cy_d) * depth * fy_d);
    result[2] = float(depth);
    return result;
}

void GenerateSignal() {
    socket_t publisher(context, ZMQ_PUB);

    int hwm = 1;
    publisher.setsockopt(ZMQ_SNDHWM, &hwm, sizeof(hwm));
    int conflate = 1;
    publisher.setsockopt(ZMQ_CONFLATE, &conflate, sizeof(conflate));

    cout<<"bypass feature extractor"<<endl;
    publisher.bind("ipc:///tmp/features.pipe");

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

    bool exit = false;
    for (size_t timeStamp=0; !exit; timeStamp++) {

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
        //arma::vec v = arma::randn<arma::vec>(3);
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

        //signal = mixingMatrix * sample;
        //cout<<"numberOfChannels "<<numberOfChannels<<endl;
        //cout<<"t: "<<timeStamp<<endl;

        signal(0) = xSignalAmp * sqrt(dx+baselinePower);
        signal(1) = ySignalAmp * sqrt(dy+baselinePower);
        signal(2) = zSignalAmp * sqrt(dz+baselinePower);
        
	cout<<"signal: "<<signal<<endl;

        //message_t zmqMessage(sizeof(float)*numberOfChannels+sizeof(size_t));
        message_t zmqMessage(sizeof(float)*numberOfChannels+sizeof(size_t));
        memcpy(zmqMessage.data(), &timeStamp, sizeof(size_t));
        memcpy(static_cast<size_t*>(zmqMessage.data())+1, signal.data(), sizeof(float)*numberOfChannels);
        //memcpy(static_cast<size_t*>(zmqMessage.data())+1, tsignal.data(), sizeof(float)*60);

        //cout<<"signal: "<<signal<<endl;
        publisher.send(zmqMessage);

        // direct mode imitator: introduce a delay between features issued by the imitator in order to avoid flooding the receiving filter with features.
       this_thread::sleep_for(chrono::microseconds(static_cast<int>(100000.0)));
        
    }
}

int main()
{
    socket_t subscriber(context, ZMQ_SUB);

    subscriber.connect("ipc:///tmp/ksignal.pipe");
    subscriber.setsockopt(ZMQ_SUBSCRIBE, "", 0);

    thread broadcast(GenerateSignal);

    for (;;)   {
        message_t ksig_msg;
        subscriber.recv(&ksig_msg);
        string msg = string(static_cast<char*>(ksig_msg.data()), ksig_msg.size());

        istringstream iss(msg);
        iss>>x>>y>>z;
        //cout<<"x:"<<x<<"\ty:"<<y<<"\tz:"<<z<<endl;

        diffx = (x - prevx)/640.0f * 4.0;
        diffy = -(y - prevy)/480.0f * 4.0;
        diffz = 0.0f;//z - prevz;

        //cout<<"dx:"<<diffx<<"\tdy:"<<diffy<<"\tdz:"<<diffz<<endl;

        prevx = x; prevy = y; prevz = z;
    }

    return 0;
}
