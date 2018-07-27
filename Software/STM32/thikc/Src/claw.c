#include "claw.h"

/*
 * Initialize claw positions
 */
void claw_init(TIM_HandleTypeDef *htim)
{
    actuate(htim, TIM_CHANNEL_2, 41);
    HAL_Delay(2000);
    actuate(htim, TIM_CHANNEL_3, 110);
    HAL_Delay(2000);
}

/*
 * 0 <= degrees <= 180
 */
uint16_t get_pulse_length(uint16_t degrees)
{
    return (uint16_t)((MAX - MIN) * (degrees) / 180.0 + MIN);
}

/*
 * htim must be initialized and started BEFORE executing this function
 */
void actuate(TIM_HandleTypeDef *htim, uint16_t tim_channel, uint16_t angle)
{
    __HAL_TIM_SET_COMPARE(htim, tim_channel,  get_pulse_length(angle)
}

/*
 * assuming timCH1 is at 0 degrees initially,
 *          timCH2 is at 0 degrees initially
 */
void actuatengo(TIM_HandleTypeDef *htim, uint16_t channel_claw, uint16_t channel_arm)
{
    actuate(htim, channel_claw, 0);
    HAL_Delay(1000);
    actuate(htim, channel_arm, 0);
    HAL_Delay(1000);
    actuate(htim, channel_claw, 41);
    HAL_Delay(1000);
    actuate(htim, channel_arm, 100);
    HAL_Delay(1000);
}
