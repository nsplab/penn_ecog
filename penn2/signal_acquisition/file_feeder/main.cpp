#include <iostream>
#include <chrono>
#include <thread>
#include <signal.h>

#include <zmq.hpp>

#include "../../libs/getpot/GetPot.h"

using namespace std;
using namespace zmq;

bool quit = false;
void signal_callback_handler(int signum) {
    quit = true;
}

int main() {
    signal(SIGINT, signal_callback_handler);

    const unsigned numberOfChannels = 64;

    class Row {
    public:
        size_t timeStamp;
        vector<float> cols;
    };
    Row row;
    row.cols.resize(numberOfChannels);

    string signalFilename;
    unsigned samplingRate = 25000;
    string cfgFile("../config.cfg");

    // check if the config file exists
    ifstream testFile(cfgFile.c_str());
    if (! testFile.good()) {
        cout<<"Could not open the config file: "<<cfgFile<<endl;
        return 1;
    } else {
        testFile.close();
    }

    // parse config file
    GetPot ifile(cfgFile.c_str(), "#", "\n");
    ifile.print();

    signalFilename = ifile("signalFile", "");
    samplingRate = ifile("samplingRate", 25000);
    unsigned formatVersion = ifile("formatVersion", 1);

    ifstream signalFile(signalFilename.c_str(), ios::in | ios::binary);

    context_t context(1);
    socket_t publisher(context, ZMQ_PUB);
    publisher.bind("ipc:///tmp/signal.pipe");

    chrono::microseconds sleepTime(unsigned(1000000.0/samplingRate));

    size_t timeStamp = 0;

    this_thread::sleep_for(chrono::seconds(5));

    while(signalFile && !quit) {
        if (formatVersion == 1)
            signalFile.read((char *) &(row.timeStamp), sizeof(size_t));
        signalFile.read((char *) (row.cols.data()), sizeof(float) * numberOfChannels);

        timeStamp ++;

        message_t zmq_message(row.cols.size() * sizeof(float) + sizeof(size_t));
        memcpy(zmq_message.data(), &timeStamp, sizeof(size_t));
        memcpy((size_t*)(zmq_message.data())+1, row.cols.data(), zmq_message.size());
        publisher.send(zmq_message);

        if (timeStamp % 10000 == 0)
            cout<<"time: "<<timeStamp<<endl;

        this_thread::sleep_for(sleepTime);
    }

    cout<<endl;

    return 0;
}

