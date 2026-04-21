/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "tim.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "hw_key.h"
#include "hw_smg.h"
#include "hw_led.h"
#include "hw_ultrasonic.h"
#include "hw_passive_beep.h"
#include "mid_timer.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
enum
{
	key1 = 0,
	key2,
	key3,
	key4
};
enum
{
	led1 = 0,
	led2
};
enum
{
	NO_SET = 0,					//无设置状态
	SET_HIGH_THRESHOLD,	//设置高阈值状态
	SET_LOW_THRESHOLD		//设置低阈值状态
};
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
	struct key_class key_handle_t[4];	//按键句柄
	struct led_class led_handle_t[2];	//led句柄
	BEEP_HandleTypeDef beep_handle_t;	//蜂鸣器句柄
	uint8_t set_state = NO_SET;				//设置状态
	uint8_t sr04_run_state = RUN_MS_TIMER;	//超声波传感器运行状态
	uint8_t high_threshold = 30;			//默认高阈值
	uint8_t low_threshold = 5;				//默认低阈值
	uint16_t key_time_val = 0;				//按键定时器值
	uint16_t sr04_time_val = 0;				//超声波定时器值
	uint32_t ultrasonic_val_cm = 0;		//超声波测距值
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
  MX_TIM1_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  /* USER CODE BEGIN 2 */
	CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;	//初始化DWT用于高精度计时
	DWT->CYCCNT = 0;
	DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
	
	key_handle_t[key1] = key_init(KEY1_GPIO_Port,KEY1_Pin,GPIO_PIN_RESET);	//注册按键
	key_handle_t[key2] = key_init(KEY2_GPIO_Port,KEY2_Pin,GPIO_PIN_RESET);
	key_handle_t[key3] = key_init(KEY3_GPIO_Port,KEY3_Pin,GPIO_PIN_RESET);
	key_handle_t[key4] = key_init(KEY4_GPIO_Port,KEY4_Pin,GPIO_PIN_RESET);
	
	led_handle_t[led1] = led_init(LED1_GPIO_Port,LED1_Pin,GPIO_PIN_SET);	//注册led
	led_handle_t[led2] = led_init(LED2_GPIO_Port,LED2_Pin,GPIO_PIN_SET);
	
	beep_handle_t = Buzzer_Init();	//初始化蜂鸣器
	HAL_TIM_Base_Start_IT(&htim1);	//开始定时器中断计时
	HAL_TIM_Base_Start_IT(&htim2);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
		key_time_val = get_key_timer_value();	//获取按键定时器时长
		sr04_time_val = get_sr04_timer_value();	//获取sr04定时器时长
		
		if(key_time_val >= 10)
		{
			//10ms扫描一次
			set_key_bit_value(PAUSE_MS_TIMER);	//暂停按键扫描
			
			key_scanf(&key_handle_t[key1]);	//扫描按键
			key_scanf(&key_handle_t[key2]);
			key_scanf(&key_handle_t[key3]);
			key_scanf(&key_handle_t[key4]);
			
			key_time_val = 0;
			set_key_timer_value(key_time_val);
			set_key_bit_value(RUN_MS_TIMER);	//恢复按键扫描
		}
		
		if((key_handle_t[key1].key_state & key_handle_t[key2].key_state & 
				key_handle_t[key3].key_state & key_handle_t[key4].key_state) != KEY_NoPress)
		{
			//如果有按键按下
			HAL_TIM_Base_Stop_IT(&htim1);	//停止定时器中断
			if(key_handle_t[key1].key_state != KEY_NoPress)
			{
				if(key_handle_t[key1].key_state == KeyPress)
				{
					if(sr04_run_state == RUN_MS_TIMER){
						sr04_run_state = PAUSE_MS_TIMER;
					}
					else{
						sr04_run_state = RUN_MS_TIMER;
					}
				}
				key_handle_t[key1].key_state = KEY_NoPress;	
			}
			if(key_handle_t[key2].key_state != KEY_NoPress)
			{
				set_state++;
				if(set_state == SET_HIGH_THRESHOLD){
					clear_show();
					Seg7_DisplayNumber(high_threshold);
					Seg7_SetDigit(11,0,ENABLE);
				}
				else if(set_state == SET_LOW_THRESHOLD){
					clear_show();
					Seg7_DisplayNumber(low_threshold);
					Seg7_SetDigit(12,0,ENABLE);
				}
				else{
					set_state = NO_SET;
					sr04_run_state = RUN_MS_TIMER;
				}
				key_handle_t[key2].key_state = KEY_NoPress;	
			}
			if(key_handle_t[key3].key_state != KEY_NoPress)
			{
				if(set_state == SET_HIGH_THRESHOLD){
					high_threshold++;
					if(high_threshold >= 100){
						high_threshold = 100;
					}
					clear_show();
					Seg7_DisplayNumber(high_threshold);
					Seg7_SetDigit(11,0,ENABLE);
				}
				else if(set_state == SET_LOW_THRESHOLD){
					low_threshold++;
					if(low_threshold >= high_threshold){
						low_threshold = low_threshold - 1;
					}
					clear_show();
					Seg7_DisplayNumber(low_threshold);
					Seg7_SetDigit(12,0,ENABLE);
				}								
				key_handle_t[key3].key_state = KEY_NoPress;	
			}
			if(key_handle_t[key4].key_state != KEY_NoPress)
			{
				if(set_state == SET_HIGH_THRESHOLD){
					high_threshold--;
					if(high_threshold <= low_threshold){
						high_threshold = high_threshold + 1;
					}
					clear_show();
					Seg7_DisplayNumber(high_threshold);
					Seg7_SetDigit(11,0,ENABLE);
				}
				else if(set_state == SET_LOW_THRESHOLD){
					low_threshold--;
					if(low_threshold <= 1){
						low_threshold = 1;
					}
					clear_show();
					Seg7_DisplayNumber(low_threshold);
					Seg7_SetDigit(12,0,ENABLE);
				}
				key_handle_t[key4].key_state = KEY_NoPress;	
			}
			key_time_val = 0;
			set_key_timer_value(key_time_val);
			__HAL_TIM_SET_COUNTER(&htim1,0);	//复位计数值
			HAL_TIM_Base_Start_IT(&htim1);		//恢复定时器中断
		}
		
		if(sr04_time_val >= 500)
		{
			HAL_TIM_Base_Stop_IT(&htim1);	//停止定时器中断
			//每隔500ms扫描一次距离值
			if((sr04_run_state == RUN_MS_TIMER) && (set_state == NO_SET))
			{
				ultrasonic_val_cm = Ultrasonic_MeasureDistanceCm();
				Seg7_DisplayNumber(ultrasonic_val_cm);
				led_turn_on(&led_handle_t[led1]);
				led_turn_off(&led_handle_t[led2]);
				if((ultrasonic_val_cm < low_threshold) || (ultrasonic_val_cm > high_threshold))
				{
					led_turn_off(&led_handle_t[led1]);
					led_turn_on(&led_handle_t[led2]);
					Buzzer_Alert(&beep_handle_t);
				}
			}
			sr04_time_val = 0;
			set_sr04_timer_value(sr04_time_val);
			__HAL_TIM_SET_COUNTER(&htim1,0);	//复位计数值
			HAL_TIM_Base_Start_IT(&htim1);		//恢复定时器中断
		}
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

#ifdef  USE_FULL_ASSERT
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
