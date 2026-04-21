#include "mid_timer.h"
#include "hw_smg.h"
#include "tim.h"

static __IO uint16_t key_timer_value;			//按键定时器
static __IO uint16_t sr04_timer_value;			//超声波定时器
static uint8_t key_timer_bit = RUN_MS_TIMER;		//ms定时器标志位
static uint8_t sr04_timer_bit = RUN_MS_TIMER;		//ms定时器标志位
/*
 * 函数内容：定时器更新中断回调函数
 * 函数参数：TIM_HandleTypeDef *htim -- 定时器句柄
 * 返回值：无
 */
 
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if(htim->Instance == TIM1)
	{
		if(key_timer_bit == RUN_MS_TIMER)
		{
			key_timer_value++;
			if(key_timer_value >= 10000){
				key_timer_value = 0;		//防止数据越界
			}
		}
		if(sr04_timer_bit == RUN_MS_TIMER)
		{
			sr04_timer_value++;
			if(sr04_timer_value >= 10000){
				sr04_timer_value = 0;		//防止数据越界
			}
		}
	}
	else if(htim->Instance == TIM2)
	{
		Seg7_Refresh();
	}
}

/*
 * 函数内容：得到按键定时器值
 * 函数参数：无
 * 返回值：uint16_t -- 定时器值
 */
uint16_t get_key_timer_value(void)
{
	return key_timer_value;
}

/*
 * 函数内容：得到超声波定时器值
 * 函数参数：无
 * 返回值：uint16_t -- 定时器值
 */
uint16_t get_sr04_timer_value(void)
{
	return sr04_timer_value;
}

/*
 * 函数内容：设置按键定时器值
 * 函数参数：无
 * 返回值：uint16_t -- 定时器值
 */
void set_key_timer_value(uint16_t value)
{
	key_timer_value =  value;
}

/*
 * 函数内容：设置超声波定时器值
 * 函数参数：无
 * 返回值：uint16_t -- 定时器值
 */
void set_sr04_timer_value(uint16_t value)
{
	sr04_timer_value =  value;
}

/*
 * 函数内容：设置按键定时器标志位值
 * 函数参数：uint8_t value
 * 返回值：无
 */
void set_key_bit_value(uint8_t value)
{
	key_timer_bit = value;
}

/*
 * 函数内容：设置超声波定时器标志位值
 * 函数参数：uint8_t value
 * 返回值：无
 */
void set_sr04_bit_value(uint8_t value)
{
	sr04_timer_bit = value;
}