import cv2
import numpy as np
import argparse
import time
import math


ERR_THRESH = 5


ap = argparse.ArgumentParser()
ap.add_argument('-v', '--video', required=False, help='Path to video file')
ap.add_argument('-c', '--cam', required=False, help='Use live video from PiCamera')
args = vars(ap.parse_args())

lowerBound=np.array([20,100,100])
upperBound=np.array([40,255,255])


def smooth(image, kernel_size=15):
    """
    kernel_size must be postivie and odd
    """
    return cv2.GaussianBlur(image, (kernel_size, kernel_size), 0)


def get_mask(width, height, m_width, m_height):
    """
    width, height - the dimensions of the image
    m_width - the width of the mask about the center of the image
    m_height - the height of the mask, measured up from the bottom of the image
    """

    mask = np.empty((height, width), dtype=np.uint8)
    x_i = int((width - m_width) / 2)
    x_f = x_i + m_width
    y_i = height - m_height
    mask[y_i:,x_i:x_f] = 255
    return mask


def get_edges(img):
    gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
    gray = smooth(gray, 3)
    cv2.imshow('gray',gray)
    edges = cv2.Canny(gray,150, 250, apertureSize=3)
    return edges


def get_edge_points(edge, y_samples, x_bar):

    w = len(edge[0,:])
    left = []
    right = []
    center = []

    for y in y_samples:
        l = [x for x in range(x_bar) if edge[y,x] == 255]
        r = [x for x in range(x_bar, w) if edge[y,x] == 255]
        if l: left.append((l[-1],y))
        if r: right.append((r[0],y))
        if l and r: center.append((int((r[0] + l[-1]) / 2), y))
    
    """
    TODO: Filter out outliers moving bottom up
    """
    
    return np.asarray(left), np.asarray(right), np.asarray(center)


def navigate(img):
    height, width, z = img.shape
    edges = get_edges(img)

    x_bar = int(width / 2)
    y_samples = range(250, height, 5)

    cv2.line(img, (x_bar, 0), (x_bar, height), (0,255,0), 5)    


    left, right, center = get_edge_points(edges, y_samples, x_bar)

    if np.any(center):
        mc, bc = np.polyfit(center[:,0], center[:,1], 1)
        y1 = int(bc)
        y2 = int(mc * width + bc)
        cv2.line(img, (0,y1), (width, y2), (0,0,0), 5)
        for x,y in center:
                cv2.circle(img, (x,y), 4, (0,0,0), 4)

    if True:
        ml = 0
        mr = 0
        if np.any(left):
            ml, bl = np.polyfit(left[:,0], left[:,1], 1)

            y1 = int(bl)
            y2 = int(ml * width + bl)
            cv2.line(img, (0,y1), (width, y2), (255,0,0), 5)

            for x,y in left:
                cv2.circle(img, (x,y), 4, (255,0,0), 4)

        if np.any(right):
            mr, br = np.polyfit(right[:,0], right[:,1], 1)

            y3 = int(br)
            y4 = int(mr * width + br)
            cv2.line(img, (0,y3), (width, y4), (0,0,255), 5)

            for x,y in right:
                cv2.circle(img, (x,y), 4, (0,0,255), 4)

    print('ml: {}  |  mr: {}'.format(ml,mr))
    cv2.imshow('img', img)
    cv2.imshow('edges', edges)

    """
    TODO: 
    1. Detect bridge or pole and advance state accordingly
    2. Estimate heading err
    """


def main():
    """
    General setup for testing the navigation function. This will be different for the final robot
    """
    cap = cv2.VideoCapture(args["video"])
    while(cap.isOpened()):
        # Capture frame-by-frame
        ret, frame = cap.read()
        if ret == True:

            navigate(frame)
            
            # Press Q on keyboard to  exit
            if cv2.waitKey(25) & 0xFF == ord('q'):
                break

        # Break the loop
        else: 
            break

    cap.release()
    # Closes all the frames
    cv2.destroyAllWindows()


if __name__ == '__main__':
    main()