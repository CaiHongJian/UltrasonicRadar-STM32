/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : 基于STM32的超声波雷达系统主程序
  ******************************************************************************
  * 【功能描述】
  * 1. 上电后舵机开始0-180度循环旋转，像雷达一样扫描
  * 2. 同步进行超声波测距，并将距离实时显示在OLED上
  * 3. 当距离小于设定阈值时：LED呼吸灯闪烁 + 蜂鸣器鸣叫 + 舵机停止
  * 4. Key1按下：舵机继续旋转扫描
  * 5. Key2按下：舵机立即停止旋转（任何时候都有效）
  * 6. 整个过程中，超声波测距和OLED显示一直进行，不受舵机状态影响
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "tim.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "delay.h"
#include "key.h"
#include "oled.h"
#include "ultrasonic.h"
#include "servo.h"
#include "breath_led.h"
#include "buzzer.h"
#include <stdio.h>

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* ==================== 系统参数配置 ==================== */
#define DISTANCE_THRESHOLD    30.0f   /* 距离报警阈值：小于30cm触发报警 */
#define SERVO_ROTATE_PERIOD   50      /* 舵机步进周期，单位：毫秒（值越大转越慢） */
#define ULTRASONIC_PERIOD     100     /* 超声波测距周期，单位：毫秒 */
#define BREATH_LED_PERIOD     20      /* 呼吸灯更新周期，单位：毫秒 */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

float distance = 0.0f;                /* 超声波测得的距离值 */
uint8_t alarm_flag = 0;               /* 报警标志：1=距离小于阈值，触发报警 */

/* 软件定时器变量 - 实现非阻塞延时 */
uint32_t timer_servo = 0;             /* 舵机定时器 */
uint32_t timer_ultrasonic = 0;        /* 超声波定时器 */
uint32_t timer_breath = 0;            /* 呼吸灯定时器 */

char oled_buf[32];                    /* OLED显示缓冲区 */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  /* USER CODE BEGIN 2 */
    
    /* ======================================
               模块初始化
       ====================================== */
    // delay_init();               /* 初始化延时函数 */
    OLED_Init();                /* 初始化OLED屏幕 */
    Key_Init();                 /* 初始化按键（中断方式） */
    Ultrasonic_IC_Init();       /* 初始化超声波模块（输入捕获模式） */
    Servo_Init();               /* 初始化舵机 */
    BreathLED_Init();           /* 初始化呼吸灯 */
    Buzzer_Init();              /* 初始化蜂鸣器 */
    
    /* 设置舵机步进角度 - 每次走3度，扫描速度适中 */
    Servo_SetStep(3.0f);
    
    /* 开机欢迎界面 */
    OLED_Clear();
    OLED_ShowString(1, 1, "Ultrasonic Radar");
    OLED_ShowString(2, 1, "   System Boot");
    OLED_ShowString(3, 1, " Initializing...");
    HAL_Delay(1500);
    OLED_Clear();
    
    /* 显示静态界面 */
    OLED_ShowString(1, 1, "Angle:");
    OLED_ShowString(2, 1, "Dist :");
    OLED_ShowString(3, 1, "Alarm:");
    OLED_ShowString(4, 1, "Servo:");
    
    /* 初始化软件定时器 - 获取当前系统时间 */
    timer_servo = HAL_GetTick();
    timer_ultrasonic = HAL_GetTick();
    timer_breath = HAL_GetTick();

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
        /* ======================================
                   0. 按键消抖处理
           【重要】必须放在最前面执行
                 中断只设标志，消抖在这里判断
           ====================================== */
        Key_DebounceProcess();
        
        
        /* ======================================
                   1. 按键命令处理
           消抖确认后的按键才执行操作
           ====================================== */
        
        /* Key1按下：舵机继续旋转 */
        if(Key1_Flag == 1)
        {
            Servo_SetRunState(1);          /* 舵机开始旋转 */
            Key_ClearFlag(1);              /* 清除按键标志 */
        }
        
        /* Key2按下：舵机立即停止 - 任何时候都有效！ */
        if(Key2_Flag == 1)
        {
            Servo_SetRunState(0);          /* 舵机立即停止 */
            Key_ClearFlag(2);              /* 清除按键标志 */
        }
        
        
        /* ======================================
                   2. 超声波测距（定时执行）
                   每100ms测量一次
           ====================================== */
        if(HAL_GetTick() - timer_ultrasonic >= ULTRASONIC_PERIOD)
        {
            timer_ultrasonic = HAL_GetTick();  /* 重置定时器 */
            
            /* 触发超声波测量 - 非阻塞 */
            Ultrasonic_IC_Trigger();
            
            /* 读取距离值 */
            distance = Ultrasonic_IC_Read_cm();
            
            /* ======================================
                       3. 距离阈值判断
               ====================================== */
            if(distance < DISTANCE_THRESHOLD && distance > 0)
            {
                /* 距离小于阈值，触发报警 */
                alarm_flag = 1;
                BreathLED_Enable(1);         /* 开启LED呼吸灯 */
                Buzzer_Alarm(1);             /* 开启蜂鸣器 */
                Servo_SetRunState(0);        /* 舵机停止转动 */
            }
            else
            {
                /* 距离正常，解除报警
                   注意：这里不自动恢复舵机旋转，需要按Key1手动恢复 */
                alarm_flag = 0;
                BreathLED_Enable(0);         /* 关闭呼吸灯 */
                Buzzer_Alarm(0);             /* 关闭蜂鸣器 */
            }
        }
        
        
        /* ======================================
                   4. 舵机步进旋转（定时执行）
                   每50ms步进一次
           ====================================== */
        if(HAL_GetTick() - timer_servo >= SERVO_ROTATE_PERIOD)
        {
            timer_servo = HAL_GetTick();     /* 重置定时器 */
            Servo_StepRotate();              /* 舵机步进（内部判断是否允许运行） */
        }
        
        
        /* ======================================
                   5. 呼吸灯效果更新（定时执行）
                   每20ms更新一次亮度
           ====================================== */
        if(HAL_GetTick() - timer_breath >= BREATH_LED_PERIOD)
        {
            timer_breath = HAL_GetTick();    /* 重置定时器 */
            BreathLED_Process();             /* 更新呼吸灯亮度 */
        }
        
        
        /* ======================================
                   6. OLED实时显示
                   每次循环都更新显示
           ====================================== */
        
        /* 显示当前舵机角度 */
        OLED_ShowNum(1, 7, (uint8_t)Servo_CurrentAngle, 3);
        OLED_ShowString(1, 10, "deg");
        
        /* 显示距离 */
        if(distance < 400.0f)  /* 有效距离 */
        {
            uint16_t int_part = (uint16_t)distance;
            uint8_t dec_part = (uint8_t)((distance - int_part) * 10);
            OLED_ShowNum(2, 7, int_part, 3);
            OLED_ShowChar(2, 10, '.');
            OLED_ShowNum(2, 11, dec_part, 1);
            OLED_ShowString(2, 12, "cm");
        }
        else  /* 超出测量范围 */
        {
            OLED_ShowString(2, 7, "---.-cm");
        }
        
        /* 显示报警状态 */
        if(alarm_flag == 1)
        {
            OLED_ShowString(3, 7, "WARN!");
        }
        else
        {
            OLED_ShowString(3, 7, "SAFE ");
        }
        
        /* 显示舵机状态 */
        if(Servo_RunFlag == 1)
        {
            OLED_ShowString(4, 7, "RUN  ");
        }
        else
        {
            OLED_ShowString(4, 7, "STOP ");
        }

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
