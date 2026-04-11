#include "buzzer.h"

uint8_t Buzzer_State = 0;  /* 蜂鸣器状态：1=鸣叫，0=关闭 */

/**
 * @brief 蜂鸣器初始化（低电平触发型）
 * 
 * PB12引脚配置为推挽输出，初始输出高电平（关闭蜂鸣器）
 * GPIO配置已在CubeMX中完成
 */
void Buzzer_Init(void)
{
    Buzzer_Off();  /* 上电默认关闭蜂鸣器（输出高电平）*/
    Buzzer_State = 0;
}

/**
 * @brief 蜂鸣器开启
 * 
 * PB12输出低电平，蜂鸣器鸣叫（低电平触发型）
 */
void Buzzer_On(void)
{
    HAL_GPIO_WritePin(Buzzer_GPIO_Port, Buzzer_Pin, GPIO_PIN_RESET);
    Buzzer_State = 1;
}

/**
 * @brief 蜂鸣器关闭
 * 
 * PB12输出高电平，蜂鸣器停止鸣叫（低电平触发型）
 */
void Buzzer_Off(void)
{
    HAL_GPIO_WritePin(Buzzer_GPIO_Port, Buzzer_Pin, GPIO_PIN_SET);
    Buzzer_State = 0;
}

/**
 * @brief 蜂鸣器状态翻转
 */
void Buzzer_Toggle(void)
{
    HAL_GPIO_TogglePin(Buzzer_GPIO_Port, Buzzer_Pin);
    Buzzer_State = !Buzzer_State;
}

/**
 * @brief 蜂鸣器报警控制
 * 
 * @param enable 1=开启报警，0=停止报警
 */
void Buzzer_Alarm(uint8_t enable)
{
    if(enable == 1)
    {
        Buzzer_On();
    }
    else
    {
        Buzzer_Off();
    }
}
