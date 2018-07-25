
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
uint16_t LEFT_SPEED = 20000;
uint16_t RIGHT_SPEED = 20000;
uint32_t dma_buffer[2048];
uint32_t ir_values[2048];
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

    /* Initialize other stuffs*/
    print("Starting...", 0);
    /*
       ENCODER_t left_enc = encoder_Init(TIM3);
       ENCODER_t right_enc = encoder_Init(TIM4);
       PID_t left_pid = pid_Init(100, 10, 1, 2, 2);
       PID_t right_pid = pid_Init(100, 10, 1, 2, 2);
       */
    //PID_t pid_struct = menu();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
    while (1) 
    {

        // encoder_pid(&left_pid, &left_enc, &right_pid,  &right_enc);
        //do_pid(&pid_struct);
        pi_navigation();


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

  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
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

void pi_navigation(){
    HAL_ADC_Start(&hadc2);
    print("Pi nav", 0);
    while(1){
        if (HAL_ADC_PollForConversion(&hadc2, 1000000) == HAL_OK)
        {
            float heading = calculate_heading(HAL_ADC_GetValue(&hadc2));
            uint32_t lspeed=LEFT_SPEED;
            uint32_t rspeed=RIGHT_SPEED;
            if (heading < 0)
            {
                lspeed = LEFT_SPEED-heading*20000;
                rspeed = LEFT_SPEED+heading*20000;
                __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, lspeed);
                __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_3, rspeed);
            }
            else if (heading > 0)
            {
                rspeed = RIGHT_SPEED+heading*20000;
                lspeed = RIGHT_SPEED-heading*20000;
                __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, lspeed);
                __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_3, rspeed);
            }else{
                __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, lspeed);
                __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_3, rspeed);
            }
            char msg[20] = "";
            sprintf(msg, "L: %lu", lspeed);
            print(msg, 0);
            sprintf(msg, "R: %lu", rspeed);
            print(msg, 1);
        }else{
            print("No adc", 0);
        }
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
float calculate_heading(uint32_t adc_val){
    return adc_val/4096.0-0.5;
}

void frequency_comparison(uint16_t freq1, uint16_t freq2, uint16_t GPIO_Pin)
{
    uint16_t offset = GPIO_Pin == IR_1_Pin ? 0 : GPIO_Pin == IR_2_Pin ? 1 : 2;
    HAL_ADC_Start_DMA(&hadc1, dma_buffer, sizeof(dma_buffer)/sizeof(dma_buffer[0]));
    //TODO calculate time needed to fill first buffer
    HAL_Delay(500);
    //TODO figure out thresholds and what we want to look for
    while(1){
        char msg[18] = "";
        // Sampling frequency: 10.6667e6/(2*(239.5+15))
        // freq one
        double val1 = goertzel(ir_values, 20956, freq1, sizeof(dma_buffer)/sizeof(dma_buffer[0]), offset);
        int predec = (int)(val1 / 1);
        int postdec = (int)((val1 - predec) * 1000);
        sprintf(msg, "%d.%d\n", predec, postdec);
        print(msg, 0);
        HAL_UART_Transmit(&huart2, (uint8_t *)msg, strlen(msg), 0xFFFF);
        //freq2
        double val2 = goertzel(ir_values, 20956, freq2, sizeof(dma_buffer)/sizeof(dma_buffer[0]), offset);
        predec = (int)(val2 / 1);
        postdec = (int)((val2 - predec) * 1000);
        sprintf(msg, "%d.%d", predec, postdec);
        HAL_UART_Transmit(&huart2, (uint8_t *)msg, strlen(msg), 0xFFFF);
        //compare
        if(val1>val2){break;}
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
   char msg[20]="";
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

void encoder_pid(PID_t *left_pid, ENCODER_t *left_enc, PID_t *right_pid, ENCODER_t *right_enc)
{
    /* Get error */
    left_pid->err = update_encoder(left_enc) - ENCODER_GOAL;
    right_pid->err = update_encoder(right_enc) - ENCODER_GOAL;

    /* Get gain */
    int16_t left_gain = pid_GetGain(left_pid);
    int16_t right_gain = pid_GetGain(right_pid);
    char msg[18]="";
    sprintf(msg, "%d", (int) left_gain);
    print(msg, 0);
    sprintf(msg, "%d", (int) right_gain);
    print(msg, 1);

    /* Set Motor Speeds*/
    int lspeed = LEFT_SPEED - left_gain;
    int rspeed = RIGHT_SPEED - right_gain;
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, lspeed);
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_3, rspeed);
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
