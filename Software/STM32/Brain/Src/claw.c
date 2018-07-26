#include "claw.h"
#define MAX 300.0
#define MIN 80.0

/*
 * 0 <= degrees <= 180
 */
uint16_t get_pulse_length(uint16_t degrees)
{
    return (MAX - MIN) * (degrees) / 180.0 + MIN;
}

/*
 * htim must be initialized and started BEFORE executing this function
 */
void actuate(TIM_HandleTypeDef *htim, uint16_t tim_channel, uint16_t angle)
{
    __HAL_TIM_SET_COMPARE(htim, tim_channel, get_pulse_length(angle));
}

/*
 * assuming timCH1 is at 0 degrees initially,
 *          timCH2 is at 0 degrees initially
 */
void actuatengo(TIM_HandleTypeDef *htim, uint16_t tim_channel1, uint16_t tim_channel2)
{
    actuate(htim, tim_channel1, 135);
    actuate(htim, tim_channel2, 120);
    HAL_Delay(1000);
    actuate(htim, tim_channel1, 0);
    actuate(htim, tim_channel2, 0);
}