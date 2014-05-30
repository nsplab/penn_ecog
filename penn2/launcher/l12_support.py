#! /usr/bin/env python
#
# Support module generated by PAGE version 4.2.3
# In conjunction with Tcl version 8.6
#    Apr. 21, 2014 10:03:36 AM


import sys
import time
from subprocess import call
import threading

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
import tkFileDialog
from configobj import ConfigObj
from signal import SIGINT
import os
from Dialog import Dialog

# to use the zmq library
import zmq

context = zmq.Context()

# socket used to check the streaming status of the signal acquisition module
statusSocket = context.socket(zmq.SUB)
# receive all the messages, sets the filter to an empty string
statusSocket.setsockopt(zmq.CONFLATE, 1)

statusSocket.connect("ipc:///tmp/signalstream.pipe")
statusSocket.setsockopt(zmq.SUBSCRIBE, "")

def check_streaming_status():
    global quitLauncher, Label31, streamingState
    print "test"
    print "quitLauncher ", quitLauncher

    if not quitLauncher:

        try:
            msg = statusSocket.recv(zmq.DONTWAIT)
            print "msg ", msg
        except:
            pass
        if msg == "1":
            Label31.configure(background="#30ff30")
            streamingState.set("Streaming from data acquisitoin system")
        else:
            Label31.configure(background="#ff3030")
            streamingState.set("Not streaming from data acquisitoin system")

        streamingStatusTimer = threading.Timer(2.0, check_streaming_status)
        streamingStatusTimer.start()


def set_Tk_var():
    # These are Tk variables used passed to Tkinter and must be
    # defined before the widgets using them are created.
    global TComboboxFrq

    global annotationBox

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

    global Label31
    global streamingLabel
    streamingLabel = StringVar()

    global selectedSession
    selectedSession = ""

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
    bciMovingaverageXChannels.set(",".join(secLog['BciMovingaverageXChannels']))
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
pSignal = None                                                    # pgTec - gtec module
quitLauncher = False

def LoadDriver():
    Record("Load Driver")

    global pSignal
    print machineBeingUsed.get()
    if machineBeingUsed.get() == "gHIamp":
        if not (pSignal is None):
            pSignal.send_signal(SIGINT)
            pSignal = None
        pSignal = Popen([r'../../gtec/build/gtec', '1'],
                cwd=r'../signal_acquisition/gtec/build/')
        time.sleep(0.1)
    elif machineBeingUsed.get() == "TDT":
        call(["killall", "PO8eBroadcast"])
        call(["/home/user/Desktop/loaddriver.py"])
        if not (pSignal is None):
            pSignal.send_signal(SIGINT)
            pSignal = None
        pSignal = Popen([r'../tdt/PO8eBroadcast'],
                cwd=r'../signal_acquisition/tdt/')
        time.sleep(0.1)
    elif machineBeingUsed.get() == "Imitator":
        if not (pSignal is None):
            pSignal.send_signal(SIGINT)
            pSignal = None
        pSignal = Popen([r'../../imitator/build/imitator'],
                cwd=r'../signal_acquisition/imitator/build/')
        time.sleep(0.1)

    streamingStatusTimer = threading.Timer(2.0, check_streaming_status)
    streamingStatusTimer.start()

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

class MessageBox(object):

    def __init__(self, msg, b1, b2, frame, t, entry):

        root = self.root = tkinter.TK()
        root.title('Message')
        self.msg = str(msg)
        # ctrl+c to copy self.msg
        root.bind('<Control-c>', func=self.to_clip)
        # remove the outer frame if frame=False
        if not frame: root.overrideredirect(True)
        # default values for the buttons to return
        self.b1_return = True
        self.b2_return = False
        # if b1 or b2 is a tuple unpack into the button text & return value
        if isinstance(b1, tuple): b1, self.b1_return = b1
        if isinstance(b2, tuple): b2, self.b2_return = b2
        # main frame
        frm_1 = tkinter.Frame(root)
        frm_1.pack(ipadx=2, ipady=2)
        # the message
        message = tkinter.Label(frm_1, text=self.msg)
        message.pack(padx=8, pady=8)
        # if entry=True create and set focus
        if entry:
            self.entry = tkinter.Entry(frm_1)
            self.entry.pack()
            self.entry.focus_set()
        # button frame
        frm_2 = tkinter.Frame(frm_1)
        frm_2.pack(padx=4, pady=4)
        # buttons
        btn_1 = tkinter.Button(frm_2, width=8, text=b1)
        btn_1['command'] = self.b1_action
        btn_1.pack(side='left')
        if not entry: btn_1.focus_set()
        btn_2 = tkinter.Button(frm_2, width=8, text=b2)
        btn_2['command'] = self.b2_action
        btn_2.pack(side='left')
        # the enter button will trigger the focused button's action
        btn_1.bind('<KeyPress-Return>', func=self.b1_action)
        btn_2.bind('<KeyPress-Return>', func=self.b2_action)
        # roughly center the box on screen
        # for accuracy see: http://stackoverflow.com/a/10018670/1217270
        root.update_idletasks()
        xp = (root.winfo_screenwidth() // 2) - (root.winfo_width() // 2)
        yp = (root.winfo_screenheight() // 2) - (root.winfo_height() // 2)
        geom = (root.winfo_width(), root.winfo_height(), xp, yp)
        root.geometry('{0}x{1}+{2}+{3}'.format(*geom))
        # call self.close_mod when the close button is pressed
        root.protocol("WM_DELETE_WINDOW", self.close_mod)
        # a trick to activate the window (on windows 7)
        root.deiconify()
        # if t is specified: call time_out after t seconds
        if t: root.after(int(t*1000), func=self.time_out)

    def b1_action(self, event=None):
        try: x = self.entry.get()
        except AttributeError:
            self.returning = self.b1_return
            self.root.quit()
        else:
            if x:
                self.returning = x
                self.root.quit()

    def b2_action(self, event=None):
        self.returning = self.b2_return
        self.root.quit()

    # remove this function and the call to protocol
    # then the close button will act normally
    def close_mod(self):
        pass

    def time_out(self):
        try: x = self.entry.get()
        except AttributeError: self.returning = None
        else: self.returning = x
        finally: self.root.quit()

    def to_clip(self, event=None):
        self.root.clipboard_clear()
        self.root.clipboard_append(self.msg)

def mbox(msg, b1='OK', b2='Cancel', frame=True, t=False, entry=False):
    """Create an instance of MessageBox, and get data back from the user.
    msg = string to be displayed
    b1 = text for left button, or a tuple (<text for button>, <to return on press>)
    b2 = text for right button, or a tuple (<text for button>, <to return on press>)
    frame = include a standard outerframe: True or False
    t = time in seconds (int or float) until the msgbox automatically closes
    entry = include an entry widget that will have its contents returned: True or False
    """
    msgbox = MessageBox(msg, b1, b2, frame, t, entry)
    msgbox.root.mainloop()
    # the function pauses here until the mainloop is quit
    msgbox.root.destroy()
    return msgbox.returning

def RunBCI():
        global pFeature, pFilter, pSupervisor, pGraphics, selectedSession
        UpdateDemoMode('0')
        print ('l12_support.RunBCI')
        sys.stdout.flush()

        options = {}
        options['defaultextension'] = '.txt'
        options['filetypes'] = [('text files', '.txt')]
        options['initialdir'] = '../data/'
        #options['initialfile'] = 'myfile.txt'
        #options['parent'] = master
        options['title'] = 'Select a session log file'

        #answer = tkMessageBox.askyesno("Filter parameter values",
        #    "Choose your parameter file")
        #answer = mbox('Choose your filter parameter file', ('Most recent', 'r'), ('Other', 'o'))
        answer = Dialog(title   = 'Loading filter parameters...',
             text    = 'Choose your filter parameter file ',
             bitmap  = 'questhead',
             default = 0, strings = ('Most recent', 'Other'))

        if answer.num  == 1:
            filename = tkFileDialog.askopenfilename(**options)

            if filename:
                print "selected file: ", filename
                selectedSession = filename
                #return open(filename, 'r')
            else:
                return
        else:
            selectedSession = ""


        Record('BCI Task')

        if not (pFeature is None):
            pFeature.send_signal(SIGINT)
            pFeature = None

        pFeature = Popen([r'../../feature_extract_cpp/build/feature_extract_cpp'],
                cwd=r'../feature_extraction/feature_extract_cpp/build/')
        time.sleep(0.1)

        if not (pFilter is None):
            pFilter.send_signal(SIGINT)
            pFilter = None

        pFilter = Popen([r'../../cpp/build/filter'],
                cwd=r'../filter/cpp/build/')
        time.sleep(0.1)

        if not (pSupervisor is None):
            pSupervisor.send_signal(SIGINT)
            pSupervisor = None

        pSupervisor = Popen([r'./supervisor.py'],
                cwd=r'../supervisor/elam3/')
        time.sleep(0.1)

        if not (pGraphics is None):
            pGraphics.send_signal(SIGINT)
            pGraphics = None

        pGraphics = Popen([r'../../elam3/build/elam3'],
                cwd=r'../graphics/elam3/build/')
        time.sleep(0.1)


def CalibrateBCI():

        answer = tkMessageBox.askokcancel("Calibrate",
                                          "Please ask the subject to stay still for 30 seconds.")
        if answer is False:
            return

        # show the countdown/stopwatch in a separate process
        pxclock = Popen(['./gstopwatch'],
                cwd=r'../libs/gstopwatch/')

        global pFeature
        UpdateDemoMode('0')
        UpdateBaselineMode('1')
        print ('l12_support.CalibrateBCI')
        sys.stdout.flush()

        if not (pFeature is None):
            pFeature.send_signal(SIGINT)
            pFeature = None

        Record('Calibrate')

        pFeature = Popen([r'../../feature_extract_cpp/build/feature_extract_cpp'],
                cwd=r'../feature_extraction/feature_extract_cpp/build/')
        time.sleep(0.1)
        UpdateBaselineMode('0')

        time.sleep(30)
        pxclock.send_signal(SIGINT)


def StartDemoBCI():
        global pFeature, pFilter, pSupervisor, pGraphics
        UpdateDemoMode('1')
        print ('l12_support.StartDemoBCI')
        sys.stdout.flush()

        Record('Demo BCI')

        if not (pFeature is None):
            pFeature.send_signal(SIGINT)
            pFeature = None

        pFeature = Popen([r'../../feature_extract_cpp/build/feature_extract_cpp'],
                cwd=r'../feature_extraction/feature_extract_cpp/build/')
        time.sleep(0.1)

        if not (pFilter is None):
            pFilter.send_signal(SIGINT)
            pFilter = None

        pFilter = Popen([r'../../cpp/build/filter'],
                cwd=r'../filter/cpp/build/')
        time.sleep(0.1)

        if not (pSupervisor is None):
            pSupervisor.send_signal(SIGINT)
            pSupervisor = None

        pSupervisor = Popen([r'./supervisor.py'],
                cwd=r'../supervisor/elam3/')
        time.sleep(0.1)

        if not (pGraphics is None):
            pGraphics.send_signal(SIGINT)
            pGraphics = None

        pGraphics = Popen([r'../../elam3/build/elam3'],
                cwd=r'../graphics/elam3/build/')
        time.sleep(0.1)

def StopBCITask():
        global pFeature, pFilter, pSupervisor, pGraphics
        print ('l12_support.StopBCITask')
        sys.stdout.flush()

        if not (pGraphics is None):
            pGraphics.send_signal(SIGINT)
            pGraphics = None

        if not (pSupervisor is None):
            pSupervisor.send_signal(SIGINT)
            pSupervisor = None

        if not (pFilter is None):
            pFilter.send_signal(SIGINT)
            pFilter = None

        if not (pFeature is None):
            pFeature.send_signal(SIGINT)
            pFeature = None


def MachineChanged(event):
        print machineBeingUsed.get()
        print ('l12_support.MachineChanged')
        if machineBeingUsed.get() == "gHIamp":
            TComboboxFrq['values'] = ['9600', '4800', '256']
            TComboboxFrq.current(0)
        if machineBeingUsed.get() == "TDT":
            TComboboxFrq['values'] = ['24414.062500']
            TComboboxFrq.current(0)
        if machineBeingUsed.get() == "Imitator":
            TComboboxFrq['values'] = ['-']
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
    filterCfg = ConfigObj('../config/filter.cfg', file_error=True)
    filterSec = filterCfg['filter']
    filterSec['DemoMode'] = state
    filterCfg.write()
    # supervisor
    supervisorCfg = ConfigObj('../config/supervisor_config.cfg', file_error=True)
    supervisorSec = supervisorCfg['supervisor']
    supervisorSec['DemoMode'] = state
    supervisorCfg.write()
    # graphics


def UpdateBaselineMode(state):
    # feature extractor
    featureCfg = ConfigObj('../config/feature_extract_config.cfg', file_error=True)
    featureSec = featureCfg['feature']
    featureSec['baseline'] = state
    featureCfg.write()


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

    logFile.write('Annotation = <START_OF_ANNOTATION ' + annotationBox.get('1.0', 'end')
                     + ' END_OF_ANNOTATION>')

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
    featureSec['samplingRate'] = samplingRate.get()
    featureSec['numChannels'] = "64"
    featureSec['featureChannels'] = (bciMovingaverageXChannels.get() + '*' +
                                     bciMovingaverageYChannels.get() + '*' +
                                     bciMovingaverageZChannels.get())
    featureSec['featureFrequencies'] = (bciMovingaverageXFrequencies.get() + '*' +
                                     bciMovingaverageYFrequencies.get() + '*' +
                                     bciMovingaverageZFrequencies.get())
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
    filterSec['dataPath'] = dataPath
    filterSec['selectedSession'] = selectedSession
    filterCfg.write()

    # update config file of supervisor
    supervisorCfg = ConfigObj('../config/supervisor_config.cfg', file_error=True)
    supervisorSec = supervisorCfg['supervisor']
    supervisorSec['dataPath'] = dataPath
    supervisorCfg.write()

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
    global pSignal, quitLauncher

    print "quit launcher"

    quitLauncher = True

    if not (pSignal is None):
        pSignal.send_signal(SIGINT)
        pSignal = None

    # Function which closes the window.
    global top_level
    top_level.destroy()
    top_level = None