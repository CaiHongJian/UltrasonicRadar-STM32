#ifndef __BUZZER_H
#define __BUZZER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

/**
 * @brief 蜂鸣器初始化
 * 
 * PB12引脚输出，初始化蜂鸣器为关闭状态
 */
void Buzzer_Init(void);

/**
 * @brief 蜂鸣器鸣叫
 */
void Buzzer_On(void);

/**
 * @brief 关闭蜂鸣器
 */
void Buzzer_Off(void);

/**
 * @brief 蜂鸣器翻转
 */
void Buzzer_Toggle(void);

/**
 * @brief 蜂鸣器报警使能
 * 
 * @param enable 1=开启报警鸣叫，0=关闭报警
 */
void Buzzer_Alarm(uint8_t enable);

#ifdef __cplusplus
}
#endif

#endif
