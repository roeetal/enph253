#include <stdio.h>
#include <stdlib.h>
#include <darknet.h>
#include <wiringPi.h>
#include <raspicam_cv.h>


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

#define CFG_PATH "darkstuff/cfg/yolo-tiny-253.cfg"
#define WEIGHT_PATH "darkstuff/yolo-tiny-253.weights"


void setup(void);
void binary_out(float err);


int main(void) {
    network *net = load_network(CFG_PATH, WEIGHT_PATH, 0);
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


