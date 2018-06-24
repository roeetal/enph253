#include"stm32f4xx_hal.h"

#ifndef pid
#define pid

typedef struct{
    int kp;
    int kd;
    int ki;
    double i;
    int i_max;
    int gain;
    int err;
    int count;
} PID_t;

int last_time;

PID_t pid_Init(int kp, int kd, int ki, int i_max, int g);
double pid_GetGain(PID_t *PID, TIM_HandleTypeDef *timer);

#endif