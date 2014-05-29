#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <sys/resource.h>
#include <iostream>
#include <stdint.h>
#include <signal.h>

#include <chrono>

#include <thread>

// for lpt
//#include <unistd.h>
//#include <sys/io.h>

#include <zmq.hpp>  //provides protocol for communicating between different modules in the ECoG system

#include "PO8e.h"   //code provided by TDT to stream/read signal amplitudes
//#include "GetPot.h" //code for reading config files (eg. signal.cfg) that contain parameters for the experiment
#include "../../libs/inih/cpp/INIReader.h"

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

bool quit = false;
void signal_callback_handler(int signum) {
    signal(signum, SIG_IGN);
    quit = true;
}

bool dataBeingStreamed = false;
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

int main(int argc, char** argv) {
//    if (ioperm(lptDataBase,1,1))
//        fprintf(stderr, "Couldn't get the port at %x\n", lptDataBase), exit(1);

    signal(SIGINT, signal_callback_handler);


//    if (ioperm(lptDataBase,1,1))
//        fprintf(stderr, "Couldn't get the port at %x\n", lptDataBase), exit(1);

//    outb(0x10, lptDataBase);

    // parse command line arguments
    //GetPot cl(argc, argv);

    // check if the config file exists (extension .cfg)
/*    ifstream testCfgFile(cfgFile.c_str());
    if (! testCfgFile.good()) {
        cout<<"Could not open the config file needed to set recording parameters: "<<cfgFile<<endl;
        return 1;
    } else {
        testCfgFile.close();
    }
*/

    INIReader reader("../../data/log.txt");
    if (reader.ParseError() < 0) {
        std::cout << "Can't load '../../data/log.txt'\n";
        return 1;
    }

    // parse config file to load parameter values
    //GetPot ifile(cfgFile.c_str(), "#", "\n");
    //ifile.print();	//print the config file to the terminal for user reference

    //const size_t numberOfChannels = ifile("numberOfChannels", 0);	//load total number of channels (ECoG plus any other channels)
    const size_t numberOfChannels = reader.GetInteger("ExperimentLog", "TotalNumberOfChannels", 64);
    cout<<"numberOfChannels: "<<numberOfChannels<<endl;			//print number of channels to the screen

    //if (argc > 1) {
    //    numberOfChannels = atoi(argv[1]);
    //}

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
    setvbuf (pFile, NULL, _IOFBF, 10*numberOfChannels*sizeof(float));


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
    card->stopCollecting();
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
    float dBuff[numberOfChannels * 10000];
    float dBuffSeq[numberOfChannels * 10000];
    float* tempBuff;


    //Initializing ZMQ protocol that allows us to communicate between modules of the code
    context_t context(1);			//number of threads used by ZMQ
    socket_t publisher(context, ZMQ_PUB);	//socket used to broadcast data

    socket_t recordSocket(context, ZMQ_REP);
    recordSocket.bind("ipc:///tmp/record.pipe");

    int hwm = 100;				//hwm - high water mark - determines buffer size for
    						//data passed through ZMQ. hwm = 1 makes the ZMQ buffer 
    						//size = 1. This means that if no module has accessed a
    						//value written through ZMQ, new values will be dropped
    						//until any module reads the value
    publisher.setsockopt(ZMQ_SNDHWM, &hwm, sizeof(hwm));
    publisher.bind("ipc:///tmp/signal.pipe");	//gives address of data that other modules can reference
						//ipc (interprocess communication) is a Linux standard
						//for communication between programs, used by ZMQ
						//Note that ZMQ also uses TCP or UDP, not preferred here
						//because ipc is robust to changes in ports and local IP addresses
						//unlike TCP

    size_t timeStamp = 0;			//timeStamp is our counter on the PC that is used as the system clock
    						//for aligning ECoG and other TDT data with task events on the PC
    						//such as the value of an on-screen stimulus

    thread broadcastThread(BroadcastStatus);

    bool record = false;

    std::chrono::time_point<std::chrono::system_clock> start, end;

    string filename("../../data/data_");

    // main loop - runs an infinite loop until any key on the keyboard is hit.
    while(!quit) {

        numberOfSamples = card->samplesReady();  // how many samples are currently available for reading from the PO8e

        if (numberOfSamples < 1) {		//if no samples are ready,
            dataBeingStreamed = false;
            continue;				//skip remaining code in the while loop and start a new cycle
        }

        dataBeingStreamed = true;
        cout<<"some samples are ready"<<endl;

        start = std::chrono::system_clock::now();
        if (card->readBlock(dBuff, numberOfSamples) != numberOfSamples) {	//readblock(tempBuff,1) loads one sample from every channel into tempBuff, returning 1 if successful, and advances to the next sample
            cout<<"reading sample "<<" failed!"<<endl;  //if there is a problem accessing the buffer, print failure message to terminal
            return 1;
        }
        end = std::chrono::system_clock::now();

        // advance PO8e buffer pointer
        card->flushBufferedData(numberOfSamples);

            // change grouping from channel-based to sample-based
            for (unsigned ch=0; ch<numberOfChannels; ch++) {
                for (unsigned sample=0; sample<numberOfSamples; sample++) {
                    dBuffSeq[sample*numberOfChannels + ch] = dBuff[ch * numberOfSamples + sample];
                }
            }

        for(size_t i = 0; i < numberOfSamples; i++, timeStamp++) {//increment timeStamp based on number of samples received - assumes samples reeceived regularly

            cout<<"i: "<<i<<endl;
            //use the ZMQ protocol to broadcast the following values:
            //	tempBuff - the single current sample from every channel
            //	timeStamp - the counter on the PC that serves as the system clock to align ECoG with task events
            tempBuff = &(dBuffSeq[i * numberOfChannels]);

            zmq::message_t zmq_message(sizeof(float)*numberOfChannels+sizeof(size_t)); //form a ZMQ message object; note this will cause problems if it is moved out from the loop
            //form the zmq message that contains timeStamp and tempBuff
            //(memcpy is a standard C function that copies the timeStamp into the memory address given by zmq_message.data())
            memcpy(zmq_message.data(), &timeStamp, sizeof(size_t)*1);  
            memcpy(static_cast<size_t*>(zmq_message.data())+1, tempBuff, sizeof(float)*numberOfChannels);  

            if ((timeStamp % 24000) == 0) {
                //if (timeStamp == 25000) {
                //outb(0x0, lptDataBase);
                //}
                cout<<"number of channels: "<<card->numChannels()<<endl;
                cout<<"t: "<<timeStamp<<endl;	//every 50 timeStamps, print the timeStamp
                //cout<<zmq_message.size()<<endl;			//
                cout<<numberOfSamples<<endl;			//and # of samples that were ready in the PO8e buffer
                std::chrono::duration<double> elapsed_seconds = end-start;
                cout<<"elapsed time: " << elapsed_seconds.count() << "s\n";
            }

            //char tstr[] = "10001 this that";
            //memcpy(zmq_message.data(), tstr, strlen(tstr));
            //snprintf ((char *) zmq_message.data(), 20 ,
            //    "%05d %d", 10001, timeStamp);
            //s_sendmore(publisher, "A");
            publisher.send(zmq_message);	//ZMQ command to trigger the broadcast of tempBuff and timeStamp
        }

        // get record/stop command from launcher/gui script
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
                setvbuf (pFile, NULL, _IOFBF, 10*numberOfChannels*sizeof(float));

            }
            recordSocket.send("ok", 2);
        }

        // write timeStamp and tempBuff into the data file on the PC harddrive
        // fwrite(&timeStamp, sizeof(size_t), 1, pFile);
        if (record) {
            fwrite(&(dBuff[0]), sizeof(float), numberOfChannels*numberOfSamples, pFile);
        }

    } // end of main loop

    broadcastThread.join();

    fclose(pFile);		//close the data file that records all TDT channels with timeStamp onto the PC (Puget)
    PO8e::releaseCard(card);	//for PO8e
    publisher.close();		//for ZMQ
    cout<<"terminated gracefully"<<endl;

    return 0;
}

