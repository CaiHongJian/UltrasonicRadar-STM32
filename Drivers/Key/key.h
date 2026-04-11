#ifndef __KEY_H
#define __KEY_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

#define KEY_DEBOUNCE_TIME  10   /* 消抖时间10ms */

/* 按键状态变量 */
extern volatile uint8_t Key1_Flag;         /* Key1最终按下标志，1=已消抖确认 */
extern volatile uint8_t Key2_Flag;         /* Key2最终按下标志，1=已消抖确认 */

extern volatile uint8_t Key1_RawFlag;      /* Key1原始触发标志，中断立即设置 */
extern volatile uint32_t Key1_TimeStamp;   /* Key1触发时的时间戳 */
extern volatile uint8_t Key2_RawFlag;      /* Key2原始触发标志，中断立即设置 */
extern volatile uint32_t Key2_TimeStamp;   /* Key2触发时的时间戳 */

/**
 * @brief 按键初始化（外部中断方式）
 * 
 * 注意：GPIO和EXTI配置已在CubeMX的gpio.c中完成
 *       此函数主要用于变量初始化
 */
void Key_Init(void);

/**
 * @brief 按键消抖处理（必须在主循环中定时调用）
 * 
 * 采用时间戳方式消抖，中断中不延时
 */
void Key_DebounceProcess(void);

/**
 * @brief 清除按键标志
 * 
 * @param KeyNum 按键号(1或2)
 */
void Key_ClearFlag(uint8_t KeyNum);

#ifdef __cplusplus
}
#endif

#endif
