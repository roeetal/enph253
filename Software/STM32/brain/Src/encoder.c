#include"encoder.h"
#include"stm32f4xx.h"

ENCODER_t encoder_Init(){
    ENCODER_t enc;
    enc.cnt = 0;
    enc.time = 0;
    enc.speed = 0.0;
    return enc;    
}

/**
 * 16 bit uint for 16 bit timer
 * 16 bit uint for 16 bit count value
 * Speed of TIM9: 72MHz/65535
 * Angular velocity w = 2*pi*n/(N*t)*R
 * n: number of encoder counts
 * N: number of encoder counts in revolution
 * t: time
 * R: radius
 *
 *  dt is number of clock ticks, need to convert to time.
 *
 * @param   pointer to encoder struct
 * @param   pointer to encoder timer handle
 * @modifies    speed stored in struct
 * @returns    speed
 **/
float update_encoder(ENCODER_t *enc, TIM_HandleTypeDef *htim){
    uint16_t temp_time = TIM9->CNT;
    uint16_t temp_cnt = __HAL_TIM_GetCounter(htim);
    int16_t d_cnt;
    uint16_t d_t = temp_time<enc->time? temp_time+1+(enc->time^65535): temp_time - enc->time;
    //uint8_t dt = temp_time - time;
    if(enc->speed>0 && temp_cnt<enc->cnt){
        d_cnt = temp_cnt+1+(enc->cnt^65535);
    }else if(enc->speed<0 && temp_cnt>enc->cnt){
        d_cnt = -(enc->cnt+1+(temp_cnt^65535));
    }else if(temp_cnt>65000 && enc->cnt == 0 && enc->speed==0){
        d_cnt = -(1+(temp_cnt^65535));
    }else if(temp_cnt<500 && enc->cnt == 65535 && enc->speed==0){
        d_cnt = 1+temp_cnt;
    }else{
        d_cnt = temp_cnt - enc->cnt;
    }
    enc->speed = 2*3.14159265359*d_cnt/(ENCODER_N*d_t/(72000000.0/65535))*ENCODER_R;
    enc->cnt = temp_cnt;
    enc->time = temp_time; 
    return enc->speed;
}
