
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  ** This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * COPYRIGHT(c) 2018 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f1xx_hal.h"
#include "adc.h"
#include "dma.h"
#include "i2c.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* USER CODE BEGIN Includes */
#include "fonts.h"
#include "ssd1306.h"
#include "encoder.h"
#include "pid.h"
#include "filter.h"
#include "extern_vars.h"
#include <String.h>
#include "claw.h"

/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/
uint16_t LEFT_SPEED = 41000;
uint16_t RIGHT_SPEED = 45000;
uint32_t dma_buffer[1024];
uint32_t ir_values[1024];
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/
void print(char msg[], int row);
void do_pid(PID_t *pid_struct);
PID_t menu();
void frequency_comparison(uint16_t freq1, uint16_t freq2, uint16_t GPIO_Pin);
void pi_navigation();
float calculate_heading(uint32_t adc_val);
void encoder_pid(PID_t *enc_pid, ENCODER_t *left_enc, ENCODER_t *right_enc);
void set_motor_speed(uint32_t channel, uint32_t speed);
void turn();
void turn_deg(uint8_t);
void alarm_detect();

/* USER CODE END PFP */

/* USER CODE BEGIN 0 */
/**
 * @brief Circu;ar DMA buffer loading on each full buffer
 * DMA uses dma_buffer, transfers data to ir_values for us to use
 * Order of buffer: ir1, ir2, ir1, ir2...
 */
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
    memcpy(ir_values, dma_buffer, sizeof(dma_buffer));
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  *
  * @retval None
  */
int main(void)
{
    /* USER CODE BEGIN 1 */

    /* USER CODE END 1 */

    /* MCU Configuration----------------------------------------------------------*/

    /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
    HAL_Init();

    /* USER CODE BEGIN Init */

    /* USER CODE END Init */

    /* Configure the system clock */
    SystemClock_Config();

    /* USER CODE BEGIN SysInit */

    /* USER CODE END SysInit */

    /* Initialize all configured peripherals */
    MX_GPIO_Init();
    MX_DMA_Init();
    MX_TIM4_Init();
    MX_TIM3_Init();
    MX_TIM1_Init();
    MX_ADC1_Init();
    MX_TIM2_Init();
    MX_USART2_UART_Init();
    MX_ADC2_Init();
    MX_I2C1_Init();
    /* USER CODE BEGIN 2 */

    /* Initialize peripherals */
    HAL_TIM_Encoder_Start(&htim3, TIM_CHANNEL_ALL);
    HAL_TIM_Encoder_Start(&htim4, TIM_CHANNEL_ALL);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_4);
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_3);
    ssd1306_Init();
    print("Starting...", 0);

    /* Initialize other stuffs*/
    // 3 * gain * kp = 20,000
    ENCODER_t left_enc = encoder_Init(TIM3);
    ENCODER_t right_enc = encoder_Init(TIM4);
    PID_t left_pid = pid_Init(1000, 250, 0, 2, 2);
    PID_t right_pid = pid_Init(6000, 500, 0, 2, 2);
    char msg[18] = "";

    //PID_t pid_struct = menu();
    /* USER CODE END 2 */

    /* Infinite loop */
    /* USER CODE BEGIN WHILE */

    while (1)
    {
        print("set to I to go", 0);
        while (HAL_GPIO_ReadPin(MENU_GPIO_Port, MENU_Pin) == GPIO_PIN_RESET)
            ;
        print("set to 0 to stop", 0);

        claw_init(&htim2);
        uint8_t ewok_cnt = 0;
        set_motor_speed(TIM_CHANNEL_1, LEFT_SPEED);
        set_motor_speed(TIM_CHANNEL_3, RIGHT_SPEED);

        int start = HAL_GetTick();
        while (HAL_GetTick() - start < 6000)
        {
            sprintf(msg, "TIM3->CNT: %d", TIM3->CNT);
            print(msg, 0);
            sprintf(msg, "TIM4->CNT: %d", TIM4->CNT);
            print(msg, 3);
            HAL_Delay(50);
        }
        set_motor_speed(TIM_CHANNEL_1, 0);
        set_motor_speed(TIM_CHANNEL_3, 0);
        HAL_Delay(1000);
        CLAW_INT_STATE = NOT_FLAGGED;
        PI_INT_STATE = NOT_FLAGGED;

        while (HAL_GPIO_ReadPin(MENU_GPIO_Port, MENU_Pin) == GPIO_PIN_SET)
        {

            if (PI_INT_STATE == FLAGGED)
            {
                print("in pi int", 0);
                turn();
                CLAW_INT_STATE = NOT_FLAGGED;

                set_motor_speed(TIM_CHANNEL_1, LEFT_SPEED);
                set_motor_speed(TIM_CHANNEL_3, RIGHT_SPEED);
                int start = HAL_GetTick();
                while (HAL_GetTick() - start < 4000)
                {
                    // encoder_dist_pid(&left_pid);
                    if (CLAW_INT_STATE == FLAGGED)
                    {
                        HAL_Delay(300);
                        set_motor_speed(TIM_CHANNEL_1, 0);
                        set_motor_speed(TIM_CHANNEL_3, 0);
                        actuatenNo(&htim2, TIM_CHANNEL_2, TIM_CHANNEL_3);
                        ++ewok_cnt;
                        msg[18] = "";
                        sprintf(msg, "wok_cnt: %d", ewok_cnt);
                        print(msg, 0);
                        if (ewok_cnt == 1)
                        {
                            arm_up_to_deg(&htim2, 80);
                            turn_deg(-150);
                            set_motor_speed(TIM_CHANNEL_1, LEFT_SPEED);
                            set_motor_speed(TIM_CHANNEL_3, RIGHT_SPEED);
                            HAL_Delay(2500);
                            arm_down(&htim2);
                        }
                        if (ewok_cnt == 2)
                        {
                            arm_up_to_deg(&htim2, 80);
                            turn_deg(-120);
                            alarm_detect();
                            print("haha yes", 0);
                            set_motor_speed(TIM_CHANNEL_1, LEFT_SPEED);
                            set_motor_speed(TIM_CHANNEL_3, RIGHT_SPEED + 5000);
                            HAL_Delay(1500);
                        }
                        CLAW_INT_STATE = NOT_FLAGGED;
                        break;
                    }
                }
                // char pic_plz = "1";
                // HAL_UART_Transmit(&huart2, pic_plz, sizeof(pic_plz), 10000);
                PI_INT_STATE = NOT_FLAGGED;
                set_motor_speed(TIM_CHANNEL_1, 0);
                set_motor_speed(TIM_CHANNEL_3, 0);
            }
        }

        /* USER CODE END WHILE */

        /* USER CODE BEGIN 3 */
    }
    /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{

    RCC_OscInitTypeDef RCC_OscInitStruct;
    RCC_ClkInitTypeDef RCC_ClkInitStruct;
    RCC_PeriphCLKInitTypeDef PeriphClkInit;

    /**Initializes the CPU, AHB and APB busses clocks 
    */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = 16;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI_DIV2;
    RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL16;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        _Error_Handler(__FILE__, __LINE__);
    }

    /**Initializes the CPU, AHB and APB busses clocks 
    */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
    {
        _Error_Handler(__FILE__, __LINE__);
    }

    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
    PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
    {
        _Error_Handler(__FILE__, __LINE__);
    }

    /**Configure the Systick interrupt time 
    */
    HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq() / 1000);

    /**Configure the Systick 
    */
    HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

    /* SysTick_IRQn interrupt configuration */
    HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

/* USER CODE BEGIN 4 */

/*
 * Assume motors are not on.
 * Reads adc, turns left or right based on voltage. Left max= 0 , no turn = 1.65 V, right max = 3.3V.
 */
void turn()
{
    HAL_ADC_Start(&hadc2);
    if (HAL_ADC_PollForConversion(&hadc2, 100) == HAL_OK)
    {
        float volts = calculate_heading(HAL_ADC_GetValue(&hadc2));
        uint16_t counts = TURN_CONST * fabs(volts);
        TIM3->CNT = 0;
        TIM4->CNT = 0;

        char msg[18] = "";
        sprintf(msg, "cnts: %d", counts);
        print(msg, 0);
        int pre_dec = (int)(volts / 1);
        int post_dec = (int)((volts - pre_dec) * 1000);
        sprintf(msg, "vlts: %d.%d", pre_dec, post_dec);
        print(msg, 2);

        if (volts < -TURN_TOLERANCE) // FIXME: Ben changed this
        {
            set_motor_speed(TIM_CHANNEL_1, 0);
            set_motor_speed(TIM_CHANNEL_3, RIGHT_SPEED);
            while (TIM4->CNT < counts)
            {
                sprintf(msg, "TIM4->CNT: %d", TIM4->CNT);
                print(msg, 4);
            }
            sprintf(msg, "TIM4->CNT: %d", TIM4->CNT);
            print(msg, 4);
            TIM4->CNT = 0;
        }
        else if (volts > TURN_TOLERANCE) // FIXME: Ben changed this
        {
            set_motor_speed(TIM_CHANNEL_1, LEFT_SPEED);
            set_motor_speed(TIM_CHANNEL_3, 0);
            while (TIM3->CNT < counts)
            {
                sprintf(msg, "TIM3->CNT: %d", TIM3->CNT);
                print(msg, 4);
            }
            sprintf(msg, "TIM3->CNT: %d", TIM3->CNT);
            print(msg, 4);
            TIM3->CNT = 0;
        }
        set_motor_speed(TIM_CHANNEL_1, 0);
        set_motor_speed(TIM_CHANNEL_3, 0);
    }
    else
    {
        print("Shits fucked yo", 0);
    }
    HAL_ADC_Stop(&hadc2);
}

/*
 * Assume motors are not on.
 * Reads adc, turns left or right based on voltage. Left max= 0 , no turn = 1.65 V, right max = 3.3V.
 */
void turn_deg(uint8_t deg)
{
    HAL_ADC_Start(&hadc2);
    if (HAL_ADC_PollForConversion(&hadc2, 100) == HAL_OK)
    {
        uint16_t counts = 50.0 / 90.0 * (deg - 90) + 50;
        TIM3->CNT = 0;
        TIM4->CNT = 0;

        char msg[18] = "";

        if (deg > 0) // FIXME: Ben changed this
        {
            set_motor_speed(TIM_CHANNEL_1, 0);
            set_motor_speed(TIM_CHANNEL_3, 30000);
            while (TIM4->CNT < counts)
            {
                sprintf(msg, "TIM4->CNT: %d", TIM4->CNT);
                print(msg, 4);
            }
            sprintf(msg, "TIM4->CNT: %d", TIM4->CNT);
            print(msg, 4);
            TIM4->CNT = 0;
        }
        else if (deg < 0) // FIXME: Ben changed this
        {
            set_motor_speed(TIM_CHANNEL_1, 30000);
            set_motor_speed(TIM_CHANNEL_3, 0);
            while (TIM3->CNT < counts)
            {
                sprintf(msg, "TIM3->CNT: %d", TIM3->CNT);
                print(msg, 4);
            }
            sprintf(msg, "TIM3->CNT: %d", TIM3->CNT);
            print(msg, 4);
            TIM3->CNT = 0;
        }
        set_motor_speed(TIM_CHANNEL_1, 0);
        set_motor_speed(TIM_CHANNEL_3, 0);
    }
    else
    {
        print("Shits fucked yo", 0);
    }
    HAL_ADC_Stop(&hadc2);
}

void pi_navigation()
{
    HAL_ADC_Start(&hadc2);
    print("Pi nav", 0);
    if (HAL_ADC_PollForConversion(&hadc2, 1000000) == HAL_OK)
    {
        float heading = calculate_heading(HAL_ADC_GetValue(&hadc2));
        if (heading < 0)
        {
            set_motor_speed(TIM_CHANNEL_2, 10000);
            set_motor_speed(TIM_CHANNEL_3, 10000);
            heading *= -1;
        }
        else if (heading > 0)
        {
            set_motor_speed(TIM_CHANNEL_1, 10000);
            set_motor_speed(TIM_CHANNEL_4, 10000);
        }
        HAL_Delay(2000 * heading);
        set_motor_speed(TIM_CHANNEL_1, 20000);
        set_motor_speed(TIM_CHANNEL_3, 20000);
    }
    else
    {
        print("No adc", 0);
    }
    HAL_ADC_Stop(&hadc2);
    PI_INT_STATE = NOT_FLAGGED;
}

/**
 * Resistor ladder: 8-bit, 0->3.3v.
 * ADC: 12-bit, 0->3.3V
 *
 * @param adc_val, value read from ADC.
 * @return heading as a percentage from -50 (left) to 50 (right).
 **/
float calculate_heading(uint32_t adc_val)
{
    return adc_val / 4096.0 - 0.5;
}

void alarm_detect()
{
    HAL_ADC_Start_DMA(&hadc1, dma_buffer, sizeof(dma_buffer) / sizeof(dma_buffer[0]));
    //TODO calculate time needed to fill first buffer
    HAL_Delay(100);
    while (goertzel(ir_values, 41912, 1000, sizeof(dma_buffer) / sizeof(dma_buffer[0])) < 10);
    while (goertzel(ir_values, 41912, 1000, sizeof(dma_buffer) / sizeof(dma_buffer[0])) > 100);
    HAL_ADC_Stop_DMA(&hadc1);
    print("Go", 0);
    IR_INT_STATE = NOT_FLAGGED;
}

/**
 * @brief prints string to row, rows from 0 - 6, resets screen when printing from row 0
 * @param msg, string to print
 * @param row, row to print msg to
 */
void print(char *msg, int row)
{
    if (row == 0)
    {
        ssd1306_Fill(Black);
    }
    ssd1306_SetCursor(0, row * 10);
    ssd1306_WriteString(msg, Font_7x10, White);
    ssd1306_UpdateScreen();
}

PID_t menu()
{
    print("Starting", 0);
    char msg[20] = "";
    int pid_select = 0;
    uint32_t values[3] = {0, 0, 0};
    while (1)
    {
        if (HAL_GPIO_ReadPin(MENU_GPIO_Port, MENU_Pin) == 0)
        {
            sprintf(msg, "%lu", values[pid_select]);
            print(msg, 0);
            TIM4->CNT = values[pid_select];
            while (HAL_GPIO_ReadPin(MENU_GPIO_Port, MENU_Pin) == 0)
            {
                values[pid_select] = TIM4->CNT;
                sprintf(msg, "%lu", values[pid_select]);
                print(msg, 0);
            }
            ++pid_select;
        }
        if (pid_select == 3)
            break;
    }
    while (1)
    {
        int speed = 20000;
        if (HAL_GPIO_ReadPin(MENU_GPIO_Port, MENU_Pin) == 0)
        {
            if (pid_select == 3)
            {
                HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
                HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_3);
            }
            else
            {
                HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);
                HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);
            }
            sprintf(msg, "%d", speed);
            print(msg, 0);
            TIM4->CNT = speed;
            while (HAL_GPIO_ReadPin(MENU_GPIO_Port, MENU_Pin) == 0)
            {
                speed = TIM4->CNT;
                sprintf(msg, "%d", speed);
                print(msg, 0);
                if (pid_select == 3)
                {
                    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, speed);
                    LEFT_SPEED = speed;
                }
                else
                {
                    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_3, speed);
                    RIGHT_SPEED = speed;
                }
            }
            ++pid_select;
        }
        if (pid_select == 5)
        {
            break;
        }
    }
    HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);
    HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_3);
    sprintf(msg, "P %lu", values[0]);
    print(msg, 0);
    sprintf(msg, "D %lu", values[1]);
    print(msg, 1);
    sprintf(msg, "I %lu", values[2]);
    print(msg, 2);
    sprintf(msg, "L %u", LEFT_SPEED);
    print(msg, 3);
    sprintf(msg, "R %u", RIGHT_SPEED);
    print(msg, 4);
    HAL_Delay(1000);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);
    return pid_Init(values[0], values[1], values[2], 5, 100);
}

void do_pid(PID_t *pid_struct)
{
    /* Read sensors */
    uint8_t left = HAL_GPIO_ReadPin(TAPE_LEFT_GPIO_Port, TAPE_LEFT_Pin) ? 0 : 1;
    uint8_t right = HAL_GPIO_ReadPin(TAPE_RIGHT_GPIO_Port, TAPE_RIGHT_Pin) ? 0 : 1;

    /* Get error */
    if (left && right)
    {
        pid_struct->err = 0;
    }
    else if (left && !right)
    {
        pid_struct->err = 1;
    }
    else if (!left && right)
    {
        pid_struct->err = -1;
    }
    else if (!left && !right && (pid_struct->err < 0))
    {
        pid_struct->err = -5;
    }
    else if (!left && !right && (pid_struct->err > 0))
    {
        pid_struct->err = 5;
    }

    /* Get gain */
    int16_t gain = pid_GetGain(pid_struct);
    char msg[20] = "";
    sprintf(msg, "%d", (int)gain);
    print(msg, 0);
    /* Set Motor Speeds*/
    int lspeed = LEFT_SPEED;
    int rspeed = RIGHT_SPEED;
    if (gain < 0)
    {
        lspeed = LEFT_SPEED - gain;
    }
    else if (gain > 0)
    {
        rspeed = RIGHT_SPEED + gain;
    }
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, lspeed);
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_3, rspeed);
}

void set_motor_speed(uint32_t channel, uint32_t speed)
{
    if (channel == TIM_CHANNEL_1)
    {
        __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, 0);
    }
    else if (channel == TIM_CHANNEL_2)
    {
        __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 0);
    }
    else if (channel == TIM_CHANNEL_3)
    {
        __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_4, 0);
    }
    else if (channel == TIM_CHANNEL_4)
    {
        __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_3, 0);
    }
    __HAL_TIM_SET_COMPARE(&htim1, channel, speed>65535?65535:speed);
}

void encoder_pid(PID_t *enc_pid, ENCODER_t *left_enc, ENCODER_t *right_enc)
{
    /* Get error */
    uint32_t lcnt = TIM3->CNT;
    uint32_t rcnt = TIM4->CNT;
    enc_pid->err = lcnt - rcnt;

    /* Get gain */
    int gain = pid_GetGain(enc_pid);
    char msg[18] = "";
    sprintf(msg, "ENC G: %d", gain);
    print(msg, 0);

    /* Set Motor Speeds*/
    int lspeed = LEFT_SPEED - gain;
    int rspeed = RIGHT_SPEED + gain;

    if(update_encoder(left_enc)==0 && update_encoder(right_enc)==0){
        lspeed += 10000;
        rspeed += 10000;
    }

    sprintf(msg, "LS: %d", lspeed);
    print(msg, 1);
    sprintf(msg, "RS: %d", rspeed);
    print(msg, 2);
    set_motor_speed(TIM_CHANNEL_1, lspeed);
    set_motor_speed(TIM_CHANNEL_3, rspeed);

    /* Prevent weird overflow shit */
    if(lcnt>60000 || rcnt>60000){
        TIM3->CNT -= 50000;
        TIM3->CNT -= 50000;
    }
}

void encoder_dist_pid(PID_t *pid_obj)
{
    /* Get error */
    uint32_t l_enc = TIM3->CNT;
    uint32_t r_enc = TIM4->CNT;
    pid_obj->err = l_enc - r_enc;

    /* Get gain */
    int32_t gain = pid_GetGain(pid_obj);

    // Print
    int l_predec = (int)(l_enc / 1);
    int l_postdec = (int)((l_enc - l_predec) * 1000);
    int r_predec = (int)(r_enc / 1);
    int r_postdec = (int)((r_enc - r_predec) * 1000);
    char msg[18] = "";
    sprintf(msg, "LG: %d.%d", l_predec, l_postdec);
    print(msg, 0);
    sprintf(msg, "RG: %d.%d", r_predec, r_postdec);
    print(msg, 1);
    // Print

    /* Set Motor Speeds*/
    int lspeed = LEFT_SPEED - gain;
    int rspeed = RIGHT_SPEED + gain;

    // Print
    sprintf(msg, "err: %d", (int)pid_obj->err);
    print(msg, 3);
    sprintf(msg, "gain: %d", (int)gain);
    print(msg, 4);
    // Print

    // set_motor_speed
    set_motor_speed(TIM_CHANNEL_1, lspeed);
    set_motor_speed(TIM_CHANNEL_3, rspeed);

    // Reset CNTs for overflows
    TIM4->CNT = 0;
    TIM3->CNT = 0;
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  file: The file name as string.
  * @param  line: The line in file as a number.
  * @retval None
  */
void _Error_Handler(char *file, int line)
{
    /* USER CODE BEGIN Error_Handler_Debug */
    /* User can add his own implementation to report the HAL error return state */
    while (1)
    {
    }
    /* USER CODE END Error_Handler_Debug */
}

#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
    /* USER CODE BEGIN 6 */
    /* User can add his own implementation to report the file name and line number, 
tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
    /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
