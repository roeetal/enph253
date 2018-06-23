#include"stm32f4xx_hal.h"

#ifndef pid
#define pid

typedef struct{
    uint16_t kp;
    uint16_t kd;
    uint16_t ki;
    double i;
    uint16_t i_max;
    uint16_t gain;
    uint32_t last_time;
    double last_err;
    double curr_err;
} PID_t;

int last_time;

PID_t pid_Init(uint16_t kp, uint16_t kd, uint16_t ki, uint16_t i_max, uint16_t g);
double pid_GetGain(PID_t *PID, TIM_HandleTypeDef *timer);

#endif