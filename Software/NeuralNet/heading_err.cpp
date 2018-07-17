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
#include <ctime>
#include <sys/time.h>


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

#define CFG_PATH "darknet/cfg/ewok-tiny.cfg"
#define WEIGHT_PATH "darknet/ewok-tiny_50000.weights"

using namespace cv;
using std::cout;

char *voc_names[] = {"ewok"};

void setup(void);
void binary_out(float err);


int main(void) {
    network *net = load_network(CFG_PATH, WEIGHT_PATH, 0);
    image img = load_image("Images/1397.jpg", 0, 0, 3);
    struct timeval stop, start;
    image sized = resize_image(img, net->w, net->h);

    layer l = net->layers[net->n-1];
    int nboxes = 0;
    gettimeofday(&start, NULL);
    network_predict(net, sized.data);
    detection *dets = get_network_boxes(net, img.w, img.h, 0.4, 0, 0, 1, &nboxes);
    gettimeofday(&stop, NULL);

    // do_nms_sort(dets, l.side*l.side*l.n, l.classes, 0.4);

    int max_i = 0;
    int max_j = 0;
    for(int i = 0; i < nboxes; i++){
        if(dets[i].prob[0] > dets[max_i].prob[0]){
            max_i = i;
        }
    }

    float x = dets[max_i].bbox.x;
    float y = dets[max_i].bbox.y;
    float w = dets[max_i].bbox.w;
    float h = dets[max_i].bbox.h;

    float time_diff = (stop.tv_sec + stop.tv_usec / 1000000.0) - (start.tv_sec + start.tv_usec/1000000.0);
    cout << "Ewok" << " found in " << time_diff << "s [" << dets[max_i].prob[max_j] << "]\n";
    draw_bbox(img, dets[max_i].bbox, 2, 255, 0, 0);
    save_image(img, "output");


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


