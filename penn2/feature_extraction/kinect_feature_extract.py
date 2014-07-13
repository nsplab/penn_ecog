#!/usr/bin/env python
import zmq
import numpy as np

context = zmq.Context()

subscriber = context.socket(zmq.SUB)
subscriber.connect("ipc:///tmp/signal.pipe")
subscriber.setsockopt(zmq.SUBSCRIBE, '')

context_p = zmq.Context()
socket = context_p.socket(zmq.PUB)
socket.bind("ipc:///tmp/features.pipe")


coefficients = np.array([1.0,1.0,1.0])

while True:
  vec_str = subscriber.recv()
  vec = vec_str.split(",")

  features = np.array([0.0,0.0,0.0])
  features[0] = coefficients[0] * vec[0]
  features[1] = coefficients[1] * vec[1]
  features[2] = coefficients[2] * vec[2]
  timestamp = "1"
  vecsize = "3"
  features_msg = timestamp+","+vecsize+","+str(features[0])+","+str(features[1])+","+str(features[2])
  socket.send(features_msg)

