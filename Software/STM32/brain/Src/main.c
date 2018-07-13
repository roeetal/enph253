
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
#include "stm32f4xx_it.h"
#include "adc.h"
#include "i2c.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* USER CODE BEGIN Includes */
#include "fonts.h"
#include "ssd1306.h"
#include "pid.h"
#include "encoder.h"

/* USER CODE END Includes */

/* USER CODE BEGIN GV */
/* Global variables ---------------------------------------------------------*/
uint8_t PUSH_BUTTON_STATE = 0;
/* USER CODE END GV */

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/
void print(char msg[], int row);
void update_motor_speed(int m, uint32_t speed[]);
void do_pid(PID_t *pid_struct);
PID_t menu(void);

/* USER CODE END PFP */

/* USER CODE BEGIN 0 */

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
    MX_TIM3_Init();
    MX_TIM5_Init();
    MX_TIM1_Init();
    MX_TIM2_Init();
    MX_USART6_UART_Init();
    MX_ADC1_Init();
    MX_I2C1_Init();
    MX_TIM4_Init();
    MX_TIM9_Init();
    ssd1306_Init();
    /* USER CODE BEGIN 2 */

    /* Initialize all timer related stuffs*/
    HAL_TIM_Encoder_Start(&htim4,TIM_CHANNEL_ALL);
    HAL_TIM_Encoder_Start(&htim5,TIM_CHANNEL_ALL);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);
    HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);
    HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_3);
    HAL_TIM_Base_Start(&htim9);


    /* Initialize struct stuffs*/
    PID_t pid_s = menu();

    /* declare external variables for use with interrupts*/


    /* USER CODE END 2 */
    
    /* Infinite loop */
    /* USER CODE BEGIN WHILE */
    while (1)
    {
        do_pid(&pid_s);

        /* USER CODE END WHILE */

        /* USER CODE BEGIN 3 */

    }
    /* USER CODE END 3 */

}

PID_t menu(void){
    print("Starting", 0);
    char msg[20] = "";
    int pid_select = 0;
    uint32_t values[3] = {0,0,0};
    while(1){
        if(HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13)==0){
            sprintf(msg, "%lu", values[pid_select]);
            print(msg, 0);
            TIM4->CNT = values[pid_select];
            while(HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13)==0){
                values[pid_select] = TIM4->CNT;
                sprintf(msg, "%lu", values[pid_select]);
                print(msg, 0);
            }
            ++pid_select;
        }
        if(pid_select==3) break;
    }
    sprintf(msg, "P %lu", values[0]);
    print(msg, 0);
    sprintf(msg, "D %lu", values[1]);
    print(msg, 1);
    sprintf(msg, "I %lu", values[2]);
    print(msg, 2);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);
    return pid_Init(values[0],values[1],values[2],5,2);
}

void do_pid(PID_t *pid_struct){
    /* Read sensors */
    uint8_t left = HAL_GPIO_ReadPin(TAPE_LEFT_GPIO_Port, TAPE_LEFT_Pin)? 0 : 1;
    uint8_t right = HAL_GPIO_ReadPin(TAPE_RIGHT_GPIO_Port, TAPE_RIGHT_Pin)? 0 : 1;

    /* Get error */
    if(left && right){ pid_struct->err = 0; }
    else if(left && !right){ pid_struct->err = 1; }
    else if(!left && right){ pid_struct->err = -1; }
    else if(!left && !right && (pid_struct->err < 0)){ pid_struct->err = -5; }
    else if(!left && !right && (pid_struct->err > 0)){ pid_struct->err = 5; }

    /* Get gain */
    double gain = pid_GetGain(pid_struct, &htim9);
    int g = (int) gain;

    /* Set Motor Speeds*/
    int left_speed = 840;
    int right_speed = 840;
    if(g<0){
        left_speed -= g;
    }else if(g>0){
        right_speed += g;
    }
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, left_speed);
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_3, right_speed);
}

void update_motor_speed(int m, uint32_t speed[]){
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, speed[0]/100.0*MOTOR_SPEED);
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_3, speed[1]/100.0*MOTOR_SPEED);
    char msg[20] = "";
    sprintf(msg, "L: %lu", speed[0]);
    print(msg, 0);
    sprintf(msg, "R: %lu", speed[1]);
    print(msg, 1);
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
/*
 * Rows from 0 - 6
 * Reset screen when printing from row 0
 */
void print(char* msg, int row){
    if(row==0){
        ssd1306_Fill(Black);
    }
    ssd1306_SetCursor(0,row*10);
    ssd1306_WriteString(msg,Font_7x10,White);
    ssd1306_UpdateScreen();
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
    while(1)
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
