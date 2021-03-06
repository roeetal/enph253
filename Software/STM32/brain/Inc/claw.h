#ifndef _CLAW_H_
#define _CLAW_H_

#include "stm32f4xx_hal.h"
#include "main.h"
#include "extern_vars.h"

#define MAX 2400.0
#define MIN 450.0

uint16_t get_pulse_length(uint16_t degrees);
void actuate(TIM_HandleTypeDef *htim, uint16_t tim_channel, uint16_t angle);
void actuatengo(TIM_HandleTypeDef *htim, uint16_t tim_channel1, uint16_t tim_channel2);
void claw_init();
void basket_up_up_and_away(TIM_HandleTypeDef *htim);
void slow_actuate(TIM_HandleTypeDef *htim, uint16_t tim_channel, uint16_t start_angle, uint16_t end_angle);
void basket_init();
void arm_down(TIM_HandleTypeDef *htim);
void close_claw(TIM_HandleTypeDef *htim);
void open_claw(TIM_HandleTypeDef *htim);
void arm_up_to_deg(TIM_HandleTypeDef *htim, uint8_t deg);
void arm_up(TIM_HandleTypeDef *htim);

#endif
 
