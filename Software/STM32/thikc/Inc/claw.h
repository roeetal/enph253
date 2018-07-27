#ifndef _CLAW_H_
#define _CLAW_H_

#include "stm32f1xx_hal.h"
#include "main.h"

#define MAX 300.0
#define MIN 80.0

uint16_t get_pulse_length(uint16_t degrees);
void actuate(TIM_HandleTypeDef *htim, uint16_t tim_channel, uint16_t angle);
void actuatengo(TIM_HandleTypeDef *htim, uint16_t tim_channel1, uint16_t tim_channel2);
void claw_init();
void basket_init();
void basket_up_up_and_away(TIM_HandleTypeDef *htim);
void slow_actuate(TIM_HandleTypeDef *htim, uint16_t tim_channel, uint16_t start_angle, uint16_t end_angle);

#endif
 