#ifndef _CLAW_H_
#define _CLAW_H_

#include "stm32f1xx_hal.h"
#include "main.h"

#define MAX 300.0
#define MIN 80.0

uint16_t get_pulse_length(uint16_t degrees);
void actuate(TIM_HandleTypeDef *htim, uint16_t tim_channel, uint16_t angle);
void actuatengo(TIM_HandleTypeDef *htim, uint16_t tim_channel1, uint16_t tim_channel2);
void actuatenNo(TIM_HandleTypeDef *htim, uint16_t channel_claw, uint16_t channel_arm);
void claw_init();
void basket_up_up_and_away(TIM_HandleTypeDef *htim);
void slow_actuate(TIM_HandleTypeDef *htim, uint16_t tim_channel, uint16_t start_angle, uint16_t end_angle);
void basket_init();
void arm_down(TIM_HandleTypeDef *htim);
void arm_up (TIM_HandleTypeDef *htim);
void arm_up_to_deg(TIM_HandleTypeDef *htim, uint8_t deg);
void close_claw(TIM_HandleTypeDef *htim);
void open_claw(TIM_HandleTypeDef *htim);

#endif
 