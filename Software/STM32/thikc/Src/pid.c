#include"pid.h"
#include"stm32f1xx.h"

PID_t pid_Init(uint8_t kp, uint8_t kd, uint8_t ki, int8_t i_max, uint8_t g){
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

int16_t pid_GetGain(PID_t *PID){
    /* Get current values for P, I, and D */
    PID->count += 10;
    int16_t p = PID->kp * PID->err;
    int16_t d = PID->kd * PID->err / (PID->count);  // / 1000 to get seconds
    PID->i = PID->i + PID->ki * PID->err;
    
    /* Cap absolute value of integral control to keep it stable */
    if(PID->i < -1 * PID->i_max){ PID->i = -1 * PID->i_max; }
    if(PID->i > PID->i_max){ PID->i = PID->i_max; }

    /* Reset derivative term if err returns to 0 */
    if(PID->err == 0){ PID->count = 0; }
    
    return (PID->i + p + d) * PID->gain;
}
