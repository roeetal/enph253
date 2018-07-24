#ifndef _CLAW_H_
#define _CLAW_H_

#include "stm32f1xx.h"
#include "main.h"

uint16_t get_pulse_length(uint16_t degrees);
void actuate(TIM_HandleTypeDef *htim, uint16_t tim_channel, uint16_t angle);
void actuatengo(TIM_HandleTypeDef *htim, uint16_t tim_channel1, uint16_t tim_channel2);

#endif