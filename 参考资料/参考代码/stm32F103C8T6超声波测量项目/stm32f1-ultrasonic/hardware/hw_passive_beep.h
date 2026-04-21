#ifndef __HW_PASSIVE_BEEP_H
#define __HW_PASSIVE_BEEP_H

#include "main.h"

typedef struct {
    TIM_HandleTypeDef *htim;  // 无源蜂鸣器pwm定时器
    uint32_t channel;         // pwm输出通道
}BEEP_HandleTypeDef;

// 函数声明
BEEP_HandleTypeDef Buzzer_Init(void);
void Buzzer_SetFreq(BEEP_HandleTypeDef *beep_t, float freq, float duty_cycle);
void Buzzer_Beep(BEEP_HandleTypeDef *beep_t, float freq, uint32_t duration_ms);
void Buzzer_Alert(BEEP_HandleTypeDef *beep_t);
void Buzzer_PlayMelody(BEEP_HandleTypeDef *beep_t);
void Buzzer_Stop(BEEP_HandleTypeDef *beep_t);

#endif

