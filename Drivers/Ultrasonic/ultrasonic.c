#include "ultrasonic.h"
#include "delay.h"  /* 用于微秒级延时 */
#include "tim.h"
#include "main.h"  /* 引入引脚宏定义，方便移植 */

/*
 * ==============================================
 *           【两种工作模式说明】
 * ==============================================
 *
 * 【模式1：软件查询模式】
 *   - 调用：Ultrasonic_Init() + Ultrasonic_Read_cm()
 *   - 接线：ECHO -> PA7
 *   - 特点：阻塞式读取，调用即返回结果
 *   - 优点：逻辑简单直观
 *   - 缺点：调用时阻塞，占用CPU
 *
 * 【模式2：输入捕获中断模式（B站Keysking）】
 *   - 调用：Ultrasonic_IC_Init() + Ultrasonic_IC_Trigger() + Ultrasonic_IC_Read_cm()
 *   - 接线：ECHO -> PB4 (重要！！不是PA7！)
 *   - 特点：触发和读取分离，纯硬件计时
 *   - 优点：非阻塞，节省CPU，精度高
 *   - 缺点：需要理解中断和回调机制
 *
 * ==============================================
 */
/*
 * ==============================================
 *           【重要！之前失败的原因找到了！】
 * ==============================================
 * TIM3初始化后必须调用 HAL_TIM_Base_Start() 才会开始计数！
 * 之前TIM3配置了但是没有启动，计数器一直是0！
 * ==============================================
 */



/*
 * 函数名：Ultrasonic_Init
 * 功能：  初始化超声波模块
 * 重点：  【启动TIM3计数器！】这是关键！
 */
void Ultrasonic_Init(void)
{
    /* 【最重要的一步！】启动TIM3，让计数器开始跑！ */
    HAL_TIM_Base_Start(&htim3);
    
    /* TRIG引脚先拉低，准备触发 */
    HAL_GPIO_WritePin(TRIG_GPIO_Port, TRIG_Pin, GPIO_PIN_RESET);
    
    /* 等待模块稳定 */
    HAL_Delay(10);
}

/*
 * 函数名：Ultrasonic_Read_cm
 * 功能：  测量距离，单位：厘米
 * 说明：  用TIM3硬件计时，非常准确！
 */
float Ultrasonic_Read_cm(void)
{
    uint32_t timeout = 0;
    uint32_t echo_time = 0;
    float distance = 0;
    
    /*=====================
      第一步：发触发脉冲
      =====================*/
    /* 先拉低2us，确保干净 */
    HAL_GPIO_WritePin(TRIG_GPIO_Port, TRIG_Pin, GPIO_PIN_RESET);
    HAL_Delay_us(2);
    
    /* TRIG给15us高电平（数据手册要求至少10us） */
    HAL_GPIO_WritePin(TRIG_GPIO_Port, TRIG_Pin, GPIO_PIN_SET);
    HAL_Delay_us(15);
    HAL_GPIO_WritePin(TRIG_GPIO_Port, TRIG_Pin, GPIO_PIN_RESET);
    
    /*=====================
      第二步：等ECHO变高
      =====================*/
    timeout = 0;
    while(HAL_GPIO_ReadPin(ECHO_GPIO_Port, ECHO_Pin) == GPIO_PIN_RESET)
    {
        timeout++;
        if(timeout > 10000)
        {
            return 999.0f;  /* 超时，没收到信号 */
        }
    }
    
    /* ECHO变高了！清零定时器，开始计时 */
    __HAL_TIM_SET_COUNTER(&htim3, 0);
    
    /*=====================
      第三步：等ECHO变低
      =====================*/
    timeout = 0;
    while(HAL_GPIO_ReadPin(ECHO_GPIO_Port, ECHO_Pin) == GPIO_PIN_SET)
    {
        /* 用TIM3做超时检测 */
        if(__HAL_TIM_GET_COUNTER(&htim3) > US_TIMEOUT_US)
        {
            return 999.0f;  /* 超出最大测量范围 */
        }
    }
    
    /*=====================
      第四步：计算距离
      =====================*/
    /* 从TIM3读取准确的高电平时间 */
    echo_time = __HAL_TIM_GET_COUNTER(&htim3);
    
    /* 按照数据手册公式计算：距离(cm) = 时间(us) / 58 */
    distance = (float)echo_time / 58.0f;
    
    /* 范围检查：有效范围是2cm ~ 400cm */
    if(distance < US_MIN_DISTANCE || distance > US_MAX_DISTANCE)
    {
        return 999.0f;
    }
    
    return distance;
}

/*
 * 函数名：Ultrasonic_Read_mm
 * 功能：  测量距离，单位：毫米
 */
float Ultrasonic_Read_mm(void)
{
    uint32_t timeout = 0;
    uint32_t echo_time = 0;
    float distance = 0;
    
    /* 发触发脉冲 */
    HAL_GPIO_WritePin(TRIG_GPIO_Port, TRIG_Pin, GPIO_PIN_RESET);
    HAL_Delay_us(2);
    HAL_GPIO_WritePin(TRIG_GPIO_Port, TRIG_Pin, GPIO_PIN_SET);
    HAL_Delay_us(15);
    HAL_GPIO_WritePin(TRIG_GPIO_Port, TRIG_Pin, GPIO_PIN_RESET);
    
    /* 等待ECHO变高 */
    timeout = 0;
    while(HAL_GPIO_ReadPin(ECHO_GPIO_Port, ECHO_Pin) == GPIO_PIN_RESET)
    {
        timeout++;
        if(timeout > 10000) { return 9999.0f; }
    }
    
    /* 开始计时 */
    __HAL_TIM_SET_COUNTER(&htim3, 0);
    
    /* 等待ECHO变低 */
    while(HAL_GPIO_ReadPin(ECHO_GPIO_Port, ECHO_Pin) == GPIO_PIN_SET)
    {
        if(__HAL_TIM_GET_COUNTER(&htim3) > US_TIMEOUT_US) { return 9999.0f; }
    }
    
    /* 计算距离：距离(mm) = 时间(us) / 5.8 */
    echo_time = __HAL_TIM_GET_COUNTER(&htim3);
    distance = (float)echo_time / 5.8f;
    
    /* 范围检查：20mm ~ 4000mm */
    if(distance < 20.0f || distance > 4000.0f) { return 9999.0f; }
    
    return distance;
}

/* ==================================================
 *          【模式2】输入捕获模式驱动函数
 *          参考：B站Keysking TIM_InputCapture
 * ================================================== */

/*
 * 函数名：Ultrasonic_IC_Init
 * 功能：  初始化输入捕获模式
 * 注意：  ECHO必须接PB4！不是PA7！
 */
void Ultrasonic_IC_Init(void)
{
    /* 【最重要】启动TIM3输入捕获
     *  - 启动定时器基础时钟
     *  - 启动CH1输入捕获（上升沿，无中断）
     *  - 启动CH2输入捕获（下降沿，带中断）
     */
    TIM3_IC_Start();
    
    /* TRIG引脚先拉低，准备触发 */
    HAL_GPIO_WritePin(TRIG_GPIO_Port, TRIG_Pin, GPIO_PIN_RESET);
    
    /* 等待模块稳定 */
    HAL_Delay(10);
}

/*
 * 函数名：Ultrasonic_IC_Trigger
 * 功能：  发送一次触发脉冲
 * 特点：  非阻塞！发送后立即返回，硬件自动捕获
 * 说明：  触发后约25ms可以读取结果
 *         适合在定时器中断或操作系统任务中调用
 */
void Ultrasonic_IC_Trigger(void)
{
    /* 调用tim.c中实现的触发函数 */
    TIM3_IC_Trigger();
}

/*
 * 函数名：Ultrasonic_IC_Read_cm
 * 功能：  读取最新测量结果（厘米）
 * 特点：  非阻塞！直接读取全局变量
 * 说明：  返回的是上一次触发后的测量值
 *         调用前请确保已经调用过Ultrasonic_IC_Trigger()
 */
float Ultrasonic_IC_Read_cm(void)
{
    /* 直接从tim.c中导出的函数获取值 */
    return TIM3_IC_GetDistance_cm();
}

/*
 * 函数名：Ultrasonic_IC_Read_mm
 * 功能：  读取最新测量结果（毫米）
 * 特点：  非阻塞！直接读取全局变量
 */
float Ultrasonic_IC_Read_mm(void)
{
    /* 直接从tim.c中导出的函数获取值 */
    return TIM3_IC_GetDistance_mm();
}
