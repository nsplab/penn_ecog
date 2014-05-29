#include <iostream>
#include <iomanip>
#include <ctime>
#include <thread>
#include <chrono>
#include <signal.h>

#include <zmq.hpp>

#include "eeg_receiver.h"

using namespace std;
using namespace zmq;

bool quit = false;
void signal_callback_handler(int signum) {
    signal(signum, SIG_IGN);
    quit = true;
}

bool dataBeingStreamed = false;
// if within the past 3 seconds received data form the device/driver then tell the other modules data is being streaming
// otherwise tell the others data is not being streamed!
// this works for our devices, in gtec at high sampling rates the data packets are sent just once in about every 2 seconds
// in TDT, the packets are sent more frequently than by gtec, so this method still works fine
void BroadcastStatus() {

    context_t context(1);			//number of threads used by ZMQ
    socket_t publisher(context, ZMQ_PUB);	//socket used to broadcast data
    int hwm = 1;				//hwm - high water mark - determines buffer size for
    //data passed through ZMQ. hwm = 1 makes the ZMQ buffer
    //size = 1. This means that if no module has accessed a
    //value written through ZMQ, new values will be dropped
    //until any module reads the value
    publisher.setsockopt(ZMQ_SNDHWM, &hwm, sizeof(hwm));
    int conflate = 1;
    publisher.setsockopt(ZMQ_CONFLATE, &conflate, sizeof(conflate));
    publisher.bind("ipc:///tmp/signalstream.pipe");	//gives address of data that other modules can reference

    chrono::time_point<chrono::system_clock> lastReceiveTime;
    lastReceiveTime = chrono::system_clock::now();

    char status = '0';
    publisher.send(&status, sizeof(char));

    //cout<<"dataBeingStreamed "<<dataBeingStreamed<<endl

    while (!quit) {
        if (dataBeingStreamed) {
            lastReceiveTime = chrono::system_clock::now();
        }

        if ((dataBeingStreamed) && (status == '0')) {
            status = '1';
            cout<<"published status 1"<<endl;

        } else {
            std::chrono::duration<double> elapsed_seconds = chrono::system_clock::now()-lastReceiveTime;
            if ((status == '1') && (elapsed_seconds.count()>3.0)) {
                status = '0';
                cout<<"published status 0"<<endl;
            }
        }

        publisher.send(&status, sizeof(char));
    }

    publisher.close();
    context.close();
}


int main(int argc, char** argv)
{
    signal(SIGINT, signal_callback_handler);

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

    int numberOfChannels = 64;

    size_t timestamp = 0;

    bool record = false;

    string filename("/media/ssd/system_specification/data_");

    FILE* pFile = NULL;

    thread broadcastThread(BroadcastStatus);

    for(;!quit;) {

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

        dataBeingStreamed = false;
        receiver.receive(timestampDouble, channels);
        dataBeingStreamed = true;

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

