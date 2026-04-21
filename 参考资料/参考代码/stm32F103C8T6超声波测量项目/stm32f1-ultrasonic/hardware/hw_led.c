#include "hw_led.h"

/*
 * 函数内容：点亮LED
 * 函数参数：struct led_class--led类指针
 * 返回值：无
 */
void led_turn_on(struct led_class *led)
{
	if(led->led_on_level == GPIO_PIN_RESET)
	{
		HAL_GPIO_WritePin(led->led_gpio,led->led_pin,GPIO_PIN_RESET);
	}
	else
	{
		HAL_GPIO_WritePin(led->led_gpio,led->led_pin,GPIO_PIN_SET);
	}
	led->led_state = LED_ON;
}

/*
 * 函数内容：熄灭LED
 * 函数参数：struct led_class--led类指针
 * 返回值：无
 */
void led_turn_off(struct led_class *led)
{
	if(led->led_on_level == GPIO_PIN_RESET)
	{
		HAL_GPIO_WritePin(led->led_gpio,led->led_pin,GPIO_PIN_SET);
	}
	else
	{
		HAL_GPIO_WritePin(led->led_gpio,led->led_pin,GPIO_PIN_RESET);
	}
	led->led_state = LED_OFF;
}

/*
 * 函数内容：翻转LED
 * 函数参数：struct led_class--led类指针
 * 返回值：无
 */
void led_toggle(struct led_class *led)
{
	if(led->led_state == LED_ON)
	{
		led_turn_off(led);
	}
	else
	{
		led_turn_on(led);
	}
}

/*
 * 函数内容：注册led实例
 * 函数参数：
					GPIO_TypeDef *led_gpio -- 端口号
					uint16_t led_pin -- 引脚号
					uint8_t led_on_level -- 打开电平标准
 * 返回值：struct led_class -- led实例句柄
 */
struct led_class led_init(GPIO_TypeDef *led_gpio,uint16_t led_pin,uint8_t led_on_level)
{
	struct led_class led_handle;
	led_handle.led_gpio = led_gpio;
	led_handle.led_pin = led_pin;
	led_handle.led_on_level = led_on_level;
	led_handle.led_state = GPIO_PIN_RESET;
	return led_handle;
}

/*
 * 函数内容：打开所有LED，直接根据硬件来，不做封装
 * 函数参数：
 * 返回值：无
 */
void led_turn_on_all(void)
{
	HAL_GPIO_WritePin(LED1_GPIO_Port,LED1_Pin,GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LED2_GPIO_Port,LED2_Pin,GPIO_PIN_RESET);
}

/*
 * 函数内容：关闭所有LED，直接根据硬件来，不做封装
 * 函数参数：
 * 返回值：无
 */
void led_turn_off_all(void)
{
	HAL_GPIO_WritePin(LED1_GPIO_Port,LED1_Pin,GPIO_PIN_SET);
	HAL_GPIO_WritePin(LED2_GPIO_Port,LED2_Pin,GPIO_PIN_SET);
}

/*
 * 函数内容：翻转，直接根据硬件来，不做封装
 * 函数参数：
 * 返回值：无
 */
void led_toggle_all(void)
{
	HAL_GPIO_TogglePin(LED1_GPIO_Port,LED1_Pin);
	HAL_GPIO_TogglePin(LED2_GPIO_Port,LED2_Pin);
}

