#include <iostream>
#include <zmq.hpp>

#include "eeg_receiver.h"

using namespace std;
using namespace zmq;

int main()
{

    size_t numChannels = 64;
    size_t numScans = 1;
    size_t dataSize = numChannels * numScans;

    double time;

    float channels[dataSize];
    EegReceiver receiver(numChannels, numScans);

    context_t context(1);
    socket_t publisher(context, ZMQ_PUB);
    publisher.bind("ipc:///tmp/signal.pipe");

    int numberOfChannels = 64;

    size_t timestamp = 0;

    for(;;) {
        receiver.receive(time, channels);

        for (unsigned i=0; i<numScans; i++) {
            timestamp += 1;

            cout<<"timestamp: "<<time<<endl;
            cout<<"channel 0: "<<channels[numChannels * i + 0]<<endl;
            cout<<"channel 1: "<<channels[numChannels * i + 1]<<endl;

            message_t zmqMessage(sizeof(float)*numberOfChannels+sizeof(size_t));
            memcpy(zmqMessage.data(), &timestamp, sizeof(size_t));
            memcpy(static_cast<size_t*>(zmqMessage.data())+1, &(channels[numChannels * i]), sizeof(float)*numberOfChannels);

            publisher.send(zmqMessage);
        }
    }
}

