#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <sys/resource.h>
#include <iostream>

#include <zmq.hpp>

#include "PO8e.h"
#include "GetPot.h"

using namespace std;
using namespace zmq;

string cfgFile("/home/user/penn2/config/penn.cfg");

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
    ifstream testCfgFile(cfgFile);
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
    string dataFilename = string("data_")+ctime(&rawtime);
    FILE* pFile;
    pFile = fopen("data", "wb");
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

    // main loop
    while(!kbhit()) {

        // how many samples are buffered
        numberOfSamples = card->samplesReady();
        cout<<"#buffered samples: "<<numberOfSamples<<endl;

        for(size_t i = 0; i < numberOfSamples; i++) {
            if (card->readBlock(tempBuff, 1) != 1) {
                cout<<"reading sample "<<i<<" failed!"<<endl;
                return 1;
            }
            // advance PO8e buffer pointer
            card->flushBufferedData(1);

            // write into file
            fwrite(&tempBuff, sizeof(float), numberOfChannels, pFile);
        }

    } // main loop

    fclose(pFile);
    PO8e::releaseCard(card);

    return 0;
}
