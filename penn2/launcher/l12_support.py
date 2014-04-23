#! /usr/bin/env python
#
# Support module generated by PAGE version 4.2.3
# In conjunction with Tcl version 8.6
#    Apr. 21, 2014 10:03:36 AM


import sys
import time

try:
    from Tkinter import *
except ImportError:
    from tkinter import *

try:
    import ttk
    py3 = 0
except ImportError:
    import tkinter.ttk as ttk
    py3 = 1

from subprocess import Popen
import tkMessageBox
from configobj import ConfigObj
from signal import SIGINT
import os

def set_Tk_var():
    # These are Tk variables used passed to Tkinter and must be
    # defined before the widgets using them are created.
    global TComboboxFrq

    global age
    age = StringVar()

    global gender
    gender = StringVar()

    global subjectNumber
    subjectNumber = StringVar()

    global hand
    hand = StringVar()

    global gridHemisphereRight
    gridHemisphereRight = StringVar()

    global gridHemisphereLeft
    gridHemisphereLeft = StringVar()

    global machineBeingUsed
    machineBeingUsed = StringVar()

    global samplingRate
    samplingRate = StringVar()

    global ecogChannels
    ecogChannels = StringVar()

    global forceSensorChannel
    forceSensorChannel = StringVar()

    global emgChannels
    emgChannels = StringVar()

    global synchronizingPulseChannel
    synchronizingPulseChannel = StringVar()

    global totalNumberOfChannels
    totalNumberOfChannels = StringVar()

    global sensorHand
    sensorHand = StringVar()

    global algorithm
    algorithm = StringVar()

    global bciMovingaverageXChannels
    bciMovingaverageXChannels = StringVar()

    global bciMovingaverageXFrequencies
    bciMovingaverageXFrequencies = StringVar()

    global bciMovingaverageYChannels
    bciMovingaverageYChannels = StringVar()

    global bciMovingaverageYFrequencies
    bciMovingaverageYFrequencies = StringVar()

    global bciMovingaverageZChannels
    bciMovingaverageZChannels = StringVar()

    global bciMovingaverageZFrequencies
    bciMovingaverageZFrequencies = StringVar()

    global bciJointrseChannels
    bciJointrseChannels = StringVar()

    global bciJointrseFrequencies
    bciJointrseFrequencies = StringVar()

    global psdWindowType
    psdWindowType = StringVar()

    global psdFeatureRate
    psdFeatureRate = StringVar()

    global psdWindowLength
    psdWindowLength = StringVar()

    global game
    game = StringVar()

    global workspace
    workspace = StringVar()

    global barWidth
    barWidth = StringVar()

    global barLength
    barLength = StringVar()

    global streamingState
    streamingState = StringVar()
    streamingState.set("Not streaming from data acquisitoin system")

    global recordingState
    recordingState = StringVar()
    recordingState.set("Not recording to local disk")

    # load last used parameters
    lastLog = ConfigObj('../data/log.txt', file_error=True)
    secLog = lastLog['ExperimentLog']

    # subject
    age.set(secLog['Age'])
    gender.set(secLog['Gender'])
    subjectNumber.set(secLog['SubjectNumber'])
    hand.set(secLog['HandDominance'])
    gridHemisphereLeft.set(secLog['GridOnLeftHemisphere'])
    gridHemisphereRight.set(secLog['GridOnRighttHemisphere'])

    # data acquistion
    machineBeingUsed.set(secLog['MachineBeingUsed'])
    samplingRate.set(secLog['SamplingRate'])
    ecogChannels.set(secLog['EcogChannels'])
    forceSensorChannel.set(secLog['ForceSensorChannel'])
    emgChannels.set(secLog['EmgChannels'])
    synchronizingPulseChannel.set(secLog['SynchronizingPulseChannel'])
    totalNumberOfChannels.set(secLog['TotalNumberOfChannels'])

    # squeeze
    sensorHand.set(secLog['SensorInHand'])

    # bci
    algorithm.set(secLog['Algorithm'])
    bciMovingaverageXChannels.set(secLog['BciMovingaverageXChannels'])
    bciMovingaverageXFrequencies.set(secLog['BciMovingaverageXFrequencies'])
    bciMovingaverageYChannels.set(secLog['BciMovingaverageYChannels'])
    bciMovingaverageYFrequencies.set(secLog['BciMovingaverageYFrequencies'])
    bciMovingaverageZChannels.set(secLog['BciMovingaverageZChannels'])
    bciMovingaverageZFrequencies.set(secLog['BciMovingaverageZFrequencies'])
    bciJointrseChannels.set(secLog['BciJointrseChannels'])
    bciJointrseFrequencies.set(secLog['BciJointrseFrequencies'])
    psdWindowType.set(secLog['PsdWindowType'])
    psdFeatureRate.set(secLog['PsdFeatureRate'])
    psdWindowLength.set(secLog['PsdWindowLength'])
    game.set(secLog['Game'])
    workspace.set(secLog['Workspace'])
    barWidth.set(secLog['BarWidth'])
    barLength.set(secLog['BarLength'])


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


def RunDemoSqueeze():
        UpdateDemoMode('0')
        global pSqueeze
        #try:
            #dataacquisitionSocket.send("squeeze_task")
        pSqueeze = Popen([r'../../squeeze/build/squeeze'],
                         cwd=r'../graphics/squeeze/build/')
        time.sleep(0.1)

def RunSqueeze():
        UpdateDemoMode('1')
        Record("Squeeze")
        global pSqueeze
        #try:
            #dataacquisitionSocket.send("squeeze_task")
        pSqueeze = Popen([r'../../squeeze/build/squeeze'],
                         cwd=r'../graphics/squeeze/build/')
        time.sleep(0.1)
        #except:
        #    pass


def StopSqueezeTask():
    global pSqueeze
    pSqueeze.send_signal(SIGINT)

    #try:
        # stop tdt recording
        #dataacquisitionSocket.send("stop")
        #dataacquisitionSocket.recv()
        # terminate squeeze and feature_extraxtor modules
        #pSqueeze.send_signal(SIGINT)
    #except:
        #pass

def RunBCI():
        UpdateDemoMode('0')
        print ('l12_support.RunBCI')
        sys.stdout.flush()

def CalibrateBCI():
        UpdateDemoMode('1')
        print ('l12_support.CalibrateBCI')
        sys.stdout.flush()

def StartDemoBCI():
        UpdateDemoMode('1')
        print ('l12_support.StartDemoBCI')
        sys.stdout.flush()

def StopBCITask():
        print ('l12_support.StopBCITask')
        sys.stdout.flush()

def MachineChanged(event):
        print machineBeingUsed.get()
        print ('l12_support.MachineChanged')
        if machineBeingUsed.get() == "gHIamp":
            TComboboxFrq['values'] = ['9600', '4800', '256']
            TComboboxFrq.current(0)
        if machineBeingUsed.get() == "TDT":
            TComboboxFrq['values'] = ['24414.062500']
            TComboboxFrq.current(0)
        sys.stdout.flush()

def UpdateDemoMode(state):
    # signal acquisition
    # feature extractor
    featureCfg = ConfigObj('../config/feature_extract_config.cfg', file_error=True)
    featureSec = featureCfg['feature']
    featureSec['DemoMode'] = state
    featureCfg.write()
    # filter
    # supervisor
    # graphics


def Record(task):
    res,msg = CheckSubjectData()
    if not res:
        tkMessageBox.showinfo("Cannot continue!", message = '"'+msg+'" is left blank!')

    res,msg = CheckDataAcquisition()
    if not res:
        tkMessageBox.showinfo("Cannot continue!", message = '"'+msg+'" is left blank!')

    res,msg = CheckSqueeze()
    if not res:
        tkMessageBox.showinfo("Cannot continue!", message = '"'+msg+'" is left blank!')

    res,msg = CheckBCI()
    if not res:
        tkMessageBox.showinfo("Cannot continue!", message = '"'+msg+'" is left blank!')

    timestr = time.strftime("%m_%d_%Y")
    directory = '../data/subject_' + subjectNumber.get() + '_@_' + timestr
    if not os.path.exists(directory):
        os.makedirs(directory)

    global dataPath
    dataPath = os.path.abspath(directory)
    print dataPath

    timestr = time.strftime("%Y_%m_%d-%H_%M_%S")
    ptimestr = time.strftime("%Y/%m/%d %H:%M:%S")
    logFile = open(directory + '/log_' + timestr + ".txt", 'w')
    logFileBackup = open('../data/log.txt', 'w')

    logFile.write('[ExperimentLog]\n')
    logFileBackup.write('[ExperimentLog]\n')

    logFile.write('Time = ' + ptimestr + "\n")
    logFileBackup.write('Time = ' + ptimestr + "\n")

    logFile.write('Task = ' + task + "\n")
    logFileBackup.write('Task = ' + task + "\n")

    RecordSubject(logFile, logFileBackup)
    RecordDataAcquisition(logFile, logFileBackup)
    RecordSqueeze(logFile, logFileBackup)
    RecordBCI(logFile, logFileBackup)

    logFile.close()
    logFileBackup.close()

def RecordSubject(logFile, logFileBackup):
    logText = ''

    logText += 'Age = ' + age.get() + "\n"
    logText += 'Gender = ' + gender.get() + "\n"
    logText += 'SubjectNumber = ' + subjectNumber.get() + "\n"
    logText += 'HandDominance = ' + hand.get() + "\n"
    logText += 'GridOnLeftHemisphere = ' + gridHemisphereLeft.get() + "\n"
    logText += 'GridOnRighttHemisphere = ' + gridHemisphereRight.get() + "\n"

    logFile.write(logText)
    logFileBackup.write(logText)


def RecordDataAcquisition(logFile, logFileBackup):
    logText = ''

    logText += 'MachineBeingUsed = ' + machineBeingUsed.get() + "\n"
    logText += 'SamplingRate = ' + samplingRate.get() + "\n"
    logText += 'EcogChannels = ' + ecogChannels.get() + "\n"
    logText += 'ForceSensorChannel = ' + forceSensorChannel.get() + "\n"
    logText += 'EmgChannels = ' + emgChannels.get() + "\n"
    logText += 'SynchronizingPulseChannel = ' + synchronizingPulseChannel.get() + "\n"
    logText += 'TotalNumberOfChannels = ' + totalNumberOfChannels.get() + "\n"

    logFile.write(logText)
    logFileBackup.write(logText)

def RecordSqueeze(logFile, logFileBackup):
    logText = ''

    logText += 'SensorInHand = ' + sensorHand.get() + "\n"

    logFile.write(logText)
    logFileBackup.write(logText)

def RecordBCI(logFile, logFileBackup):
    logText = ''

    logText += 'Algorithm = ' + algorithm.get() + "# comment goes here\n"
    logText += 'BciMovingaverageXChannels = ' + bciMovingaverageXChannels.get() + "\n"
    logText += 'BciMovingaverageXFrequencies = ' + bciMovingaverageXFrequencies.get() + "\n"
    logText += 'BciMovingaverageYChannels = ' + bciMovingaverageYChannels.get() + "\n"
    logText += 'BciMovingaverageYFrequencies = ' + bciMovingaverageYFrequencies.get() + "\n"
    logText += 'BciMovingaverageZChannels = ' + bciMovingaverageZChannels.get() + "\n"
    logText += 'BciMovingaverageZFrequencies = ' + bciMovingaverageZFrequencies.get() + "\n"
    logText += 'BciJointrseChannels = ' + bciJointrseChannels.get() + "\n"
    logText += 'BciJointrseFrequencies = ' + bciJointrseFrequencies.get() + "\n"
    logText += 'PsdWindowType = ' + psdWindowType.get() + "\n"
    logText += 'PsdFeatureRate = ' + psdFeatureRate.get() + "\n"
    logText += 'PsdWindowLength = ' + psdWindowLength.get() + "\n"
    logText += 'Game = ' + game.get() + "\n"
    logText += 'Workspace = ' + workspace.get() + "\n"
    logText += 'BarWidth = ' + barWidth.get() + "\n"
    logText += 'BarLength = ' + barLength.get() + "\n"

    logFile.write(logText)
    logFileBackup.write(logText)

    # update config file of feature extractor
    featureCfg = ConfigObj('../config/feature_extract_config.cfg', file_error=True)
    featureSec = featureCfg['feature']
    featureSec['fftWinSize'] = psdWindowLength.get()
    featureSec['outputRate'] = psdFeatureRate.get()
    if psdWindowType.get() == 'Rectangle':
        featureSec['fftWinType'] = 0
    if psdWindowType.get() == 'Hamming':
        featureSec['fftWinType'] = 1
    if psdWindowType.get() == 'Blackman-Harris':
        featureSec['fftWinType'] = 2
    global dataPath
    featureSec['dataPath'] = dataPath
    featureCfg.write()

    # update config file of filter
    print 'algo', algorithm.get()
    filterCfg = ConfigObj('../config/filter.cfg', file_error=True)
    filterSec = filterCfg['filter']
    if algorithm.get() == 'Running Average':
        filterSec['filterType'] = 0
    if algorithm.get() == 'Testing JointRSE':
        filterSec['filterType'] = 1
    if algorithm.get() == 'Training JointRSE':
        filterSec['filterType'] = 2
    filterCfg.write()



def CheckSubjectData():
    if not age.get():
        return False, "Age"
    if not gender.get():
        return False, "Sex"
    if not subjectNumber.get():
        return False, "Subject number"
    if not hand.get():
        return False, "Hand dominance"
    return True, "OK"

def CheckDataAcquisition():
    if not machineBeingUsed.get():
        return False, "Machine Being Used"
    if not samplingRate.get():
        return False, "Sampling Rate"
    if not ecogChannels.get():
        return False, "ECoG Channels"
    if not forceSensorChannel.get():
        return False, "Force Sensor Channel"
    if not emgChannels.get():
        return False, "EMG Channels"
    if not synchronizingPulseChannel.get():
        return False, "Synchronizing Pulse Channel"
    return True, "OK"

def CheckSqueeze():
    if not sensorHand.get():
        return False, "Force Sensor In ... Hand"
    return True, "OK"

def CheckBCI():
    if not algorithm.get():
        return False, "BCI: Algorithm"
    if not bciMovingaverageXChannels.get():
        return False, "BCI: Moving average X Channels"
    if not bciMovingaverageXFrequencies.get():
        return False, "BCI: Moving average X Frequencies"
    if not bciMovingaverageYChannels.get():
        return False, "BCI: Moving average Y Channels"
    if not bciMovingaverageYFrequencies.get():
        return False, "BCI: Moving average Y Frequencies"
    if not bciMovingaverageZChannels.get():
        return False, "BCI: Moving average Z Channels"
    if not bciMovingaverageZFrequencies.get():
        return False, "BCI: Movingaverage Z Frequencies"
    if not bciJointrseChannels.get():
        return False, "BCI: Jointrse Channels"
    if not bciJointrseFrequencies.get():
        return False, "BCI: Jointrse Frequencies"
    if not psdWindowType.get():
        return False, "BCI: PSD Window Type"
    if not psdFeatureRate.get():
        return False, "BCI: PSD Feature Rate"
    if not psdWindowLength.get():
        return False, "BCI: PSD Window Length"
    if not game.get():
        return False, "BCI: Game"
    if not workspace.get():
        return False, "BCI: Workspace"
    if not barWidth.get():
        return False, "BCI: Bar Width"
    if not barLength.get():
        return False, "BCI: Bar Length"
    return True, "OK"

def init(top, gui, arg=None):
    global w, top_level, root
    w = gui
    top_level = top
    root = top

def destroy_window ():
    # Function which closes the window.
    global top_level
    top_level.destroy()
    top_level = None