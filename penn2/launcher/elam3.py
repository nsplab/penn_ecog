#!/usr/bin/env python
from Tkinter import *
import ttk
import ConfigParser
import tkMessageBox
import subprocess
import time
from subprocess import Popen
from signal import SIGINT
import os
import sys
import zmq


context = zmq.Context()
dataacquisitionSocket = context.socket(zmq.REQ)
dataacquisitionSocket.connect("ipc:///tmp/record.pipe")

statusSocket = context.socket(zmq.SUB)
statusSocket.connect("ipc:///tmp/signalstream.pipe")


##################################################################
# parse the config file
##################################################################

config = ConfigParser.RawConfigParser()
config.read('tdt.cfg')

sampleRate = config.getfloat('TDT', 'sampleRate')
forceSensorCh = config.getint('TDT', 'forceSensorCh')
digitalInCh = config.getint('TDT', 'digitalInCh')
firstEcogCh = config.getint('TDT', 'firstEcogCh')
numberOfEcogChs = config.getint('TDT', 'numberOfEcogChs')
channelBCI = config.getint('TDT', 'channelBCI')

print 'sampleRate: ', sampleRate

# to be removed ?
#def saveconfig(*args):
    #try:
        #print 'test'
        #value = float(channelNumber.get())
    #except ValueError:
        #pass


##################################################################
# process IDs (one for each program that we run)
# these are handles to individual processes for starting
# and terminating them individually.
##################################################################

pFeature = None                                                   # pFeature - feature extractor
pFilter = None                                                    # pFilter - filter module
pSupervisor = None                                                # pSupervisor - supervisor module
pGraphics = None                                                  # pGraphics - graphics module
pSqueez = None                                                    # pSqueez - squeeze task module



##################################################################
# function called when start squeeze button is pressed
##################################################################
def StartSqueeze(*args):
    global pSqueeze
    global pFeature

    try:
        if not WriteData():
            return

        dataacquisitionSocket.send("squeeze_task")

        # generate the spatial matrix
        pSqueeze = Popen([r'../../squeeze/build/squeeze'],
                cwd=r'../graphics/squeeze/build/')
        time.sleep(0.1)

        # start feature extractor
        pFeature = Popen([r'../../feature_extract_cpp/build/feature_extract_cpp', '1'],
                cwd=r'../feature_extraction/feature_extract_cpp/build/')
        time.sleep(0.1)

    except:
        pass





##################################################################
# function called when Stop Squeeze Button is pressed
##################################################################
def StopSqueeze(*args):
    global pSqueeze
    global pFeature

    try:
        # stop tdt recording
        dataacquisitionSocket.send("stop")
        dataacquisitionSocket.recv()
        # terminate squeeze and feature_extraxtor modules
        pSqueeze.send_signal(SIGINT)
        pFeature.send_signal(SIGINT)
    except:
        pass

def StartCalibrate(*args):
    global pSqueeze
    global pFeature

    try:
        if not WriteData('Started Calibration'):
            return

        # start the squeeze task
        pSqueeze = Popen([r'../../squeeze/build/squeeze', '1'],
                cwd=r'../graphics/squeeze/build/')
        time.sleep(0.1)

        # start feature extractor
        pFeature = Popen([r'../../feature_extract_cpp/build/feature_extract_cpp', '1',
                          sampleRateV.get(),
                          str(int(firstEcogChV.get()) + int(numberOfEcogChsV.get()) - 1)],
                cwd=r'../feature_extraction/feature_extract_cpp/build/')
        time.sleep(0.1)

    except ValueError:
        pass

##################################################################
# function called when start BCI button is pressed
##################################################################
def StartBCI(*args):
    global pFeature
    global pFilter
    global pSupervisor
    global pGraphics
    global pSqueez

    try:
        if not WriteData():
            return

        dataacquisitionSocket.send("bci_task")

        # generate the spatial matrix
        Popen([r'../../feature_extract_cpp/spatial_matrix/build/spatial_matrix',
                '3', str(int(firstEcogChV.get()) + int(numberOfEcogChsV.get()) - 2),
                 str(int(chNum_entry.get()) - 1), '2', '3', 'featuremx.csv'],
                cwd=r'../feature_extraction/feature_extract_cpp/build/')
        time.sleep(1)

        # start feature extractor
        pFeature = Popen([r'../../feature_extract_cpp/build/feature_extract_cpp'],
                cwd=r'../feature_extraction/feature_extract_cpp/build/')
        time.sleep(0.1)

        # start supervisor
        pSupervisor = Popen([r'./supervisor.py'],
                cwd=r'../supervisor/elam3/')
        time.sleep(0.1)

        # start graphics
        pGraphics = Popen([r'../../elam3/build/elam3',
                '3', str(int(firstEcogChV.get()) + int(numberOfEcogChsV.get()) - 2),
                 str(int(chNum_entry.get()) - 1), '2', '3', 'featuremx.csv'],
                cwd=r'../graphics/elam3/build/')
        time.sleep(0.1)

        # start filter
        filterTypeStr = filterType.get()
        # TODO:
        # send the filter type to the filter process 
        pFilter = Popen([r'../../cpp/build/filter'],
                cwd=r'../filter/cpp/build')
        time.sleep(0.1)

    except:
        pass



##################################################################
# function called when Stop BCI Button is pressed
# tell acquisition module to stop recording
# terminates processes involved in the BCI task
# (pFeature, pSupervisor, pGraphics, pFilter)
##################################################################
def StopBCI(*args):
    global pFeature
    global pFilter
    global pSupervisor
    global pGraphics
    global pSqueez

    try:
        # stop tdt recording
        dataacquisitionSocket.send("stop")
        dataacquisitionSocket.recv()
        # terminate all modules
        pFeature.send_signal(SIGINT)
        pSupervisor.send_signal(SIGINT)
        pGraphics.send_signal(SIGINT)
        pFilter.send_signal(SIGINT)
    except:
        pass




##################################################################
# creates a log file for each run of experiement
##################################################################
def WriteData(action):
        missingInfo = False
        if not gender.get():
            missingInfo = True
        if not age.get():
            missingInfo = True
        if not hand.get():
            missingInfo = True
        if not grid.get():
            missingInfo = True
        if not invertPower.get():
            missingInfo = True
        if missingInfo:
            tkMessageBox.showinfo("Cannot continue!", message='Some fields are left blank!')
            return False
        print 'Gender', gender.get()
        print 'Age', age.get()
        print 'Dominant hand', hand.get()
        print 'Grid Hemisphere', grid.get()
        timestr = time.strftime("%Y_%m_%d-%H_%M_%S")
        ptimestr = time.strftime("%Y/%m/%d %H:%M:%S")
        logFile = open('log_' + timestr + ".txt", 'w')
        logFileBackup = open('log.txt', 'w')

        logFile.write('Time: ' + ptimestr + "\n")
        logFile.write('Gender: ' + gender.get() + "\n")
        logFile.write('Age: ' + age.get() + "\n")
        logFile.write('Dominant hand: ' + hand.get() + "\n")
        logFile.write('Grid Hemisphere: ' + grid.get() + "\n")
        logFile.write('BCI channel: ' + chNum_entry.get() + "\n")
        logFile.write('Inverted Power: ' + invertPower.get() + "\n")
        logFile.write('SamplingRate: ' + sampleRateV.get() + "\n")
        logFile.write('Force Sensor Channel: ' + forceCh.get() + "\n")
        logFile.write('Force Sensor in Hand: ' + forceSensorHand.get() + "\n")
        logFile.write('SamplingRate: ' + sampleRateV.get() + "\n")
        logFile.write('First ECoG Channel: ' + firstEcogChV.get() + "\n")
        logFile.write('Number of ECoG Channels: ' + numberOfEcogChsV.get() + "\n")
        logFile.write('Action: ' + 'Started BCI task' + "\n")
        logFile.write('Block Width in percent of workspace width: ' + blockWidth.get() + "\n")
        logFile.write('Block Lendth in Seconds: ' + blockLength.get() + "\n")

        logFileBackup.write('Time: ' + ptimestr + "\n")
        logFileBackup.write('Gender: ' + gender.get() + "\n")
        logFileBackup.write('Age: ' + age.get() + "\n")
        logFileBackup.write('Dominant hand: ' + hand.get() + "\n")
        logFileBackup.write('Grid Hemisphere: ' + grid.get() + "\n")
        logFileBackup.write('BCI channel: ' + chNum_entry.get() + "\n")
        logFileBackup.write('Inverted Power: ' + invertPower.get() + "\n")
        logFileBackup.write('SamplingRate: ' + sampleRateV.get() + "\n")
        logFileBackup.write('Force Sensor Channel: ' + forceCh.get() + "\n")
        logFileBackup.write('Force Sensor in Hand: ' + forceSensorHand.get() + "\n")
        logFileBackup.write('SamplingRate: ' + sampleRateV.get() + "\n")
        logFileBackup.write('First ECoG Channel: ' + firstEcogChV.get() + "\n")
        logFileBackup.write('Number of ECoG Channels: ' + numberOfEcogChsV.get() + "\n")
        logFileBackup.write('Action: ' + action + "\n")
        logFileBackup.write('Block Width in percent of workspace width: ' + blockWidth.get() + "\n")
        logFileBackup.write('Block Lendth in Seconds: ' + blockLength.get() + "\n")
        #logFile.write()
        logFile.close()
        logFileBackup.close()

        return True


##################################################################
# Prepare the gui window - making and positioning buttons, etc.
##################################################################
root = Tk()
root.title("Elam3 Launcher")
root.geometry("-1+1")

# main frame
mainframe = ttk.Frame(root, padding="3 3 12 12")
mainframe.grid(column=0, row=0, sticky=(N, W, E, S))
mainframe.columnconfigure(0, weight=1)
mainframe.rowconfigure(0, weight=1)

channelNumber = StringVar()
channelNumber.set(channelBCI)
blockWidth = StringVar()

#########################
### GUI: Subject's info
# defines GUI elements
# for the subject info
#########################

s = ttk.Style()
s.configure('s1.TLabelframe.Label', background="#D8A499")
s.configure('s2.TLabelframe.Label', background='#C6CDF7')
s.configure('s3.TLabelframe.Label', background='#7294D4')
s.configure('s4.TLabelframe.Label', background='#E680A5')
s.configure('s5.Label', background='#75EB8B')
s.configure('s6.Label', background='#FA7373')

lfSubject = ttk.Labelframe(mainframe, text='Subject: ', style='s1.TLabelframe')
lfSubject.grid(column=1, row=1, sticky=(W, E))

## GUI form to enter gender
rowNumber = 1                                                                                    #go to row 1 of the baseline/squeeze GUI frame
ttk.Label(lfSubject, text="Gender:").grid(column=1, row=rowNumber, sticky=E)
gender = StringVar()
maleE = ttk.Radiobutton(lfSubject, text='Male', variable=gender, value='Male')
femaleE = ttk.Radiobutton(lfSubject, text='Female', variable=gender, value='Female')
maleE.grid(column=2, row=rowNumber, sticky=(W, E))
rowNumber += 1
femaleE.grid(column=2, row=rowNumber, sticky=(W, E))
rowNumber += 1

## GUI form to enter age
age = StringVar()
ttk.Label(lfSubject, text="Age:").grid(column=1, row=rowNumber, sticky=E)
ageE = ttk.Entry(lfSubject, width=7, textvariable=age)
ageE.grid(column=2, row=rowNumber, sticky=(W, E))
rowNumber += 1

## GUI form to enter hand dominance
ttk.Label(lfSubject, text="Dominant Hand:").grid(column=1, row=rowNumber, sticky=E)
hand = StringVar()
rightHandE = ttk.Radiobutton(lfSubject, text='Right-handed', variable=hand, value='Right-handed')
leftHandE = ttk.Radiobutton(lfSubject, text='Left-handed', variable=hand, value='Left-handed')
rightHandE.grid(column=2, row=rowNumber, sticky=(W, E))
rowNumber += 1
leftHandE.grid(column=2, row=rowNumber, sticky=(W, E))
rowNumber += 1

## GUI form to enter grid hemisphere
ttk.Label(lfSubject, text="Grid Hemisphere:").grid(column=1, row=rowNumber, sticky=E)
grid = StringVar()
rightGridE = ttk.Radiobutton(lfSubject, text='Right', variable=grid, value='Right')
leftGridE = ttk.Radiobutton(lfSubject, text='Left', variable=grid, value='Left')
rightGridE.grid(column=2, row=rowNumber, sticky=(W, E))
rowNumber += 1
leftGridE.grid(column=2, row=rowNumber, sticky=(W, E))



####################################
### GUI Frame for: Baseline/Squeeze
####################################
rowNumber = 1                                                                                        #go to row 1 of the baseline/squeeze GUI frame
lfSqueeze = ttk.LabelFrame(mainframe, text='Squeeze Task: ', style='s2.TLabelframe')                 #define a new GUI frame for the squeeze task
lfSqueeze.grid(column=1, row=2, sticky=(W, E))                                                       #positions the squeeze task frame relative to the whole window

## GUI field to specify force sensor channel
tlabel = ttk.Label(lfSqueeze, text="Force Sensor in:")
tlabel.grid(column=1, row=rowNumber, sticky=E)                                                        #positions the force sensor field relative to the squeeze task frame
#tlabel.config(background='green')
forceSensorHand = StringVar()
forceSensorHandRE = ttk.Radiobutton(lfSqueeze, text='Right Hand', variable=forceSensorHand,
                                    value='Right Hand')

forceSensorHandLE = ttk.Radiobutton(lfSqueeze, text='Left Hand', variable=forceSensorHand,
                                    value='Left Hand')
forceSensorHandRE.grid(column=2, row=rowNumber, sticky=(W, E))
rowNumber += 1
forceSensorHandLE.grid(column=2, row=rowNumber, sticky=(W, E))
rowNumber += 1

## buttons to start and stop the squeeze task
ttk.Button(lfSqueeze, text="Start Squeeze Task", command=StartSqueeze).grid(column=1, row=rowNumber,
                                                                          sticky='we')
ttk.Button(lfSqueeze, text="Stop Squeeze Task", command=StopSqueeze).grid(column=2, row=rowNumber,
                                                                         sticky='we')

##################
### Calibrate
##################

# button
#ttk.Button(bciTab, text="Calibrate", command=StartCalibrate).grid(column=1, row=rowNumber,
#                                                                          sticky='we')
#ttk.Button(bciTab, text="Stop", command=StopCalibrate).grid(column=2, row=rowNumber,
#                                                                          sticky='we')

####################################
### GUI Frame for BCI Task
####################################
rowNumber = 1

lfBCI = ttk.Labelframe(mainframe, text='BCI Task: ', style='s3.TLabelframe')
lfBCI.grid(column=1, row=3, sticky=(W, E))

# GUI drop-down list to select the filter type
ttk.Label(lfBCI, text="BCI Algorithm being used:").grid(column=1, row=rowNumber, sticky=E)
filterTypeVar = StringVar()
filterType = ttk.Combobox(lfBCI, textvariable=filterTypeVar, state='readonly')
filterType['values'] = ('MovingAverage', 'JointRSE_test', 'JointRSE_train')
filterType.grid(column=2, row=rowNumber, sticky=(W, E))
filterType.current(0)
rowNumber += 1


# GUI radiobutton for option to invert the power (multiply all powers by -1)
ttk.Label(lfBCI, text="Invert the power:").grid(column=1, row=rowNumber, sticky=E)
invertPower = StringVar()
invertPowerYE = ttk.Radiobutton(lfBCI, text='Yes', variable=invertPower, value='Yes')
invertPowerNE = ttk.Radiobutton(lfBCI, text='No', variable=invertPower, value='No')
invertPowerNE.grid(column=2, row=rowNumber, sticky=(W, E))
rowNumber += 1
invertPowerYE.grid(column=2, row=rowNumber, sticky=(W, E))
rowNumber += 1


# GUI drag down window to select filter type - should be commented in detail
# - TO DO 4/8/14 @ 9am

# GUI input box for list of channels to use - as well as brief example syntax: 1, 2-5, 6, 7
# set this up so that it loads the previous list of channels
# - TO DO 4/8/14 @ 9am
# the below GUI input box for single channel could be removed
# frequency band for all channels to use
#

# GUI input box for one single channel used in the Moving Average Filter
ttk.Label(lfBCI, text="Channel Number for\n Moving Average Filter:").grid(column=1, row=rowNumber, sticky=E)
chNum_entry = ttk.Entry(lfBCI, width=7, textvariable=channelNumber)
chNum_entry.grid(column=2, row=rowNumber, sticky=(W, E))
rowNumber += 1

# input box for block width
blockWidth = StringVar()
blockWidth.set(0.25)
ttk.Label(lfBCI, text="Block Width percent\n of Screen:").grid(column=1, row=rowNumber,
          sticky=E)
blockWidthE = ttk.Entry(lfBCI, width=7, textvariable=blockWidth)
blockWidthE.grid(column=2, row=rowNumber, sticky='e')
rowNumber += 1

# input box for block width
blockLength = StringVar()
blockLength.set(10)
ttk.Label(lfBCI, text="Block Length in Seconds:").grid(column=1, row=rowNumber,
          sticky=E)
blockLengthE = ttk.Entry(lfBCI, width=7, textvariable=blockLength)
blockLengthE.grid(column=2, row=rowNumber, sticky=(W, E))
rowNumber += 1

## buttons
ttk.Button(lfBCI, text="Start BCI Task", command=StartBCI).grid(column=1, row=rowNumber,
                                                                    sticky='e')
ttk.Button(lfBCI, text="Stop BCI Task", command=StopBCI).grid(column=2, row=rowNumber,
                                                                     sticky='e')
rowNumber += 1

## separator
#ttk.Separator(mainframe, orient=HORIZONTAL).grid(row=rowNumber, columnspan=5, sticky='WE')
rowNumber += 1

##################
### GUI: TDT config
##################

lfTDT = ttk.Labelframe(mainframe, text='TDT: ', style='s4.TLabelframe')
lfTDT.grid(column=1, row=4, sticky=(W, E))

streamingLabel = ttk.Label(lfTDT, text="Streaming", style='s5.Label')
streamingLabel.grid(column=1, row=rowNumber, sticky=E)
#streamingLabel.state(['disabled'])
notStreamingLabel = ttk.Label(lfTDT, text="Not streaming", style='s6.Label')
notStreamingLabel.grid(column=2, row=rowNumber, sticky=E)
#notStreamingLabel.state(['disabled'])
rowNumber += 1

ttk.Label(lfTDT, text="Sampling Rate:").grid(column=1, row=rowNumber, sticky=E)
sampleRateV = StringVar()
sampleRateV.set(sampleRate)
sampleRateE = ttk.Entry(lfTDT, textvariable=sampleRateV)
sampleRateE.grid(column=2, row=rowNumber, sticky='e')
rowNumber += 1

ttk.Label(lfTDT, text="Force sensor Ch Number:").grid(column=1, row=rowNumber, sticky=E)
forceCh = StringVar()
forceCh.set(forceSensorCh)
forceChE = ttk.Entry(lfTDT, textvariable=forceCh)
forceChE.grid(column=2, row=rowNumber, sticky='e')
rowNumber += 1

ttk.Label(lfTDT, text="First ECoG Ch Number:").grid(column=1, row=rowNumber, sticky=E)
firstEcogChV = StringVar()
firstEcogChV.set(firstEcogCh)
firstEcogChE = ttk.Entry(lfTDT, textvariable=firstEcogChV)
firstEcogChE.grid(column=2, row=rowNumber, sticky='e')
rowNumber += 1

ttk.Label(lfTDT, text="Number of ECoG Channels:").grid(column=1, row=rowNumber, sticky=E)
numberOfEcogChsV = StringVar()
numberOfEcogChsV.set(numberOfEcogChs)
numberOfEcogChsE = ttk.Entry(lfTDT, textvariable=numberOfEcogChsV)
numberOfEcogChsE.grid(column=2, row=rowNumber, sticky='e')

#ttk.Label(mainframe, textvariable=channelNumber).grid(column=2, row=2, sticky=(W, E))
#ttk.Label(mainframe, text="").grid(column=1, row=2, sticky=E)
#ttk.Label(mainframe, text="").grid(column=3, row=2, sticky=W)

for child in mainframe.winfo_children():
    child.grid_configure(padx=5, pady=3)

for child in lfTDT.winfo_children():
    child.grid_configure(padx=5, pady=3)

for child in lfSqueeze.winfo_children():
    child.grid_configure(padx=5, pady=3)

for child in lfBCI.winfo_children():
    child.grid_configure(padx=5, pady=3)

for child in lfSubject.winfo_children():
    child.grid_configure(padx=5, pady=3)

ageE.focus()
#root.bind('<Return>', saveconfig)

##################################################################
# prepare the gui window
##################################################################
# 0. load kernel module
cwd = os.getcwd()
os.chdir("../signal_acquisition/tdt/")
lproc = Popen([r'sudo', './loaddriver.sh'], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
(outmsg, errmsg) = lproc.communicate()
print 'errmsg :', errmsg
print 'outmsg :', outmsg
os.chdir(cwd)

if errmsg:
    tkMessageBox.showinfo(message='Could not load the kernel module! Make sure the TDT kernel module is compiled for the current kernel version.')
    #sys.exit("Couldn't load TDT kernel module")

# 1. set TDT mode

# 2. check signal acquisi

# 3.


##################################################################
# check TDT streaming status and update the GUI element
##################################################################
def checkStatus():
    msg = ""
    try:
        msg = socket.recv(zmq.DONTWAIT)
    except:
        pass

    status = 0
    if msg == "1":
        status = 1

    if status == 0:
        streamingLabel.state(['disabled'])
        notStreamingLabel.state(['!disabled'])
    else:
        streamingLabel.state(['!disabled'])
        notStreamingLabel.state(['disabled'])

    root.update()

    root.after(0, checkStatus)

##################################################################
# run main loop of the gui
##################################################################
root.after(0, checkStatus)
root.mainloop()
