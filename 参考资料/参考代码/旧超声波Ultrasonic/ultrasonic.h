#ifndef __ULTRASONIC_H
#define __ULTRASONIC_H

#include "stm32f1xx_hal.h"

#define TRIG_PIN        GPIO_PIN_6
#define TRIG_PORT       GPIOA
#define ECHO_PIN        GPIO_PIN_7
#define ECHO_PORT       GPIOA

typedef struct {
    uint32_t echo_start_time;
    uint32_t echo_end_time;
    uint32_t echo_duration;
    float distance_cm;
    uint8_t measurement_done;
} Ultrasonic_HandleTypeDef;

void Ultrasonic_Init(void);
void Ultrasonic_Trigger(void);
float Ultrasonic_GetDistance(void);
void Ultrasonic_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim);
float Ultrasonic_GetDistance_Simple(void);

#endif
