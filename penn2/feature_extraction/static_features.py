import zmq
import numpy as np

context = zmq.Context()

context_p = zmq.Context()
socket = context_p.socket(zmq.PUSH)
socket.bind("ipc:///tmp/features.pipe")

while True:
	socket.send("1,3,0.0,1.0,2.0")
