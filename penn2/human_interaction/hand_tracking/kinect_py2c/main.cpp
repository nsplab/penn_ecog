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

bool quit = false;
size_t timestamp;

context_t context(2);

/*float RawDepthToMeters(int depthValue)
{
    if (depthValue < 2047)
    {
        return float(1.0 / (double(depthValue) * -0.0030711016 + 3.3309495161));
    }
    return 0.0f;
}*/

/*Vector3f DepthToWorld(int x, int y, int depthValue)
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
}*/

void GetTimestamp() {
    socket_t signal(context, ZMQ_SUB);
    signal.connect("ipc:///tmp/signal.pipe");
    signal.setsockopt(ZMQ_SUBSCRIBE, NULL, 0);

    for (; !quit;) {
        message_t signal_msg;
        cout<<"recv"<<endl;
        signal.recv(&signal_msg);
        cout<<"recvd"<<endl;
        memcpy(&timestamp, signal_msg.data(), sizeof(size_t));
        cout<<"timestamp "<<timestamp<<endl;
    }
}

int kbhit()	//detects if keyboard is hit or not. this is used to press any key to close files and exit the code
{
    struct timeval tv = { 0L, 0L };
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(0, &fds);
    return select(1, &fds, NULL, NULL, &tv);
}

int main()
{
    socket_t subscriber(context, ZMQ_SUB);

    subscriber.connect("ipc:///tmp/kinecthand.pipe");
    subscriber.setsockopt(ZMQ_SUBSCRIBE, "", 0);

    socket_t supervisor(context, ZMQ_REQ);
    supervisor.connect("ipc:///tmp/supervisor.pipe");

    thread getTime(GetTimestamp);

    time_t rawtime;
    time(&rawtime);
    char nameBuffer[24];
    tm * ptm = localtime(&rawtime);
    strftime(nameBuffer, 24, "%a_%d.%m.%Y_%H:%M:%S", ptm);
    string dataFilename = string("hand_data")+string(nameBuffer);

    FILE* pFile;
    pFile = fopen(dataFilename.c_str(), "wb");



    for (;!kbhit();)   {
        message_t ksig_msg;
        subscriber.recv(&ksig_msg);
        string msg = string(static_cast<char*>(ksig_msg.data()), ksig_msg.size());

        istringstream iss(msg);
        iss>>x>>y>>z;

        cout<<"t: "<<timestamp<<" :"<<x<<" "<<y<<" "<<z<<endl;

        fwrite(&timestamp, sizeof(size_t),1 , pFile);
        fwrite(&x, sizeof(float),1 , pFile);
        fwrite(&y, sizeof(float),1 , pFile);
        fwrite(&z, sizeof(float),1 , pFile);

        x = -x*30.0; y = -y*30.0;
        z = (z-1.0) * 30.0;

        cout<<"t: "<<timestamp<<" :"<<x<<" "<<y<<" "<<z<<endl;

        fwrite(&x, sizeof(float),1 , pFile);
        fwrite(&y, sizeof(float),1 , pFile);
        fwrite(&z, sizeof(float),1 , pFile);

        stringstream message;
        message<<timestamp<<" "<<x<<" "<<y<<" "<<z<<endl;
        zmq::message_t zmq_message(message.str().length());
        memcpy((char *) zmq_message.data(), message.str().c_str(), message.str().length());
        supervisor.send(zmq_message);
        zmq::message_t supervisor_msg;
        supervisor.recv(&supervisor_msg);

    }

    fclose(pFile);
    quit = true;
    getTime.join();

    return 0;
}
