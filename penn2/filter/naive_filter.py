import zmq
import numpy as np

context = zmq.Context()

subscriber = context.socket(zmq.SUB)
subscriber.bind("ipc:///tmp/features.pipe")
subscriber.setsockopt(zmq.SUBSCRIBE, '')

context_p = zmq.Context()
socket = context_p.socket(zmq.PUSH)
socket.connect("ipc:///tmp/hand_position.pipe")

coefficients = np.array([1.0,0.5,0.333333])
hand_position = np.array([0.0,0.0,0.0])

while True:
	vec_str = subscriber.recv()
	vec = vec_str.split(",")
	
	velocity = np.array([0.0,0.0,0.0])
	velocity[0] = float(vec[0]) * coefficients[0]
	velocity[1] = float(vec[1]) * coefficients[1]
	velocity[2] = float(vec[2]) * coefficients[2]
	

	hand_position += velocity
	print hand_position

	socket.send(str(hand_position[0])+","+str(hand_position[1])+","+str(hand_position[2]))
	
