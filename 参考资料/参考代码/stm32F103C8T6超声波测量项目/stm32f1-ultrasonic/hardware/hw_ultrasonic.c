#include "hw_ultrasonic.h"
#include <math.h>

/**
 * @brief  获取ECHO引脚的高电平持续时间（微秒）
 * @param  无
 * @retval 高电平持续时间（us），超时返回0
 */
static uint32_t Ultrasonic_GetEchoTime(void) {
    uint32_t timeout = 0;
    uint32_t start_time = 0;
    uint32_t current_time = 0;
    
    // 等待ECHO引脚变为高电平
    while (HAL_GPIO_ReadPin(ECHO_GPIO_Port, ECHO_Pin) == GPIO_PIN_RESET) {
        // 超时判断（每循环约1us）
        if (timeout++ > US_TIMEOUT_US) {
            return 0;  // 超时
        }
        // 简单延时约1us（根据系统时钟调整）
        for (uint8_t i = 0; i < SystemCoreClock / 1000000 / 4; i++);
    }
    
    // 记录高电平开始时间（使用DWT计数器）
    start_time = DWT->CYCCNT;
    
    // 等待ECHO引脚变为低电平
    timeout = 0;
    while (HAL_GPIO_ReadPin(ECHO_GPIO_Port, ECHO_Pin) == GPIO_PIN_SET) {
        // 超时判断
        current_time = DWT->CYCCNT;
        // 计算时间差（单位：us）
        uint32_t elapsed = (current_time - start_time) / (SystemCoreClock / 1000000);
        if (elapsed > US_TIMEOUT_US) {
            return 0;  // 超时
        }
    }
    
    // 计算高电平持续时间（us）
    return (DWT->CYCCNT - start_time) / (SystemCoreClock / 1000000);
}

/**
 * @brief  测量距离（米）
 * @param  无
 * @retval 距离（m），测量失败返回-1.0f
 */
float Ultrasonic_MeasureDistanceM(void) {
    uint32_t echo_time;
    float distance;
    float calibrated_speed;
    
    // 触发超声波测量：发送10us高电平
    HAL_GPIO_WritePin(TRIG_GPIO_Port, TRIG_Pin, GPIO_PIN_SET);
    // 精确延时10us（根据系统时钟调整循环次数）
    for (uint16_t i = 0; i < SystemCoreClock / 100000; i++);
    HAL_GPIO_WritePin(TRIG_GPIO_Port, TRIG_Pin, GPIO_PIN_RESET);
    
    // 获取回波时间
    echo_time = Ultrasonic_GetEchoTime();
    if (echo_time == 0) {
        return -1.0f;  // 测量失败
    }
    
    // 根据温度校准声速：v = 331.5 + 0.6*T (m/s)
    calibrated_speed = 331.5f + 0.6f * TEMPERATURE;
    
    // 计算距离：距离 = 声速 * 时间 / 2（往返路程）
    distance = (calibrated_speed * echo_time * 0.000001f) / 2.0f;
    
    return distance;
}

/**
 * @brief  测量距离（厘米）
 * @param  无
 * @retval 距离（cm），测量失败返回0
 */
uint16_t Ultrasonic_MeasureDistanceCm(void) {
    float distance_m = Ultrasonic_MeasureDistanceM();
    
    // 转换为厘米（1m = 100cm）并四舍五入
    if (distance_m > 0) {
        return (uint16_t)(distance_m * 100.0f + 0.5f);
    }
    
    return 0;  // 测量失败
}