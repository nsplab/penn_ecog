import zmq
import numpy as np

import setpos

# ZMQ connections, 3 threads
context = zmq.Context(3)

# socket to publish state to filter modules
socket = context.socket(zmq.PUB)
socket.bind("ipc:///tmp/supervisor.pipe")

# socket to publish state to graphics modules
gsocket = context.socket(zmq.PUB)
gsocket.bind("ipc:///tmp/graphics.pipe")

# socket to receive estimated hand movement from filter
hsocket = context.socket(zmq.SUB)
hsocket.connect ("ipc:///tmp/hand_position.pipe")
hsocket.setsockopt(zmq.SUBSCRIBE, '' ) # subscribe with no filter

# parameters
run = True # keep main loop iterating
init_obj_pos = True # initialize positions of ball and box
start_trial = False # if trial has started
goal_is_ball = True # ball or box is goal
trial_id = 0 

# 0: training, 1: test
triaining_test_seq = np.array([0,0,0,1,1])

ball_pos = np.array([0.0,0.0,0.0])
box_pos = np.array([0.0,0.0,0.0])
hand_pos = np.array([-6.0,0.0,0.0])

while run:

	# put ball and box in random positions 
	if init_obj_pos:
		ball_pos,box_pos = setpos.SetBallBoxPos(hand_pos)
		init_obj_pos = False

	# broadcast state
	if goal_is_ball:
		goal_pos = ball_pos
	else :
		goal_pos = box_pos
	goal_pos_str = np.char.mod('%f', goal_pos)
	socket.send(str(trial_id)+","+str(",".join(goal_pos_str))+","+str(triaining_test_seq[trial_id % len(triaining_test_seq)])+","+str(start_trial))
	trial_id += 1
		
	try:
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
	graphics_msg=str(",".join(ball_pos_str))+","+str(",".join(box_pos_str))+","+str(",".join(hand_pos_str))
	print graphics_msg
	gsocket.send(graphics_msg)
	
