from picamera import PiCamera
import argparse

ap = argparse.ArgumentParser()
ap.add_argument("-n", "--name", help="name of output file (without .h264 extension)")
args = vars(ap.parse_args())

cam = PiCamera()
cam.start_preview()
cam.start_recording('/home/pi/{}.h264'.format(args['name']))

input()

cam.stop_recording()
cam.stop_preview()