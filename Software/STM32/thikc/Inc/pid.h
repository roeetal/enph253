#include"stm32f1xx_hal.h"

#ifndef pid
#define pid

typedef struct{
    uint8_t kp;
    uint8_t kd;
    uint8_t ki;
    int16_t i;
    int8_t i_max;
    uint8_t gain;
    int16_t err;
    int16_t count;
} PID_t;

PID_t pid_Init(uint8_t kp, uint8_t kd, uint8_t ki, int8_t i_max, uint8_t g);
int16_t pid_GetGain(PID_t *PID);

#endif
