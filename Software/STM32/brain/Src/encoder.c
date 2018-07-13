#include"encoder.h"
#include"stm32f4xx.h"

ENCODER_t encoder_Init(){
    ENCODER_t enc;
    enc.cnt = 0;
    enc.time = 0;
    enc.speed = 0.0;
    return enc;    
}

double update_encoder(ENCODER_t *enc){
    int temp_time = TIM9->CNT;
    int temp_cnt = TIM5->CNT;
    enc->speed = 2*3.14159265359*(temp_cnt - enc->cnt)/(ENCODER_N*(temp_time - enc->time))*ENCODER_R;
    enc->cnt = temp_cnt;
    enc->time = temp_time; 
    return enc->speed;
}
