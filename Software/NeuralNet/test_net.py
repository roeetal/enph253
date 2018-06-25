# Imports
from keras.preprocessing.image import img_to_array
from keras.models import load_model
import numpy as np
import argparse
import imutils
import cv2
import os


# Functions
def predict_image(image, wait=True):
    orig = image.copy()
 
    # pre-process the image for classification
    image = cv2.resize(image, (28, 28))
    image = image.astype("float") / 255.0
    image = img_to_array(image)
    image = np.expand_dims(image, axis=0)

    # load the trained convolutional neural network
    print("[INFO] loading network...")
    
    # classify the input image
    (notewok, ewok) = model.predict(image)[0]

    # build the label
    label = "Ewok" if ewok > notewok else "Not Ewok"
    proba = ewok if ewok > notewok else notewok
    label = "{}: {:.2f}%".format(label, proba * 100)
    
    # draw the label on the image
    output = imutils.resize(orig, width=400)
    cv2.putText(output, label, (10, 25),  cv2.FONT_HERSHEY_SIMPLEX,
        0.7, (0, 255, 0), 2)
    
    # show the output image
    cv2.imshow("Output", output)
    if wait: cv2.waitKey(0)


def get_direction(image, wait=True):
    orig = image.copy()

    height, width = image.shape[:2]
    
    div = int(width / 3)

    print("h: {}, w: {}, d:{}".format(height, width, div))

    left = image[0:,:div]
    mid = image[0:,div:2*div]
    right = image[0:,2*div:]

    images = [left, mid, right]
    labels = [None, None, None]
    scores = [0, 0, 0]
    
    # pre-process the image for classification
    for i, image in enumerate(images):
        image = cv2.resize(image, (28, 28))
        image = image.astype("float") / 255.0
        image = img_to_array(image)
        image = np.expand_dims(image, axis=0)
        
        # classify the input image
        (notewok, ewok) = model.predict(image)[0]

        # build the label
        label = "Ewok" if ewok > notewok else "Nothing"
        proba = ewok if ewok > notewok else 0

        labels[i] = label
        scores[i] = proba

    label = "Nothing"
    pos = ["left", "middle", "right"]
    idx = np.argmax(scores)
    if scores[idx] != 0: label = pos[idx] + ": " + str(scores[idx])
    

    # draw the label on the image
    output = imutils.resize(orig, width=400)
    cv2.putText(output, label, (10, 25),  cv2.FONT_HERSHEY_SIMPLEX,
        0.7, (0, 255, 0), 2)    
    # show the output image
    cv2.imshow("Output", output)

    if wait: cv2.waitKey(0)

# Main
# construct the argument parse and parse the arguments
ap = argparse.ArgumentParser()
ap.add_argument("-m", "--model", required=True, help="path to trained model model")
ap.add_argument("-i", "--image", required=False, help="path to input image")
ap.add_argument("-d", "--dir", required=False, help="path to input dir")
ap.add_argument("-v", "--video", required=False, help="path to input video")
args = vars(ap.parse_args())

model = load_model(args["model"])

if args["dir"]:
    for file in os.listdir(args["dir"]):
        img = cv2.imread(args["dir"] + "/" + file)
        predict_image(img)
elif args["image"]:
    # load the image
    img = cv2.imread(args["image"])
    get_direction(img)
elif args["video"]:
    cap = cv2.VideoCapture(args["video"])
    while(cap.isOpened()):
        # Capture frame-by-frame
        ret, frame = cap.read()
        if ret == True:
 
            # Display the resulting frame
            get_direction(frame, wait=False)
    
            # Press Q on keyboard to  exit
            if cv2.waitKey(25) & 0xFF == ord('q'):
                break
 
        # Break the loop
        else: 
            break

    cap.release()
    # Closes all the frames
    cv2.destroyAllWindows()
else:
    print("Bruh gimme a photo or a video or some shit")
