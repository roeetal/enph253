"""
Full file for controlling all Raspberry Pi functions on the robot.
"""


import os
import cv2
import time
import math
import imutils
import argparse
import numpy as np

# import RPi.GPIO as GPIO
# from picamera import PiCamera
# from picamera.array import PiRGBArray


from keras.models import load_model
from keras.preprocessing.image import img_to_array

# Edge Detection
LINE_ERR_THRESH = 5
SLOPE_ERR_THRESH = 1
INTERCEPT_ERR_THRESH = 30
C_AVG = []

# Ewok Detection
MODEL = 'ewok.model'

# Pinout
BIN_OUT1 = 5  # MSB
BIN_OUT2 = 6
BIN_OUT3 = 13
BIN_OUT4 = 19
BIN_OUT5 = 26
BIN_OUT6 = 12
BIN_OUT7 = 16
BIN_OUT8 = 20  # LSB
INTERRUPT_PIN = 21


def setup_GPIO():
    """
    Setup GPIO pins and load the network
    """

    FN = 'setup_GPIO'

    log(FN, 'Started GPIO setup')

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

    log(FN, 'GPIO setup complete')


def log(function, message):
    """
    Write a message to the logs.
    function - the name of the function sending writing the message
    message - the error or data being logged
    """
    os.system('echo "{}: {}  |  {}" >> log.txt'.format(function, message, time.time()))


def smooth(image, kernel_size=15):
    """
    kernel_size must be postivie and odd
    """
    return cv2.GaussianBlur(image, (kernel_size, kernel_size), 0)


def DAC_output(err):
    """
    Output error between -1 and 1 as an analog voltage between
    0 and 3.3V
    err - decimal error between -1 and 1
    """

    FN = 'DAC_output'

    out_val = int(err * 127 + 127)
    binary = bin(out_val)[2:]

    # Orded msb -> lsb
    out_bits = [int(x) for x in list(binary)]
    for _ in range(len(out_bits),8):
        out_bits.insert(0,0)

    log(FN, 'Err: {}  |  Writing to DAC: {} -> {}'.format(err, out_val, out_bits))

    GPIO.output(BIN_OUT1, out_bits[0])
    GPIO.output(BIN_OUT2, out_bits[1])
    GPIO.output(BIN_OUT3, out_bits[2])
    GPIO.output(BIN_OUT4, out_bits[3])
    GPIO.output(BIN_OUT5, out_bits[4])
    GPIO.output(BIN_OUT6, out_bits[5])
    GPIO.output(BIN_OUT7, out_bits[6])
    GPIO.output(BIN_OUT8, out_bits[7])


def find_ewoks(images, model):
    FN = 'find_ewoks'

    ewoks = [0, 0, 0]
    
    for i, image in enumerate(images):
        # pre-process the image for classification
        image = cv2.resize(image, (28, 28))
        image = image.astype("float") / 255.0
        image = img_to_array(image)
        image = np.expand_dims(image, axis=0)
        
        # classify the input image
        (notewok, ewok) = model.predict(image)[0]
        if ewok > notewok and ewok > 0.5: 
            print(ewok, notewok)
            ewoks[i] = 1

    # Return error if there were any ewoks
    if 1 in ewoks:
        log(FN, 'Found ewok in frames: {}'.format(ewoks))
        return (ewoks[2] - ewoks[0]) / (sum(ewoks))
    else: return None

    
def get_edges(img):
    gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
    gray = smooth(gray, 3)
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
    

def process(img, model):

    FN = 'process'

    height, width = img.shape[:2]

    '''
    EWOKS
    ------------------------------------------------------------
    '''

    div = int(width / 3)
    
    left = img[0:,:div]
    mid = img[0:,div:2*div]
    right = img[0:,2*div:]

    images = [left, mid, right]
    ewok_err = find_ewoks(images, model)

    if ewok_err is not None: return ewok_err
    
    '''
    EDGES
    ------------------------------------------------------------
    '''

    global C_AVG

    edges = get_edges(img)

    x_bar = int(width / 2)
    y_samples = range(250, height, 5)
    
    left, right, center = get_edge_points(edges, y_samples, x_bar)

    height, width, z = img.shape
    edges = get_edges(img)

    x_bar = int(width / 2)
    y_samples = range(250, height, 5)

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
        # cv2.line(img, (0,y1), (width, y2), (255,0,0), 5)

        # for x,y in left:
        #     cv2.circle(img, (x,y), 4, (255,0,0), 4)

    if np.any(right):
        mr, br = np.polyfit(right[:,0], right[:,1], 1)

        y3 = int(br)
        y4 = int(mr * width + br)
        # cv2.line(img, (0,y3), (width, y4), (0,0,255), 5)

        # for x,y in right:
        #     cv2.circle(img, (x,y), 4, (0,0,255), 4)

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

                # cv2.circle(img, A, 4, (0,0,0), 4)
                # cv2.circle(img, B, 4, (0,0,0), 4)
                # cv2.circle(img, C, 4, (0,0,0), 4)
                
                # cv2.circle(img, O, 4, (255,255,255), 4)

                return (O[0] - width / 2) / (width / 2)
        
    return None



def live(model):

    FN = 'live'

    # Camera setup
    log(FN, 'Setting up camera')
    cam = PiCamera()
    cam.resolution = (240, 240)
    cam.framerate = 32
    raw = PiRGBArray(cam, size=(240,240))
    stream = cam.capture_continuous(raw, format='bgr', use_video_port=True)
    log(FN, 'Camera setup done')

    # GPIO setup
    setup_GPIO()

    log(FN, 'Starting stream')
    for f in stream:
        frame = f.array

        err = process(frame, model)
        if err:
            GPIO.output(INTERRUPT_PIN, 0)
            DAC_output(err)
            GPIO.output(INTERRUPT_PIN, 1)
        
        raw.truncate(0)


def video(file, model):

    FN = 'video'

    cap = cv2.VideoCapture(file)
    while(cap.isOpened()):
        # Capture frame-by-frame
        ret, frame = cap.read()
        if ret == True:
 
            # Display the resulting frame
            err = process(frame, model)
            cv2.putText(frame, 'Err: {}'.format(err), (50, 50), cv2.FONT_HERSHEY_SIMPLEX, 0.7, (255,0,0), 2)
            cv2.imshow('Frame', frame)
    
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
    ap = argparse.ArgumentParser()
    ap.add_argument('-v', '--video', required=False, help='Path to video file')
    args = vars(ap.parse_args())
    model = load_model(MODEL)
    if not video:
        live(model)
    else:
        video(args['video'], model)

