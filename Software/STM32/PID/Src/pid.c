
#include"pid.h"

PID_t pid_Init(uint16_t kp, uint16_t kd, uint16_t ki, uint16_t i_max, uint16_t g){
    PID_t PID;
    PID.kp = kp;
    PID.kd = kd;
    PID.ki = ki;
    PID.i = 0;
    PID.i_max = i_max;
    PID.gain = g;
    PID.last_time = 0;
    PID.last_err = 0;

    return PID;
}

double pid_GetGain(PID_t *PID, TIM_HandleTypeDef *timer){
    /* Get current values for P, I, and D */
    uint32_t current_time = __HAL_TIM_GetCounter(timer);
    double p = PID->kp * PID->curr_err;
    double d = 1000 * PID->kd * (PID->curr_err - PID->last_err) / (current_time - PID->last_time);  // * 1000 to get seconds
    PID->i = PID->i + PID->ki * PID->curr_err;
    
    /* Cap absolute value of integral control to keep it stable */
    if(PID->i < -1 * PID->i_max){ PID->i = -1 * PID->i_max; }
    if(PID->i > PID->i_max){ PID->i = PID->i_max; }

    /* Update struct vals */
    PID->last_err = PID->curr_err;
    PID->last_time = current_time;

    return (PID->i + p + d) * PID->gain;
}
