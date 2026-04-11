#include "key.h"

/* ====================== 按键消抖方案 ======================
 *
 * 【工业级标准消抖方案】
 *
 * 1. 中断触发时：不延时！立即记录时间戳，设置原始标志
 * 2. 主循环中：定时检查，10ms后再判断电平
 * 3. 好处：中断快速退出，不影响系统实时性
 *
 * ======================================================== */

/* 最终按下标志 - 消抖确认后才置1 */
volatile uint8_t Key1_Flag = 0;    /* Key1有效按下标志 */
volatile uint8_t Key2_Flag = 0;    /* Key2有效按下标志 */

/* 中断层使用的变量 - 中断中立即设置 */
volatile uint8_t Key1_RawFlag = 0;     /* Key1原始触发标志 */
volatile uint32_t Key1_TimeStamp = 0;  /* Key1触发时间戳 */
volatile uint8_t Key2_RawFlag = 0;     /* Key2原始触发标志 */
volatile uint32_t Key2_TimeStamp = 0;  /* Key2触发时间戳 */

/**
 * @brief 按键初始化
 * 
 * 初始化所有按键标志变量
 * GPIO和EXTI中断配置在CubeMX中已完成
 */
void Key_Init(void)
{
    Key1_Flag = 0;
    Key2_Flag = 0;
    Key1_RawFlag = 0;
    Key2_RawFlag = 0;
    Key1_TimeStamp = 0;
    Key2_TimeStamp = 0;
}

/**
 * @brief GPIO外部中断回调函数（HAL库标准回调）
 * 
 * 【最重要原则：中断服务函数要快进快出！】
 * 这里只做两件事，绝不延时！
 *   1. 记录触发时刻的时间戳
 *   2. 设置原始触发标志
 * 
 * 消抖判断放在主循环做！
 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    /* Key1(PB13)中断触发 */
    if(GPIO_Pin == Key1_Pin)
    {
        if(Key1_RawFlag == 0)  /* 防止重复触发 */
        {
            Key1_TimeStamp = HAL_GetTick();  /* 记录触发时刻 */
            Key1_RawFlag = 1;                /* 标记待消抖处理 */
        }
    }
    
    /* Key2(PB15)中断触发 */
    if(GPIO_Pin == Key2_Pin)
    {
        if(Key2_RawFlag == 0)  /* 防止重复触发 */
        {
            Key2_TimeStamp = HAL_GetTick();  /* 记录触发时刻 */
            Key2_RawFlag = 1;                /* 标记待消抖处理 */
        }
    }
}

/**
 * @brief 按键消抖处理函数（主循环中调用）
 * 
 * 【消抖原理】
 * 1. 中断触发后，等10ms
 * 2. 10ms后再次检测引脚电平
 * 3. 还是低电平 = 确实按下了
 * 4. 弹回去了 = 抖动干扰，忽略
 */
void Key_DebounceProcess(void)
{
    /* ---------------- Key1消抖处理 ---------------- */
    if(Key1_RawFlag == 1)
    {
        /* 等到消抖时间到 */
        if(HAL_GetTick() - Key1_TimeStamp >= KEY_DEBOUNCE_TIME)
        {
            /* 10ms后再次检测：确实还按着才算有效按下 */
            if(HAL_GPIO_ReadPin(Key1_GPIO_Port, Key1_Pin) == GPIO_PIN_RESET)
            {
                Key1_Flag = 1;  /* 确认有效按下 */
            }
            Key1_RawFlag = 0;   /* 处理完成，清除原始标志 */
        }
    }
    
    /* ---------------- Key2消抖处理 ---------------- */
    if(Key2_RawFlag == 1)
    {
        /* 等到消抖时间到 */
        if(HAL_GetTick() - Key2_TimeStamp >= KEY_DEBOUNCE_TIME)
        {
            /* 10ms后再次检测：确实还按着才算有效按下 */
            if(HAL_GPIO_ReadPin(Key2_GPIO_Port, Key2_Pin) == GPIO_PIN_RESET)
            {
                Key2_Flag = 1;  /* 确认有效按下 */
            }
            Key2_RawFlag = 0;   /* 处理完成，清除原始标志 */
        }
    }
}

/**
 * @brief 清除按键标志
 * 
 * @param KeyNum 按键号：1=清除Key1标志，2=清除Key2标志
 */
void Key_ClearFlag(uint8_t KeyNum)
{
    if(KeyNum == 1)
    {
        Key1_Flag = 0;
    }
    else if(KeyNum == 2)
    {
        Key2_Flag = 0;
    }
}
