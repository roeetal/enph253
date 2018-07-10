import cv2
import numpy as np
from matplotlib import pyplot as plt

IMAGE_H = 3024
IMAGE_W = 4032

src = np.float32([[0, IMAGE_H], [1207, IMAGE_H], [0, 0], [IMAGE_W, 0]])
dst = np.float32([[569, IMAGE_H], [711, IMAGE_H], [0, 0], [IMAGE_W, 0]])
M = cv2.getPerspectiveTransform(src, dst) # The transformation matrix
Minv = cv2.getPerspectiveTransform(dst, src) # Inverse transformation

img = cv2.imread('./bridge.jpg') # Read the test img
#img = img[450:(450+IMAGE_H), 0:IMAGE_W] # Apply np slicing for ROI crop
warped_img = cv2.warpPerspective(img, M, (IMAGE_W, IMAGE_H)) # Image warping
gray = cv2.cvtColor(warped_img, cv2.COLOR_BGR2GRAY)
denoised = cv2.GaussianBlur(gray,(3,3),0)
sobelx = cv2.Sobel(denoised,cv2.CV_64F,1,0,ksize=5)
plt.subplot(121), plt.imshow(gray, cmap = "gray")
plt.subplot(122), plt.imshow(denoised, cmap = "gray") # Show results
plt.show()

#img_inv = cv2.warpPerspective(warped_img, Minv, (IMAGE_W, IMAGE_H)) # Inverse transformation