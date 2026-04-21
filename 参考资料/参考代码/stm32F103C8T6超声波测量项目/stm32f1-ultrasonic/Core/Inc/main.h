/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define SMG_A_Pin GPIO_PIN_0
#define SMG_A_GPIO_Port GPIOA
#define SMG_B_Pin GPIO_PIN_1
#define SMG_B_GPIO_Port GPIOA
#define SMG_C_Pin GPIO_PIN_2
#define SMG_C_GPIO_Port GPIOA
#define SMG_D_Pin GPIO_PIN_3
#define SMG_D_GPIO_Port GPIOA
#define SMG_E_Pin GPIO_PIN_4
#define SMG_E_GPIO_Port GPIOA
#define SMG_F_Pin GPIO_PIN_5
#define SMG_F_GPIO_Port GPIOA
#define SMG_G_Pin GPIO_PIN_6
#define SMG_G_GPIO_Port GPIOA
#define SMG_DP_Pin GPIO_PIN_7
#define SMG_DP_GPIO_Port GPIOA
#define DIG4_Pin GPIO_PIN_0
#define DIG4_GPIO_Port GPIOB
#define DIG3_Pin GPIO_PIN_1
#define DIG3_GPIO_Port GPIOB
#define DIG2_Pin GPIO_PIN_10
#define DIG2_GPIO_Port GPIOB
#define DIG1_Pin GPIO_PIN_11
#define DIG1_GPIO_Port GPIOB
#define KEY1_Pin GPIO_PIN_12
#define KEY1_GPIO_Port GPIOB
#define KEY2_Pin GPIO_PIN_13
#define KEY2_GPIO_Port GPIOB
#define KEY3_Pin GPIO_PIN_14
#define KEY3_GPIO_Port GPIOB
#define KEY4_Pin GPIO_PIN_15
#define KEY4_GPIO_Port GPIOB
#define LED2_Pin GPIO_PIN_6
#define LED2_GPIO_Port GPIOB
#define LED1_Pin GPIO_PIN_7
#define LED1_GPIO_Port GPIOB
#define TRIG_Pin GPIO_PIN_8
#define TRIG_GPIO_Port GPIOB
#define ECHO_Pin GPIO_PIN_9
#define ECHO_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
