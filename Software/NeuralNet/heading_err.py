'''
Wrapper for a YOLO neural network that runs on a Raspberry Pi. Responsible
for capturing images from the PiCamera, passing the image to a trained 
network, calculating the heading error from the returned information, 
and finally outputting the heading error to 8GPIO pins connected to an R2R
DAC
'''

import RPi.GPIO as GPIO
from picamera.array import PiRGBArray
from picamera import PiCamera
import math


 # 1 is MSB, 8 is LSB
BIN_OUT1 = 5
BIN_OUT2 = 6
BIN_OUT3 = 13
BIN_OUT4 = 19
BIN_OUT5 = 26
BIN_OUT6 = 12
BIN_OUT7 = 16
BIN_OUT8 = 20
INTERRUPT_PIN = 21


def setup():

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



def DAC_output(err):
    # 255 is ewok at right edge
    # 0 is ewok at left edge
    # 127 is ewok in the center
    out_val = int(err * 127 + 127)
    binary = bin(out_val)[2:]

    # Orded msb -> lsb
    out_bits = [int(x) for x in list(binary)]
    GPIO.output(BIN_OUT1, out_bits[0])
    GPIO.output(BIN_OUT2, out_bits[1])
    GPIO.output(BIN_OUT3, out_bits[2])
    GPIO.output(BIN_OUT4, out_bits[3])
    GPIO.output(BIN_OUT5, out_bits[4])
    GPIO.output(BIN_OUT6, out_bits[5])
    GPIO.output(BIN_OUT7, out_bits[6])
    GPIO.output(BIN_OUT8, out_bits[7])


def main():
    
    # Camera setup
    cam = PiCamera()
    cam.resolution = (240, 240)
    cam.framerate = 32
    raw = PiRGBArray(cam, size=(240,240))
    stream = cam.capture_continuous(raw, format='bgr', use_video_port=True)

    # Main loop
    for f in stream:
        frame = f.array

        '''
        TODO: FIX THIS PSEUDO-CODE
        >>>>>>>>>>>>>
        '''
        bbox = YoloNet.find_ewok(frame)  # Get the location of any bounding boxes
        
        # If there is an ewok, get the error and output to DAC
        if bbox:
            GPIO.output(INTERRUPT_PIN, 1)
            err = (bbox.x_bar - frame.x_bar) / frame.x_bar  # Returns error between -1 and 1       
            DAC_ouput(err)
            GPIO.output(INTERRUPT_PIN, 0)
        '''
        <<<<<<<<<<<<<<
        '''


if __name__ == '__main__':
    # Catch and ignore any errors
    while True:
        try:
            main()
        except Exception as e:
            print(e)
            pass
