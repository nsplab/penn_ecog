import zmq
import numpy as np
from visual import *

context = zmq.Context()
socket = context.socket(zmq.SUB)
# format of messages: ball_x,ball_y,ball_z,box_x,box_y,box_z,hand_x,hand_y,hand_z
socket.connect("ipc:///tmp/graphics.pipe")
socket.setsockopt(zmq.SUBSCRIBE, '' )

scene2 = display(title='PnP Graphics',width=1200, height=900)
scene2.select()

ball = sphere(pos=(-5,0,0), radius=0.5, color=color.cyan, opacity=0.9)
box =  box(pos=(6,0,0), size=(1.0,1.0,1.0), color=color.green, opacity=0.6)
pointer = arrow(pos=(0,2,1), axis=(0,2,0), shaftwidth=0.5)

while True:
	#rate(50)
	res = socket.recv()
	#print "a ",res
	if res != zmq.EAGAIN:
		objs_pos = np.array([float(x) for x in res.split(',')])
		ball.pos = vector(objs_pos[0],objs_pos[1],objs_pos[2])
		box.pos  = vector(objs_pos[3],objs_pos[4],objs_pos[5])
		pointer.pos = vector(objs_pos[6]/-20.0,objs_pos[7]/-20.0,objs_pos[8]/2.0)
		print pointer.pos
			
