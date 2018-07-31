#include"stm32f1xx_hal.h"

#ifndef pid
#define pid

typedef struct{
    uint8_t kp;
    uint8_t kd;
    uint8_t ki;
    int16_t i;
    int8_t i_max;
    uint16_t gain;
    int32_t err;
    int16_t count;
} PID_t;

PID_t pid_Init(uint16_t kp, uint16_t kd, uint16_t ki, int16_t i_max, uint16_t g);
int32_t pid_GetGain(PID_t *PID);

#endif
