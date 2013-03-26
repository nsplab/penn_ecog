import zmq
import numpy as np

context = zmq.Context()

feat_subscriber = context.socket(zmq.SUB)
feat_subscriber.connect("ipc:///tmp/features.pipe")
feat_subscriber.setsockopt(zmq.SUBSCRIBE, '')

sup_subscriber = context.socket(zmq.SUB)
sup_subscriber.connect("ipc:///tmp/supervisor.pipe")
sup_subscriber.setsockopt(zmq.SUBSCRIBE, '')

context_p = zmq.Context()
socket = context_p.socket(zmq.PUSH)
socket.bind("ipc:///tmp/hand_position.pipe")

coefficients = np.array([1.0,0.5,0.333333])
hand_position = np.array([0.0,0.0,0.0])

# main loop

while True:
	'''vec_str = feat_subscriber.recv()
	vec = vec_str.split(",")
	
	velocity = np.array([0.0,0.0,0.0])
	velocity[0] = float(vec[0]) * coefficients[0]
	velocity[1] = float(vec[1]) * coefficients[1]
	velocity[2] = float(vec[2]) * coefficients[2]
	

	hand_position += velocity'''
	print hand_position

	res = sup_subscriber.recv()
	trial_id = int(res.split(',')[0])

	socket.send(str(trial_id)+","+str(hand_position[0])+","+str(hand_position[1])+","+str(hand_position[2]))
	
