#!/usr/bin/env python
# to use the zmq library
import zmq

# to capture the kill signal and terminate the process gracefully
import signal

# provides th function to reinitialize the hand and target positions
import setpos

# contains the configuration parameters
import config

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
gsocket.setsockopt(zmq.HWM, 1)
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

# main loop
while run:

# put ball and box in random positions
    if init_obj_pos:
        gameState.ball_pos, gameState.box_pos = \
                                 setpos.SetInitialPositions(gameState.hand_pos)
        init_obj_pos = False

    #print 'filterState.serialize() ', filterState.serialize()
    vec_str = ssocket.recv()
    print 'vec_str ', vec_str
    vec = vec_str.split(" ")
    gameState.hand_pos[0] = -float(vec[0])
    gameState.hand_pos[1] = -float(vec[1])
    #gameState.hand_pos[2] += float(vec[1])

    # broadcast state
    if goal_is_ball:
        filterState.target_pos = gameState.ball_pos
    else:
        filterState.target_pos = gameState.box_pos

    if gameState.updateState():
        # a new trial started
        gameState.ball_pos, gameState.box_pos = \
                                 setpos.SetInitialPositions(gameState.hand_pos)
        filterState.trial += 1

    # if a new trial started for filter
    if gameState.pickedBall:
        filterState.trial += 1
        gameState.pickedBall = False

    gsocket.send(gameState.serialize())

    ssocket.send(filterState.serialize())

    #gameState.hand_pos[0] = (float(vec[0]) - 320) / 26.0
    #gameState.hand_pos[1] = -(float(vec[1]) - 240) / 26.0
    #gameState.hand_pos[2] = -(float(vec[1]) - 12) / 3.0
