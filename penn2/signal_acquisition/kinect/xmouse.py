#!/usr/bin/env python
from Xlib import display
import zmq

context = zmq.Context()
socket = context.socket(zmq.PUB)
socket.bind("ipc:///tmp/ksignal.pipe")

def mousepos():
    data = display.Display().screen().root.query_pointer()._data
    return data["root_x"], data["root_y"]

while True:
    x, y = mousepos()
    print("The mouse position on the screen is {0}".format(mousepos()))
    signal_msg = str(x) + " " + str(y) + " 0.0"
    socket.send(signal_msg)

