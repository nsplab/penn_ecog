
import zmq
import random
import numpy as np

context = zmq.Context()
socket = context.socket(zmq.PUB)

socket.connect("ipc:///tmp/supervisor.pipe")

run = True
init_obj_pos = True
start_session = False

ball_pos = np.array([0.0,0.0,0.0])
box_pos = np.array([0.0,0.0,0.0])

while run:
	if not start_session:
		continue

	if init_obj_pos:
		# magic numbers keep the ball and box within reachable distance
		ball_pos[0] = random.random() * -5.0
		ball_pos[1] = random.random() * -2.0 - 4.0
		ball_pos[2] = random.random() * -4.0 + 2.0

		box_pos[0] = random.random() * -5.0
		box_pos[1] = random.random() * -2.0 - 4.0
		box_pos[2] = random.random() * -4.0 + 2.0

		dist = np.sqrt(ball_pos.dot(box_pos))

		while dist < 2.0:
			box_pos[0] = random.random() * -5.0
			box_pos[1] = random.random() * -2.0 - 4.0
			box_pos[2] = random.random() * -4.0 + 2.0
			dist = np.sqrt(ball_pos.dot(box_pos))
			
		socket.send("ball:"+str(ball_pos))
		socket.send("box:"+str(box_pos))
		init_obj_pos = false
		



