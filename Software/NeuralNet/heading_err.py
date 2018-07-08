'''
Wrapper for a YOLO neural network that runs on a Raspberry Pi. Responsible
for capturing images from the PiCamera, passing the image to a trained 
network, calculating the heading error from the returned information, 
and finally outputting the heading error to 8GPIO pins connected to an R2R
DAC
'''

# import RPi.GPIO as GPIO
# from picamera.array import PiRGBArray
# from picamera import PiCamera
from darkflow.net.build import TFNet
import cv2
import math
import os
import time
import argparse


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

# LOGS
EWOK_DETECTION = 'ewok_detection'
EDGE_DETECTION = 'edge_detection'

last_pos = 0
frame_center = 240
seen = [0,0,0,0,0]



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


def get_net():
    """
    Get a TFNet object
    """

    # Load network
    options = {"pbLoad":"Networks/network-v2.pb", "metaLoad":"Networks/network-v2.meta"}
    return TFNet(options)


def log(function, message, log):
    """
    Write a message to the logs.
    function - the name of the function sending writing the message
    message - the error or data being logged
    log - the log file to write to (ewok_detection, edge_detection, etc.)
    """
    os.system('echo "{}: {}  |  {}" >> Logs/{}.txt'.format(function, message, time.time(), log))


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


def get_pos(bbox):
    if len(bbox) == 1:
        return bbox[0]["bottomright"]["x"] - bbox[0]["topleft"]["x"]
    else:
        idx = 0
        for i,x in enumerate(bbox):
            if x["confidence"] > bbox[idx]["confidence"]:
                idx = i
        return bbox[idx]["bottomright"]["x"] - bbox[idx]["topleft"]["x"]


def live():
    
    # Camera setup
    cam = PiCamera()
    cam.resolution = (240, 240)
    cam.framerate = 32
    raw = PiRGBArray(cam, size=(240,240))
    stream = cam.capture_continuous(raw, format='bgr', use_video_port=True)

    # Network and GPIO setup
    setup_GPIO()
    net = get_net()

    # Main loop
    for f in stream:
        frame = f.array
        bbox = net.return_predict(frame)  # Get the location of any bounding boxes
        
        if bbox:
            pos = get_pos(bbox)
            last_pos = pos
            seen.append(1)
        else:
            pos = last_pos
            seen.append(0)

        del seen[0]

        if sum(seen) > 0:
            GPIO.output(INTERRUPT_PIN, 1)
            err = (pos - frame_center) / frame_center  # Returns error between -1 and 1       
            DAC_ouput(err)
            GPIO.output(INTERRUPT_PIN, 0)


def demo(video):
    net = get_net()
    cap = cv2.VideoCapture(video)

    while(cap.isOpened()):
        # Capture frame-by-frame
        ret, frame = cap.read()
        if ret == True:
            bbox = net.return_predict(frame)
            
            if bbox:
                pos = get_pos(bbox)
                last_pos = pos
                seen.append(1)
            else:
                pos = last_pos
                seen.append(0)

            del seen[0]

            if sum(seen) > 0:
                err = (pos - frame_center) / frame_center  # Returns error between -1 and 1       
                print(bbox)
                print(pos)
                print(err)
                print('')

        # Break the loop
        else: 
            break

    cap.release()
    # Closes all the frames
    cv2.destroyAllWindows()


if __name__ == '__main__':
    # Catch and ignore any errors
    ap = argparse.ArgumentParser()
    ap.add_argument('-v', '--video', help='Path to video file')
    args = vars(ap.parse_args())
    print(args)

    if args['video']:
        demo(args['video'])
    else:
        while True:
            try:
                live()
            except Exception as e:
                print(e)
                pass
