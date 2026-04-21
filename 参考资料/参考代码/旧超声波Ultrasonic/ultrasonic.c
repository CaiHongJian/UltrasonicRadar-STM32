#include "ultrasonic.h"

extern TIM_HandleTypeDef htim3;

static Ultrasonic_HandleTypeDef ultrasonic = {0};

void Ultrasonic_Init(void)
{
    HAL_GPIO_WritePin(TRIG_PORT, TRIG_PIN, GPIO_PIN_RESET);
    ultrasonic.measurement_done = 1;
}

void Ultrasonic_Trigger(void)
{
    HAL_GPIO_WritePin(TRIG_PORT, TRIG_PIN, GPIO_PIN_SET);
    HAL_Delay_us(15);
    HAL_GPIO_WritePin(TRIG_PORT, TRIG_PIN, GPIO_PIN_RESET);
    
    ultrasonic.measurement_done = 0;
    __HAL_TIM_SET_COUNTER(&htim3, 0);
    HAL_TIM_IC_Start_IT(&htim3, TIM_CHANNEL_1);
}

void Ultrasonic_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM3 && htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1)
    {
        if (HAL_GPIO_ReadPin(ECHO_PORT, ECHO_PIN) == GPIO_PIN_SET)
        {
            ultrasonic.echo_start_time = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
            TIM_RESET_CAPTUREPOLARITY(htim, TIM_CHANNEL_1);
            TIM_SET_CAPTUREPOLARITY(htim, TIM_CHANNEL_1, TIM_INPUTCHANNELPOLARITY_FALLING);
        }
        else
        {
            ultrasonic.echo_end_time = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
            
            if (ultrasonic.echo_end_time > ultrasonic.echo_start_time)
            {
                ultrasonic.echo_duration = ultrasonic.echo_end_time - ultrasonic.echo_start_time;
            }
            else
            {
                ultrasonic.echo_duration = (65535 - ultrasonic.echo_start_time) + ultrasonic.echo_end_time;
            }
            
            ultrasonic.distance_cm = ultrasonic.echo_duration * 0.017f;
            
            if (ultrasonic.distance_cm > 400.0f)
            {
                ultrasonic.distance_cm = 400.0f;
            }
            
            ultrasonic.measurement_done = 1;
            
            HAL_TIM_IC_Stop_IT(htim, TIM_CHANNEL_1);
            TIM_RESET_CAPTUREPOLARITY(htim, TIM_CHANNEL_1);
            TIM_SET_CAPTUREPOLARITY(htim, TIM_CHANNEL_1, TIM_INPUTCHANNELPOLARITY_RISING);
        }
    }
}

float Ultrasonic_GetDistance(void)
{
    if (ultrasonic.measurement_done)
    {
        return ultrasonic.distance_cm;
    }
    return -1.0f;
}

float Ultrasonic_GetDistance_Simple(void)
{
    uint32_t timeout = 0;
    uint32_t echo_time = 0;
    
    HAL_GPIO_WritePin(TRIG_PORT, TRIG_PIN, GPIO_PIN_SET);
    HAL_Delay_us(15);
    HAL_GPIO_WritePin(TRIG_PORT, TRIG_PIN, GPIO_PIN_RESET);
    
    timeout = 0;
    while (HAL_GPIO_ReadPin(ECHO_PORT, ECHO_PIN) == GPIO_PIN_RESET)
    {
        timeout++;
        if (timeout > 10000)
        {
            return 999.0f;
        }
    }
    
    timeout = 0;
    while (HAL_GPIO_ReadPin(ECHO_PORT, ECHO_PIN) == GPIO_PIN_SET)
    {
        echo_time++;
        timeout++;
        HAL_Delay_us(1);
        if (timeout > 25000)
        {
            return 999.0f;
        }
    }
    
    return echo_time * 0.017f;
}
