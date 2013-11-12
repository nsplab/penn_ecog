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

    Vec3f result;
    const double depth = RawDepthToMeters(depthValue);
    result.x = float((x - cx_d) * depth * fx_d);
    result.y = float((y - cy_d) * depth * fy_d);
    result.z = float(depth);
    return result;
}

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

        sample(0) = cos(2.0*M_PI * float(i)/float(samplingRate) * xSignalFrq) * xSignalAmp * sqrt(dx+baselinePower);
        sample(1) = cos(2.0*M_PI * float(i)/float(samplingRate) * ySignalFrq) * ySignalAmp * sqrt(dy+baselinePower);
        sample(2) = cos(2.0*M_PI * float(i)/float(samplingRate) * zSignalFrq) * zSignalAmp * sqrt(dz+baselinePower);

        signal = mixingMatrix * sample;
        cout<<"signal: "<<signal<<endl;

        message_t zmqMessage(sizeof(float)*numberOfChannels+sizeof(size_t));
        memcpy(zmqMessage.data(), &timeStamp, sizeof(size_t)*1);
        memcpy(static_cast<size_t*>(zmqMessage.data())+1, signal.data(), sizeof(float)*numberOfChannels);

        publisher.send(zmqMessage);

        this_thread::sleep_for(chrono::microseconds(1000));
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
