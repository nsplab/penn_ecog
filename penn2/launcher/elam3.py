#!/usr/bin/env python
from Tkinter import *
import ttk
import ConfigParser
import tkMessageBox
import subprocess
import time
from subprocess import Popen
from signal import SIGINT

config = ConfigParser.RawConfigParser()
config.read('tdt.cfg')

sampleRate = config.getfloat('TDT', 'sampleRate')
forceSensorCh = config.getint('TDT', 'forceSensorCh')
digitalInCh = config.getint('TDT', 'digitalInCh')
firstEcogCh = config.getint('TDT', 'firstEcogCh')
numberOfEcogChs = config.getint('TDT', 'numberOfEcogChs')
channelBCI = config.getint('TDT', 'channelBCI')

print 'sampleRate: ', sampleRate


def saveconfig(*args):
    try:
        print 'test'
        value = float(channelNumber.get())
    except ValueError:
        pass

pFeature = None
pFilter = None
pSupervisor = None
pGraphics = None
pSqueez = None


def StopBCI(*args):
    global pFeature
    global pFilter
    global pSupervisor
    global pGraphics
    global pSqueez

    try:
        pFeature.send_signal(SIGINT)
        pSupervisor.send_signal(SIGINT)
        pGraphics.send_signal(SIGINT)
        pFilter.send_signal(SIGINT)
    except ValueError:
        pass


def StopSqueeze(*args):
    global pSqueeze
    global pFeature

    try:
        pSqueeze.send_signal(SIGINT)
        pFeature.send_signal(SIGINT)
    except ValueError:
        pass


def WriteData(*args):
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
        logFileBackup.write('Action: ' + 'Started BCI task' + "\n")
        logFileBackup.write('Block Width in percent of workspace width: ' + blockWidth.get() + "\n")
        logFileBackup.write('Block Lendth in Seconds: ' + blockLength.get() + "\n")
        #logFile.write()
        logFile.close()
        logFileBackup.close()

        return True


def StartSqueeze(*args):
    global pSqueeze
    global pFeature

    try:
        if not WriteData():
            return

        # generate the spatial matrix
        pSqueeze = Popen([r'../../squeeze/build/squeeze'],
                cwd=r'../graphics/squeeze/build/')
        time.sleep(0.1)

        # start feature extractor
        pFeature = Popen([r'../../feature_extract_cpp/build/feature_extract_cpp', '1'],
                cwd=r'../feature_extraction/feature_extract_cpp/build/')
        time.sleep(0.1)

    except ValueError:
        pass


def StartBCI(*args):
    global pFeature
    global pFilter
    global pSupervisor
    global pGraphics
    global pSqueez

    try:
        if not WriteData():
            return

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
        pFilter = Popen([r'../../cpp/build/filter'],
                cwd=r'../filter/cpp/build')
        time.sleep(0.1)

    except ValueError:
        pass

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

##################
### Subject's info
##################

rowNumber = 1

## gender
ttk.Label(mainframe, text="Gender:").grid(column=1, row=rowNumber, sticky=E)
gender = StringVar()
maleE = ttk.Radiobutton(mainframe, text='Male', variable=gender, value='Male')
femaleE = ttk.Radiobutton(mainframe, text='Female', variable=gender, value='Female')
maleE.grid(column=2, row=rowNumber, sticky=(W, E))
rowNumber += 1
femaleE.grid(column=2, row=rowNumber, sticky=(W, E))
rowNumber += 1

## age
age = StringVar()
ttk.Label(mainframe, text="Age:").grid(column=1, row=rowNumber, sticky=E)
ageE = ttk.Entry(mainframe, width=7, textvariable=age)
ageE.grid(column=2, row=rowNumber, sticky=(W, E))
rowNumber += 1

## dominant hand
ttk.Label(mainframe, text="Dominant Hand:").grid(column=1, row=rowNumber, sticky=E)
hand = StringVar()
rightHandE = ttk.Radiobutton(mainframe, text='Right-handed', variable=hand, value='Right-handed')
leftHandE = ttk.Radiobutton(mainframe, text='Left-handed', variable=hand, value='Left-handed')
rightHandE.grid(column=2, row=rowNumber, sticky=(W, E))
rowNumber += 1
leftHandE.grid(column=2, row=rowNumber, sticky=(W, E))
rowNumber += 1

## grid hemisphere
ttk.Label(mainframe, text="Grid Hemisphere:").grid(column=1, row=rowNumber, sticky=E)
grid = StringVar()
rightGridE = ttk.Radiobutton(mainframe, text='Right', variable=grid, value='Right')
leftGridE = ttk.Radiobutton(mainframe, text='Left', variable=grid, value='Left')
rightGridE.grid(column=2, row=rowNumber, sticky=(W, E))
rowNumber += 1
leftGridE.grid(column=2, row=rowNumber, sticky=(W, E))
rowNumber += 1

## separator
ttk.Separator(mainframe, orient=HORIZONTAL).grid(row=rowNumber, columnspan=5, sticky='WE')
rowNumber += 1

##################
### Baseline/Squeeze
##################

## sensor
ttk.Label(mainframe, text="Force Sensor in:").grid(column=1, row=rowNumber, sticky=E)
forceSensorHand = StringVar()
forceSensorHandRE = ttk.Radiobutton(mainframe, text='Right Hand', variable=forceSensorHand,
                                    value='Right Hand')
forceSensorHandLE = ttk.Radiobutton(mainframe, text='Left Hand', variable=forceSensorHand,
                                    value='Left Hand')
forceSensorHandRE.grid(column=2, row=rowNumber, sticky=(W, E))
rowNumber += 1
forceSensorHandLE.grid(column=2, row=rowNumber, sticky=(W, E))
rowNumber += 1

## buttons
ttk.Button(mainframe, text="Start Squeeze Task", command=StartSqueeze).grid(column=1, row=rowNumber,
                                                                          sticky='we')
ttk.Button(mainframe, text="Stop Squeeze Task", command=StopSqueeze).grid(column=2, row=rowNumber,
                                                                         sticky='we')
rowNumber += 1

## separator
ttk.Separator(mainframe, orient=HORIZONTAL).grid(row=rowNumber, columnspan=5, sticky='WE')
rowNumber += 1

##################
### BCI Task
##################

# invert the power
ttk.Label(mainframe, text="Invert the power:").grid(column=1, row=rowNumber, sticky=E)
invertPower = StringVar()
invertPowerYE = ttk.Radiobutton(mainframe, text='Yes', variable=invertPower, value='Yes')
invertPowerNE = ttk.Radiobutton(mainframe, text='No', variable=invertPower, value='No')
invertPowerNE.grid(column=2, row=rowNumber, sticky=(W, E))
rowNumber += 1
invertPowerYE.grid(column=2, row=rowNumber, sticky=(W, E))
rowNumber += 1


# input box for channel number
ttk.Label(mainframe, text="Channel Number:").grid(column=1, row=rowNumber, sticky=E)
chNum_entry = ttk.Entry(mainframe, width=7, textvariable=channelNumber)
chNum_entry.grid(column=2, row=rowNumber, sticky=(W, E))
rowNumber += 1

# input box for block width
blockWidth = StringVar()
blockWidth.set(0.25)
ttk.Label(mainframe, text="Block Width percent\n of Screen:").grid(column=1, row=rowNumber,
          sticky=E)
blockWidthE = ttk.Entry(mainframe, width=7, textvariable=blockWidth)
blockWidthE.grid(column=2, row=rowNumber, sticky=(W, E))
rowNumber += 1

# input box for block width
blockLength = StringVar()
blockLength.set(10)
ttk.Label(mainframe, text="Block Length in Seconds:").grid(column=1, row=rowNumber,
          sticky=E)
blockLengthE = ttk.Entry(mainframe, width=7, textvariable=blockLength)
blockLengthE.grid(column=2, row=rowNumber, sticky=(W, E))
rowNumber += 1

## buttons
ttk.Button(mainframe, text="Start BCI Task", command=StartBCI).grid(column=1, row=rowNumber,
                                                                    sticky='we')
ttk.Button(mainframe, text="Stop BCI Task", command=StopBCI).grid(column=2, row=rowNumber,
                                                                     sticky='we')
rowNumber += 1

## separator
ttk.Separator(mainframe, orient=HORIZONTAL).grid(row=rowNumber, columnspan=5, sticky='WE')
rowNumber += 1

##################
### TDT config
##################


ttk.Label(mainframe, text="Sampling Rate:").grid(column=1, row=rowNumber, sticky=E)
sampleRateV = StringVar()
sampleRateV.set(sampleRate)
sampleRateE = ttk.Entry(mainframe, textvariable=sampleRateV)
sampleRateE.grid(column=2, row=rowNumber, sticky='e')
rowNumber += 1

ttk.Label(mainframe, text="Force sensor Ch Number:").grid(column=1, row=rowNumber, sticky=E)
forceCh = StringVar()
forceCh.set(forceSensorCh)
forceChE = ttk.Entry(mainframe, textvariable=forceCh)
forceChE.grid(column=2, row=rowNumber, sticky='e')
rowNumber += 1

ttk.Label(mainframe, text="First ECoG Ch Number:").grid(column=1, row=rowNumber, sticky=E)
firstEcogChV = StringVar()
firstEcogChV.set(firstEcogCh)
firstEcogChE = ttk.Entry(mainframe, textvariable=firstEcogChV)
firstEcogChE.grid(column=2, row=rowNumber, sticky='e')
rowNumber += 1

ttk.Label(mainframe, text="Number of ECoG Channels:").grid(column=1, row=rowNumber, sticky=E)
numberOfEcogChsV = StringVar()
numberOfEcogChsV.set(numberOfEcogChs)
numberOfEcogChsE = ttk.Entry(mainframe, textvariable=numberOfEcogChsV)
numberOfEcogChsE.grid(column=2, row=rowNumber, sticky='e')

#ttk.Label(mainframe, textvariable=channelNumber).grid(column=2, row=2, sticky=(W, E))
#ttk.Label(mainframe, text="").grid(column=1, row=2, sticky=E)
#ttk.Label(mainframe, text="").grid(column=3, row=2, sticky=W)

for child in mainframe.winfo_children():
    child.grid_configure(padx=5, pady=5)

ageE.focus()
#root.bind('<Return>', saveconfig)

root.mainloop()