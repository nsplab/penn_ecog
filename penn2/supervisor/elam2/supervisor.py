import zmq
import numpy as np
import signal

import setpos
import config

from state import GameState
from state import FilterState

# ZMQ connections, 3 threads
context = zmq.Context(3)

# socket to publish state to filter modules
ssocket = context.socket(zmq.PUB)
ssocket.bind("ipc:///tmp/supervisor.pipe")

# socket to publish state to graphics modules
gsocket = context.socket(zmq.PUB)
gsocket.setsockopt(zmq.HWM, 1)
gsocket.bind("ipc:///tmp/graphics.pipe")

# socket to receive estimated hand movement from filter
hsocket = context.socket(zmq.SUB)
#hsocket.connect("ipc:///tmp/hand_position.pipe")
hsocket.connect("ipc:///tmp/ksignal.pipe")
hsocket.setsockopt(zmq.SUBSCRIBE, '')  # subscribe with no filter

# parameters
run = True  # keep main loop iterating
init_obj_pos = True  # initialize positions of ball and box
start_trial = False  # if trial has started
goal_is_ball = True  # ball or box is goal

gameState = GameState()
filterState = FilterState()


def signal_handler(signal, frame):
    global run
    run = False

signal.signal(signal.SIGINT, signal_handler)

# main loop
while run:

# put ball and box in random positions
    if init_obj_pos:
        gameState.ball_pos, gameState.box_pos = \
                                 setpos.SetInitialPositions(gameState.hand_pos)
        init_obj_pos = False

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
    gsocket.send(gameState.serialize())
    ssocket.send(filterState.serialize())

    vec_str = hsocket.recv()
    vec = vec_str.split(" ")

    gameState.hand_pos[0] = (float(vec[0]) - 320) / 26.0
    gameState.hand_pos[1] = -(float(vec[1]) - 240) / 26.0
    #gameState.hand_pos[2] = -(float(vec[1]) - 12) / 3.0


'''    try:
        res = hsocket.recv(zmq.NOBLOCK)
        #print "res: "+res[:-2]
        # if received data
        if res != zmq.EAGAIN:
            #print res
            # convert string array to numpy array
            hand_mov = np.array([float(x) for x in res[:-2].split(',')])
            hand_pos[0] = hand_mov[0]
            hand_pos[1] = hand_mov[1]
            hand_pos[2] = hand_mov[2]
            #print hand_mov
    except:
        pass

    ball_pos_str = np.char.mod('%f', ball_pos)
    box_pos_str = np.char.mod('%f', box_pos)
    hand_pos_str = np.char.mod('%f', hand_pos)
    graphics_msg = str(",".join(ball_pos_str)) + ","
    graphics_msg += str(",".join(box_pos_str)) + ","
    graphics_msg += str(",".join(hand_pos_str))
    print(graphics_msg)
    gsocket.send(graphics_msg)
'''
