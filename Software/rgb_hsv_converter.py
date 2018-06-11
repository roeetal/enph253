import sys
import numpy as np
import cv2

blue = sys.argv[3]
green = sys.argv[2]
red = sys.argv[1]  
color = np.uint8([[[blue, green, red]]])
hsv_color = cv2.cvtColor(color, cv2.COLOR_BGR2HSV)
hue = hsv_color[0][0][0]
print("Lower bound is :"),
print("[" + str(hue-20) + ", 100, 100]\n")
print("Upper bound is :"),
print("[" + str(hue + 20) + ", 255, 255]")