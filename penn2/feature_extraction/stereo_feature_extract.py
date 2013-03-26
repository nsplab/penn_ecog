import zmq
import numpy as np

context = zmq.Context()

subscriber = context.socket(zmq.SUB)
subscriber.connect("ipc:///tmp/signal.pipe")
subscriber.setsockopt(zmq.SUBSCRIBE, '')

context_p = zmq.Context()
socket = context_p.socket(zmq.PUSH)
socket.bind("ipc:///tmp/features.pipe")


first_value = True
prev_vec = None

coefficients = np.array([1.0,2.0,3.0])

while True:
	vec_str = subscriber.recv()
	vec = vec_str.split(",")
	if first_value:
		prev_vec = vec
		first_value = False
		continue
	
	velocity = np.array([0.0,0.0,0.0])
	velocity[0] = float(vec[0]) - float(prev_vec[0])
	velocity[1] = float(vec[1]) - float(prev_vec[1])
	velocity[2] = float(vec[2]) - float(prev_vec[2])
	prev_vec = vec
	#print velocity

	features = np.array([0.0,0.0,0.0])
	features[0] = coefficients[0] * velocity[0]
	features[1] = coefficients[1] * velocity[1]
	features[2] = coefficients[2] * velocity[2]
	print str(features[0])+","+str(features[1])+","+str(features[2])
	socket.send(str(features[0])+","+str(features[1])+","+str(features[2]))
	
	
