=========
penn_ecog
=========

.. contents:: Contents
   :backlinks: top
   

Structure
=========

This ECoG code implements the Remote Collaborative Platform, 3D virtual environment, Video Game, and Static/Adaptive Filters

penn2:
  The recent software for the penn project.

ver1_kevins_code_eeg:
  The first version of Kevin's code adapted to be used with EEG.


Streaming Data from the Gtech into Windows 7 Virtual Environment into the Linux Environment
===========================================================================================

This functionality is needed for penn2, easter, and impedance measurements. Below, we describe how to stream data from the Windows 7 virtual environment and how the related code is structured to achieve this.

Running the gtech code on the windows virtual environment (for penn code and rabbit experiment)
-----------------------------------------------------------------------------------------------

Required Software: Oracle VM Virtual Box, Version 4.3.6 r91406; runs Windows 7 on Linux
  - Free software from https://www.virtualbox.org/
  - Runs Windows 7 on Linux

File / Import Appliance - allows you to import the Windows 7 environment with the g.tech code fully installed
  - Note: Visual studio 2008 is installed on the Windows7 virtual environment
  - Icons on on the desktop:
      - ghiamp_record - program that receives data from the gtech and uses ZMQ to broadcast the signal. Code for this program was modified from the gHIampDemo.cpp program, in the originally provided Windows SDK (sent by email to Mosalam). This code is located in C:\Users\nsplab\Desktop\gHIampCAPI\C++ Projects\gHIampDemo\gHIampDemo.sln - “solution file” contains the Visual Studio project for the GHIampDemo. Double click on this file to open this collection of code.
          - To record: Double click on ghiamp_record - if the window says “receiving data...”, then everything is working fine.
          - If you close the ghiamp_record program, you will need to cycle power on the g.Hiamp machine in order to restart the ghiamp_record. If you turn off the g.Hiamp with ghiamp_record running, that program will crash and you will need to first turn the g.Hiamp on, next wait about 2 minutes for the g.Hiamp to boot and be recognized by Windows 7 (in the virtual box, go to Devices/USB Devices to check to see if g.Hiamp is a recognized device), and then restart the ghiamp_record program. In general, you should be fine running ghiamp_record for days if the g.Hiamp machine doesn’t get turned off.

          - Important sampling parameters set in gHIampDemo:
              - C:\Users\nsplab\Desktop\gHIampCAPI\Doc\gHIampCAPI.pdf - this is the manual for the gHIamp API. It doesn’t discuss the demo program, but it does describe each function separately, including these critical parameters:
              - SAMPLE_RATE_HZ - 9600 Hz is hardcorded in the source code (line 52 of gHIampDemo.cpp - SAMPLE_RATE_HZ).
              - NUMBER_OF_SCANS - how many samples collected into a single packet before being sent from g.tech to computer. This value needs to be taken from a table in the above PDF based on the sampling rate.
              - NUMBER_OF_CHANNELS - hardcoded in line 57 of gHIampDemo.cpp
              - ENABLE_TRIGGER - adds the first digital input channel as the last channel in the data. Channels are indexed from 0 to 63 for a 64 channel recording system. If ENABLE_TRIGGER = TRUE, then 65 channels are recorded; channel with index 64 is the digital input channel. This channel is a stream of floating points (not 0’s and 1’s, but approximately 0.000 and 1.000) sampled at the SAMPLE_RATE_HZ sampling rate.
             
              - _deviceSerial - this number needs to match the serial number from the top of the machine

          - ZMQ broadcasting in gHIampDemo.cpp
            __________________________________________________________
            Quick note about ZMQ: 
            Overall, we only use 2 ZMQ protocols (only the first one in gHIampDemo.cpp)
            (1) ZMQ_PUB / ZMQ_SUB - broadcasting with infinite buffering (limited by system memory) / subscribing to that broadcast

            (2) ZMQ_REQ / ZMQ_REP  - peer to peer, one requests and the other responds; the receivers can be blocking (waits to receive message before proceeding) or nonblocking
            __________________________________________________________

            - gHIampDemo.cpp sets up ZMQ broadcasting using TCP protocol. We used TCP for this particular program instead of IPC (which we used for all of the penn2 modules) because we’re broadcasting from Windows and receiving on Linux (windows doesn’t have IPC).
            - The windows virtual machine's local network adapter is configured with a static IP, which is needed to run ZMQ TCP protocol. As a result, the virtual machine is not connected to the internet, i.e. is not sharing the internet connection of the host OS (Ubuntu). This way the code on Linux connects to the socket with the static IP.
            - To share files between the Windows and the Linux machine, there is a shared directory set in the virtual box. 
            - Linux path: /home/user/Download if moved to another Linux host, remember to change the address here.
            - Windows path: E:\
            - Example: on the windows machine there is no PDF reader, to read the gTech C API document, you would copy the pdf to the download directory on Linux and view it there. A more technical usage example is the screenshot impedances from the gRecorder application that can be copied to Linux.
            - tcp://192.168.56.110:5556 - this is the specific TCP socket that is used, specifically a socket of the ZMQ type: ZMQ_PUB
            - impedance_measure - measures impedances - an infinite loop measures impedances (resistance at 1 kHz (?)) of channels one at a time and prints them in the terminal.

            - gRecorder Shortcut - shortcut to gRecorder given by g.tech for recording and live viewing of signals. This program needs the usb key that is taped to the g.Hiamp, which is properly detected in the Windows virtual environment.Recording from the g.tech in Linux

             (1) Make sure the Windows7 virtual environment is running, and that you’ve started ghiamp_record from the Windows desktop.

             (2) On LInux, in terminal, change to the directory containing the relevant Linux code for signal_acquisition from the g.Hiamp
             $ cd penn_ecog/penn2/signal_acquisition/gtec/build/

             (3) Begin recording (default hard-coded 64 analog channels @ 9600 Hz, set in the Windows code, described above).

             To include the Digital-In channel in the recording (make sure you’ve written ENABLE_TRIGGER=TRUE in the windows code), run this command:

             $ ./gtec 1  

             To exclude the Digital-In channel from the recording (make sure you’ve set ENABLE_TRIGGER=FALSE in the windows code), run this command instead:
             $ ./gtec 

             This program receives the data from the windows machine by subscribing to the TCP subscriber socket implemented through a ZMQ_SUB socket at the same address that the Windows code broadcasts: tcp://192.168.56.110:5556

             Next, the program broadcasts the same data to make it available to the other Linux modules. For this, the program uses the Linux interprocess communication (IPC) protocol implemented through a ZMQ_PUB socket given at the address ipc:///tmp/signal.pipe.

             The program also looks for RECORD/STOP-RECORD commands  (not specifically those strings - keep reading below) through ZMQ using ZMQ_REP socket address at ipc:///tmp/record.pipe  .  In this way, other modules in Linux can control whether this program is recording or not.

             The command to start recording over the IPC socket is any string (character array) except this array: 'stop' without the quotation marks. This string is added to the filename, which is a convenient way to add additional data to the file (such as by using ‘VEP’ if the recording is a visual evoked potential.

             When this command comes in, a new data file is created in the directory ‘/penn2/data/’  with filename ‘data_STRING-SENT-IN_RECORD-COMMAND_timestamp
             ’.
             (3)  To stop recording, you will need to send a stop command ‘stop’ (in all lower case without quotes) at the Linux terminal. For example, in Easter, this command is executed in Python (file: , line number: , location: ):
             recording/vep/record_VEP.py


             As another example, in Penn2, from the Launcher, this same command is also executed in Python (file: , line number: , location: )



             Tracing a sample through the entire Penn2 pipeline

             Penn2 has the following modules:
             data_acqiusition


             (1) The sample is acquired by the data acquisition system module. A separate data acquisition module is available for each of the following devices: g.HIamp, TDT,Kinect, Imitator (simulated user control policy and EEG signals ), Imitator-Direct (simulated user control policy where the EEG signals are just the intended 3x1 velocity vector of the user)

             (a) for the g.HIamp, the sample is acquired in the Windows 7 virtual environment by ghiamp_record and broadcast through a ZMQ TCP socket. Then penn2/signal_acquisition/gtec/build/gtec broadcasts the data over a ZMQ IPC socket.

             (b) for the TDT, the executable ./PO8eBroadcast contained in penn2/signal_acquisition/tdt (compiled using penn2/signal_acquisition/tdt/makepo.sh) interfaces with the kernel module (like a driver) provided by TDT. This kernel module is important, and must be compiled with reference to a specific Linux version (we are assuming Linux version 3.8.0-30-generic and ubuntu  (you can get this information with $uname -a).The source code for the kernel module is at  penn2 / signal_acquisition / tdt / PO8eStreaming/.   Information provided by the TDT company about these files is located here on github. To compile this kernel module source code, run the shell script buildPLX.  This script probably does not need to be modified. Note that PLX is a generic PCI card company, and the PO8e is built on this platform. (Note that Linux versions greater than v3.10 are not supported.)

             (c) for the Kinect, the executable ./kinectv2.py  contained in penn2/signal_acquisition/kinect


             (d) for the imitator, the executable ./imitator contained in penn2/signal_acquisition/imitator/build

             (e) for the imitator-direct, the executable ./imitator contained in penn2/signal_acquisition/imitator/build



             (2) The data acquisition module broadcasts 


             Kinect

             - must use USB 2, probably will not work on USB 3

             Config Files

             config/supervisor_config.cfg
             config/feature_extract_signal.cfg
             config/signal_tdt.cfg
             config/feature_extract_config.cfg
             config/signal_gtec.cfg
             config/penn.cfg
             config/graphics.cfg
             config/filter.cfg
             graphics/elam2/elam2.cfg
             graphics/cardgame/cardgame.cfg
             feature_extraction/feature_extract_cpp/kinect_signal.cfg
             data/filter_settings.cfg
             launcher/tdt.cfg
             filter/cpp/filter.cfg


