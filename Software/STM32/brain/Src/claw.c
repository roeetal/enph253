#include "claw.h"

/*
 * Initialize claw positions
 */
void claw_init(TIM_HandleTypeDef *htim)
{
    open_claw(htim);
    arm_down(htim);
    HAL_Delay(2000);
}

void basket_init(TIM_HandleTypeDef *htim)
{
    actuate(htim, TIM_CHANNEL_3, 10);
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
    __HAL_TIM_SET_COMPARE(htim, tim_channel, get_pulse_length(angle));
}

/*
 * assuming timCH1 is at 0 degrees initially,
 *          timCH2 is at 0 degrees initially
 */
void actuatengo(TIM_HandleTypeDef *htim, uint16_t channel_claw, uint16_t channel_arm)
{
    // Close claw
    actuate(htim, channel_claw, 80);
    HAL_Delay(1000);
    // Arm up
    actuate(htim, channel_arm, 0);
    HAL_Delay(1500);
    // Open Claw
    actuate(htim, channel_claw, 20);
    HAL_Delay(500);
    // Lower arm gently
    slow_actuate(htim, channel_arm, 0, 160);
    HAL_Delay(1000);
    // Open claw again
    open_claw(htim);
}

void close_claw(TIM_HandleTypeDef *htim)
{
    actuate(htim, TIM_CHANNEL_1, 80);
    HAL_Delay(1000);
}

void open_claw(TIM_HandleTypeDef *htim)
{
    actuate(htim, TIM_CHANNEL_1, 0);
    HAL_Delay(200);
    actuate(htim, TIM_CHANNEL_1, 10);
    HAL_Delay(200);
}

void arm_down(TIM_HandleTypeDef *htim)
{
    actuate(htim, TIM_CHANNEL_2, 100);
    HAL_Delay(1000);
}

void slow_actuate(TIM_HandleTypeDef *htim, uint16_t tim_channel, uint16_t start_angle, uint16_t end_angle)
{
    uint16_t start = start_angle;
    uint8_t increment;
    if (start_angle < end_angle)
    {
        increment = 5;
    }
    else
    {
        increment = -5;
    }

    while (start < end_angle)
    {
        actuate(htim, tim_channel, start);
        HAL_Delay(100);
        start = start + increment;
    }
}

/*
 * Raise then lower the basket
 */
void basket_up_up_and_away(TIM_HandleTypeDef *htim)
{
    slow_actuate(htim, TIM_CHANNEL_3, 10, 180);
    HAL_Delay(1000);
    actuate(htim, TIM_CHANNEL_3, 10);
    HAL_Delay(1000);
}
