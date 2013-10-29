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

void GenerateSignal() {
    socket_t publisher(context, ZMQ_PUB);
    publisher.bind("ipc:///tmp/signal.pipe");

    float xSignalAmp = 2.0f;
    float xSignalFrq = 10.0f; // Hz

    float ySignalAmp = 2.0f;
    float ySignalFrq = 20.0f; // Hz

    float zSignalAmp = 2.0f;
    float zSignalFrq = 30.0f; // Hz

    size_t samplingRate = 1000; // Hz

    // the synthetic signals are mixed to generate the output singnals/channels
    int numberOfChannels = 4; // number of output channels
    Matrix<float, Dynamic, Dynamic> mixingMatrix(numberOfChannels,3);
    mixingMatrix << 1, 0, 0,
                    0, 1, 0,
                    0, 0, 1,
                    1, 1, 1;

    Matrix<float, Dynamic, Dynamic> signal(numberOfChannels,1); // output signal
    Vector3f sample; // synthetic signal based on x,y,z from kinect

    for (size_t timeStamp=0;;timeStamp++) {

        size_t i = timeStamp % samplingRate;

        // assume intended velocity is power modulated
        float maxSpeed = 50.0;
        float baselinePower = maxSpeed;
        if (diffx >= maxSpeed)
            diffx = maxSpeed;
        else if (diffx <= -maxSpeed)
            diffx = -maxSpeed;
        if (diffy >= maxSpeed)
            diffy = maxSpeed;
        else if (diffy <= -maxSpeed)
            diffy = -maxSpeed;
        if (diffz >= maxSpeed)
            diffz = maxSpeed;
        else if (diffz <= -maxSpeed)
            diffz = -maxSpeed;
        sample(0) = cos(2.0*M_PI * float(i)/float(samplingRate) * xSignalFrq) * xSignalAmp * sqrt(diffx+baselinePower);
        sample(1) = cos(2.0*M_PI * float(i)/float(samplingRate) * ySignalFrq) * ySignalAmp * sqrt(diffy+baselinePower);
        sample(2) = cos(2.0*M_PI * float(i)/float(samplingRate) * zSignalFrq) * zSignalAmp * sqrt(diffz+baselinePower);

        signal = mixingMatrix * sample;
        cout<<"signal: "<<signal<<endl;

        if (isnan(signal(0))) {
            signal(0) = 0.0;
            signal(1) = 0.0;
            signal(2) = 0.0;
        }

        message_t zmqMessage(sizeof(float)*numberOfChannels+sizeof(size_t));
        memcpy(zmqMessage.data(), &timeStamp, sizeof(size_t)*1);
        memcpy(static_cast<size_t*>(zmqMessage.data())+1, signal.data(), sizeof(float)*numberOfChannels);

        publisher.send(zmqMessage);

        this_thread::sleep_for(chrono::microseconds(100));
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

        diffx = x - prevx;
        diffy = y - prevy;
        diffz = z - prevz;

        //cout<<"dx:"<<diffx<<"\tdy:"<<diffy<<"\tdz:"<<diffz<<endl;

        prevx = x; prevy = y; prevz = z;
    }

    return 0;
}
