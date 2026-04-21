#ifndef __HW_ULTRASONIC_H
#define __HW_ULTRASONIC_H

#include "main.h"

// 超声波参数
#define US_TIMEOUT_US     30000   // 超时时间（约对应6米距离）
#define SOUND_SPEED       343.2f  // 声速（m/s）
#define TEMPERATURE       26.0f   // 环境温度（℃），用于校准声速

// 函数声明
float Ultrasonic_MeasureDistance(void);
uint32_t Ultrasonic_GetEchoTime(void);
uint16_t Ultrasonic_MeasureDistanceCm(void);

#endif

