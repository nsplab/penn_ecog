#!/usr/bin/env python
import zmq
import numpy as np
import struct

context = zmq.Context()

subscriber = context.socket(zmq.SUB)
subscriber.connect("ipc:///tmp/ksignal.pipe")
subscriber.setsockopt(zmq.SUBSCRIBE, '')

context_p = zmq.Context()
socket = context_p.socket(zmq.PUB)
socket.bind("ipc:///tmp/features.pipe")


coefficients = np.array([1.0 / 640, -1.0 / 480, 1.0 / 400])
old = [640 / 2, 480, 500]

while True:
  vec_str = subscriber.recv()
  vec = [float(x) for x in vec_str.split()]

  vel = [(vec[i] - old[i]) for i in range(len(vec))]
  old = vec

  features = np.array([0.0,0.0,0.0])
  features[0] = coefficients[0] * vel[0]
  features[1] = coefficients[1] * vel[1]
  features[2] = coefficients[2] * vel[2]
  timestamp = 1
  vecsize = "3"
  features_msg = struct.pack("<Q3f", timestamp, features[0], features[1], features[2])

  socket.send(features_msg)

