#include <stdio.h>
#include <stdlib.h>
#include <iostream>
extern "C"{
    #include <network.h>    
    #include <image.h>
}
#include <wiringPi.h>
#include <raspicam_cv.h>
#include <opencv2/imgproc.hpp>


#define BIN_OUT1 5  // MSB
#define BIN_OUT2 6
#define BIN_OUT3 13
#define BIN_OUT4 19
#define BIN_OUT5 26
#define BIN_OUT6 12
#define BIN_OUT7 16
#define BIN_OUT8 20  // LSB

#define INTERRUPT 21

#define TRUE 1
#define FALSE 0

#define CFG_PATH "darknet/cfg/yolov2-tiny-voc.cfg"
#define WEIGHT_PATH "darknet/yolov2-tiny-voc.weights"

using namespace cv;
using std::cout;

char *voc_names[] = {"aeroplane", "bicycle", "bird", "boat", "bottle", "bus", "car", "cat", "chair", "cow", "diningtable", "dog", "horse", "motorbike", "person", "pottedplant", "sheep", "sofa", "train", "tvmonitor"};

void setup(void);
void binary_out(float err);


int main(void) {
    network *net = load_network(CFG_PATH, WEIGHT_PATH, 0);
    image img = load_image("darknet/data/eagle.jpg", 0, 0, 3);
    image sized = resize_image(img, net->w, net->h);
    layer l = net->layers[net->n-1];
    show_image(img,"Original");

    int nboxes = 0;
    image **alphabet = load_alphabet();
    network_predict(net, sized.data);
    detection *dets = get_network_boxes(net, img.w, img.h, 0.005, 0, 0, 1, &nboxes);
    draw_detections(img, dets, l.side*l.side*l.n, 0.005, voc_names, alphabet, 20);
    save_image(img, "output");
    int max_i = 0;
    int max_j = 0;
    for(int i = 0; i < nboxes; i++){
        for(int j = 0; j < 20; j++){
            if(dets[i].prob[j] > dets[max_i].prob[max_j]){
                max_i = i;
                max_j = j;
            }
        }
    }

    float x = dets[max_i].bbox.x;
    float y = dets[max_i].bbox.y;
    float w = dets[max_i].bbox.w;
    float h = dets[max_i].bbox.h;

    char *label = voc_names[max_j];

    cout << label << " at: (" << x << ", " << y << ")" << " [" << dets[max_i].prob[max_j] << "]\n";

    Rect r=Rect(x, y, w, h);
    Mat cv2_img= Mat(img.h, img.w, CV_32F, img.data);
    namedWindow("Origin");
    imshow("Origin", cv2_img);
    // rectangle(cv2_img, r, Scalar(255, 0, 0), 1, 8, 0);
    // namedWindow("Box");
    // imshow("Box", cv2_img);



    // float *data = img.ptr<float>();
    // network_predict(net, data);
}


void setup(void) {
    wiringPiSetup();
    pinMode(BIN_OUT1, OUTPUT);
    pinMode(BIN_OUT2, OUTPUT);
    pinMode(BIN_OUT3, OUTPUT);
    pinMode(BIN_OUT4, OUTPUT);
    pinMode(BIN_OUT5, OUTPUT);
    pinMode(BIN_OUT6, OUTPUT);
    pinMode(BIN_OUT7, OUTPUT);
    pinMode(BIN_OUT8, OUTPUT);
    pinMode(INTERRUPT, OUTPUT);
}


void binary_out(float err) {
    digitalWrite(INTERRUPT, LOW);

    int out_val = (int) err * 127 + 127;
    int bin[8] = {0};
    for(int i = 0; i < 8; i++){
        bin[7-i] = out_val % 2;
        out_val /= 2;
    }

    digitalWrite(BIN_OUT1, bin[0]);
    digitalWrite(BIN_OUT2, bin[1]);
    digitalWrite(BIN_OUT3, bin[2]);
    digitalWrite(BIN_OUT4, bin[3]);
    digitalWrite(BIN_OUT5, bin[4]);
    digitalWrite(BIN_OUT6, bin[5]);
    digitalWrite(BIN_OUT7, bin[6]);
    digitalWrite(BIN_OUT8, bin[7]);

    digitalWrite(INTERRUPT, HIGH);
}


