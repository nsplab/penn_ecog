#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <sys/resource.h>
#include <iostream>
#include <stdint.h>

#include <zmq.hpp>  //provides protocol for communicating between different modules in the ECoG system

#include "zhelpers.hpp"  //helper functions used by the program in conjunction with zmq
#include "PO8e.h"   //code provided by TDT to stream/read signal amplitudes
#include "GetPot.h" //code for reading config files (eg. signal.cfg) that contain parameters for the experiment


using namespace std;
using namespace zmq;

string cfgFile("penn.cfg");

int kbhit()	//detects if keyboard is hit or not. this is used to press any key to close files and exit the code
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

    // check if the config file exists (extension .cfg)
    ifstream testCfgFile(cfgFile.c_str());
    if (! testCfgFile.good()) {
        cout<<"Could not open the config file needed to set recording parameters: "<<cfgFile<<endl;
        return 1;
    } else {
        testCfgFile.close();
    }

    // parse config file to load parameter values
    GetPot ifile(cfgFile.c_str(), "#", "\n");
    ifile.print();	//print the config file to the terminal for user reference

    const size_t numberOfChannels = ifile("numberOfChannels", 0);	//load total number of channels (ECoG plus any other channels)
    cout<<"numberOfChannels: "<<numberOfChannels<<endl;			//print number of channels to the screen

    // define the data file that will contain all data received by the TDT,
    //timestamped using a counter running on the PC (Puget)
    //form the file name in the format "data_wed_16.10.2013_14:41:59"
    //open the file - form the variables record all data received by the TDT
    time_t rawtime;
    time(&rawtime);
    char nameBuffer[24];
    tm * ptm = localtime(&rawtime);
    strftime(nameBuffer, 24, "%a_%d.%m.%Y_%H:%M:%S", ptm);
    string dataFilename = string("data_")+string(nameBuffer);
    FILE* pFile;
    pFile = fopen(dataFilename.c_str(), "wb");
    setvbuf (pFile, NULL, _IOFBF, numberOfChannels*sizeof(float));


    // check the PO8e card to:
    // 	find the card
    //  check if it can connect to the hardware using the TDT-provided "kernel module"
    //	various things can cause this step to fail:
    //		inadvertently upgrading the Linux kernel version on the PC
    //		PO8e card removed or damaged
    //		TDT PO8e kernel module is removed or not loaded
    PO8e *card = NULL;
    int totalNumCards = PO8e::cardCount();	//check the number of PO8e cards installed in the PC (Puget)
    cout<<"Found "<<totalNumCards<<" card(s) in the system"<<endl;
    if (0 == totalNumCards) {
        cout<<"Did not find any PO8e cards"<<endl;
        return 1;
    }
    card = PO8e::connectToCard(0);	//returns a pointer to an instance of the PO8e class
    if (! card->startCollecting()) {	//connectToCard(0) checks card 0
        cout<<"startCollecting() failed with: "<<card->getLastError()<<endl;
        PO8e::releaseCard(card);
    }

    // clear contents of the buffer containing data received by the PO8e
    //one way to flush the buffer
    card->flushBufferedData(card->samplesReady());  
    						    //remove a certain number of samples from the buffer specified by samplesRead()
    //another way to flush the buffer, just to be sure
    card->flushBufferedData(-1);	  //flushBufferedData(-1) flushes all data in the buffer

    // number of samples ready in buffer at each iteration
    size_t numberOfSamples = 0;	//initialized to zero, but will be updated later

    // temporary buffer to copy data from PO8e's buffer.
    // this buffer only contains one float per channel
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

