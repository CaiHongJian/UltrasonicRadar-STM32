#ifndef __SERVO_H
#define __SERVO_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

/* 舵机运行状态 */
extern volatile uint8_t Servo_RunFlag;     /* 舵机运行标志，1=运行，0=停止 */
extern float Servo_CurrentAngle;           /* 舵机当前角度 */
extern float Servo_Step;                   /* 舵机每次步进角度 */
extern int8_t Servo_Direction;             /* 旋转方向：1=正转，-1=反转 */

/**
 * @brief 初始化舵机驱动模块。
 * 
 * 该函数会初始化PWM定时器，为舵机控制提供基础支持。
 */
void Servo_Init(void);

/**
 * @brief 设置指定舵机的角度。
 * 
 * 将目标角度转换为对应的PWM比较值，并通过PWM通道输出控制信号。
 * 角度范围通常为0-180度，对应PWM脉宽500-2500微秒。
 * 
 * @param[in] ServoId  舵机ID号（1-4），对应不同的PWM通道。
 *                     舵机1使用PA0引脚（TIM2_CH1）
 * @param[in] Angle    目标角度值，范围为0.0到180.0度。
 */
void Servo_SetAngle(uint8_t ServoId, float Angle);

/**
 * @brief 舵机步进旋转
 * 
 * 按照设定的步长旋转舵机，到达180度后自动回到0度循环
 * 只有当Servo_RunFlag为1时才执行旋转
 */
void Servo_StepRotate(void);

/**
 * @brief 设置舵机运行状态
 * 
 * @param state 1=开始旋转，0=停止旋转
 */
void Servo_SetRunState(uint8_t state);

/**
 * @brief 设置舵机步进角度
 * 
 * @param step 每次步进的角度，建议1-10度
 */
void Servo_SetStep(float step);

#ifdef __cplusplus
}
#endif

#endif
