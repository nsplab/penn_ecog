import cv2
import numpy as np
import threading, os, sys
import select,time
import math
import sqlite3
import zmq

context = zmq.Context()
socket = context.socket(zmq.PUSH)
socket.connect("ipc:///tmp/signal.pipe")

#os.system('echo "0" > /dev/ttyACM0')

os.system("uvcdynctrl -d video0 -s 'Auto Exposure' 0")
os.system("uvcdynctrl -d video1 -s 'Auto Exposure' 0")
os.system("uvcdynctrl -d video0 -s 'Exposure' 11")
os.system("uvcdynctrl -d video1 -s 'Exposure' 11")
os.system("uvcdynctrl -d video0 -s 'Auto Gain' 0")
os.system("uvcdynctrl -d video1 -s 'Auto Gain' 0")
os.system("uvcdynctrl -d video0 -s 'Main Gain' 11")
os.system("uvcdynctrl -d video1 -s 'Main Gain' 11")


'''folder_name = 'asy_cam_'+str(int(time.time()*1000000.0))
os.makedirs(folder_name)
accel_db= folder_name+"/cam.db"
connection=sqlite3.connect(accel_db)
connection.execute("PRAGMA journal_mode = WAL")
connection.execute("PRAGMA synchronous = OFF")
connection.execute("PRAGMA count_changes = OFF")
connection.execute("PRAGMA journal_mode = MEMORY")
connection.execute("PRAGMA temp_store = MEMORY")
cursor=connection.cursor()
'''
create_str = "CREATE TABLE cam (time INTEGER PRIMARY KEY"
for i in range(1, 4):
        create_str += ", c"+str(i)+" REAL"
create_str += ", valid INTEGER);"
#cursor.execute(create_str)
print create_str


def heardEnter():
    i,o,e = select.select([sys.stdin],[],[],0.00001)
    for s in i:
        return False
#        if s == sys.stdin:
#            input = sys.stdin.readline()
    return True


class ThreadBGR2HSV(threading.Thread):
  def __init__(self, frame):
    self.hsv_frame = 0
    self.frame = frame
    threading.Thread.__init__(self)

  def GetFrame(self):
    return self.hsv_frame

  def run(self):
    self.hsv_frame = cv2.cvtColor(self.frame, cv2.COLOR_BGR2HSV)

# ****************************** OPTIONS ******************************

num_cameras = 2 # Number of cameras

h = 480 # Height of the image
w = 640 # Width of the image

square_size = 4.7 # The size of each repeat of the pattern in cm
patternSize = (4, 11) # The number of times the pattern is repeated
# NOTE: For the asymmetric circles pattern, the order that the values
#       appear in matters. Swap the numbers if it is not detected.
gridType = "asymmetric circle" # The type of grid used
                               # Options:
                               #     - "chessboard"
                               #     - "asymmetric circle"

# *********************************************************************

# Calculate the X, Y, Z coordinates of a point given the pixels it appears in,
# the matrix of intrinsic parameters, and rotation and translation matrices.
def triangulate(pixel1, pixel2, C1, C2, R, T):
  
  # Focal point of the first camera is the origin
  n1 = np.zeros((3, 1), np.float64)
  # Focal point of the second camera is determined by the translation
  n2 = T

  #print '1: ', n1
  #print '2: ', n2

  # Convert to homogeneous coordinates
  '''y1 = np.ones((3, 1), np.float64)
  y1[:2] = pixel1[:2]
  y2 = np.ones((3, 1), np.float64)
  y2[:2] = pixel2[:2]
  '''

  # First camera has no rotation or translation
  RT1 = np.zeros((3, 4), np.float64)
  for i in range(3):
    RT1[i][i] = 1
  # Second camera has rotation R and translation T
  RT2 = np.zeros((3, 4), np.float64)
  for i in range(3):
    for j in range(3):
      RT2[i][j] = R[i][j]
    RT2[i][3] = T[i]

  # pixel1 = A * z1
  A = C1.dot(RT1)
  # pixel2 = B * z2
  B = C2.dot(RT2)

  l_p = np.array([[pixel1[0]], [pixel1[1]]], np.float32)
  r_p = np.array([[pixel2[0]], [pixel2[1]]], np.float32)
  p4d = cv2.triangulatePoints(A, B, l_p, r_p)
  #print 'p4d ', p4d
  p3d = cv2.convertPointsFromHomogeneous(p4d.reshape(-1,1,4))
  #print 'p3d ', p3d

  # Pseudoinverses of A and B
  '''inv1 = np.transpose(A).dot(np.linalg.inv(A.dot(np.transpose(A))))
  inv2 = np.transpose(B).dot(np.linalg.inv(B.dot(np.transpose(B))))

  z1 = inv1.dot(y1)
  z2 = inv2.dot(y2)

  # Remove remove the last element of homogeneous coordinates
  # Only the direction of the vector matters, not the scale
  z1 = z1[:3]
  z2 = z2[:3]

  # Scale so that z = 1
  z1[:] /= z1[2]
  z2[:] /= z2[2]
  
  d1 = z1 - n1
  d2 = z2 - n2
  dn = n2 - n1

  A = np.zeros((2, 2), np.float64)
  A[0][0] = np.transpose(d1).dot(d1)
  A[0][1] = -np.transpose(d1).dot(d2)
  A[1][0] = np.transpose(d2).dot(d1)
  A[1][1] = -np.transpose(d2).dot(d2)

  b = np.zeros((2, 1), np.float64)
  b[0][0] = np.transpose(d1).dot(dn)
  b[1][0] = np.transpose(d2).dot(dn)

  t = np.linalg.inv(A).dot(b)

  x1 = (1 - t[0]) * n1 + t[0] * z1
  x2 = (1 - t[1]) * n2 + t[1] * z2

  x = (x1 + x2) / 2
  '''
  #return x
  return p3d


# Open the cameras
camera = [0 for i in range(num_cameras)]
for i in range(num_cameras):
  camera[i] = cv2.VideoCapture(i) # TODO: Remove the + 1
  camera[i].set(cv2.cv.CV_CAP_PROP_FRAME_WIDTH, w)
  camera[i].set(cv2.cv.CV_CAP_PROP_FRAME_HEIGHT, h)

# Create the windows

cv2.namedWindow("Cams")

rotation = np.loadtxt('stereo_r.cam')
translation = np.loadtxt('stereo_t.cam')

cam_mtx0 = np.loadtxt('stereo_cam0.cam')
cam_mtx1 = np.loadtxt('stereo_cam1.cam')

print "r: ",rotation
print "t: ",translation
print "m0: ",cam_mtx0
print "m1: ",cam_mtx1

counter = 0
conti = True

#while conti:
#	conti = heardEnter()


lower_color = np.ones(3, np.uint8) * 250
upper_color = np.ones(3, np.uint8) * 250
lower_color[0] = 1
lower_color[1] = 55
lower_color[2] = 88
upper_color[0] = 29
upper_color[0] = 236
upper_color[0] = 255



# Initialize the timer
start = time.time()

prjPoints = [np.zeros((2, 1), np.int32) for i in range(num_cameras)]

send_synch = True

conti = True
while conti:

  # Prepare to get the next frame from all cameras
  error = False
  for i in range(num_cameras):
    if not camera[i].grab():
      error = True
      break
  if error:
    print("Error occurred in grab.")
    exit()

  # Capture the image from both cameras
  flag = [0 for i in range(num_cameras)]
  frame = [0 for i in range(num_cameras)]
  for i in range(num_cameras):
    flag[i], frame[i] = camera[i].retrieve()
    if not flag[i]:
      # The frame was not captured properly
      error = True
      break
  if error:
    print("Error occurred in grab.")
    exit()
  grab_time = time.time()
  if send_synch == True:
    os.system('echo "1" > /dev/ttyACM0')
    send_synch = False


  elapsed = (time.time() - start)
  #print 'ip-1: ', elapsed

  thread1 = ThreadBGR2HSV(frame[0])
  thread2 = ThreadBGR2HSV(frame[1])

  thread1.start()
  thread2.start()

  thread1.join()
  thread2.join()

  hsv_img1 = thread1.GetFrame()
  hsv_img2 = thread2.GetFrame()

  #elapsed = (time.time() - start)
  #print 'ip-1.5: ', elapsed

  blob_img1 = cv2.inRange(hsv_img1, lower_color, upper_color)
  blob_img2 = cv2.inRange(hsv_img2, lower_color, upper_color)

  #elapsed = (time.time() - start)
  #print 'ip-2: ', elapsed

  l_moments = cv2.moments(blob_img1)
  r_moments = cv2.moments(blob_img2)

  #elapsed = (time.time() - start)
  #print 'ip-3: ', elapsed

  valid_point = False

  if (l_moments['m00'] != 0) and (r_moments['m00'] != 0):
    prjPoints[0][0] = int(l_moments['m10']/l_moments['m00'])
    prjPoints[0][1] = int(l_moments['m01']/l_moments['m00'])
    
    prjPoints[1][0] = int(r_moments['m10']/r_moments['m00'])
    prjPoints[1][1] = int(r_moments['m01']/r_moments['m00'])
    valid_point = True
    cv2.rectangle(frame[0], (prjPoints[0][0]-7, prjPoints[0][1]-7), (prjPoints[0][0]+7,prjPoints[0][1]+7), (255,50,50), 4)
    cv2.rectangle(frame[1], (prjPoints[1][0]-7, prjPoints[1][1]-7), (prjPoints[1][0]+7,prjPoints[1][1]+7), (255,50,50), 4)

  #blob_img1_bgr = cv2.cvtColor(blob_img1, cv2.COLOR_GRAY2BGR)
  #blob_img2_bgr = cv2.cvtColor(blob_img2, cv2.COLOR_GRAY2BGR)

  # Create the images of all cameras
  vis = np.zeros((h, w*num_cameras+5*(num_cameras-1)), np.uint8)
  vis = cv2.cvtColor(vis, cv2.COLOR_GRAY2BGR)
  for i in range(num_cameras):
    vis[:h, i*(w+5):i*(w+5)+w] = frame[i]

  elapsed = (time.time() - start)
  print 'ip0: ', elapsed

  # Initialize the image that indicates which points have been recorded
  # Also determine the average location of the light
  '''marked = np.zeros((h, num_cameras*(w+5)-5), np.uint8)
  marked[:] = 255
  marked = cv2.cvtColor(marked, cv2.COLOR_GRAY2BGR)
  projPoints = [np.zeros((2, 1), np.int32) for i in range(num_cameras)]
  elapsed = (time.time() - time1)
  print 'ip1: ', elapsed
  for i in range(num_cameras):
    marked[:h, i*(w+5):w+i*(w+5)] = 0 # Set mostly to black

    # Detect where RGB is 255
    f = (frame[i] == np.array([255, 255, 255], np.uint8))
    # Set brightest to white
    marked[0:h, i*(w+5):w+i*(w+5)][frame[i] == np.array([255, 255, 255])] = 255

    # Find locations where RGB is maximized
    a = np.where(f)
    if (len(a[0]) != 0):
      # Set locations if light is detected
      projPoints[i][0] = np.mean(a[0])
      projPoints[i][1] = np.mean(a[1])

  elapsed = (time.time() - time1)
  print 'ip2: ', elapsed
  '''
  # Display the images
  cv2.imshow("Cams", vis)
  #cv2.imshow("Marked", marked)
  key = cv2.waitKey(1) # This is needed to allow the images to be displayed
  if key == 27:
     break

  colTranslate = np.array(translation)[np.newaxis]
  #point = triangulate(projPoints[0], projPoints[1], cam_mtx0, cam_mtx1, rotation, np.transpose(colTranslate))
  if valid_point == True:
     point = triangulate(prjPoints[0], prjPoints[1], cam_mtx0, cam_mtx1, rotation, np.transpose(colTranslate))
     '''tuple_row = (int(round(grab_time*100000)), float(point[0][0][0]), float(point[0][0][1]), float(point[0][0][2]), int(valid_point))
  else:
     tuple_row = (int(round(grab_time*100000)), float(0.0), float(0.0), float(0.0), int(valid_point))
  #print point
  '''

  elapsed = (time.time() - start)
  print(elapsed)
  start = time.time()

  #tuple_row = (int(round(grab_time*100000)),point[0][0], point[1][0], point[2][0], int(valid_point))
  #print 'tuple: ', tuple_row

  #holders = ','.join('?' * (5))
  #sql = 'INSERT INTO cam VALUES({0})'.format(holders)
  #print 'sql: ',sql
  try:
     #cursor.execute(sql, tuple_row)
     if valid_point == True:
        print point[0][0]
        socket.send(str(point[0][0][0])+","+str(point[0][0][1])+","+str(point[0][0][2]))
        
  except:
     print "Warning, ", sys.exc_info()[1]
     continue

  counter += 1
  if counter == 10:
     print 'commit'
     #connection.commit()
     #thread_sql = ThreadSQLCommit(connection)
     #thread_sql.start()
     counter = 0

  conti = heardEnter()


#connection.commit()
#connection.close()
cv2.destroyAllWindows() # Close the windows
#os.system('echo "0" > /dev/ttyACM0')

