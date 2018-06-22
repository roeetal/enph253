from picamera.array import PiRGBArray
from picamera import PiCamera
from keras.models import load_model
import RPi.GPIO as GPIO
import time
import cv2

GPIO.setmode(GPIO.BCM)
GPIO.setup(14, GPIO.OUT)
GPIO.output(14, 0)

cam = PiCamera()
cam.resolution = (240, 240)
cam.framerate = 32
raw = PiRGBArray(cam, size=(240,240))
stream = cam.capture_continuous(raw, format='bgr', use_video_port=True)
model = load_model('ewok.model')

time.sleep(2)

count = 0
time = time.time()

for f in stream:
    frame = f.array

    not_ewok, ewok = model.predict(frame)[0]
    if ewok > not_ewok: GPIO.output(14,1)
    else: GPIO.output(14,0)
    
    raw.truncate(0)

    count += 1
    if time.time() - time > 3:
        print('{}FPS'.format(count))
        count = 0
        time = time.time()


