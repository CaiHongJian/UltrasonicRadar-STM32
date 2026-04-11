/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    tim.c
  * @brief   This file provides code for the configuration
  *          of the TIM instances.
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
/* Includes ------------------------------------------------------------------*/
#include "tim.h"

/* USER CODE BEGIN 0 */

/*
 * ==============================================
 *          TIM输入捕获驱动（B站Keysking方案）
 * ==============================================
 * 【PWM输入捕获模式工作原理】
 * 1. TIM3_CH1(PB4)配置为直接通道：捕获ECHO上升沿
 * 2. TIM3_CH2配置为间接通道：捕获ECHO下降沿（同一输入引脚）
 * 3. 从模式：复位模式 - 上升沿触发时自动清零计数器
 * 4. 下降沿触发中断，在中断中计算高电平持续时间
 * 
 * 【优点】
 * - 纯硬件捕获，不需要CPU轮询，节省资源
 * - 精度高，没有软件查询延迟
 * - 自动处理计数器溢出
 */

#include "stdio.h"
#include "main.h"  /* 引入引脚宏定义，方便移植 */
#include "delay.h"
/* 输入捕获全局变量 */
uint32_t IC_Val1 = 0;        /* 上升沿捕获值（实际在PWM模式下恒为0） */
uint32_t IC_Val2 = 0;        /* 下降沿捕获值 = 高电平持续时间(us) */
uint32_t IC_Difference = 0;  /* 高电平时间(us) */
float    IC_Distance = 0;    /* 计算得到的距离值(cm) */

/*
 * 函数名：HAL_TIM_IC_CaptureCallback
 * 功能：  输入捕获中断回调函数
 * 说明：  【B站Keysking原版PWM输入模式】
 *         TIM3_CH1配置为直接通道，捕获上升沿
 *         TIM3_CH2配置为间接通道，捕获下降沿
 *         两个通道使用同一个输入引脚(PB4)
 *         在CH2下降沿中断中同时读取两个通道的值
 */
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
    /* 只有TIM3的通道2中断才处理
     * 通道1直接捕获上升沿，通道2间接捕获下降沿
     * 下降沿到来时表示ECHO脉冲结束，此时读取两个通道的值
     */
    if (htim->Instance == TIM3 && htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2)
    {
        /* 通道1保存的是上升沿时刻（开始时间） */
        IC_Val1 = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
        
        /* 通道2保存的是下降沿时刻（结束时间） */
        IC_Val2 = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_2);
        
        /* 计算高电平持续时间
         * 因为TIM3配置了从模式-复位模式，上升沿时计数器自动清零
         * 所以IC_Val2就是高电平持续时间！这里是PWM输入模式的精髓！
         */
        IC_Difference = IC_Val2;
        
        /* 换算距离：距离(cm) = 时间(us) / 58
         *  原理：声速340m/s = 0.034cm/us，往返距离除以2
         *  简化：(时间 * 0.034) / 2 = 时间 / 58.8 ≈ 时间 / 58
         */
        IC_Distance = (float)IC_Difference / 58.0f;
    }
}

/* USER CODE END 0 */

TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;

/* TIM2 init function */
void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 72-1;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 20000 - 1;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_3) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_4) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */
  HAL_TIM_MspPostInit(&htim2);

}
/* TIM3 init function */
void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_SlaveConfigTypeDef sSlaveConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_IC_InitTypeDef sConfigIC = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 72-1;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 65535;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_IC_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sSlaveConfig.SlaveMode = TIM_SLAVEMODE_RESET;
  sSlaveConfig.InputTrigger = TIM_TS_TI1FP1;
  sSlaveConfig.TriggerPolarity = TIM_INPUTCHANNELPOLARITY_RISING;
  sSlaveConfig.TriggerFilter = 0;
  if (HAL_TIM_SlaveConfigSynchro(&htim3, &sSlaveConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_ENABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_RISING;
  sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
  sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
  sConfigIC.ICFilter = 0;
  if (HAL_TIM_IC_ConfigChannel(&htim3, &sConfigIC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_FALLING;
  sConfigIC.ICSelection = TIM_ICSELECTION_INDIRECTTI;
  if (HAL_TIM_IC_ConfigChannel(&htim3, &sConfigIC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */

}

void HAL_TIM_Base_MspInit(TIM_HandleTypeDef* tim_baseHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(tim_baseHandle->Instance==TIM2)
  {
  /* USER CODE BEGIN TIM2_MspInit 0 */

  /* USER CODE END TIM2_MspInit 0 */
    /* TIM2 clock enable */
    __HAL_RCC_TIM2_CLK_ENABLE();

    /* TIM2 interrupt Init */
    HAL_NVIC_SetPriority(TIM2_IRQn, 3, 0);
    HAL_NVIC_EnableIRQ(TIM2_IRQn);
  /* USER CODE BEGIN TIM2_MspInit 1 */

  /* USER CODE END TIM2_MspInit 1 */
  }
  else if(tim_baseHandle->Instance==TIM3)
  {
  /* USER CODE BEGIN TIM3_MspInit 0 */

  /* USER CODE END TIM3_MspInit 0 */
    /* TIM3 clock enable */
    __HAL_RCC_TIM3_CLK_ENABLE();

    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**TIM3 GPIO Configuration
    PB4     ------> TIM3_CH1
    */
    GPIO_InitStruct.Pin = IC_ECHO_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(IC_ECHO_GPIO_Port, &GPIO_InitStruct);

    __HAL_AFIO_REMAP_TIM3_PARTIAL();

    /* TIM3 interrupt Init */
    HAL_NVIC_SetPriority(TIM3_IRQn, 2, 0);
    HAL_NVIC_EnableIRQ(TIM3_IRQn);
  /* USER CODE BEGIN TIM3_MspInit 1 */

  /* USER CODE END TIM3_MspInit 1 */
  }
}
void HAL_TIM_MspPostInit(TIM_HandleTypeDef* timHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(timHandle->Instance==TIM2)
  {
  /* USER CODE BEGIN TIM2_MspPostInit 0 */

  /* USER CODE END TIM2_MspPostInit 0 */

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**TIM2 GPIO Configuration
    PA0-WKUP     ------> TIM2_CH1
    PA1     ------> TIM2_CH2
    PA2     ------> TIM2_CH3
    PA3     ------> TIM2_CH4
    */
    GPIO_InitStruct.Pin = S1_TIM2_Pin|S2_TIM2_Pin|S3_TIM2_Pin|S4_TIM2_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* USER CODE BEGIN TIM2_MspPostInit 1 */

  /* USER CODE END TIM2_MspPostInit 1 */
  }

}

void HAL_TIM_Base_MspDeInit(TIM_HandleTypeDef* tim_baseHandle)
{

  if(tim_baseHandle->Instance==TIM2)
  {
  /* USER CODE BEGIN TIM2_MspDeInit 0 */

  /* USER CODE END TIM2_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_TIM2_CLK_DISABLE();

    /* TIM2 interrupt Deinit */
    HAL_NVIC_DisableIRQ(TIM2_IRQn);
  /* USER CODE BEGIN TIM2_MspDeInit 1 */

  /* USER CODE END TIM2_MspDeInit 1 */
  }
  else if(tim_baseHandle->Instance==TIM3)
  {
  /* USER CODE BEGIN TIM3_MspDeInit 0 */

  /* USER CODE END TIM3_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_TIM3_CLK_DISABLE();

    /**TIM3 GPIO Configuration
    PB4     ------> TIM3_CH1
    */
    HAL_GPIO_DeInit(IC_ECHO_GPIO_Port, IC_ECHO_Pin);

    /* TIM3 interrupt Deinit */
    HAL_NVIC_DisableIRQ(TIM3_IRQn);
  /* USER CODE BEGIN TIM3_MspDeInit 1 */

  /* USER CODE END TIM3_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */

/*
 * 函数名：TIM3_IC_Start
 * 功能：  启动TIM3输入捕获模式
 * 说明：  【重要！】CubeMX只负责初始化，必须手动调用此函数启动！
 * 调用时机：在main函数初始化中调用，或者在Ultrasonic_Init中调用
 */
void TIM3_IC_Start(void)
{
    /* 1. 启动定时器基础时钟 */
    HAL_TIM_Base_Start(&htim3);
    
    /* 2. 启动通道1的输入捕获（上升沿，不带中断）
     *    通道1是直接捕获通道
     */
    HAL_TIM_IC_Start(&htim3, TIM_CHANNEL_1);
    
    /* 3. 启动通道2的输入捕获中断（下降沿，带中断）
     *    通道2是间接捕获通道，捕获完成进入中断计算距离
     */
    HAL_TIM_IC_Start_IT(&htim3, TIM_CHANNEL_2);
    
    /* 4. 清零计数器，准备第一次捕获 */
    __HAL_TIM_SET_COUNTER(&htim3, 0);
}

/*
 * 函数名：TIM3_IC_GetDistance_cm
 * 功能：  获取输入捕获模式测量的距离（厘米）
 * 返回值：距离值，单位cm
 */
float TIM3_IC_GetDistance_cm(void)
{
    /* 范围检查，有效范围2~400cm */
    if (IC_Distance < 2.0f || IC_Distance > 400.0f)
    {
        return 999.0f;  /* 返回999表示无效值 */
    }
    return IC_Distance;
}

/*
 * 函数名：TIM3_IC_GetDistance_mm
 * 功能：  获取输入捕获模式测量的距离（毫米）
 * 返回值：距离值，单位mm
 */
float TIM3_IC_GetDistance_mm(void)
{
    float dist_mm = IC_Distance * 10.0f;
    
    /* 范围检查，有效范围20~4000mm */
    if (dist_mm < 20.0f || dist_mm > 4000.0f)
    {
        return 9999.0f;  /* 返回9999表示无效值 */
    }
    return dist_mm;
}

/*
 * 函数名：TIM3_IC_Trigger
 * 功能：  发送超声波触发脉冲
 * 说明：  触发HC-SR04开始一次测量
 * 注意：  TRIG引脚需要在CubeMX配置为GPIO输出
 *         默认使用PA6，如需修改请自行修改引脚定义
 */
void TIM3_IC_Trigger(void)
{
    /* 1. 先拉低，保证脉冲干净 */
    HAL_GPIO_WritePin(TRIG_GPIO_Port, TRIG_Pin, GPIO_PIN_RESET);
    HAL_Delay_us(2);
    
    /* 2. 发送15us高电平触发脉冲（数据手册要求≥10us） */
    HAL_GPIO_WritePin(TRIG_GPIO_Port, TRIG_Pin, GPIO_PIN_SET);
    HAL_Delay_us(15);
    HAL_GPIO_WritePin(TRIG_GPIO_Port, TRIG_Pin, GPIO_PIN_RESET);
    
    /* 3. 清零计数器，准备捕获 */
    __HAL_TIM_SET_COUNTER(&htim3, 0);
}

/* USER CODE END 1 */
