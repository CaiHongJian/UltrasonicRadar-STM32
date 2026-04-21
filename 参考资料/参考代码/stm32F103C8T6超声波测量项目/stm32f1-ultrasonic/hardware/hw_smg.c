#include "hw_smg.h"
#include <math.h>

// 共阴数码管段码表 (0-9, 10: '-', 11: 空白)
const uint8_t SEG_TABLE[14] = {
    0x3F, // 0: a+b+c+d+e+f
    0x06, // 1: b+c
    0x5B, // 2: a+b+d+e+g
    0x4F, // 3: a+b+c+d+g
    0x66, // 4: b+c+f+g
    0x6D, // 5: a+c+d+f+g
    0x7D, // 6: a+c+d+e+f+g
    0x07, // 7: a+b+c
    0x7F, // 8: 全亮
    0x6F, // 9: a+b+c+d+f+g
    0x40, // 10: '-' (仅g段亮)
		0x76,	// 11:'H'
		0x38,	// 12:'L'
    0x00  // 13: 空白
};

// 存储每个位置要显示的数字
static uint8_t display_buffer[4] = {13, 13, 13, 13};
// 存储每个位置的小数点状态
static FunctionalState dp_state[4] = {DISABLE, DISABLE, DISABLE, DISABLE};

/**
 * @brief  设置指定位置显示的数字和小数点状态
 * @param  digit: 要显示的数字（0-9）
 * @param  position: 位置（0-3，分别对应第1-4位）
 * @param  dp_state: 小数点状态（ENABLE/DISABLE）
 * @retval 无
 */
void Seg7_SetDigit(uint8_t digit, uint8_t position, FunctionalState dp_sta) {
    if (position > 3) return;  // 检查位置有效性
    if (digit > 13) digit = 13;  // 检查数字有效性
	
    display_buffer[position] = digit;
    dp_state[position] = dp_sta;
}

/**
 * @brief  显示一个4位数字（0-9999）
 * @param  number: 要显示的数字（0-9999）
 * @retval 无
 */
void Seg7_DisplayNumber(uint16_t number) {
    if (number > 9999) number = 9999;  // 限制最大显示值
    
    // 分解各位数字
    display_buffer[0] = number / 1000;             // 千位
    display_buffer[1] = (number % 1000) / 100;     // 百位
    display_buffer[2] = (number % 100) / 10;       // 十位
    display_buffer[3] = number % 10;               // 个位
    
    // 默认关闭所有小数点
    for (uint8_t i = 0; i < 4; i++) {
        dp_state[i] = DISABLE;
    }
		
		// 处理前导零
    if (display_buffer[0] == 0) {
        display_buffer[0] = 13;  // 空白
        if (display_buffer[1] == 0) {
            display_buffer[1] = 13;  // 空白
            if (display_buffer[2] == 0) {
                display_buffer[2] = 13;  // 空白
            }
        }
    }
}

/**
 * @brief  清除显示数据
 * @param  无
 * @retval 无
 */
void clear_show(void)
{
	// 清除之前的显示
	for (uint8_t i = 0; i < 4; i++) {
			display_buffer[i] = 13;
			dp_state[i] = DISABLE;
	}
}

/**
 * @brief  自动判断小数点位置并显示浮点数
 * @param  value: 要显示的浮点数
 * @retval 无（超出范围显示"----"）
 */
void Seg7_DisplayFloatAuto(float value) {
    // 清除之前的显示
    for (uint8_t i = 0; i < 4; i++) {
        display_buffer[i] = 13;
        dp_state[i] = DISABLE;
    }
    
    // 检查数值范围
    if (value < -999.9f || value > 9999.0f) {
        // 超出范围显示"----"
        for (uint8_t i = 0; i < 4; i++) {
            display_buffer[i] = 10;  // 显示横线
        }
        return;
    }
    
    // 处理负数
    uint8_t is_negative = 0;
    if (value < 0) {
        is_negative = 1;
        value = -value;
    }
    
    // 四舍五入到合适的精度
    uint32_t scaled_value;
    
    // 根据数值大小自动选择小数点位置
    if (value >= 1000.0f) {
        // 1000.0 - 9999.0: 整数显示，无小数位
        scaled_value = (uint32_t)(value + 0.5f);
        Seg7_DisplayNumber((uint16_t)scaled_value);
        if (is_negative) display_buffer[0] = 10;  // 显示负号
    }
    else if (value >= 100.0f) {
        // 100.0 - 999.9: 1位小数
        scaled_value = (uint32_t)(value * 10.0f + 0.5f);
        display_buffer[0] = (scaled_value / 1000) % 10;  // 百位
        display_buffer[1] = (scaled_value / 100) % 10;   // 十位
        display_buffer[2] = (scaled_value / 10) % 10;    // 个位
        display_buffer[3] = scaled_value % 10;           // 小数位
        dp_state[2] = ENABLE;  // 在个位后显示小数点
        
        // 处理前导零
        if (display_buffer[0] == 0) display_buffer[0] = 13;
    }
    else if (value >= 10.0f) {
        // 10.00 - 99.99: 2位小数
        scaled_value = (uint32_t)(value * 100.0f + 0.5f);
        display_buffer[0] = (scaled_value / 1000) % 10;  // 十位
        display_buffer[1] = (scaled_value / 100) % 10;   // 个位
        display_buffer[2] = (scaled_value / 10) % 10;    // 小数第一位
        display_buffer[3] = scaled_value % 10;           // 小数第二位
        dp_state[1] = ENABLE;  // 在个位后显示小数点
        
        // 处理前导零
        if (display_buffer[0] == 0) display_buffer[0] = 13;
    }
    else {
        // 0.000 - 9.999: 3位小数
        scaled_value = (uint32_t)(value * 1000.0f + 0.5f);
        display_buffer[0] = (scaled_value / 1000) % 10;  // 个位
        display_buffer[1] = (scaled_value / 100) % 10;   // 小数第一位
        display_buffer[2] = (scaled_value / 10) % 10;    // 小数第二位
        display_buffer[3] = scaled_value % 10;           // 小数第三位
        dp_state[0] = ENABLE;  // 在个位后显示小数点
        
        // 处理前导零（个位为0时不显示）
        if (display_buffer[0] == 0) display_buffer[0] = 13;
    }
    
    // 处理负数显示
    if (is_negative) {
        // 找到第一个非空白位置，在其左边显示负号
        for (int8_t i = 0; i < 4; i++) {
            if (display_buffer[i] != 13) {
                if (i > 0) {
                    display_buffer[i-1] = 10;  // 显示负号
                }
                break;
            }
        }
    }
}


/**
 * @brief  刷新显示（需要周期性调用，建议频率200Hz即5ms刷新一次）
 * @param  无
 * @retval 无
 */
void Seg7_Refresh(void) {
    static uint8_t current_pos = 0;  // 当前显示的位置
    
    // 关闭所有数码管
    HAL_GPIO_WritePin(DIG1_GPIO_Port, DIG1_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(DIG2_GPIO_Port, DIG2_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(DIG3_GPIO_Port, DIG3_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(DIG4_GPIO_Port, DIG4_Pin, GPIO_PIN_RESET);
    
    // 获取当前位置的段码数据
    uint8_t seg_data = SEG_TABLE[display_buffer[current_pos]];
    if (dp_state[current_pos] == ENABLE) {
        seg_data |= 0x80;  // 点亮小数点
    }
    
    // 定义段码引脚数组，与seg_data的位对应
    const uint16_t seg_pins[8] = {SMG_A_Pin, SMG_B_Pin, SMG_C_Pin, SMG_D_Pin, 
                                 SMG_E_Pin, SMG_F_Pin, SMG_G_Pin, SMG_DP_Pin};
    
    // 循环设置各段码
    for(uint8_t i = 0; i < 8; i++) {
        HAL_GPIO_WritePin(GPIOA, seg_pins[i], 
                         (seg_data & (1 << i)) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    }
    
    // 打开当前位置的数码管（共阴，低电平有效）
    switch (current_pos) {
        case 0:
            HAL_GPIO_WritePin(DIG1_GPIO_Port, DIG1_Pin, GPIO_PIN_SET);
            break;
        case 1:
            HAL_GPIO_WritePin(DIG2_GPIO_Port, DIG2_Pin, GPIO_PIN_SET);
            break;
        case 2:
            HAL_GPIO_WritePin(DIG3_GPIO_Port, DIG3_Pin, GPIO_PIN_SET);
            break;
        case 3:
            HAL_GPIO_WritePin(DIG4_GPIO_Port, DIG4_Pin, GPIO_PIN_SET);
            break;
    }
    
    // 切换到下一个位置
    current_pos = (current_pos + 1) % 4;
}


