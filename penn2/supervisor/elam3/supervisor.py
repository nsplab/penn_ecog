#!/usr/bin/env python

# add requests from the filter module to start a new trial
# to save time if the filter is not learning

# to use the zmq library
import zmq

# to capture the kill signal and terminate the process gracefully
import signal

# provides the function to reinitialize the hand and target positions
import setpos

# contains the configuration parameters
import config

import threading

import datetime

import sys

import time

# class used to read the config (ini) files written by the launcher script
from configobj import ConfigObj


# the state machines to present the state of the graphics and filter modules
from state import GameState
from state import FilterState

# ZMQ connections, 3 threads
context = zmq.Context(3)

# socket to publish state to filter modules
ssocket = context.socket(zmq.REP)
ssocket.bind("ipc:///tmp/supervisor.pipe")

# socket to publish state to graphics modules
gsocket = context.socket(zmq.REP)
#gsocket.setsockopt(zmq.HWM, 1)
gsocket.bind("ipc:///tmp/graphics.pipe")

# parameters
run = True  # keep main loop iterating
init_obj_pos = True  # initialize positions of ball and box
start_trial = False  # if trial has started
goal_is_ball = True  # ball or box is goal

if (len(sys.argv) > 1):
    config.blockWidth = float(sys.argv[1]) * config.workspaceRadius

if (len(sys.argv) > 2):
    config.blockLengthTime = float(sys.argv[2])


# read the config parameters that the launcher script has written
# and use the relevant ones for the supervisor module
lastLog = ConfigObj('../../data/log.txt', file_error=True)
secLog = lastLog['ExperimentLog']

config.blockWidthPercent = float(secLog['BarWidth'])
config.blockWidth = config.workspaceRadius * float(config.blockWidthPercent) / 100.0
config.blockLengthTime = float(secLog['BarLength'])

# create state objects
gameState = GameState(config.workspaceRadius, config.blockWidth, config.blockLengthTime)
gameState.generateBlocks()
filterState = FilterState()



# capture the kill signal and terminate the process
def signal_handler(signal, frame):
    global run
    run = False

# register the signal handler
signal.signal(signal.SIGINT, signal_handler)

trialTimeoutThread = None


trialTimeout = 8  # seconds
if config.trackingMode:
    trialTimeout = 100


def StartNewTrial():
    global goal_is_ball, trialTimeoutThread, run
    global gameState, filterState
    print 'should start a new trial ***********************************'
    trialTimeoutThread.cancel()
    #goal_is_ball = not goal_is_ball
    gameState.box_pos = \
                         setpos.SetInitialPositions(gameState.hand_pos)
    filterState.trial += 1
    trialTimeoutThread = threading.Timer(trialTimeout, StartNewTrial)
    if run:
        trialTimeoutThread.start()

timestamp = 0

# create the data log file to store the parameters of the game and the score
filename = 'supervisor_event_%s.txt' % datetime.datetime.utcnow().strftime("%Y-%m-%d-%H%M%S")
f = open(filename, 'w')

f.write("timestamp")
f.write(' ')

f.write("epoch_seconds")
f.write(' ')

f.write("trial_number")
f.write(' ')
f.write("target_pos_x")
f.write(' ')
f.write("target_pos_y")
f.write(' ')
f.write("target_pos_z")
f.write(' ')
f.write("hand_pos_x")
f.write(' ')
f.write("hand_pos_y")
f.write(' ')
f.write("hand_pos_z")
f.write(' ')

f.write("score_total")
f.write(' ')

f.write("score_per_second")
f.write(' ')

f.write("block_width_percent")
f.write(' ')

f.write("block_length_time")
f.write(' ')

f.write("workspace_width")
f.write(' ')

f.write("jumpe_to_start")
f.write(' ')

f.write("score_distance_threshold")
f.write(' ')

f.write("number_of_features")
f.write(' ')

f.write("number_of_parameters")
f.write(' ')

f.write("features")
f.write(' ')

f.write("parameters")
f.write(' ')
f.write('\n')

scorePerMin = 0

# main loop
while run:

# put ball and box in random positions
    if init_obj_pos:
        gameState.box_pos = \
                             setpos.SetInitialPositions(gameState.hand_pos)
        init_obj_pos = False
        #trialTimeoutThread = threading.Timer(trialTimeout, StartNewTrial)
        #trialTimeoutThread.start()

    #print 'filterState.serialize() ', filterState.serialize()

    if gameState.updateState():
        # a new trial started
        gameState.box_pos = \
                            setpos.SetInitialPositions(gameState.hand_pos)
        filterState.trial += 1

    # broadcast state
    filterState.target_pos = gameState.box_pos

    print 'recv gsocket'
    gvec = gsocket.recv()
    print 'recvd: ', gvec
    if gvec[0] == "p":
        gameState.pause = True
    elif gvec[0] == "c":
        gameState.pause = False

    #gsocket.send(gameState.serialize())
    print 'sent'
    sb = gameState.serializeBlocks() + gameState.serialize()
    print 'sb: ', sb
    gsocket.send(sb)

    if gameState.scorePlot > -0.5:
        scorePerMin = gameState.scorePlot

    numFeatures = 0
    numParameters = 0
    vec = None
    if not gameState.pause:
        vec_str = ssocket.recv()
        print 'vec_str ', vec_str
        vec = vec_str.split(" ")
        if vec[0] != "pass":
            if config.trackingMode:
                timestamp = long(vec[0])
                gameState.hand_pos[0] = float(vec[1])
                if config.dimension > 1:
                    gameState.hand_pos[1] = float(vec[2])
                    #gameState.hand_pos[2] = float(vec[3])
            else:
                timestamp = long(vec[0])
                gameState.hand_pos[0] += float(vec[1])
                gameState.hand_pos[1] += float(vec[2])
                gameState.hand_pos[2] += float(vec[3])
                if gameState.hand_pos[0] > (config.workspaceRadius / 2):
                    gameState.hand_pos[0] = config.workspaceRadius / 2
                elif gameState.hand_pos[0] < -(config.workspaceRadius / 2):
                    gameState.hand_pos[0] = -config.workspaceRadius / 2
                numFeatures = int(vec[4])
                numParameters = int(vec[5])
                #print "numFeatures",numFeatures
                #print "numParameters", numParameters
                print timestamp

    filterState.hand_pos[0] = gameState.hand_pos[0]
    filterState.hand_pos[1] = gameState.hand_pos[1]
    filterState.hand_pos[2] = gameState.hand_pos[2]

    if not gameState.pause:
        ssocket.send(filterState.serialize())

    print 'filterState.serialize() ', filterState.serialize()

    print 'filterState.trial ', filterState.trial

    # if the game is paused do not write the log file
    if gameState.pause:
        continue
# 1
    f.write(str(timestamp))
    f.write(' ')

# 2
    f.write(str(time.time()))
    f.write(' ')

# 3
    f.write(str(filterState.trial))
    f.write(' ')
    f.write(str(gameState.box_pos[0]))
    f.write(' ')
    f.write(str(gameState.box_pos[1]))
    f.write(' ')
    f.write(str(gameState.box_pos[2]))
    f.write(' ')
    f.write(str(gameState.hand_pos[0]))
    f.write(' ')
    f.write(str(gameState.hand_pos[1]))
    f.write(' ')
    f.write(str(gameState.hand_pos[2]))
    f.write(' ')

# 10
    f.write(str(gameState.score))
    f.write(' ')

# 11
    f.write(str(scorePerMin))
    f.write(' ')

# 12
    f.write(str(config.blockWidthPercent))
    f.write(' ')

# 13
    f.write(str(config.blockLengthTime))
    f.write(' ')

# 14
    f.write(str(config.workspaceRadius))
    f.write(' ')

# 15
    if config.jumpetoStart is False:
        f.write("0")
    else:
        f.write("1")
    f.write(' ')

# 16
    f.write(str(config.scoreDistanceThreshold))
    f.write(' ')

# 17
    f.write(str(numFeatures))
    f.write(' ')

# 18
    f.write(str(numParameters))
    f.write(' ')

# 19
    for nF in range(6, numFeatures + 6):
        f.write(vec[nF])
        f.write(' ')
    for nP in range(numFeatures + 7, numParameters + numFeatures + 7):
        f.write(vec[nP])
        f.write(' ')

    f.write('\n')

    #gameState.hand_pos[0] = (float(vec[0]) - 320) / 26.0
    #gameState.hand_pos[1] = -(float(vec[1]) - 240) / 26.0
    #gameState.hand_pos[2] = -(float(vec[1]) - 12) / 3.0

trialTimeoutThread.cancel()
f.close()
