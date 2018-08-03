
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
#include "stm32f4xx_hal.h"
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
uint16_t LEFT_SPEED = 0.55 * MOTOR_SPEED;
uint16_t RIGHT_SPEED = 0.60 * MOTOR_SPEED;
uint32_t dma_buffer[1];
uint32_t adc_values[1];

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
void encoder_pid(PID_t *left_pid, ENCODER_t *left_enc, PID_t *right_pid, ENCODER_t *right_enc);
void set_motor_speed(uint32_t channel, uint32_t speed);
void turn();
void turn_deg(uint8_t);
void alarm_detect();

/* USER CODE END PFP */

/* USER CODE BEGIN 0 */
/**
 * @brief Circu;ar DMA buffer loading on each full buffer
 * DMA uses dma_buffer, transfers data to adc_values for us to use
 * Order of buffer: ir1, ir2, pi, ir1, ir2...
 */
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
    memcpy(adc_values, dma_buffer, sizeof(dma_buffer));
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
    MX_TIM3_Init();
    MX_TIM1_Init();
    MX_TIM2_Init();
    MX_USART6_UART_Init();
    MX_ADC1_Init();
    MX_I2C1_Init();
    MX_TIM4_Init();
    MX_TIM9_Init();
    MX_TIM5_Init();
    /* USER CODE BEGIN 2 */

    /* Initialize peripherals */
    /* 
     * Claw jaw: TIM3 CH1
     * Claw arm: TIM3 CH2
     * Basket: TIM3 CH3
     * left forward: TIM1 CH1
     * left backward: TIM1 CH2
     * right forward: TIM1 CH3
     * right backward: TIM1 CH4
     * left encoder: TIM4
     * right encoderL TIM5
     * spare clocks: TIM2, TIM9
     */

    HAL_TIM_Encoder_Start(&htim4, TIM_CHANNEL_ALL);
    HAL_TIM_Encoder_Start(&htim5, TIM_CHANNEL_ALL);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_4);
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_3);

    /* Initialize other stuffs */
    ssd1306_Init();
    print("Starting", 0);
    claw_init(&htim3);
    ///basket_init(&htim3);

    /*
    ENCODER_t left_enc = encoder_Init(TIM4);
    ENCODER_t right_enc = encoder_Init(TIM5);
    PID_t left_pid = pid_Init(5, 12, 0, 2, 2);
    PID_t right_pid = pid_Init(30, 25, 0, 2, 2);
    */
    uint8_t ewok_cnt = 0;

    // set_motor_speed(TIM_CHANNEL_1, LEFT_SPEED);
    // set_motor_speed(TIM_CHANNEL_3, RIGHT_SPEED);
    // HAL_Delay(5000);
    // set_motor_speed(TIM_CHANNEL_1, 0);
    // set_motor_speed(TIM_CHANNEL_3, 0);
    /* Initially disabled IR, PI and Claw INT*/
    HAL_NVIC_EnableIRQ(EXTI0_IRQn);
    HAL_NVIC_EnableIRQ(EXTI4_IRQn);
    // HAL_NVIC_EnableIRQ(EXTI1_IRQn); IR INT
    HAL_Delay(500);
    HAL_ADC_Start_DMA(&hadc1, dma_buffer, sizeof(dma_buffer) / sizeof(dma_buffer[0]));
    /* USER CODE END 2 */

    /* Infinite loop */
    /* USER CODE BEGIN WHILE */
    while (1)
    {
        /*
         * Servo Stuff
         */
        // actuatengo(&htim3, TIM_CHANNEL_2, TIM_CHANNEL_3);
        // HAL_Delay(500);
        /*
         * Drive Straight
         *
         encoder_pid(&left_pid, &left_enc, &right_pid, &right_enc);
         */
        char msg[15] = "";
        HAL_UART_Receive(&huart6, (uint8_t *)msg, sizeof(msg), HAL_MAX_DELAY);
        print(msg, 0);
        HAL_Delay(1000);
        print("refresh", 0);

        /*
         * Pi Turning
        //  */
        // if (PI_INT_STATE == FLAGGED)
        // {
        //     print("in pi int", 0);
        //     turn();

        //     // set_motor_speed(TIM_CHANNEL_1, LEFT_SPEED);
        //     // set_motor_speed(TIM_CHANNEL_3, RIGHT_SPEED);
        //     // int start = HAL_GetTick();
        //     // while (HAL_GetTick() - start < 4000)
        //     // {
        //     //     // encoder_dist_pid(&left_pid);
        //     //     if (CLAW_INT_STATE == FLAGGED)
        //     //     {
        //     //         HAL_Delay(200);
        //     //         set_motor_speed(TIM_CHANNEL_1, 0);
        //     //         set_motor_speed(TIM_CHANNEL_3, 0);
        //     //         actuatengo(&htim3, TIM_CHANNEL_2, TIM_CHANNEL_3);
        //     //         CLAW_INT_STATE = NOT_FLAGGED;
        //     //         ++ewok_cnt;
        //     //         char msg[18] = "";
        //     //         sprintf(msg, "wok_cnt: %d", ewok_cnt);
        //     //         print(msg, 0);
        //     //         // if (ewok_cnt == 1)
        //     //         // {
        //     //         //     turn_deg(-120);
        //     //         //     arm_up_to_deg(&htim3, 80);
        //     //         //     set_motor_speed(TIM_CHANNEL_1, LEFT_SPEED);
        //     //         //     set_motor_speed(TIM_CHANNEL_3, RIGHT_SPEED);
        //     //         //     HAL_Delay(3000);
        //     //         //     CLAW_INT_STATE = NOT_FLAGGED;
        //     //         // }
        //     //         // if (ewok_cnt == 2)
        //     //         // {
        //     //         //     close_claw(&htim3);
        //     //         //     arm_up_to_deg(&htim3, 80);
        //     //         //     turn_deg(-120);
        //     //         //     alarm_detect();
        //     //         //     set_motor_speed(TIM_CHANNEL_1, LEFT_SPEED);
        //     //         //     set_motor_speed(TIM_CHANNEL_3, RIGHT_SPEED);
        //     //         //     HAL_Delay(1000);
        //     //         //     CLAW_INT_STATE = NOT_FLAGGED;
        //     //         // }
        //     //         break;
        //     //     }
        //     // }
        //     PI_INT_STATE = NOT_FLAGGED;
            // set_motor_speed(TIM_CHANNEL_1, 0);
            // set_motor_speed(TIM_CHANNEL_3, 0);
        //}

        /*
         * EDGE and Object detection
         *
         if(EDGE_RIGHT_STATE == FLAGGED){
         print("ER",0);
         EDGE_RIGHT_STATE = NOT_FLAGGED;
         }
         if(TAPE_RIGHT_STATE == FLAGGED){
         print("TR",0);
         TAPE_RIGHT_STATE = NOT_FLAGGED;
         }
         if(EDGE_LEFT_STATE == FLAGGED){
         print("EL",0);
         EDGE_LEFT_STATE = NOT_FLAGGED;
         }

         if(TAPE_LEFT_STATE == FLAGGED){
         print("TL",0);
         TAPE_LEFT_STATE = NOT_FLAGGED;
         }
         */

        /*
         * IR DETECTION
         */
        // print("preprint", 0);
        // if (IR_INT_STATE == FLAGGED)
        // {
        //     print("in ir int", 0);
        //     alarm_detect();
        //     set_motor_speed(TIM_CHANNEL_1, LEFT_SPEED);
        //     set_motor_speed(TIM_CHANNEL_3, RIGHT_SPEED);
        //     //drive past sensor, enough so as to not trigger interrupt again
        //     HAL_Delay(2000);
        // }

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

    /**Configure the main internal regulator output voltage 
    */
    __HAL_RCC_PWR_CLK_ENABLE();

    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

    /**Initializes the CPU, AHB and APB busses clocks 
    */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = 16;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
    RCC_OscInitStruct.PLL.PLLM = 8;
    RCC_OscInitStruct.PLL.PLLN = 72;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLQ = 4;
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
    HAL_ADC_Start_DMA(&hadc1, dma_buffer, sizeof(dma_buffer) / sizeof(dma_buffer[0]));
    //TODO calculate time needed to fill first buffer
    HAL_Delay(50);
    float volts = calculate_heading(adc_values[0]);
    uint16_t counts = TURN_CONST * fabs(volts);
    TIM4->CNT = 0;
    TIM5->CNT = 0;

    char msg[18] = "";
    sprintf(msg, "cnts: %d", counts);
    print(msg, 0);
    int pre_dec = (int)(volts / 1);
    int post_dec = (int)((volts - pre_dec) * 1000);
    sprintf(msg, "vlts: %d.%d", pre_dec, post_dec);
    print(msg, 2);
    sprintf(msg, "adc_val: %d", adc_values[0]);
    print(msg, 3);

    if (volts < -TURN_TOLERANCE)
    {
        set_motor_speed(TIM_CHANNEL_1, 0);
        set_motor_speed(TIM_CHANNEL_3, 500);
        while (TIM5->CNT < counts)
        {
            sprintf(msg, "TIM5->CNT: %lu", TIM5->CNT);
            print(msg, 4);
        }
        sprintf(msg, "TIM5->CNT: %lu", TIM5->CNT);
        print(msg, 4);
        TIM5->CNT = 0;
    }
    else if (volts > TURN_TOLERANCE)
    {
        set_motor_speed(TIM_CHANNEL_1, 500);
        set_motor_speed(TIM_CHANNEL_3, 0);
        while (TIM4->CNT < counts)
        {
            sprintf(msg, "TIM4->CNT: %lu", TIM4->CNT);
            print(msg, 4);
        }
        sprintf(msg, "TIM4->CNT: %lu", TIM4->CNT);
        print(msg, 4);
        TIM4->CNT = 0;
    }
    set_motor_speed(TIM_CHANNEL_1, 0);
    set_motor_speed(TIM_CHANNEL_3, 0);
    HAL_ADC_Stop_DMA(&hadc1);
}

/*
 * Assume motors are not on.
 * Reads adc, turns left or right based on voltage. Left max= 0 , no turn = 1.65 V, right max = 3.3V.
 */
void turn_deg(uint8_t deg)
{
    HAL_ADC_Start_DMA(&hadc1, dma_buffer, sizeof(dma_buffer) / sizeof(dma_buffer[0]));
    uint16_t counts = 50.0 / 90.0 * (deg - 90) + 50;
    TIM4->CNT = 0;
    TIM5->CNT = 0;

    char msg[18] = "";

    if (deg > 0) // FIXME: Ben changed this
    {
        set_motor_speed(TIM_CHANNEL_1, 0);
        set_motor_speed(TIM_CHANNEL_3, 30000);
        while (TIM5->CNT < counts)
        {
            sprintf(msg, "TIM5->CNT: %lu", TIM5->CNT);
            print(msg, 4);
        }
        sprintf(msg, "TIM5->CNT: %lu", TIM5->CNT);
        print(msg, 4);
        TIM5->CNT = 0;
    }
    else if (deg < 0) // FIXME: Ben changed this
    {
        set_motor_speed(TIM_CHANNEL_1, 30000);
        set_motor_speed(TIM_CHANNEL_3, 0);
        while (TIM4->CNT < counts)
        {
            sprintf(msg, "TIM4->CNT: %lu", TIM4->CNT);
            print(msg, 4);
        }
        sprintf(msg, "TIM4->CNT: %lu", TIM4->CNT);
        print(msg, 4);
        TIM4->CNT = 0;
    }
    set_motor_speed(TIM_CHANNEL_1, 0);
    set_motor_speed(TIM_CHANNEL_3, 0);
    HAL_ADC_Stop_DMA(&hadc1);
}

void pi_navigation()
{
    print("Pi nav", 0);
    HAL_ADC_Start_DMA(&hadc1, dma_buffer, sizeof(dma_buffer) / sizeof(dma_buffer[0]));
    //TODO calculate time needed to fill first buffer
    HAL_Delay(500);
    float heading = calculate_heading(adc_values[5]);
    if (heading < 0)
    {
        set_motor_speed(TIM_CHANNEL_2, 0.3 * MOTOR_SPEED);
        set_motor_speed(TIM_CHANNEL_3, 0.3 * MOTOR_SPEED);
        heading *= -1;
    }
    else if (heading > 0)
    {
        set_motor_speed(TIM_CHANNEL_1, 0.3 * MOTOR_SPEED);
        set_motor_speed(TIM_CHANNEL_4, 0.3 * MOTOR_SPEED);
    }
    HAL_Delay(2000 * heading);
    set_motor_speed(TIM_CHANNEL_1, 0.3 * MOTOR_SPEED);
    set_motor_speed(TIM_CHANNEL_3, 0.3 * MOTOR_SPEED);
    HAL_ADC_Stop_DMA(&hadc1);
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
    //HAL_NVIC_DisableIRQ (IRQn_Type IRQn) disbale ir interrupt
    //char msg[20] = "";
    HAL_Delay(500);
    while (goertzel(adc_values, 24242, 1000, sizeof(dma_buffer) / sizeof(dma_buffer[0]), 0) < 30)
        ;
    while (goertzel(adc_values, 24242, 1000, sizeof(dma_buffer) / sizeof(dma_buffer[0]), 0) > 30)
        ;
    // {
    //     //
    //     // Sampling frequency: 72e6/2/(495*3)
    //     // offset 0 : IR1
    //     val = goertzel(adc_values, 24242, 1000, sizeof(dma_buffer) / sizeof(dma_buffer[0]), 0);
    //     /*
    //        int predec = (int)(val / 1);
    //        int postdec = (int)((val - predec) * 1000);
    //        sprintf(msg, "%d.%d\n", predec, postdec);
    //        HAL_UART_Transmit(&huart6, (uint8_t *)msg, strlen(msg), 0xFFFF);
    //        */

    //     //compare
    //     // 1000 Hz: ranges to about 60, 9850 Hz: ranges to 20
    //     if (val < 30)
    //     {
    //         break;
    //     }
    // }

    HAL_ADC_Stop_DMA(&hadc1);
    HAL_NVIC_DisableIRQ(EXTI1_IRQn);
    IR_INT_STATE = NOT_FLAGGED;
}

void frequency_comparison(uint16_t freq1, uint16_t freq2, uint16_t GPIO_Pin)
{
    uint16_t offset = GPIO_Pin == IR_1_Pin ? 0 : GPIO_Pin == IR_2_Pin ? 1 : 2;
    HAL_ADC_Start_DMA(&hadc1, dma_buffer, sizeof(dma_buffer) / sizeof(dma_buffer[0]));
    //TODO calculate time needed to fill first buffer
    HAL_Delay(500);
    //TODO figure out thresholds and what we want to look for
    while (1)
    {
        char msg[18] = "";
        // Sampling frequency: 10.6667e6/(2*(239.5+15))
        // freq one
        double val1 = goertzel(adc_values, 20956, freq1, sizeof(dma_buffer) / sizeof(dma_buffer[0]), offset);
        int predec = (int)(val1 / 1);
        int postdec = (int)((val1 - predec) * 1000);
        sprintf(msg, "%d.%d\n", predec, postdec);
        print(msg, 0);
        HAL_UART_Transmit(&huart6, (uint8_t *)msg, strlen(msg), 0xFFFF);
        //freq2
        double val2 = goertzel(adc_values, 20956, freq2, sizeof(dma_buffer) / sizeof(dma_buffer[0]), offset);
        predec = (int)(val2 / 1);
        postdec = (int)((val2 - predec) * 1000);
        sprintf(msg, "%d.%d", predec, postdec);
        HAL_UART_Transmit(&huart6, (uint8_t *)msg, strlen(msg), 0xFFFF);
        //compare
        if (val1 > val2)
        {
            break;
        }
    }
    HAL_ADC_Stop_DMA(&hadc1);
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
        int speed = 0.3 * MOTOR_SPEED;
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
                    set_motor_speed(TIM_CHANNEL_1, speed);
                    LEFT_SPEED = speed;
                }
                else
                {
                    set_motor_speed(TIM_CHANNEL_3, speed);
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
    set_motor_speed(TIM_CHANNEL_1, lspeed);
    set_motor_speed(TIM_CHANNEL_3, rspeed);
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
    __HAL_TIM_SET_COMPARE(&htim1, channel, speed);
}

void encoder_pid(PID_t *left_pid, ENCODER_t *left_enc, PID_t *right_pid, ENCODER_t *right_enc)
{
    /* Get error */
    float l_enc = update_encoder(left_enc);
    float r_enc = update_encoder(right_enc);
    left_pid->err = ENCODER_GOAL - l_enc;
    right_pid->err = ENCODER_GOAL - r_enc;

    /* Get gain */
    int32_t left_gain = pid_GetGain(left_pid);
    int32_t right_gain = pid_GetGain(right_pid);

    int l_predec = (int)(l_enc / 1);
    int l_postdec = (int)((l_enc - l_predec) * 1000);
    int r_predec = (int)(r_enc / 1);
    int r_postdec = (int)((r_enc - r_predec) * 1000);
    char msg[18] = "";
    sprintf(msg, "LG: %d.%d", l_predec, l_postdec);
    print(msg, 0);
    sprintf(msg, "RG: %d.%d", r_predec, r_postdec);
    print(msg, 1);

    /* Set Motor Speeds*/
    int lspeed = LEFT_SPEED + left_gain;
    int rspeed = RIGHT_SPEED + right_gain;

    sprintf(msg, "LS: %d", (int)lspeed);
    print(msg, 3);
    sprintf(msg, "RS: %d", (int)rspeed);
    print(msg, 4);
    // set_motor_speed
    set_motor_speed(TIM_CHANNEL_1, lspeed);
    set_motor_speed(TIM_CHANNEL_3, rspeed);
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
