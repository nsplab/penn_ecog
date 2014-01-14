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

# the state machines to present the state of the graphics and filter modules
from state import GameState
from state import FilterState

# ZMQ connections, 3 threads
context = zmq.Context(3)

# socket to publish state to filter modules
ssocket = context.socket(zmq.REP)
ssocket.bind("ipc:///tmp/supervisor.pipe")

# socket to publish state to graphics modules
gsocket = context.socket(zmq.PUB)
#gsocket.setsockopt(zmq.HWM, 1)
gsocket.bind("ipc:///tmp/graphics.pipe")

# parameters
run = True  # keep main loop iterating
init_obj_pos = True  # initialize positions of ball and box
start_trial = False  # if trial has started
goal_is_ball = True  # ball or box is goal

# create state objects
gameState = GameState()
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
    gameState.ball_pos, gameState.box_pos = \
                                 setpos.SetInitialPositions(gameState.hand_pos)
    filterState.trial += 1
    trialTimeoutThread = threading.Timer(trialTimeout, StartNewTrial)
    if run:
        trialTimeoutThread.start()

timestamp = 0

filename = 'supervisor_event_%s.txt' % datetime.datetime.utcnow().strftime("%Y-%m-%d-%H%M%S")
f = open(filename, 'w')

# main loop
while run:

# put ball and box in random positions
    if init_obj_pos:
        gameState.ball_pos, gameState.box_pos = \
                                 setpos.SetInitialPositions(gameState.hand_pos)
        init_obj_pos = False
        trialTimeoutThread = threading.Timer(trialTimeout, StartNewTrial)
        trialTimeoutThread.start()

    #print 'filterState.serialize() ', filterState.serialize()
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
            print timestamp
        else:
            timestamp = long(vec[0])
            #gameState.hand_pos[2] += float(vec[1])
            filterState.hand_pos[0] = float(vec[1])
            filterState.hand_pos[1] = float(vec[2])
            filterState.hand_pos[2] = float(vec[3])

    if gameState.updateState():
        # a new trial started
        gameState.ball_pos, gameState.box_pos = \
                                 setpos.SetInitialPositions(gameState.hand_pos)
        filterState.trial += 1
        goal_is_ball = True
        trialTimeoutThread.cancel()
        trialTimeoutThread = threading.Timer(trialTimeout, StartNewTrial)
        trialTimeoutThread.start()

    # if a new trial started for filter
    if gameState.pickedBall:
        filterState.trial += 1
        gameState.pickedBall = False
        goal_is_ball = False
        print 'issue new trial'
        trialTimeoutThread.cancel()
        trialTimeoutThread = threading.Timer(trialTimeout, StartNewTrial)
        trialTimeoutThread.start()

    # broadcast state
    if goal_is_ball:
        filterState.target_pos = gameState.ball_pos
    else:
        filterState.target_pos = gameState.box_pos

    gsocket.send(gameState.serialize())

    ssocket.send(filterState.serialize())

    print 'filterState.serialize() ', filterState.serialize()

    print 'filterState.trial ', filterState.trial

    f.write(str(timestamp))
    f.write(' ')
    f.write(str(filterState.trial))
    f.write(' ')
    f.write(str(gameState.pickedBall))
    f.write(' ')
    f.write(str(gameState.box_pos))
    f.write(' ')
    f.write(str(gameState.ball_pos))
    f.write('\n')

    #gameState.hand_pos[0] = (float(vec[0]) - 320) / 26.0
    #gameState.hand_pos[1] = -(float(vec[1]) - 240) / 26.0
    #gameState.hand_pos[2] = -(float(vec[1]) - 12) / 3.0

trialTimeoutThread.cancel()
f.close()
