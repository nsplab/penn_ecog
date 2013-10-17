#!/usr/bin/env python
import freenect
import cv2
import numpy as np
import zmq


context = zmq.Context()
socket = context.socket(zmq.PUB)
socket.bind("ipc:///tmp/ksignal.pipe")

cv2.namedWindow('Depth')
cv2.namedWindow('Video')
print('Press ESC in window to stop')

prev_point = np.array([0, 0, 0])
first_iter = True

def reject_outliers(data, m=3):
    return data[abs(data - np.mean(data)) < m * np.std(data)]

def get_video():
    rgb = freenect.sync_get_video()[0]
    bgr = cv2.cvtColor(rgb,cv2.COLOR_BGR2RGB)
    return bgr

def get_depth():
    global socket
    global first_iter
    global prev_point
    img = freenect.sync_get_depth()[0]
    dep = np.empty_like(img)
    dep[:] = img
    dep = dep.astype(np.uint8)
    per5 = np.percentile(img,0.1)
    near_indices = img <= per5
    far_indices = img > per5
    dep[near_indices] = 255
    dep[far_indices] = 0
    nindices = np.nonzero(dep)
    z = np.median(img[nindices])
    z /= 25.0;
    dep_rgb = cv2.cvtColor(dep, cv2.COLOR_GRAY2BGR)
    firstDim = reject_outliers(nindices[1])
    seconDim = reject_outliers(nindices[0])
    cv2.circle(dep_rgb, (int(np.median(firstDim)), int(np.median(seconDim))), int(z+1), cv2.cv.RGB(z*5.0, z*5.0, 255), 5, cv2.CV_AA)
    if first_iter:
       first_iter = False
       prev_point = np.array([np.average(nindices[1]), np.average(nindices[0]), z])
    point = np.array([np.average(nindices[1]), np.average(nindices[0]), z])
    diff = prev_point - point
    prev_point = point
    #signal_msg = str(diff[0])+","+str(diff[1])+","+str(diff[2])
    signal_msg = str(point[0])+" "+str(point[1])+" "+str(point[2])
    print signal_msg
    socket.send(signal_msg)
    return cv2.flip(dep_rgb, 1)

while 1:
    cv2.imshow('Depth', get_depth())
    cv2.imshow('Video', get_video())
    if cv2.waitKey(10) == 27:
        break

