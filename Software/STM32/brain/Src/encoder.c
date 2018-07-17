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
 * 8 bit uint for 8 bit count value
 * Speed of TIM9: 72MHz/65535
 * Angular velocity w = 2*pi*n/(N*t)*R
 * n: number of encoder counts
 * N: number of encoder counts in revolution
 * t: time
 * R: radius
 *
 *  dt is number of clock ticks, need to convert to time.
 *
 * @params  encoder struct
 * @modifies    speed stored in struct
 * @returns    speed
 **/
float update_encoder(ENCODER_t *enc){
    uint16_t temp_time = TIM9->CNT;
    uint8_t temp_cnt = TIM4->CNT;
    int8_t d_cnt;
    uint16_t d_t = temp_time<enc->time? temp_time+1+(enc->time^255): temp_time - enc->time;
    //uint8_t dt = temp_time - time;
    if(enc->speed>0 && temp_cnt<enc->cnt){
        d_cnt = temp_cnt+1+(enc->cnt^255);
    }else if(enc->speed<0 && temp_cnt>enc->cnt){
        d_cnt = -(enc->cnt+1+(temp_cnt^255));
    }else if(temp_cnt>200 && enc->cnt == 0 && enc->speed==0){
        d_cnt = -(1+(temp_cnt^255));
    }else{
        d_cnt = temp_cnt - enc->cnt;
    }
    enc->speed = 2*3.14159265359*d_cnt/(ENCODER_N*d_t/(72000000.0/65535))*ENCODER_R;
    enc->cnt = temp_cnt;
    enc->time = temp_time; 
    return enc->speed;
}
