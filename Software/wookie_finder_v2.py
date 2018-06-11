# import the necessary packages
from collections import deque
import numpy as np
import argparse
import imutils
import cv2
import math


avg_len = 5
last_pos = [(0, 0) for _ in range(avg_len)]
last_center = (0, 0)


# Allow videos or direct camera input
ap = argparse.ArgumentParser()
ap.add_argument("-v", "--video", help="path to the (optional) video file")
args = vars(ap.parse_args())

# Get camera
if not args.get("video", False):
	camera = cv2.VideoCapture(0)
else:
	camera = cv2.VideoCapture(args["video"])

# Loop
while True:
	# grab the current frame
	(grabbed, frame) = camera.read()

	# break if no frame
	if args.get("video") and not grabbed:
		break

	# resize frame
	frame = imutils.resize(frame, width=600)
	frame = cv2.add(frame, np.array([50.0]))
	clahe = cv2.createCLAHE(clipLimit=4.0, tileGridSize=(8,8))
	gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
	frame = clahe.apply(gray)

	mask = cv2.bilateralFilter(frame,15,75,80)

	# find contours in the mask and initialize the current
	# (x, y) center of the ball
	cnts = cv2.findContours(mask.copy(), cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)[-2]
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


def dist(p1,p2):
	return math.sqrt((p2[0] - p1[0])**2 + (p2[1] - p1[1])**2 )


def avg_pos():
	return (int(sum(last_pos[x][0] for x in range (avg_len)) / avg_len), int(sum(last_pos[y][1] for y in range(avg_len)) / avg_len))
