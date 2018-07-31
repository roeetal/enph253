#include"pid.h"
#include"stm32f1xx.h"

PID_t pid_Init(uint16_t kp, uint16_t kd, uint16_t ki, int16_t i_max, uint16_t g){
    PID_t PID;
    PID.kp = kp;
    PID.kd = kd;
    PID.ki = ki;
    PID.i = 0;
    PID.i_max = i_max;
    PID.gain = g;
    PID.err = 0;
    PID.count = 0;

    return PID;
}

int32_t pid_GetGain(PID_t *PID){
    /* Get current values for P, I, and D */
    PID->count += 10;
    int32_t p = (int) PID->kp * PID->err;
    int32_t d = (int) PID->kd * PID->err / (PID->count);  // / 1000 to get seconds
    PID->i = PID->i + PID->ki * PID->err;
    
    /* Cap absolute value of integral control to keep it stable */
    if(PID->i < -1 * PID->i_max){ PID->i = -1 * PID->i_max; }
    if(PID->i > PID->i_max){ PID->i = PID->i_max; }

    /* Reset derivative term if err returns to 0 */
    if(PID->err == 0){ PID->count = 0; }
    
    return (PID->i + p + d) * PID->gain;
}
