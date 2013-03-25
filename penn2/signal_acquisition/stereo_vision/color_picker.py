#!/usr/bin/python

import numpy as np
import time
import cv2
import os


h = 480
w = 640
padding = 4

os.system("uvcdynctrl -d video1 -s 'Auto Exposure' 0")
os.system("uvcdynctrl -d video0 -s 'Exposure' 11")
os.system("uvcdynctrl -d video1 -s 'Exposure' 11")
os.system("uvcdynctrl -d video0 -s 'Auto Gain' 0")
os.system("uvcdynctrl -d video1 -s 'Auto Gain' 0")
os.system("uvcdynctrl -d video0 -s 'Main Gain' 11")
os.system("uvcdynctrl -d video1 -s 'Main Gain' 11")


cam_left = cv2.VideoCapture(0)
cam_left.set(cv2.cv.CV_CAP_PROP_FRAME_WIDTH, w)
cam_left.set(cv2.cv.CV_CAP_PROP_FRAME_HEIGHT, h)

cam_right = cv2.VideoCapture(1)
cam_right.set(cv2.cv.CV_CAP_PROP_FRAME_WIDTH, w)
cam_right.set(cv2.cv.CV_CAP_PROP_FRAME_HEIGHT, h)

print "hit Esc to exit \nhit p to un/pause"

def onmouse(event, x, y, flags, param):
	if flags & cv2.EVENT_FLAG_LBUTTON:
		global vis, initialized, lower_color, upper_color	
		vis_hsv = cv2.cvtColor(vis, cv2.COLOR_BGR2HSV)
		if not initialized:
			lower_color = vis_hsv[y,x].copy()
			upper_color = vis_hsv[y,x].copy()
			for i in range(0,3):
				print i
				if lower_color[i] > padding:
					print "min"
					lower_color[i] -= padding
				diff = 255 - upper_color[i]
				if diff > padding:
					upper_color[i] += padding
				else:
					upper_color[i] = 255
					print i,"255"
			initialized = True
		else :
			for i in range(0,3):
				tmp_lower = vis_hsv[y,x][i] - padding
				tmp_upper = vis_hsv[y,x][i] + padding
				if tmp_lower < 0:
					tmp_lower = 0
				if tmp_upper > 255:
					tmp_upper = 255
				
				if lower_color[i] > tmp_lower :
					lower_color[i] = tmp_lower
				if upper_color[i] < tmp_upper :
					upper_color[i] = tmp_upper
		
		print "lower: ",lower_color
		print "upper: ",upper_color

vis = np.zeros((h*2+5, w*2+5), np.uint8)
vis = cv2.cvtColor(vis, cv2.COLOR_GRAY2BGR)
cv2.imshow("Color Picker", vis)

cv2.setMouseCallback('Color Picker', onmouse)

lower_color = np.zeros(3, np.uint8)
upper_color = np.zeros(3, np.uint8)
initialized = False
lower_color[0] = 4
lower_color[1] = 68
lower_color[2] = 120
upper_color[0] = 34
upper_color[0] = 255
upper_color[0] = 255

detector = cv2.FeatureDetector_create('MSER')
d_red = cv2.cv.RGB(150,55,65)

pause = False
while (cam_left.grab() and cam_right.grab()):
	flag_left, frame_left = cam_left.retrieve()
	flag_right, frame_right = cam_right.retrieve()
	if (flag_left and flag_right) == False:
		break
	
	if not pause:
		vis[:h, :w] = frame_left
		vis[:h, w+5:w*2+5] = frame_right
		filtered = vis[:h, :w*2+5]
		filtered = cv2.cvtColor(filtered, cv2.COLOR_BGR2HSV)
		filtered = cv2.inRange(filtered, lower_color, upper_color)
		filtered = cv2.cvtColor(filtered, cv2.COLOR_GRAY2BGR)
		fs = detector.detect(filtered)
		for f in fs:
			cv2.circle(filtered, (int(f.pt[0]), int(f.pt[1])), int(f.size/2), d_red, 2, cv2.CV_AA)
			#print int(f.pt[0])
		vis[h+5:h*2+5, :w*2+5] = filtered

	cv2.imshow("Color Picker", vis)
	key = cv2.waitKey(1)
	if key == 27:
		break
	if key == ord('p'):
		pause = not pause
	if key == ord('r'):
		lower_color = np.zeros(3, np.uint8)
		upper_color = np.zeros(3, np.uint8)


