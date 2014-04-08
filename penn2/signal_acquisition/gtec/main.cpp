#include <iostream>
#include <zmq.hpp>
#include <iomanip>
#include <ctime>

#include "eeg_receiver.h"

using namespace std;
using namespace zmq;

int main(int argc, char** argv)
{
    // sampling rate = 9600
    size_t numChannels = 64;

    if (argc > 1) {
        if (atoi(argv[1]) == 1) {
            numChannels += 1;
            cout<<"Digital input channel is added to the list of channels."<<endl;
        }
    }


    size_t numScans = 128;
    size_t dataSize = numChannels * numScans;

    double timestampDouble;

    float channels[dataSize];
    EegReceiver receiver(numChannels, numScans);

    context_t context(2);
    socket_t publisher(context, ZMQ_PUB);
    publisher.bind("ipc:///tmp/signal.pipe");

    socket_t recordSocket(context, ZMQ_REP);
    recordSocket.bind("ipc:///tmp/record.pipe");

    int numberOfChannels = 15;

    size_t timestamp = 0;

    bool record = false;

    string filename("/media/ssd/system_specification/data_");

    FILE* pFile = NULL;

    for(;;) {

        message_t recMsg;
        if (recordSocket.recv(&recMsg, ZMQ_DONTWAIT)) {
            string strMsg((char *)recMsg.data(), recMsg.size());
            if (strMsg.compare("stop") == 0) {
                cout<<"stop recording"<<endl;
                record = false;
                if (pFile != NULL) {
                    fclose(pFile);
                    pFile = NULL;
                }
            } else {
                cout<<"str: "<<strMsg<<endl;
                time_t t = time(nullptr);
                tm ptm = *std::localtime(&t);
                char nameBuffer[34];
                strftime(nameBuffer, 34, "_%a_%d.%m.%Y_%H:%M:%S", &ptm);
                cout<<"filename: "<<filename+string(strMsg)+string(nameBuffer)<<endl;
                record = true;
                if (pFile != NULL) {
                    fclose(pFile);
                    pFile = NULL;
                }
                string finalFilename(filename+string(strMsg)+string(nameBuffer));

                pFile = fopen(finalFilename.c_str(), "wb");
                setvbuf (pFile, NULL, _IOFBF, dataSize*sizeof(float));

            }
            recordSocket.send("ok", 2);
        }

        receiver.receive(timestampDouble, channels);

        if (record) {
            fwrite(&(channels[0]), sizeof(float), dataSize, pFile);
        }

        for (unsigned i=0; i<numScans; i++) {
            timestamp += 1;

            //cout<<"timestamp: "<<timestampDouble<<endl;
            //cout<<"channel 0: "<<channels[numChannels * i + 0]<<endl;
            //cout<<"channel 1: "<<channels[numChannels * i + 1]<<endl;

            message_t zmqMessage(sizeof(float)*numberOfChannels+sizeof(size_t));
            memcpy(zmqMessage.data(), &timestamp, sizeof(size_t));
            memcpy(static_cast<size_t*>(zmqMessage.data())+1, &(channels[numChannels * i]), sizeof(float)*numberOfChannels);

            publisher.send(zmqMessage);
        }
    }
}

