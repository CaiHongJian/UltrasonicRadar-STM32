#include "hw_key.h"

/*
 * 函数内容：按键实例注册
 * 函数参数：
							GPIO_TypeDef *key_gpio -- 按键端口
							uint16_t key_pin -- 按键引脚
							key_press_level -- 按键按下时电平
 * 返回值：struct key_class -- 按键句柄
 */
struct key_class key_init(GPIO_TypeDef *key_gpio,uint16_t key_pin,uint8_t key_press_level)
{
	struct key_class key_handle;
	key_handle.key_gpio = key_gpio;
	key_handle.key_pin = key_pin;
	key_handle.key_press_level = key_press_level;
	key_handle.key_state = KEY_NoPress;
	key_handle.keyCnt = 0;
	key_handle.keyCount = 0;
	key_handle.keyFcnt = 0;
	key_handle.keyLongFlag = 0;
	return key_handle;
}

/*
 * 函数内容：扫描单个实例按键 -- 每隔10ms调用一次
 * 函数参数：struct key_class *key_handle -- 按键句柄
 * 返回值：无
 */
void key_scanf(struct key_class *key_handle)
{
	key_handle->key_state = KEY_NoPress;	//默认是没有按键按下
	//如果有按键按下
	if(HAL_GPIO_ReadPin(key_handle->key_gpio,key_handle->key_pin) == key_handle->key_press_level)
	{
		key_handle->keyCnt++;						//记录按键按下时间
		if(key_handle->keyCnt >= 120)
		{
			key_handle->keyCnt = 120;			//防止数据越界
		}
	}
	else
	{
		//如果按键抬起
		if((key_handle->keyCnt >= 100) && (key_handle->keyLongFlag == 0)){
			//按键大于1s且之前没有多击，证明是长按
			key_handle->key_state = KeyLongPress;
		}
		else if(key_handle->keyCnt >= 2)
		{
			//按键按下时间大于20ms，证明是一次按下
			key_handle->keyCount++;     			//按键按下的次数增加
			key_handle->keyFcnt = DOUBLE_TIME;   	//在指定时间内再次按下算双击
			key_handle->keyLongFlag = 1;    		//已经有一次短按了，才能等待多击结束后才可长按
		}
		key_handle->keyCnt = 0;
		if(key_handle->keyFcnt)
		{
			key_handle->keyFcnt--;  //间隔时间不断减少
			if(key_handle->keyFcnt <= 0)    
			{
					//间隔时间到了
					if(key_handle->keyCount == 1){
							//单击
							key_handle->key_state = KeyPress;
					}
					else if(key_handle->keyCount == 2){
							//双击
							key_handle->key_state = KeyDoublePress;
					}
					key_handle->keyFcnt = 0;
					key_handle->keyCount = 0;
					key_handle->keyLongFlag = 0;
			}
		}
	}
}
