import cv2
import numpy as np

# Read the picure - The 1 means we want the image in BGR
img = cv2.imread('ewok.JPG', 1) 
# resize imag to 20% in each axis
img = cv2.resize(img, (0,0), fx=0.2, fy=0.2)
# convert BGR image to a HSV image
hsv = cv2.cvtColor(img, cv2.COLOR_BGR2HSV) 
# NumPy to create arrays to hold lower and upper range 
# The “dtype = np.uint8” means that data type is an 8 bit integer
lower_range = np.array([0, 18, 18]) 
upper_range = np.array([20, 255, 255])
# create a mask for image
mask = cv2.inRange(hsv, lower_range, upper_range)

cv2.imshow("initial mask", mask)

mask = cv2.erode(mask, None, iterations=2)
mask = cv2.dilate(mask, None, iterations=2)
	
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
    center = (int(M["m10"] / M["m00"]), int(M["m01"] / M["m00"]))

    # only proceed if the radius meets a minimum size
    if radius > 4:
        # draw the circle and centroid on the frame
        cv2.circle(img, (int(x), int(y)), int(radius),
            (0, 255, 255), 2)
        cv2.circle(img, center, 5, (0, 0, 255), -1)

# show the frame to our screen
cv2.imshow("Target", img)
key = cv2.waitKey(1) & 0xFF

# display both the mask and the image side-by-side
cv2.imshow('Final mask',mask)
# cv2.imshow('image', img)
# wait to user to press [ ESC ]
while(1):
  k = cv2.waitKey(0)
  if(k == 27):
    break