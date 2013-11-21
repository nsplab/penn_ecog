#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <sys/resource.h>
#include <iostream>
#include <stdint.h>

// for lpt
#include <unistd.h>
#include <sys/io.h>

#include <zmq.hpp>  //provides protocol for communicating between different modules in the ECoG system

#include "zhelpers.hpp"  //helper functions used by the program in conjunction with zmq
#include "PO8e.h"   //code provided by TDT to stream/read signal amplitudes
#include "GetPot.h" //code for reading config files (eg. signal.cfg) that contain parameters for the experiment

#define lptDataBase 0xd010           /* printer port data base address */
//#define lptControlBase 0xd012           /* printer port control base address */

using namespace std;
using namespace zmq;

string cfgFile("penn.cfg");

int kbhit() {	//detects if keyboard is hit or not. this is used to press any key to close files and exit the code

    struct timeval tv = { 0L, 0L };
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(0, &fds);
    return select(1, &fds, NULL, NULL, &tv);
}

int main(int argc, char** argv) {
    if (ioperm(lptDataBase,1,1))
        fprintf(stderr, "Couldn't get the port at %x\n", lptDataBase), exit(1);

    outb(0x10, lptDataBase);

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
    //setvbuf (pFile, NULL, _IOFBF, numberOfChannels*sizeof(float));


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


    //Initializing ZMQ protocol that allows us to communicate between modules of the code
    context_t context(1);			//number of threads used by ZMQ
    socket_t publisher(context, ZMQ_PUB);	//socket used to broadcast data
    uint64_t hwm = 1;				//hwm - high water mark - determines buffer size for
    						//data passed through ZMQ. hwm = 1 makes the ZMQ buffer 
    						//size = 1. This means that if no module has accessed a
    						//value written through ZMQ, new values will be dropped
    						//until any module reads the value
    publisher.setsockopt(ZMQ_HWM, &hwm, sizeof(hwm));
    publisher.bind("ipc:///tmp/sig.pipe");	//gives address of data that other modules can reference
						//ipc (interprocess communication) is a Linux standard
						//for communication between programs, used by ZMQ
						//Note that ZMQ also uses TCP or UDP, not preferred here
						//because ipc is robust to changes in ports and local IP addresses
						//unlike TCP

    size_t timeStamp = 0;			//timeStamp is our counter on the PC that is used as the system clock
    						//for aligning ECoG and other TDT data with task events on the PC
    						//such as the value of an on-screen stimulus



    // main loop - runs an infinite loop until any key on the keyboard is hit.
    while(!kbhit()) {

        numberOfSamples = card->samplesReady();  // how many samples are currently available for reading from the PO8e

        if (numberOfSamples < 1)		//if no samples are ready, 
            continue;				//skip remaining code in the while loop and start a new cycle

        for(size_t i = 0; i < numberOfSamples; i++, timeStamp++) {//increment timeStamp based on number of samples received - assumes samples reeceived regularly
            if (card->readBlock(tempBuff, 1) != 1) {	//readblock(tempBuff,1) loads one sample from every channel into tempBuff, returning 1 if successful, and advances to the next sample
                cout<<"reading sample "<<i<<" failed!"<<endl;  //if there is a problem accessing the buffer, print failure message to terminal
                return 1;
            }

	//use the ZMQ protocol to broadcast the following values:
	//	tempBuff - the single current sample from every channel
	//	timeStamp - the counter on the PC that serves as the system clock to align ECoG with task events
	
            zmq::message_t zmq_message(sizeof(float)*numberOfChannels+sizeof(size_t)); //form a ZMQ message object; note this will cause problems if it is moved out from the loop
            //form the zmq message that contains timeStamp and tempBuff
            //(memcpy is a standard C function that copies the timeStamp into the memory address given by zmq_message.data())
            memcpy(zmq_message.data(), &timeStamp, sizeof(size_t)*1);  
           //memcpy(&timeStamp, zmq_message.data(), sizeof(size_t)*1);
            memcpy(static_cast<size_t*>(zmq_message.data())+1, tempBuff, sizeof(float)*numberOfChannels);  
            
            if ((timeStamp % 1000) == 0) {
                if (timeStamp == 25000) {
                    outb(0x0, lptDataBase);
                }
	            cout<<"timeStamp: "<<timeStamp<<endl;	//every 50 timeStamps, print the timeStamp
//            cout<<zmq_message.size()<<endl;			//
  	          cout<<numberOfSamples<<endl;			//and # of samples that were ready in the PO8e buffer
		}            
            
//char tstr[] = "10001 this that";
//            memcpy(zmq_message.data(), tstr, strlen(tstr));
        //snprintf ((char *) zmq_message.data(), 20 ,
        //    "%05d %d", 10001, timeStamp);
            //s_sendmore(publisher, "A");
            publisher.send(zmq_message);	//ZMQ command to trigger the broadcast of tempBuff and timeStamp

            // advance PO8e buffer pointer
            card->flushBufferedData(1);

            // write timeStamp and tempBuff into the data file on the PC harddrive
            fwrite(&timeStamp, sizeof(size_t), 1, pFile);
            fwrite(&tempBuff, sizeof(float), numberOfChannels, pFile);
        }

    } // end of main loop

    fclose(pFile);		//close the data file that records all TDT channels with timeStamp onto the PC (Puget)
    PO8e::releaseCard(card);	//for PO8e
    publisher.close();		//for ZMQ

    return 0;
}

