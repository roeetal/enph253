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
// uint16_t LEFT_SPEED = BASE_SPEED+SPEED_DIFF;
uint16_t LEFT_SPEED = BASE_SPEED;
uint16_t RIGHT_SPEED = BASE_SPEED;
uint32_t dma_buffer[2048];
uint32_t adc_values[2048];
int ewok_cnt = 0;
int searches = 0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/
void print(char msg[], int row);
PID_t menu();
float calculate_heading(uint32_t adc_val);
void encoder_pid(PID_t *enc_pid);
void set_motor_speed(uint32_t channel, uint32_t speed);
void turn();
void turn_deg(int16_t);
void alarm_detect();
void drive_straight(PID_t *enc_pid);
void drive_straight_time(PID_t *enc_pid, uint32_t lspeed, uint32_t rspeed, uint32_t millis);
void square_edge(PID_t *enc_pid);
int debounce_and_grab(PID_t *enc_pid);
void test_All();
void test_PWM_htim1();
void test_PWM_htim3();

/* USER CODE END PFP */

/* USER CODE BEGIN 0 */
/**
 * @brief Circu;ar DMA buffer loading on each full buffer
 * DMA uses dma_buffer, transfers data to adc_values for us to use
 * Order of buffer: ir1, pi, ir1, ...
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
    MX_ADC1_Init();
    MX_TIM4_Init();
    MX_TIM9_Init();
    MX_TIM5_Init();
    MX_I2C1_Init();
    MX_USART6_UART_Init();
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
    print("Starting\n", 0);
    claw_init(&htim3);
    // basket_init(&htim3);

    PID_t enc_pid = pid_Init(2, 0, 0, 2, 1);

    EDGE_LEFT_STATE = NOT_FLAGGED;
    EDGE_RIGHT_STATE = NOT_FLAGGED;

    drive_straight_time(&enc_pid, LEFT_SPEED+80, RIGHT_SPEED, 2000);

    /* Initially disabled interrupts and ADC */
    HAL_NVIC_EnableIRQ(PI_INT_EXTI_IRQn);
    HAL_NVIC_EnableIRQ(CLAW_INT_EXTI_IRQn);
    PI_INT_STATE = NOT_FLAGGED;
    CLAW_INT_STATE = NOT_FLAGGED;

    drive_straight_time(&enc_pid, LEFT_SPEED, RIGHT_SPEED+10, 5000);
    print("ewok\n", 0);

    /* USER CODE END 2 */

    /* Infinite loop */
    /* USER CODE BEGIN WHILE */
    while (1)
    {
        // /*
        //  * Pi Turning
        //  */

        if (PI_INT_STATE == FLAGGED)
        {
            // Stop Requesting Images
            print("Pi interrupt\n", 0);
            // Turn to ewok
            turn();

            // Drive forward w/ PID for 2.5 s, enable claw interrupt
            //    uint32_t start = HAL_GetTick();
            uint32_t time = HAL_GetTick();
            set_motor_speed(TIM_CHANNEL_1, LEFT_SPEED);
            set_motor_speed(TIM_CHANNEL_3, RIGHT_SPEED);
            while(HAL_GetTick()-time<2800){
                drive_straight(&enc_pid);
                // If we trip claw interrupt, debounce and grab it
                if (HAL_GPIO_ReadPin(CLAW_INT_GPIO_Port, CLAW_INT_Pin) == GPIO_PIN_SET)
                {
                    print("Claw interrupt\n", 0);
                    debounce_and_grab(&enc_pid);
                    break;
                }
            }
            set_motor_speed(TIM_CHANNEL_1, 0);
            set_motor_speed(TIM_CHANNEL_3, 0);
            print("ewok\n", 0);
            PI_INT_STATE = NOT_FLAGGED;

        }else{

            ++searches;
            if(searches == 4 && ewok_cnt == 0){
                debounce_and_grab(&encoder_pid);
            }
            else if(searches == 3 && ewok_cnt == 1){
                debounce_and_grab(&encoder_pid);
            }
            else if(searches == 8 && ewok_cnt == 2){
                debounce_and_grab(&encoder_pid);
            }else{
            uint32_t time = HAL_GetTick();
            while(HAL_GetTick()-time<2800 && PI_INT_STATE == NOT_FLAGGED);
            int thresh = ewok_cnt==1? 500:750;
            set_motor_speed(TIM_CHANNEL_1, LEFT_SPEED);
            set_motor_speed(TIM_CHANNEL_3, RIGHT_SPEED);
            time = HAL_GetTick();
            while(HAL_GetTick()-time<thresh){
                if (HAL_GPIO_ReadPin(CLAW_INT_GPIO_Port, CLAW_INT_Pin) == GPIO_PIN_SET)
                {
                    print("Claw interrupt\n", 0);
                    debounce_and_grab(&enc_pid);
                    break;
                }
                drive_straight(&enc_pid);
            }
            set_motor_speed(TIM_CHANNEL_1, 0);
            set_motor_speed(TIM_CHANNEL_3, 0);
            print("ewok\n", 0);
            }
        }
        /* 
        // Reset Claw Interrupt before driving forward
        print("MAIN | CLAW_INT UNFLAGGED\n", 0);
        while (HAL_GetTick()-start<3000)
        {
        drive_straight(&enc_pid);
        // If we trip claw interrupt, debounce and grab it
        if (HAL_GPIO_ReadPin(CLAW_INT_GPIO_Port, CLAW_INT_Pin) == SET)
        {
        print("Claw interrupt\n", 0);
        if(debounce_and_grab(ewok_cnt, enc_pid)) break;
        }
        else if (HAL_GPIO_ReadPin(EDGE_LEFT_GPIO_Port, EDGE_LEFT_Pin) == RESET)
        {
        set_motor_speed(TIM_CHANNEL_1, 0);
        set_motor_speed(TIM_CHANNEL_3, 0);
        print("Edge left triggered", 0);
        turn_deg(30);
        }
        else if (HAL_GPIO_ReadPin(EDGE_RIGHT_GPIO_Port, EDGE_RIGHT_Pin) == RESET){
        print("Edge right triggered", 0);
        set_motor_speed(TIM_CHANNEL_1, 0);
        set_motor_speed(TIM_CHANNEL_3, 0);
        turn_deg(-30);
        }
        }

        }
        else
        {

*/
        /*
         * Look for Ewok
         */
        // CLAW_INT_STATE = NOT_FLAGGED;

        // if (ewok_cnt == 4 || ewok_cnt == 5)
        // {
        //     if (EDGE_LEFT_STATE == FLAGGED && EDGE_RIGHT_STATE == NOT_FLAGGED)
        //     {
        //         drive_straight_time(&enc_pid, 0, RIGHT_SPEED - 100, 400);
        //         uint32_t temp_time = HAL_GetTick();
        //         while ((HAL_GetTick() - temp_time) < 3000 && PI_INT_STATE == NOT_FLAGGED);
        //     }
        //     else if (EDGE_LEFT_STATE == NOT_FLAGGED && EDGE_RIGHT_STATE == FLAGGED)
        //     {
        //         drive_straight_time(&enc_pid, LEFT_SPEED - 100, 0, 400);
        //         uint32_t temp_time = HAL_GetTick();
        //         while ((HAL_GetTick() - temp_time) < 3000 && PI_INT_STATE == NOT_FLAGGED);
        //     }
        // }
        /*

           drive_straight_time(&enc_pid, LEFT_SPEED - 100, RIGHT_SPEED - 100, 500);
           print("ewok\n", 0);
           if (HAL_GPIO_ReadPin(CLAW_INT_GPIO_Port, CLAW_INT_Pin) == SET)
           {
           print("Claw interrupt\n", 0);
           if(debounce_and_grab(ewok_cnt, enc_pid)) break;
           }
           else if (HAL_GPIO_ReadPin(EDGE_LEFT_GPIO_Port, EDGE_LEFT_Pin) == RESET)
           {
           set_motor_speed(TIM_CHANNEL_1, 0);
           set_motor_speed(TIM_CHANNEL_3, 0);
           print("Edge left triggered", 0);
           turn_deg(30);
           }
           else if (HAL_GPIO_ReadPin(EDGE_RIGHT_GPIO_Port, EDGE_RIGHT_Pin) == RESET){
           print("Edge right triggered", 0);
           set_motor_speed(TIM_CHANNEL_1, 0);
           set_motor_speed(TIM_CHANNEL_3, 0);
           turn_deg(-30);
           }
           uint32_t temp_time = HAL_GetTick();
           */
        /*
           else if (HAL_GPIO_ReadPin(EDGE_LEFT_GPIO_Port, EDGE_LEFT_Pin) == GPIO_PIN_RESET)
           {
           set_motor_speed(TIM_CHANNEL_1, 0);
           set_motor_speed(TIM_CHANNEL_3, 0);
           print("Edge left triggered", 0);
           turn_deg(30);
           }
           else if (HAL_GPIO_ReadPin(EDGE_RIGHT_GPIO_Port, EDGE_RIGHT_Pin) == GPIO_PIN_RESET){
           print("Edge right triggered", 0);
           set_motor_speed(TIM_CHANNEL_1, 0);
           set_motor_speed(TIM_CHANNEL_3, 0);
           turn_deg(-30);
           }
           */

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
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
        |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
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
    HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

    /**Configure the Systick 
    */
    HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

    /* SysTick_IRQn interrupt configuration */
    HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

/* USER CODE BEGIN 4 */

void square_edge(PID_t *enc_pid)
{
    while (1)
    {
        drive_straight(enc_pid);
        if (HAL_GPIO_ReadPin(EDGE_LEFT_GPIO_Port, EDGE_LEFT_Pin) == GPIO_PIN_RESET && HAL_GPIO_ReadPin(EDGE_RIGHT_GPIO_Port, EDGE_RIGHT_Pin) == GPIO_PIN_RESET)
        {
            break;
        }
        else if (HAL_GPIO_ReadPin(EDGE_LEFT_GPIO_Port, EDGE_LEFT_Pin) == GPIO_PIN_RESET || HAL_GPIO_ReadPin(EDGE_RIGHT_GPIO_Port, EDGE_RIGHT_Pin) == GPIO_PIN_RESET)
        {
            if (HAL_GPIO_ReadPin(EDGE_LEFT_GPIO_Port, EDGE_LEFT_Pin) == GPIO_PIN_RESET)
            {
                set_motor_speed(TIM_CHANNEL_1, 0);
            }
            if (HAL_GPIO_ReadPin(EDGE_RIGHT_GPIO_Port, EDGE_RIGHT_Pin) == GPIO_PIN_RESET)
            {
                set_motor_speed(TIM_CHANNEL_3, 0);
            }
        }
    }
}

void drive_straight_time(PID_t *enc_pid, uint32_t lspeed, uint32_t rspeed, uint32_t millis)
{
    uint32_t temp_time = HAL_GetTick();
    char msg[50] = "";
    uint8_t count = 0; // FIXME: Remove to speed up
    while ((HAL_GetTick() - temp_time) < millis)
    {
        if (HAL_GPIO_ReadPin(CLAW_INT_GPIO_Port, CLAW_INT_Pin) == GPIO_PIN_SET)
        {
            print("Claw interrupt\n", 0);
            if(debounce_and_grab(enc_pid)) break;
        }
        /*
           else if (HAL_GPIO_ReadPin(EDGE_LEFT_GPIO_Port, EDGE_LEFT_Pin) == GPIO_PIN_RESET)
           {
           set_motor_speed(TIM_CHANNEL_1, 0);
           set_motor_speed(TIM_CHANNEL_3, 0);
           print("Edge left triggered", 0);
           turn_deg(30);
           }
           else if (HAL_GPIO_ReadPin(EDGE_RIGHT_GPIO_Port, EDGE_RIGHT_Pin) == GPIO_PIN_RESET){
           print("Edge right triggered", 0);
           set_motor_speed(TIM_CHANNEL_1, 0);
           set_motor_speed(TIM_CHANNEL_3, 0);
           turn_deg(-30);
           }
           */
        else 
        {
            set_motor_speed(TIM_CHANNEL_1, lspeed);
            set_motor_speed(TIM_CHANNEL_3, rspeed);
            drive_straight(enc_pid);

        }
    }
    set_motor_speed(TIM_CHANNEL_1, 0);
    set_motor_speed(TIM_CHANNEL_3, 0);
}

void drive_straight(PID_t *enc_pid)
{
    encoder_pid(enc_pid);
    HAL_Delay(10);

}

/*
 * Assume motors are not on.
 * Reads adc, turns left or right based on voltage. Left max= 0 , no turn = 1.65 V, right max = 3.3V.
 */
void turn()
{
    HAL_ADC_Start_DMA(&hadc1, dma_buffer, sizeof(dma_buffer) / sizeof(dma_buffer[0]));
    //TODO calculate time needed to fill first buffer
    HAL_Delay(500);
    float volts = calculate_heading(2 * adc_values[5]);
    //double r = atan(1.57*tan()/())
    uint16_t counts = TURN_CONST * fabs(volts);
    TIM4->CNT = 0;
    TIM5->CNT = 0;

    char msg[18] = "";
    sprintf(msg, "TURN | cnts: %d\n", (int)counts);
    print(msg, 0);
    int pre_dec = (int)(volts / 1);
    int post_dec = (int)((volts - pre_dec) * 1000);
    sprintf(msg, "TURN | vlts: %d.%d\n", pre_dec, post_dec);
    print(msg, 2);
    sprintf(msg, "TURN | ADC_VAL: %lu\n", 2 * adc_values[5]);
    print(msg, 0);

    if (volts < -TURN_TOLERANCE)
    {
        set_motor_speed(TIM_CHANNEL_1, 0);
        set_motor_speed(TIM_CHANNEL_3, RIGHT_SPEED);
        while (TIM5->CNT < counts)
        {
            // sprintf(msg, "TIM5->CNT: %lu\n", TIM5->CNT);
            // print(msg, 4);
        }
        sprintf(msg, "TURN | TIM5->CNT: %lu\n", TIM5->CNT);
        print(msg, 4);
        TIM5->CNT = 0;
        set_motor_speed(TIM_CHANNEL_1, LEFT_SPEED);
    }
    else if (volts > TURN_TOLERANCE)
    {
        set_motor_speed(TIM_CHANNEL_1, LEFT_SPEED);
        set_motor_speed(TIM_CHANNEL_3, 0);
        while (TIM4->CNT < counts)
        {
            // sprintf(msg, "TIM4->CNT: %lu\n", TIM4->CNT);
            // print(msg, 4);
        }
        sprintf(msg, "TURN | TIM4->CNT: %lu\n", TIM4->CNT);
        print(msg, 4);
        TIM4->CNT = 0;
        set_motor_speed(TIM_CHANNEL_3, RIGHT_SPEED);
    }
    HAL_ADC_Stop_DMA(&hadc1);
}

/*
 * Assume motors are not on.
 * Reads adc, turns left or right based on voltage. Left max= 0 , no turn = 1.65 V, right max = 3.3V.
 */
void turn_deg(int16_t deg)
{
    HAL_ADC_Start_DMA(&hadc1, dma_buffer, sizeof(dma_buffer) / sizeof(dma_buffer[0]));
    int16_t counts = 50.0 / 90.0 * (deg - 90) + 50;
    if (counts < 0) counts = -counts;
    TIM4->CNT = 0;
    TIM5->CNT = 0;

    char msg[50] = "";
    sprintf(msg, "TURN_DEG | turn count: %d\n", counts);
    print(msg, 0);

    if (deg > 0) // FIXME: Ben changed this
    {
        set_motor_speed(TIM_CHANNEL_1, LEFT_SPEED);
        set_motor_speed(TIM_CHANNEL_3, 0);
        while (TIM4->CNT < counts)
        {
            // sprintf(msg, "TIM5->CNT: %lu\n", TIM5->CNT);
            // print(msg, 4);
        }
        sprintf(msg, "TURN_DEG | TIM5->CNT: %lu\n", TIM5->CNT);
        print(msg, 4);
        TIM5->CNT = 0;
    }
    else if (deg < 0) // FIXME: Ben changed this
    {
        set_motor_speed(TIM_CHANNEL_1, 0);
        set_motor_speed(TIM_CHANNEL_3, RIGHT_SPEED);
        while (TIM5->CNT < counts)
        {
            // sprintf(msg, "TIM4->CNT: %lu\n", TIM4->CNT);
            // print(msg, 4);
        }
        sprintf(msg, "TURN_DEG | TIM4->CNT: %lu\n", TIM4->CNT);
        print(msg, 4);
        TIM4->CNT = 0;
    }
    set_motor_speed(TIM_CHANNEL_1, 0);
    set_motor_speed(TIM_CHANNEL_3, 0);
    HAL_ADC_Stop_DMA(&hadc1);
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

// Sampling frequency: 72e6/2/(495*2)
void alarm_detect() {
    HAL_ADC_Start_DMA(&hadc1, dma_buffer, sizeof(dma_buffer) / sizeof(dma_buffer[0]));
    HAL_Delay(500);
    char msg[20]="";
    int count = 0;
    while (goertzel(adc_values, 36363, 1000, sizeof(dma_buffer) / sizeof(dma_buffer[0]), 0) < 10);
    HAL_Delay(100);
    while (goertzel(adc_values, 36363, 1000, sizeof(dma_buffer) / sizeof(dma_buffer[0]), 0) > 10);
    HAL_Delay(100);
    print("worcs", 0);
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
    // if (row == 0)
    // {
    //     ssd1306_Fill(Black);
    // }
    // ssd1306_SetCursor(0, row * 10);
    // ssd1306_WriteString(msg, Font_7x10, White);
    // ssd1306_UpdateScreen();
    HAL_UART_Transmit(&huart6, (uint8_t *) msg, strlen(msg), 100);
}

PID_t menu()
{
    print("Starting\n", 0);
    char msg[20] = "";
    int pid_select = 0;
    uint32_t values[2] = {0, 0};
    while (1)
    {
        if (HAL_GPIO_ReadPin(MENU_GPIO_Port, MENU_Pin) == 0)
        {
            sprintf(msg, "%lu\n", values[pid_select]);
            print(msg, 0);
            while (HAL_GPIO_ReadPin(MENU_GPIO_Port, MENU_Pin) == 0)
            {
                values[pid_select]++;
                sprintf(msg, "%lu\n", values[pid_select]);
                print(msg, 0);
                HAL_Delay(500);
            }
            ++pid_select;
        }
        if (pid_select == 2)
            break;
    }
    return pid_Init(values[0], values[1], 1, 1, 1);
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
    __HAL_TIM_SET_COMPARE(&htim1, channel, speed > 1000 ? 1000 : speed);
}

void encoder_pid(PID_t *enc_pid)
{
    /* Get error */
    uint32_t lcnt = TIM4->CNT;
    uint32_t rcnt = TIM5->CNT;
    enc_pid->err = lcnt - rcnt;

    /* Get gain */
    // Gain <0 for ride side faster
    int gain = pid_GetGain(enc_pid);

    /* Set Motor Speeds*/
    uint32_t lspeed = LEFT_SPEED;
    uint32_t rspeed = RIGHT_SPEED;
    if (gain < 0)
    {
        lspeed -= gain;
    }
    else if (gain > 0)
    {
        rspeed += gain;
    }

    // char msg[18] = "";
    // sprintf(msg, "LS: %lu", lspeed);
    // print(msg, 0);
    // sprintf(msg, "RS: %lu", rspeed);
    // print(msg, 1);
    // set_motor_speed(TIM_CHANNEL_1, lspeed);
    // set_motor_speed(TIM_CHANNEL_3, rspeed);

    /* Prevent weird overflow shit */
    if (lcnt > 60000 || rcnt > 60000)
    {
        TIM4->CNT -= 50000;
        TIM5->CNT -= 50000;
    }
}

int debounce_and_grab(PID_t *enc_pid)
{
    uint32_t timeee = HAL_GetTick();
    // int count = 0;
    // while (HAL_GetTick() - timeee < 50)
    // {
    //     ++count;
    //     HAL_Delay(5);
    //     if (HAL_GPIO_ReadPin(CLAW_INT_GPIO_Port, CLAW_INT_Pin) == GPIO_PIN_RESET)
    //     {
    //         print("CLAW_GRAB | Claw debounced. No ewok found\n", 0);
    //         ewok = FALSE;
    //         break;
    //     }
    // }
    // char msg[50] = "";
    // sprintf(msg, "CLAW_GRAB | Debounced with %d checks\n", count);
    // print(msg, 0);
    // If claw is still high, we have an ewok. Stop, grab it, continue on
    // hardcoded procedure
    print("CLAW_GRAB | grabbing ewok\n", 0);
    HAL_Delay(500);
    set_motor_speed(TIM_CHANNEL_1, 0);
    set_motor_speed(TIM_CHANNEL_3, 0);
    close_claw(&htim3);
    arm_up(&htim3);
    // TODO: delete next line, there for testing!!!
    HAL_Delay(100);
    ++ewok_cnt;
    // char msg[18] = "";
    // sprintf(msg, "wok_cnt: %d\n", ewok_cnt);
    // print(msg, 0);
    /*
     * Claw is up and closed
     * Ewok Count incremented
     * Motors not powered
     * CLAW_INT_STATE not FLAGGED, interrupt disabled
     */

    // For the first ewok, turn left, arm up, square robot w/ edge,
    // drive straight for 3 s, bring arm down
    if (ewok_cnt == 1)
    {
        print("First ewok captured\n", 0);
        turn_deg(-170);
        open_claw(&htim3);
        LEFT_SPEED+=100;
        set_motor_speed(TIM_CHANNEL_3, RIGHT_SPEED-50);
        set_motor_speed(TIM_CHANNEL_1, LEFT_SPEED-50);
        uint32_t time = HAL_GetTick();
        while(HAL_GetTick()-time < 1000){
            drive_straight(enc_pid);
        }
        square_edge(enc_pid);
        set_motor_speed(TIM_CHANNEL_3, RIGHT_SPEED);
        set_motor_speed(TIM_CHANNEL_1, LEFT_SPEED);
        time = HAL_GetTick();
        while(HAL_GetTick()-time < 2500){
            drive_straight(enc_pid);
        }
        arm_down(&htim3);
        set_motor_speed(TIM_CHANNEL_3, 0);
        set_motor_speed(TIM_CHANNEL_1, 0);
        // square_edge(&enc_pid);
        //        drive_straight_time(&enc_pid, LEFT_SPEED, RIGHT_SPEED, 10000);
    }

    if (ewok_cnt == 2)
    {
        print("Second ewok captured\n",0);
        if(ewok_cnt==2 && searches == 3){
        turn_deg(-200); //  Prolly have to change this
        }else{
        turn_deg(-180); //  Prolly have to change this
        }
        HAL_Delay(10);
        alarm_detect();
        HAL_Delay(10);
        open_claw(&htim3);
        HAL_Delay(10);
        uint32_t time = HAL_GetTick();
        set_motor_speed(TIM_CHANNEL_3, RIGHT_SPEED);
        set_motor_speed(TIM_CHANNEL_1, LEFT_SPEED);
        while(HAL_GetTick()-time < 8000){
            drive_straight(enc_pid);
        }
        arm_down(&htim3);
        set_motor_speed(TIM_CHANNEL_3, 0);
        set_motor_speed(TIM_CHANNEL_1, 0);
        /*
           timeee = HAL_GetTick();
           while (HAL_GetTick() - timeee < 7000)
           {
           if (TAPE_LEFT_STATE == FLAGGED && TAPE_RIGHT_STATE != FLAGGED)
           {
           set_motor_speed(TIM_CHANNEL_3, 0);
        // Tape detects object when high
        while (HAL_GPIO_ReadPin(TAPE_LEFT_GPIO_Port, TAPE_LEFT_Pin) == GPIO_PIN_SET);
        HAL_Delay(100);
        TAPE_LEFT_STATE = NOT_FLAGGED;
        set_motor_speed(TIM_CHANNEL_3, RIGHT_SPEED);
        }
        else if (TAPE_RIGHT_STATE == FLAGGED && TAPE_LEFT_STATE != FLAGGED)
        {
        set_motor_speed(TIM_CHANNEL_1, 0);
        while (HAL_GPIO_ReadPin(TAPE_RIGHT_GPIO_Port, TAPE_RIGHT_Pin) == GPIO_PIN_SET);
        HAL_Delay(100);
        TAPE_RIGHT_STATE = NOT_FLAGGED;
        set_motor_speed(TIM_CHANNEL_1, LEFT_SPEED);
        }
        }
        */
    }

    if (ewok_cnt == 3)
    {
        print("Third ewok captured\n", 0);
        turn_deg(170);
        set_motor_speed(TIM_CHANNEL_3, RIGHT_SPEED);
        set_motor_speed(TIM_CHANNEL_1, LEFT_SPEED);
        square_edge(enc_pid);
        open_claw(&htim3);
        uint32_t time = HAL_GetTick();
        set_motor_speed(TIM_CHANNEL_3, RIGHT_SPEED);
        set_motor_speed(TIM_CHANNEL_1, LEFT_SPEED);
        while(HAL_GetTick()-time < 2000){
            drive_straight(enc_pid);
        }
        arm_down(&htim3);
        set_motor_speed(TIM_CHANNEL_3, 0);
        set_motor_speed(TIM_CHANNEL_1, 0);
        uint8_t deg = 0;
        while(PI_INT_STATE != FLAGGED){
            if(deg>6){
                turn_deg(15);
            }else{
                turn_deg(-20);
            }
            ++deg;
            time = HAL_GetTick();
            while(HAL_GetTick()-time<3000 && PI_INT_STATE == NOT_FLAGGED);
        }
    }

    if (ewok_cnt == 4)
    {
        print("Fourth ewok captured\n", 0);
        turn_deg(180);
        set_motor_speed(TIM_CHANNEL_3, RIGHT_SPEED);
        set_motor_speed(TIM_CHANNEL_1, LEFT_SPEED);
        open_claw(&htim3);
        LEFT_SPEED -= 100;
        timeee = HAL_GetTick();
        while (HAL_GetTick() - timeee < 8000)
        {
            if (HAL_GPIO_ReadPin(EDGE_LEFT_GPIO_Port, EDGE_LEFT_Pin) == GPIO_PIN_RESET && HAL_GPIO_ReadPin(EDGE_RIGHT_GPIO_Port, EDGE_RIGHT_Pin) == GPIO_PIN_SET)
            {
                    set_motor_speed(TIM_CHANNEL_3, 0);
            }else if (HAL_GPIO_ReadPin(EDGE_LEFT_GPIO_Port, EDGE_LEFT_Pin) == GPIO_PIN_SET && HAL_GPIO_ReadPin(EDGE_RIGHT_GPIO_Port, EDGE_RIGHT_Pin) == GPIO_PIN_RESET){
                    set_motor_speed(TIM_CHANNEL_1, 0);
            }else if (HAL_GPIO_ReadPin(EDGE_LEFT_GPIO_Port, EDGE_LEFT_Pin) == GPIO_PIN_RESET && HAL_GPIO_ReadPin(EDGE_RIGHT_GPIO_Port, EDGE_RIGHT_Pin) == GPIO_PIN_RESET){
                    set_motor_speed(TIM_CHANNEL_1, 0);
                    set_motor_speed(TIM_CHANNEL_3, 0);
            }else{
                set_motor_speed(TIM_CHANNEL_3, RIGHT_SPEED);
                set_motor_speed(TIM_CHANNEL_1, LEFT_SPEED);
            }
            drive_straight(enc_pid);
        }
        arm_down(&htim3);
    }

    if (ewok_cnt == 5)
    {
        print("Chewie captured\n", 0);
        turn_deg(50);
        open_claw(&htim3);
        HAL_Delay(750);
        arm_down(&htim3);
        slow_actuate(&htim3, BASKET_CH, 10, 180);
        uint32_t time = HAL_GetTick();
        set_motor_speed(TIM_CHANNEL_3, RIGHT_SPEED);
        set_motor_speed(TIM_CHANNEL_1, LEFT_SPEED);
        while(HAL_GetTick()-time < 5000){
            drive_straight(enc_pid);
        }
        slow_actuate(&htim3, BASKET_CH, 180, 20);
    }
    searches = 0;
    CLAW_INT_STATE = NOT_FLAGGED;
    return TRUE;
}

// ******
// TESTS
// ******

/*
 * Instructions:
 *      Run test_All() before the main while loop
 *      - Put a pull up resistor on htim3 PWM waves, measure voltage on pin
 *      - Measure voltage on pin of htim1
 *      - Give ADC pins values and read off of screen
 */

/*
 * Test PWM
 * Output: htim1 channel 1 through 4, PWM wave. Confirm these
 *         PWM waves via an oscilliscope.
 */
void test_PWM_htim1()
{
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
    // HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);
    // HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_4);
    int s = 0;
    while (s < 1000)
    {
        __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, s);
        // __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, 500);
        __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_3, s);
        // __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_4, 500);
        s += 100;
        HAL_Delay(1000);
    }
}

/*
 * Test PWM
 * Output: htim3 channel 1 through 3, PWM wave. Confirm these
 *         PWM waves via an oscilliscope.
 */
void test_PWM_htim3()
{
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_3);

    int s = 500;
    while (s < 2400)
    {
        __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, s);
        __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, s);
        __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_3, s);
        s += 100;
        HAL_Delay(500);
    }
}

void test_ADC()
{
    char msg[18] = "";
    HAL_ADC_Start_DMA(&hadc1, dma_buffer, sizeof(dma_buffer) / sizeof(dma_buffer[0]));

    while (1)
    {
        int ch_5 = adc_values[0];
        int ch_4 = adc_values[1];

        sprintf(msg, "ch_5: %d\n", ch_5);
        print(msg, 0);

        sprintf(msg, "ch_4: %d\n", ch_4);
        print(msg, 2);

        HAL_Delay(100);
    }
}

/**
 * Test all PWM and ADC - Read PWM waves, input ADC voltages 
 * and read values on OLED
 *      htim1 -> CH1, CH2, CH3, CH4
 *      htim3 -> CH1, CH2, CH3
 *      ADC   -> CH5, CH4
 */
void test_All()
{
    test_PWM_htim1();
    test_PWM_htim3();
    test_ADC();
}
// ******
// END TESTS
// ******

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

#ifdef  USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t* file, uint32_t line)
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
