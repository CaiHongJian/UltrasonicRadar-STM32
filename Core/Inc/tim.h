/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    tim.h
  * @brief   This file contains all the function prototypes for
  *          the tim.c file
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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
#ifndef __TIM_H__
#define __TIM_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

extern TIM_HandleTypeDef htim2;

extern TIM_HandleTypeDef htim3;

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

void MX_TIM2_Init(void);
void MX_TIM3_Init(void);

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* USER CODE BEGIN Prototypes */

/*
 * TIM输入捕获模式驱动函数（B站Keysking方案）
 * 参考：TIM_InputCapture参考代码
 */

/* 启动输入捕获模式 - 必须在初始化时调用！ */
void TIM3_IC_Start(void);

/* 发送超声波触发脉冲 */
void TIM3_IC_Trigger(void);

/* 获取测量距离 - 厘米单位 */
float TIM3_IC_GetDistance_cm(void);

/* 获取测量距离 - 毫米单位 */
float TIM3_IC_GetDistance_mm(void);

/* 输入捕获中断回调函数 - HAL库自动调用 */
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim);

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __TIM_H__ */

