#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <sys/resource.h>
#include <iostream>

#include <zmq.hpp>

//#include "PO8e.h"
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
    const size_t samplingRate = ifile("sampleRate", 6000.0);
    cout<<"sampleRate: "<<samplingRate<<endl;

    // data file
    time_t rawtime;
    time(&rawtime);
    string dataFilename = string("data_")+ctime(&rawtime);
    FILE* pFile;
    pFile = fopen(dataFilename.c_str(), "wb");
    setvbuf (pFile, NULL, _IOFBF, numberOfChannels*sizeof(float));


    // check the PO8e card
    /*PO8e *card = NULL;
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
    */

    // number of samples ready in buffer at each iteration
    size_t numberOfSamples = 0;

    // temp buffer to copy date from PO8e's buffer
    // assuming at maximum 20 samples we are behind
    float tempBuff[numberOfChannels];

    // synth signal generator
    double (*funcp)(double) = cos;
    // force signal frequency
    float forceFrq = 0.25;

    size_t secondsOfData = 10;
    for (size_t t=0; t<(10*samplingRate); t++) {
        // main loop
        //while(!kbhit()) {

            // how many samples are buffered
            //numberOfSamples = card->samplesReady();
            numberOfSamples = 1;
            cout<<"#buffered samples: "<<numberOfSamples<<endl;

            for(size_t i = 0; i < numberOfSamples; i++) {
                /*if (card->readBlock(tempBuff, 1) != 1) {
                     cout<<"reading sample "<<i<<" failed!"<<endl;
                     return 1;
                }
                // advance PO8e buffer pointer
                card->flushBufferedData(1);
                */
                // simulate:
                // ch 1: force sensor
                tempBuff[0] = fabs((*funcp)(2.0*M_PI * float(t)/float(samplingRate) 
                                       * forceFrq));
                // ch 2: digital in
                tempBuff[1] = 0.0;
                // ch 3: zero
                tempBuff[2] = 0.0;
                // ch 4: zero
                tempBuff[3] = 0.0;

                // correlated channels
                for (size_t ch=4; ch<8; ch++) {
                    tempBuff[ch] = (*funcp)(2.0*M_PI * float(t)/float(samplingRate) * float(ch)*4.0 ) * tempBuff[0];
                }
                // uncorrelated channels
                for (size_t ch=8; ch<20; ch++) {
                    tempBuff[ch] = (*funcp)(2.0*M_PI * float(t)/float(samplingRate) * float(ch)*4.0 ) * 1.0;
                }

                // write into file
                fwrite(&tempBuff, sizeof(float), numberOfChannels, pFile);
            }

        //} // main loop
    } // simulation loop

    fclose(pFile);
    //PO8e::releaseCard(card);

    return 0;
}
