#include"stm32f4xx_hal.h"

#ifndef encoder
#define encoder 

#define ENCODER_N 100
#define ENCODER_R 0.5

typedef struct{
    int cnt;
    int time;
    double speed;
} ENCODER_t;

ENCODER_t encoder_Init();
double update_encoder(ENCODER_t *enc);

#endif
