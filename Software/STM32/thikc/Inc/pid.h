#include"stm32f1xx_hal.h"

#ifndef pid
#define pid

typedef struct{
    uint16_t kp;
    uint16_t kd;
    uint16_t ki;
    int32_t i;
    int16_t i_max;
    uint16_t gain;
    int32_t err;
    uint32_t count;
} PID_t;

PID_t pid_Init(uint16_t kp, uint16_t kd, uint16_t ki, int16_t i_max, uint16_t g);
int32_t pid_GetGain(PID_t *PID);

#endif
