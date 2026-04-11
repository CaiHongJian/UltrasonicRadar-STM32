#include "breath_led.h"
#include "pwm.h"
#include "tim.h"

uint8_t LED_Brightness = 0;      /* 当前亮度值 0-100 */
int8_t  LED_Direction = 1;       /* 亮度变化方向：1=增亮，-1=减暗 */
uint8_t LED_Enable = 0;          /* 呼吸灯使能标志：1=开启，0=关闭 */

/**
 * @brief 呼吸灯初始化
 * 
 * 开启TIM2_CH2通道的PWM输出，初始亮度为0
 * 使用PA1引脚输出PWM波形
 * 
 * 【接线说明】用户的接法：
 * PA1引脚 → LED阳极 → LED阴极 → GND
 * 所以：CCR值越大，LED越亮！
 */
void BreathLED_Init(void)
{
    /* 开启TIM2通道2的PWM输出 - PA1引脚 */
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);
    
    LED_Brightness = 0;       /* 初始亮度0 */
    LED_Direction = 1;        /* 默认从增亮开始 */
    LED_Enable = 0;           /* 默认关闭呼吸灯 */
    
    BreathLED_SetBrightness(0);  /* 初始状态LED熄灭 */
}

/**
 * @brief 设置呼吸灯亮度
 * 
 * @param brightness 亮度百分比 0-100
 * 
 * 【实现原理 - 已修正用户接线方式！】
 * 用户接线：PA1 → LED阳极 → LED阴极 → GND
 * TIM2周期是20000，对应50Hz
 * 亮度0%   → CCR = 0     → LED完全熄灭
 * 亮度100% → CCR = 20000 → LED完全点亮
 * CCR值越大，高电平时间越长，LED越亮！
 */
void BreathLED_SetBrightness(uint8_t brightness)
{
    uint16_t ccr = 0;
    
    /* 范围限制 */
    if(brightness > 100) brightness = 100;
    
    /* 亮度百分比转换为PWM比较值
     * 公式：CCR = brightness * 200 （因为20000/100 = 200）
     */
    ccr = brightness * 200;
    
    /* 设置PWM比较值，改变占空比 */
    PWM_SetCompare2(ccr);
}

/**
 * @brief 呼吸灯处理函数
 * 
 * 【功能说明】
 * 1. 只有LED_Enable为1时才执行呼吸效果
 * 2. 亮度从0→100→0循环变化，产生呼吸效果
 * 3. 此函数需要定时调用（建议每20ms调用一次）
 */
void BreathLED_Process(void)
{
    if(LED_Enable == 1)
    {
        /* 根据方向调整亮度 */
        LED_Brightness += LED_Direction;
        
        /* 到达边界时改变方向 */
        if(LED_Brightness >= 100)
        {
            LED_Brightness = 100;
            LED_Direction = -1;  /* 开始减暗 */
        }
        else if(LED_Brightness <= 0)
        {
            LED_Brightness = 0;
            LED_Direction = 1;   /* 开始增亮 */
        }
        
        /* 设置新的亮度 */
        BreathLED_SetBrightness(LED_Brightness);
    }
}

/**
 * @brief 呼吸灯使能控制
 * 
 * @param enable 1=启动呼吸灯效果，0=关闭LED
 */
void BreathLED_Enable(uint8_t enable)
{
    LED_Enable = enable;
    
    if(enable == 0)
    {
        BreathLED_SetBrightness(0);  /* 关闭时LED熄灭 */
        LED_Brightness = 0;
        LED_Direction = 1;           /* 下次开启从增亮开始 */
    }
}
