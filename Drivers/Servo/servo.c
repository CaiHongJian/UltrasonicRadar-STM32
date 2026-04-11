#include "servo.h"
#include "pwm.h"

/* 舵机全局状态变量 */
volatile uint8_t Servo_RunFlag = 1;    /* 舵机运行标志，上电默认开始旋转 */
float Servo_CurrentAngle = 0.0f;       /* 当前角度，上电从0度开始 */
float Servo_Step = 2.0f;               /* 默认步长2度，可以修改 */
int8_t Servo_Direction = 1;            /* 旋转方向：1=正转（角度增加），-1=反转（角度减少） */

/**
 * @brief 初始化舵机驱动模块。
 * 
 * 调用PWM初始化函数，配置定时器以产生舵机所需的PWM信号。
 * 初始化舵机到0度位置，并开启PWM输出。
 * 初始化旋转方向为正转（从0度向180度扫描）
 */
void Servo_Init(void)
{
    PWM_Init();                         /* 初始化PWM定时器 */
    Servo_CurrentAngle = 0.0f;          /* 从0度开始 */
    Servo_RunFlag = 1;                  /* 上电自动开始旋转 */
    Servo_Direction = 1;                /* 初始方向：0° → 180° */
    Servo_SetAngle(1, 0.0f);            /* 舵机1转到初始0度位置 */
}

/**
 * @brief 设置指定舵机的角度。
 * 
 * 将目标角度（0-180度）线性映射到PWM比较寄存器值（500-2500），
 * 然后根据舵机ID选择对应的PWM通道进行输出。
 * 计算公式：CCR = Angle / 180.0 * 2000 + 500
 * 
 * @param[in] ServoId  舵机ID号，取值范围1-4，分别对应不同的PWM通道。
 *                     舵机1 -> PA0 (TIM2_CH1)
 *                     舵机2 -> PA1 (TIM2_CH2) - 现在用作呼吸灯
 *                     舵机3 -> PA2 (TIM2_CH3)
 *                     舵机4 -> PA3 (TIM2_CH4)
 * @param[in] Angle    目标角度值，有效范围为0.0到180.0度。
 */
void Servo_SetAngle(uint8_t ServoId, float Angle)
{
    uint16_t CCR = 0;
    
    /* 角度范围限制，防止超出舵机机械范围 */
    if(Angle < 0.0f)   Angle = 0.0f;
    if(Angle > 180.0f) Angle = 180.0f;
    
    /* 角度转PWM脉宽：0度->500us，180度->2500us */
    CCR = (uint16_t)(Angle / 180.0f * 2000 + 500);
    
    switch (ServoId)
    {
        case 1:
            PWM_SetCompare1(CCR);   /* PA0引脚 - 我们的舵机 */
            break;
        case 2:
            PWM_SetCompare2(CCR);   /* PA1引脚 - */
            break;
        case 3:
            PWM_SetCompare3(CCR);   /* PA2引脚 */
            break;
        case 4:
            PWM_SetCompare4(CCR);   /* PA3引脚 */
            break;
        default:
            break;
    }
}

/**
 * @brief 舵机步进旋转（雷达式来回扫描模式）
 * 
 * 【功能说明】
 * 1. 只有当Servo_RunFlag为1时才执行旋转
 * 2. 每次调用按照设定的步长和方向改变角度
 * 3. 【已修复】到达180度后反转方向（180°→0°），到达0度后再次反转
 * 4. 实现真正的雷达扫描效果：0°→180°→0°→180°... 平滑往返
 * 5. 这个函数应该在主循环定时调用（比如每50ms调用一次）
 */
void Servo_StepRotate(void)
{
    /* 只有允许运行时才旋转 */
    if(Servo_RunFlag == 1)
    {
        /* 按照当前方向走一步 */
        Servo_CurrentAngle += Servo_Direction * Servo_Step;
        
        /* ======================================
                     边界换向处理
           ====================================== */
        
        /* 到达180度：反向，开始往0度走 */
        if(Servo_CurrentAngle >= 180.0f)
        {
            Servo_CurrentAngle = 180.0f;
            Servo_Direction = -1;  /* 反转：角度开始减少 */
        }
        /* 到达0度：反向，开始往180度走 */
        else if(Servo_CurrentAngle <= 0.0f)
        {
            Servo_CurrentAngle = 0.0f;
            Servo_Direction = 1;   /* 正转：角度开始增加 */
        }
        
        /* 设置舵机到新角度 */
        Servo_SetAngle(1, Servo_CurrentAngle);
    }
}

/**
 * @brief 设置舵机运行状态
 * 
 * @param state 1=允许舵机旋转，0=停止舵机旋转
 * 
 * 【注意】
 * 停止旋转时，舵机会保持在当前角度位置，不会断电
 */
void Servo_SetRunState(uint8_t state)
{
    if(state == 0)
    {
        Servo_RunFlag = 0;  /* 停止旋转 */
    }
    else
    {
        Servo_RunFlag = 1;  /* 继续旋转 */
    }
}

/**
 * @brief 设置舵机步进角度
 * 
 * @param step 每次步进的角度值，建议范围1-10度
 *             步长太小：扫描太慢，反应迟缓
 *             步长太大：扫描太快，测距点稀疏
 *             推荐值：2-5度
 */
void Servo_SetStep(float step)
{
    if(step >= 0.5f && step <= 20.0f)  /* 合理范围限制 */
    {
        Servo_Step = step;
    }
}
