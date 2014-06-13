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


Streaming Data from the Gtech into the Linux Environment
This functionality is needed for penn2, easter, and impedance measurements. Below, we describe how to stream data from the Windows 7 virtual environment and how the reltaed code is structured to achieve this.

Running the gtech code on the windows virtual environment (for penn code and rabbit experiment)

software: Oracle VM Virtual Box, Version 4.3.6 r91406; runs Windows 7 on Linux
free
		File / Import Appliance - allows you to import the Windows 7 environment with the g.tech code fully installed

		Visual studio 2008 is installed on the Windows7 virtual environment

		on the desktop:
ghiamp_record - program that receives data from the gtech and uses ZMQ to broadcast the signal. Code for this program was modified from the gHIampDemo.cpp program, in the originally provided Windows SDK (sent by email to Mosalam). This code is located in C:\Users\nsplab\Desktop\gHIampCAPI\C++ Projects\gHIampDemo\gHIampDemo.sln - “solution file” contains the Visual Studio project for the GHIampDemo. Double click on this file to open this collection of code.
To record: Double click on ghiamp_record - 
Important sampling parameters set in gHIampDemo:
C:\Users\nsplab\Desktop\gHIampCAPI\Doc\gHIampCAPI.pdf - this is the manual for the gHIamp API. It doesn’t discuss the demo program, but it does describe each function separately, including these critical parameters:

SAMPLE_RATE_HZ - 9600 Hz is hardcorded in the source code (line 52 of gHIampDemo.cpp - SAMPLE_RATE_HZ).
NUMBER_OF_SCANS - how many samples collected into a single packet before being sent from g.tech to computer. This value needs to be taken from a table in the above PDF based on the sampling rate.

NUMBER_OF_CHANNELS - hardcoded in line 57 of gHIampDemo.cpp

ENABLE_TRIGGER - adds the first digital input channel as the last channel in the data. Channels are indexed from 0 to 63 for a 64 channel recording system. If ENABLE_TRIGGER = TRUE, then 65 channels are recorded; channel with index 64 is the digital input channel. This channel is a stream of floating points (not 0’s and 1’s, but approximately 0.000 and 1.000) sampled at the SAMPLE_RATE_HZ sampling rate.
 
_deviceSerial - this number needs to match the serial number from the top of the machine
