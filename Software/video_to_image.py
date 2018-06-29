import cv2
import argparse
import os
import re

ap = argparse.ArgumentParser(description='Convert video files into training data images.')
ap.add_argument("-v", "--video", help="Path to video file")
ap.add_argument("-o", "--output", help="Output directory for files")
args = ap.parse_args()


def extract_number(f):
    s = re.findall("\d+",f)
    return (int(s[0]) if s else -1,f)


cap = cv2.VideoCapture(args.video)
out_dir = args.output
label = out_dir.split("/")[-1]

try:
    index = int(re.findall('\\d+', max(os.listdir(out_dir),key=extract_number))[0])
except:
    index = 0

while(cap.isOpened()):
    # Capture frame-by-frame
    ret, frame = cap.read()
    if ret == True:
        print("Saving: {}/{}{}.jpg".format(out_dir, label, index))
        # Save frame
        cv2.imwrite('{}/{}{}.jpg'.format(out_dir, label, index),frame)
        index += 1

    # Break the loop
    else: 
        break

cap.release()
# Closes all the frames
cv2.destroyAllWindows()
