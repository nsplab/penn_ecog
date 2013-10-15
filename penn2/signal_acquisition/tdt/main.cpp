#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <sys/resource.h>
#include <iostream>
#include <stdint.h>

#include <zmq.hpp>

#include "PO8e.h"
#include "GetPot.h"
#include "zhelpers.hpp"

using namespace std;
using namespace zmq;

string cfgFile("penn.cfg");

int kbhit()
{
    struct timeval tv = { 0L, 0L };
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(0, &fds);
    return select(1, &fds, NULL, NULL, &tv);
}

int main(int argc, char** argv) {
    // parse command line arguments
    GetPot cl(argc, argv);

    // check if config exists
    ifstream testCfgFile(cfgFile.c_str());
    if (! testCfgFile.good()) {
        cout<<"Could not open the config file: "<<cfgFile<<endl;
        return 1;
    } else {
        testCfgFile.close();
    }

    // parse config file
    GetPot ifile(cfgFile.c_str(), "#", "\n");
    ifile.print();

    const size_t numberOfChannels = ifile("numberOfChannels", 0);
    cout<<"numberOfChannels: "<<numberOfChannels<<endl;

    // data file
    time_t rawtime;
    time(&rawtime);
    char nameBuffer[24];
    tm * ptm = localtime(&rawtime);
    strftime(nameBuffer, 24, "%a_%d.%m.%Y_%H:%M:%S", ptm);
    string dataFilename = string("data_")+string(nameBuffer);
    FILE* pFile;
    pFile = fopen(dataFilename.c_str(), "wb");
    setvbuf (pFile, NULL, _IOFBF, numberOfChannels*sizeof(float));


    // check the PO8e card
    PO8e *card = NULL;
    int totalNumCards = PO8e::cardCount();
    cout<<"Found "<<totalNumCards<<" card(s) in the system"<<endl;
    if (0 == totalNumCards) {
        cout<<"Did not find any PO8e cards"<<endl;
        return 1;
    }
    card = PO8e::connectToCard(0);
    if (! card->startCollecting()) {
        cout<<"startCollecting() failed with: "<<card->getLastError()<<endl;
        PO8e::releaseCard(card);
    }

    // release buffer
    card->flushBufferedData(card->samplesReady());
    // just to make sure, -1: all buffered data
    card->flushBufferedData(-1);

    // number of samples ready in buffer at each iteration
    size_t numberOfSamples = 0;

    // temp buffer to copy date from PO8e's buffer
    // assuming at maximum 20 samples we are behind
    float tempBuff[numberOfChannels];

    context_t context(1);
    socket_t publisher(context, ZMQ_PUB);
    uint64_t hwm = 1;
    //publisher.setsockopt(ZMQ_HWM, &hwm, sizeof(hwm));
    publisher.bind("ipc:///tmp/sig.pipe");


    size_t timeStamp = 0;

    // main loop
    while(!kbhit()) {

        // wait until data is ready
        //size_t rwait = card->waitForDataReady(300000);
        //cout<<"rwait: "<<rwait<<endl;

        // how many samples are buffered
        numberOfSamples = card->samplesReady();
        //numberOfSamples = rwait;

        if (numberOfSamples < 1)
            continue;

        //cout<<"#buffered samples: "<<numberOfSamples<<endl;

        for(size_t i = 0; i < numberOfSamples; i++, timeStamp++) {
            if (card->readBlock(tempBuff, 1) != 1) {
                cout<<"reading sample "<<i<<" failed!"<<endl;
                return 1;
            }

            zmq::message_t zmq_message(sizeof(float)*numberOfChannels+sizeof(size_t));
            memcpy(zmq_message.data(), &timeStamp, sizeof(size_t)*1);
            //memcpy(&timeStamp, zmq_message.data(), sizeof(size_t)*1);
            if ((timeStamp % 50) == 0) {
	            cout<<"timeStamp: "<<timeStamp<<endl;
//            cout<<zmq_message.size()<<endl;
  	          cout<<numberOfSamples<<endl;
		}
            memcpy(static_cast<size_t*>(zmq_message.data())+1, tempBuff, sizeof(float)*numberOfChannels);
//char tstr[] = "10001 this that";
//            memcpy(zmq_message.data(), tstr, strlen(tstr));
        //snprintf ((char *) zmq_message.data(), 20 ,
        //    "%05d %d", 10001, timeStamp);
            //s_sendmore(publisher, "A");
            publisher.send(zmq_message);

            // advance PO8e buffer pointer
            card->flushBufferedData(1);

            // write into file
            fwrite(&timeStamp, sizeof(size_t), 1, pFile);
            fwrite(&tempBuff, sizeof(float), numberOfChannels, pFile);
        }

    } // main loop

    fclose(pFile);
    PO8e::releaseCard(card);
    publisher.close();

    return 0;
}

