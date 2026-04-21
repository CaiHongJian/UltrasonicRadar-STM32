#ifndef __HW_KEY_H
#define __HW_KEY_H

#include "main.h"

//公共宏定义

#define DOUBLE_TIME 40  	//400ms内按下两次算双击
#define KEY_NoPress 0xFF	
#define KEY_ERROR   0x00
#define KEY_OK      0x01

/*------- 定义按键类使用 -- */

#define KeyPress 0x10
#define KeyDoublePress 0x20 
#define KeyLongPress  0x30

struct key_class
{
	GPIO_TypeDef *key_gpio;		//KEY的端口
	uint16_t key_pin;			//KEY的引脚
	uint8_t key_state;			//KEY的状态
	uint8_t key_press_level;	//按下按键的电平
	uint8_t keyCnt;				//用于按键计时消抖
	uint8_t keyFcnt; 			//用于判断按键多久间隔算双击
	uint8_t keyCount; 		   	//按键按下的次数
	uint8_t keyLongFlag; 		//按键长按标志位，只有初级进入时判断长按才有效
};

void key_scanf(struct key_class *key_handle);
struct key_class key_init(GPIO_TypeDef *key_gpio,uint16_t key_pin,uint8_t key_press_level);

#endif

