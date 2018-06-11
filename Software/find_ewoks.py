'''
Object detection ("Ball tracking") with OpenCV
    Adapted from the original code developed by Adrian Rosebrock
    Visit original post: https://www.pyimagesearch.com/2015/09/14/ball-tracking-with-opencv/
Developed by Marcelo Rovai - MJRoBot.org @ 7Feb2018 
'''

avg_len = 5
last_pos=[(0,0) for _ in range(avg_len)]
last_center=(0,0)

def dist(p1,p2):
    return math.sqrt((p2[0] - p1[0])**2 + (p2[1] - p1[1])**2 )

def avg_pos():
    return (int(sum(last_pos[x][0] for x in range (avg_len)) / avg_len), int(sum(last_pos[y][1] for y in range(avg_len)) / avg_len))

# import the necessary packages
from collections import deque
import numpy as np
import argparse
import imutils
import cv2
import math

# construct the argument parse and parse the arguments
ap = argparse.ArgumentParser()
ap.add_argument("-v", "--video", help="path to the (optional) video file")
args = vars(ap.parse_args())

# define the lower and upper boundaries of the "yellow object"
# (or "ball") in the HSV color space, then initialize the
# list of tracked points
colorLower = (3, 100, 100) 
colorUpper = (23, 255, 255)
 
# if a video path was not supplied, grab the reference
# to the webcam
if not args.get("video", False):
	camera = cv2.VideoCapture(0)
 
# otherwise, grab a reference to the video file
else:
	camera = cv2.VideoCapture(args["video"])

# keep looping
while True:
	# grab the current frame
	(grabbed, frame) = camera.read()
 
	# if we are viewing a video and we did not grab a frame,
	# then we have reached the end of the video
	if args.get("video") and not grabbed:
		break
 
	# resize the frame, inverted ("vertical flip" w/ 180degrees),
	# blur it, and convert it to the HSV color space
	frame = imutils.resize(frame, width=600)
	frame = imutils.rotate(frame, angle=180)
	clahe = cv2.createCLAHE(clipLimit=4.0, tileGridSize=(8,8))
	gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
	frame = clahe.apply(gray)
	# hsv = cv2.cvtColor(frame, cv2.COLOR_2HSV)

	# construct a mask for the color "green", then perform
	# a series of dilations and erosions to remove any small
	# blobs left in the mask
	mask = cv2.bilateralFilter(frame,15,75,80)
	# mask = cv2.inRange(bilateral, colorLower, colorUpper)

	# mask = cv2.erode(mask, None, iterations=2)
	# mask = cv2.dilate(mask, None, iterations=2)
	
	# find contours in the mask and initialize the current
	# (x, y) center of the ball
	cnts = cv2.findContours(mask.copy(), cv2.RETR_EXTERNAL,
		cv2.CHAIN_APPROX_SIMPLE)[-2]
	center = None
 
	# only proceed if at least one contour was found
	if len(cnts) > 0:
		# find the largest contour in the mask, then use
		# it to compute the minimum enclosing circle and
		# centroid
		c = max(cnts, key=cv2.contourArea)
		((x, y), radius) = cv2.minEnclosingCircle(c)
		M = cv2.moments(c)
		try:
			center = (int(M["m10"] / M["m00"]), int(M["m01"] / M["m00"]))

			# only proceed if the radius meets a minimum size
			if radius > 10:
				# draw the circle and centroid on the frame
				last_pos = last_pos[1:]
				last_pos.append((x,y))
				if dist((x,y), avg_pos()) < 50:
					cv2.circle(frame, (int(x), int(y)), int(radius), (0, 255, 255), 2)
					cv2.circle(frame, center, 5, (0, 0, 255), -1)
					last_center = center

		except:
			pass

		

	# show the frame to our screen
	cv2.imshow("Frame", frame)
	cv2.imshow("Mask", mask)
	key = cv2.waitKey(1) & 0xFF
 
	# if the 'q' key is pressed, stop the loop
	if key == ord("q"):
		break
 
# cleanup the camera and close any open windows
camera.release()
cv2.destroyAllWindows()