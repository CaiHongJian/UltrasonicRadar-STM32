#ifndef __BREATH_LED_H
#define __BREATH_LED_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

/**
 * @brief 呼吸灯初始化
 * 
 * 使用PA1引脚（TIM2_CH2）输出PWM实现呼吸灯效果
 * 注意：TIM2在CubeMX中已配置，PWM频率50Hz
 */
void BreathLED_Init(void);

/**
 * @brief 设置呼吸灯亮度
 * 
 * @param brightness 亮度值，范围0-100，0=灭，100=最亮
 */
void BreathLED_SetBrightness(uint8_t brightness);

/**
 * @brief 启动呼吸灯效果
 * 
 * 此函数应在主循环中定时调用，产生渐变效果
 * 每调用一次，亮度变化一阶
 */
void BreathLED_Process(void);

/**
 * @brief 呼吸灯使能控制
 * 
 * @param enable 1=启动呼吸灯，0=关闭LED
 */
void BreathLED_Enable(uint8_t enable);

#ifdef __cplusplus
}
#endif

#endif
