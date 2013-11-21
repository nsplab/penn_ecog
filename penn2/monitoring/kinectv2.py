#!/usr/bin/env python
# to access kinect
import freenect
# to use opencv
import cv2
# in opencv's python binding the matrices are numpy matrices/arrays
import numpy as np
# to capture the kill signal and terminate the process gracefully
import signal

# create opencv windows to show live video stream
cv2.namedWindow('Video')
print('Press ESC in window to stop')

# retrive the next rgb video frame
def get_video():
    rgb = freenect.sync_get_video()[0]
    bgr = cv2.cvtColor(rgb, cv2.COLOR_BGR2RGB)
    return cv2.flip(bgr, 1)


# run main loop
run = True


def signal_handler(signal, frame):
    global run
    run = False

signal.signal(signal.SIGINT, signal_handler)

# main loop
while run:
    cv2.imshow('Video', get_video())
    if cv2.waitKey(10) == 27:
        run = False

