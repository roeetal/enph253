import cv2
import numpy as np
import argparse


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

def navigate(img):
    imgHSV = cv2.cvtColor(img,cv2.COLOR_BGR2HSV)
    mask = cv2.inRange(imgHSV,lowerBound,upperBound)
    gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
    gray = smooth(gray, 3)
    edges = cv2.Canny(gray, 200, 300)
    height, width = gray.shape
    middle_mask = get_mask(width, height, 400, 400) 

    edge = cv2.bitwise_and(edges, edges, mask=middle_mask)
    cv2.imshow('mask', mask)
    cv2.imshow('edge', edge)


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