#!/usr/bin/env python
# to access kinect
import freenect
# to use opencv
import cv2
# in opencv's python binding the matrices are numpy matrices/arrays
import numpy as np
# to send hand movement to the kinectpy2c module
import zmq
# to capture the kill signal and terminate the process gracefully
import signal


# create the zmq socket for sending data
context = zmq.Context()
socket = context.socket(zmq.PUB)
socket.bind("ipc:///tmp/ksignal.pipe")

# create opencv windows to show live video stream
cv2.namedWindow('Depth')
cv2.namedWindow('Video')
print('Press ESC in window to stop')

# previous hand position
prev_point = np.array([0, 0, 0])
first_iter = True

# retrive the next rgb video frame
def get_video():
    rgb = freenect.sync_get_video()[0]
    bgr = cv2.cvtColor(rgb, cv2.COLOR_BGR2RGB)
    return bgr

# Parameters for lucas kanade optical flow
lk_params = dict(winSize=(45, 45),
                  maxLevel=5,
                  criteria=(cv2.TERM_CRITERIA_EPS | cv2.TERM_CRITERIA_COUNT,
                               10, 0.03))

# used for optical flow
oldpoint = 0
prev = 0
old_gray = 0

# retrive the next depth video frame
def get_depth_raw():
    global prev, oldpoint, old_gray
    # get the depth frame
    img = freenect.sync_get_depth()[0]

    # backplane, bring very far pixels to backplane at 2000
    smallDiffs = (img > 2000)
    img[smallDiffs] = 2000
    # stretch values to two-byte values
    img = (img / 2000.0) * 65535

    # get difference between current frame and background
    diffImg = img.copy()
    diffImg = (backDepth - img.astype(np.int16))
    # if depth is less than background plus a threshold,
    # consider pixel as background
    background = diffImg <= 200.0
    timg = img.astype(np.uint16)
    # background pixels are white
    timg[background] = 65535

    #
    perc = np.percentile(timg, 0.2)
    near_indices = timg <= perc
    x = np.median(near_indices[1])
    y = np.median(near_indices[0])
    range_x = np.max(near_indices[0]) - np.min(near_indices[0])
    range_y = np.max(near_indices[1]) - np.min(near_indices[1])

    if prev == 1:
        ni = timg.copy()
        ni /= 256
        nni = ni.astype(np.uint8)
        oi = old_gray.copy()
        oi /= 256
        ooi = oi.astype(np.uint8)
        p1, st, err = cv2.calcOpticalFlowPyrLK(ooi, nni, oldpoint, None, **lk_params)
        old_gray = timg.copy()
        oldpoint = p1
        cv2.circle(timg, (int(p1[0][0][0]), int(p1[0][0][1])), 15, cv2.cv.RGB(0, 0, 0),
                 7, cv2.CV_AA)
        cv2.circle(timg, (int(p1[0][0][0]), int(p1[0][0][1])), 15, cv2.cv.RGB(65535, 65535, 65535),
                 3, cv2.CV_AA)

    if range_x ** 2 + range_y ** 2 < 7200:
        if prev == 0:
            oldpoint = np.array([[[x, y]]], dtype=np.float32)
            prev = 1
            old_gray = timg.copy()

    if range_x ** 2 + range_y ** 2 < 7200:
        cv2.circle(timg, (int(x), int(y)), 10, cv2.cv.RGB(0, 0, 0),
                 8, cv2.CV_AA)
        cv2.circle(timg, (int(x), int(y)), 10, cv2.cv.RGB(65535, 65535, 65535),
                 5, cv2.CV_AA)

    return timg

# get depth background image, 3 times in case of bad/slow initialization
tmpDepth = freenect.sync_get_depth(format=freenect.DEPTH_11BIT)[0]
tmpDepth = freenect.sync_get_depth(format=freenect.DEPTH_11BIT)[0]
tmpDepth = freenect.sync_get_depth(format=freenect.DEPTH_11BIT)[0]
# convert to two-byte image
backDepth = tmpDepth.copy().astype(np.int16)
smallDiffs = (backDepth > 2000)
backDepth[smallDiffs] = 2000  # 65535
backDepth = (backDepth / 2000.0) * 65535

# run main loop
run = True


def signal_handler(signal, frame):
    global run
    run = False

signal.signal(signal.SIGINT, signal_handler)

# main loop
while run:
    cv2.imshow('Video', get_video())
    cv2.imshow('Raw Depth', get_depth_raw())
    if cv2.waitKey(10) == 27:
        run = False

