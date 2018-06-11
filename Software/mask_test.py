import cv2
import numpy as np
import math

avg_len = 120
last_pos=[(0,0) for _ in range(avg_len)]

def dist(p1,p2):
    return math.sqrt((p2[0] - p1[0])**2 + (p2[1] - p1[1])**2 )

def avg_pos():
    return (sum(last_pos[x][0] for x in range (avg_len)) / avg_len, sum(last_pos[y][1] for y in range(avg_len)) / avg_len)

# Read the picure - The 1 means we want the image in BGR
img = cv2.imread('ewoks/photo4.jpg', 1) 
# resize imag to 20% in each axis
img = cv2.resize(img, (0,0), fx=0.2, fy=0.2)
# convert BGR image to a HSV image
hsv = cv2.cvtColor(img, cv2.COLOR_BGR2HSV) 
# NumPy to create arrays to hold lower and upper range 
# The “dtype = np.uint8” means that data type is an 8 bit integer
lower_range = np.array([7, 30, 10]) 
upper_range = np.array([20, 255, 70])
# create a mask for image

# blurred = cv2.GaussianBlur(hsv, (15, 15), 0)
blur = cv2.bilateralFilter(hsv,40,75,80)
mask = cv2.inRange(blur, lower_range, upper_range)

cv2.imshow("mask", mask)

mask = cv2.bilateralFilter(mask,15,75,80)
cv2.imshow("bilateral", mask)

# mask = cv2.erode(mask, None, iterations=2)
# mask = cv2.dilate(mask, None, iterations=2)
	
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
        last_pos = last_pos[1:]
        last_pos.append((x,y))
        if dist((x,y), avg_pos()) < 5:
            cv2.circle(img, (int(x), int(y)), int(radius),
                (0, 255, 255), 2)
            cv2.circle(img, center, 5, (0, 0, 255), -1)

# show the frame to our screen
# cv2.imshow("Image", img)
key = cv2.waitKey(1) & 0xFF

# display both the mask and the image side-by-side
cv2.imshow('Final mask',mask)
cv2.imshow('image', img)
# wait to user to press [ ESC ]
while(1):
  k = cv2.waitKey(0)
  if(k == 27):
    break