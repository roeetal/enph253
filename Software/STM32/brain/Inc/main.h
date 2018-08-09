/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H__
#define __MAIN_H__

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private define ------------------------------------------------------------*/

#define MENU_Pin GPIO_PIN_13
#define MENU_GPIO_Port GPIOC
#define PI_INT_Pin GPIO_PIN_0
#define PI_INT_GPIO_Port GPIOC
#define PI_INT_EXTI_IRQn EXTI0_IRQn
#define IR_INT_Pin GPIO_PIN_1
#define IR_INT_GPIO_Port GPIOC
#define IR_INT_EXTI_IRQn EXTI1_IRQn
#define TL_Pin GPIO_PIN_2
#define TL_GPIO_Port GPIOC
#define EDGE_RIGHT_Pin GPIO_PIN_3
#define EDGE_RIGHT_GPIO_Port GPIOC
#define EDGE_RIGHT_EXTI_IRQn EXTI3_IRQn
#define ENCODER_RIGHT_1_Pin GPIO_PIN_0
#define ENCODER_RIGHT_1_GPIO_Port GPIOA
#define ENCODER_RIGHT_2_Pin GPIO_PIN_1
#define ENCODER_RIGHT_2_GPIO_Port GPIOA
#define PI_ANALOG_Pin GPIO_PIN_4
#define PI_ANALOG_GPIO_Port GPIOA
#define IR_1_Pin GPIO_PIN_5
#define IR_1_GPIO_Port GPIOA
#define SERVO_CLAW_JAW_Pin GPIO_PIN_6
#define SERVO_CLAW_JAW_GPIO_Port GPIOA
#define SERVO_CLAW_ARM_Pin GPIO_PIN_7
#define SERVO_CLAW_ARM_GPIO_Port GPIOA
#define TR_Pin GPIO_PIN_4
#define TR_GPIO_Port GPIOC
#define EL_Pin GPIO_PIN_5
#define EL_GPIO_Port GPIOC
#define SERVO_BASKET_Pin GPIO_PIN_0
#define SERVO_BASKET_GPIO_Port GPIOB
#define ER_Pin GPIO_PIN_1
#define ER_GPIO_Port GPIOB
#define TAPE_LEFT_Pin GPIO_PIN_14
#define TAPE_LEFT_GPIO_Port GPIOB
#define TAPE_LEFT_EXTI_IRQn EXTI15_10_IRQn
#define TAPE_RIGHT_Pin GPIO_PIN_15
#define TAPE_RIGHT_GPIO_Port GPIOB
#define TAPE_RIGHT_EXTI_IRQn EXTI15_10_IRQn
#define STM_TX_Pin GPIO_PIN_6
#define STM_TX_GPIO_Port GPIOC
#define STM_RX_Pin GPIO_PIN_7
#define STM_RX_GPIO_Port GPIOC
#define MOTOR_L_F_Pin GPIO_PIN_8
#define MOTOR_L_F_GPIO_Port GPIOA
#define MOTOR_L_B_Pin GPIO_PIN_9
#define MOTOR_L_B_GPIO_Port GPIOA
#define MOTOR_R_F_Pin GPIO_PIN_10
#define MOTOR_R_F_GPIO_Port GPIOA
#define MOTOR_R_B_Pin GPIO_PIN_11
#define MOTOR_R_B_GPIO_Port GPIOA
#define EDGE_LEFT_Pin GPIO_PIN_2
#define EDGE_LEFT_GPIO_Port GPIOD
#define EDGE_LEFT_EXTI_IRQn EXTI2_IRQn
#define CLAW_INT_Pin GPIO_PIN_4
#define CLAW_INT_GPIO_Port GPIOB
#define CLAW_INT_EXTI_IRQn EXTI4_IRQn
#define ENCODER_LEFT_1_Pin GPIO_PIN_6
#define ENCODER_LEFT_1_GPIO_Port GPIOB
#define ENCODER_LEFT_2_Pin GPIO_PIN_7
#define ENCODER_LEFT_2_GPIO_Port GPIOB

/* ########################## Assert Selection ############################## */
/**
  * @brief Uncomment the line below to expanse the "assert_param" macro in the 
  *        HAL drivers code
  */
/* #define USE_FULL_ASSERT    1U */

/* USER CODE BEGIN Private defines */
#define ENCODER_GOAL 2
#define TURN_CONST 110
#define TURN_TOLERANCE 0.04
#define MOTOR_SPEED 1000
// Add to right side
#define SPEED_DIFF 80
#define BASE_SPEED 600
#define CLAW_CH TIM_CHANNEL_1
#define ARM_CH TIM_CHANNEL_2
#define BASKET_CH TIM_CHANNEL_3
#define TRUE 1
#define FALSE 0

/* USER CODE END Private defines */

#ifdef __cplusplus
 extern "C" {
#endif
void _Error_Handler(char *, int);

#define Error_Handler() _Error_Handler(__FILE__, __LINE__)
#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H__ */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
