#include"encoder.h"

/**
 * @brief structure use to store and calculate rotating speed of encoder
 * @param htim, internal timer used for counting encoder
 * @retval initialized struct
 */
ENCODER_t encoder_Init(TIM_TypeDef *htim){
    ENCODER_t enc;
    enc.cnt = 0;
    enc.time = 0;
    enc.speed = 0.0;
    enc.tim = htim;
    return enc;    
}

/**
 * @brief
 * 16 bit uint for 16 bit timer
 * 16 bit uint for 16 bit count value
 * GetTick returns milliseconds
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
 * @retval    speed
 **/
float update_encoder(ENCODER_t *enc){
    uint16_t temp_time = HAL_GetTick();
    uint16_t temp_cnt = (enc->tim)->CNT;
    int16_t d_cnt;
    uint16_t d_t = temp_time - enc->time;
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
    enc->speed = d_cnt/(ENCODER_N*d_t)*ENCODER_R;
    enc->cnt = temp_cnt;
    enc->time = temp_time; 
    return enc->speed;
}
