import cv2
import numpy as np
import argparse
import time
import math
import RPi.GPIO as GPIO
from picamera import PiCamera


LINE_ERR_THRESH = 5
SLOPE_ERR_THRESH = 1
INTERCEPT_ERR_THRESH = 30
C_AVG = []

# PINOUT
BIN_OUT1 = 5  # MSB
BIN_OUT2 = 6
BIN_OUT3 = 13
BIN_OUT4 = 19
BIN_OUT5 = 26
BIN_OUT6 = 12
BIN_OUT7 = 16
BIN_OUT8 = 20  # LSB
INTERRUPT_PIN = 21


ap = argparse.ArgumentParser()
ap.add_argument('-v', '--video', required=False, help='Path to video file')
ap.add_argument('-c', '--cam', required=False, help='Use live video from PiCamera')
args = vars(ap.parse_args())


def setup_GPIO():
    """
    Setup GPIO pins and load the network
    """

    # GPIO Setup
    GPIO.setmode(GPIO.BCM)
    GPIO.setup(BIN_OUT1, GPIO.OUT)
    GPIO.setup(BIN_OUT2, GPIO.OUT)
    GPIO.setup(BIN_OUT3, GPIO.OUT)
    GPIO.setup(BIN_OUT4, GPIO.OUT)
    GPIO.setup(BIN_OUT5, GPIO.OUT)
    GPIO.setup(BIN_OUT6, GPIO.OUT)
    GPIO.setup(BIN_OUT7, GPIO.OUT)
    GPIO.setup(BIN_OUT8, GPIO.OUT)
    GPIO.setup(INTERRUPT_PIN, GPIO.OUT)


def DAC_output(err, DBG=False):
    """txt
    Output error between -1 and 1 as an analog voltage between
    0 and 3.3V
    err - decimal error between -1 and 1
    DBG - if DBG=True, the output will be logged
    """
    FN = 'DAC_output'

    out_val = int(err * 127 + 127)
    binary = bin(out_val)[2:]

    # Orded msb -> lsb
    out_bits = [int(x) for x in list(binary)]
    for _ in range(len(out_bits),8):
        out_bits.insert(0,0)

    if DBG: log(FN, out_bits, EWOK_DETECTION)

    GPIO.output(BIN_OUT1, out_bits[0])
    GPIO.output(BIN_OUT2, out_bits[1])
    GPIO.output(BIN_OUT3, out_bits[2])
    GPIO.output(BIN_OUT4, out_bits[3])
    GPIO.output(BIN_OUT5, out_bits[4])
    GPIO.output(BIN_OUT6, out_bits[5])
    GPIO.output(BIN_OUT7, out_bits[6])
    GPIO.output(BIN_OUT8, out_bits[7])


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
    
    left.reverse()
    right.reverse()
    center.reverse()

    for i in range(3, len(left)):
        if math.fabs(left[i][0] - (sum([x[0] for x in left[i-3:i]]) / 3)) > LINE_ERR_THRESH:
            left = left[:i]
            break

    for i in range(3, len(right)):
        if math.fabs(right[i][0] - (sum([x[0] for x in right[i-3:i]]) / 3)) > LINE_ERR_THRESH:
            right = right[:i]
            break

    for i in range(3, len(center)):
        if math.fabs(center[i][0] - (sum([x[0] for x in center[i-3:i]]) / 3)) > LINE_ERR_THRESH:
            center = center[:i]
            break
    
    return np.asarray(left), np.asarray(right), np.asarray(center)


def navigate(img):
    global C_AVG

    height, width, z = img.shape
    edges = get_edges(img)

    x_bar = int(width / 2)
    y_samples = range(250, height, 5)

    cv2.line(img, (x_bar, 0), (x_bar, height), (0,255,0), 5)    

    left, right, center = get_edge_points(edges, y_samples, x_bar)

    ml = 0
    bl = 0
    mr = 0
    br = 0
    mc = 0
    bc = 0
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

    if center.size > 10 and math.fabs(ml + mr) < 0.7 and ml < 0:
        mc, bc = np.polyfit(center[:,0], center[:,1], 1)
       
        C_AVG.append((mc,bc))
        C_AVG = C_AVG[-5:]
        m_avg = sum([x[0] for x in C_AVG]) / len(C_AVG)
        b_avg = sum([x[1] for x in C_AVG]) / len(C_AVG)

        print('Slope: ', m_avg, mc)
        print('Intercept: ', b_avg, bc)

        if math.fabs(mc - m_avg) < SLOPE_ERR_THRESH:
            # y1 = int(bc)
            # y2 = int(mc * width + bc)
            # cv2.line(img, (0,y1), (width, y2), (0,0,0), 5)
            # for x,y in center:
            #         cv2.circle(img, (x,y), 4, (0,0,0), 4)

            if not 0 in [ml, mr, mc, bl, br, bc]:
                A = (int((bl - bc)/(mc - ml)), int(mc * ((bl - bc)/(mc - ml)) + bc))
                B = (int((bl - br)/(mr - ml)), int(mr * ((bl - br)/(mr - ml)) + br))
                C = (int((br - bc)/(mc - mr)), int(mc * ((br - bc)/(mc - mr)) + bc))
                O = (int((A[0] + B[0] + C[0])/3), int((A[1] + B[1] + C[1])/3))

                cv2.circle(img, A, 4, (0,0,0), 4)
                cv2.circle(img, B, 4, (0,0,0), 4)
                cv2.circle(img, C, 4, (0,0,0), 4)
                
                cv2.circle(img, O, 4, (255,255,255), 4)

                err = (O[0] - width / 2) / (width / 2)
                
            
            else:
                print('Err', 0)
    

    cv2.imshow('img', img)
    cv2.imshow('edges', edges)



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