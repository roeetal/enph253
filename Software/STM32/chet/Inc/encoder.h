#include"stm32f1xx_hal.h"

#ifndef encoder
#define encoder 

#define ENCODER_N 100
#define ENCODER_R 0.6

typedef struct{
    uint16_t cnt;
    uint16_t time;
    float speed;
} ENCODER_t;

ENCODER_t encoder_Init();
float update_encoder(ENCODER_t *enc, TIM_HandleTypeDef *htim);

#endif
