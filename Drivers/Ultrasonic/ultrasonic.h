#ifndef __ULTRASONIC_H
#define __ULTRASONIC_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f1xx_hal.h"

/*
 * ==============================================
 *          HC-SR04 超声波模块驱动
 * ==============================================
 * 【支持两种工作模式】
 *
 * 模式1：软件查询模式（默认）
 *   接线：VCC->5V, GND->GND, TRIG->PA6, ECHO->PA7
 *   特点：软件查询引脚电平，TIM3硬件计时
 *
 * 模式2：TIM输入捕获模式（B站Keysking方案）
 *   接线：VCC->5V, GND->GND, TRIG->PA6, ECHO->PB4
 *   特点：纯硬件捕获，中断回调，不占用CPU资源
 *         PWM输入模式，上升沿自动清零计数器
 *
 * 测量原理：
 *   TRIG给10us以上高电平 -> 模块发射超声波 -> ECHO变高
 *   ECHO高电平时间 = 声波往返时间
 *   距离(cm) = 高电平时间(us) / 58
 */

/* ========== 引脚定义（CubeMX中已配置，在main.h中定义）========== */
// /* 【通用引脚】 */
// #define TRIG_GPIO_Port  GPIOA
// #define TRIG_Pin        GPIO_PIN_6

// /* 【模式1：软件查询模式引脚】 */
// #define ECHO_GPIO_Port  GPIOA
// #define ECHO_Pin        GPIO_PIN_7

// /* 【模式2：输入捕获模式引脚（PB4对应TIM3_CH1）】 */
// /* 注意：CubeMX中TIM3_CH1必须配置为PWM输入模式！ */
// #define IC_ECHO_GPIO_Port  GPIOB
// #define IC_ECHO_Pin        GPIO_PIN_4

/* ========== 参数配置 ========== */
#define US_MIN_DISTANCE     2       /* 最小测量距离 2cm */
#define US_MAX_DISTANCE     400     /* 最大测量距离 400cm */
#define US_TIMEOUT_US       25000   /* 最大超时时间 25ms */

/* ========== 函数声明 ========== */

/*
 * 函数名：Ultrasonic_Init
 * 功能：  初始化超声波模块
 * 注意：  必须在main函数中调用！会启动TIM3定时器！
 */
void Ultrasonic_Init(void);

/*
 * 函数名：Ultrasonic_Read_cm
 * 功能：  读取距离，单位：厘米(cm)
 * 返回值：正常：2.0 ~ 400.0
 *         失败：999.0 （超时或超出范围）
 */
float Ultrasonic_Read_cm(void);

/*
 * 函数名：Ultrasonic_Read_mm
 * 功能：  读取距离，单位：毫米(mm)
 * 返回值：正常：20.0 ~ 4000.0
 *         失败：9999.0
 */
float Ultrasonic_Read_mm(void);

/* ===================== 输入捕获模式专用函数 ===================== */

/*
 * 函数名：Ultrasonic_IC_Init
 * 功能：  初始化输入捕获模式
 * 说明：  使用B站Keysking的PWM输入捕获方案
 *         注意：ECHO需要接PB4，不是PA7！
 */
void Ultrasonic_IC_Init(void);

/*
 * 函数名：Ultrasonic_IC_Trigger
 * 功能：  发送触发脉冲
 * 说明：  触发后硬件自动捕获，不需要阻塞等待
 */
void Ultrasonic_IC_Trigger(void);

/*
 * 函数名：Ultrasonic_IC_Read_cm
 * 功能：  获取最后一次测量结果（厘米）
 * 说明：  直接返回中断中计算好的值，无阻塞！
 */
float Ultrasonic_IC_Read_cm(void);

/*
 * 函数名：Ultrasonic_IC_Read_mm
 * 功能：  获取最后一次测量结果（毫米）
 * 说明：  直接返回中断中计算好的值，无阻塞！
 */
float Ultrasonic_IC_Read_mm(void);

#ifdef __cplusplus
}
#endif

#endif
