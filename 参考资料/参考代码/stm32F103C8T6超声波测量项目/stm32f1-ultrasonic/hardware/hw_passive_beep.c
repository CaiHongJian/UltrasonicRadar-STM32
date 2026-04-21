#include "hw_passive_beep.h"
#include "tim.h"

/**
 * @brief  初始化无源蜂鸣器（使用定时器PWM输出）
 * @param  htim: 定时器句柄指针
 * @param  channel: 定时器通道 (TIM_CHANNEL_1 ~ TIM_CHANNEL_4)
 * @retval HAL状态
 */
BEEP_HandleTypeDef Buzzer_Init(void) {
	BEEP_HandleTypeDef beep_t;	//初始化蜂鸣器句柄
	beep_t.htim = &htim3;				//分配pwm定时器
	beep_t.channel = TIM_CHANNEL_2;	//分配pwm输出通道
	return beep_t;
}

/**
 * @brief  设置蜂鸣器频率和占空比
 * @param  freq: 频率(Hz)，范围建议 500-5000Hz
 * @param  duty_cycle: 占空比(0.0-1.0)，无源蜂鸣器建议0.5
 * @retval 无
 */
void Buzzer_SetFreq(BEEP_HandleTypeDef *beep_t, float freq, float duty_cycle) {
    if (beep_t->htim == NULL || freq <= 0) return;
    
    // 限制占空比范围
    if (duty_cycle < 0.0f) duty_cycle = 0.0f;
    if (duty_cycle > 1.0f) duty_cycle = 1.0f;
    
		uint32_t timer_clock = 0;
    // 计算定时器周期和比较值
		if(beep_t->htim->Instance == TIM1)
		{
			timer_clock = HAL_RCC_GetPCLK2Freq();  	//获取APB2时钟频率
		}
    else
		{
			timer_clock = HAL_RCC_GetPCLK1Freq();  	//获取APB1时钟频率
			timer_clock = timer_clock * 2;					//通常是*2；
		}
    
    // 计算自动重装载值(ARR)和比较值(CCR)
    uint32_t arr = (uint32_t)(timer_clock / (beep_t->htim->Init.Prescaler + 1) / freq) - 1;
    uint32_t ccr = (uint32_t)(arr * duty_cycle);
    
    // 设置定时器参数
    __HAL_TIM_SET_AUTORELOAD(beep_t->htim, arr);
    switch(beep_t->channel) {
        case TIM_CHANNEL_1:
            __HAL_TIM_SET_COMPARE(beep_t->htim, TIM_CHANNEL_1, ccr);
            break;
        case TIM_CHANNEL_2:
            __HAL_TIM_SET_COMPARE(beep_t->htim, TIM_CHANNEL_2, ccr);
            break;
        case TIM_CHANNEL_3:
            __HAL_TIM_SET_COMPARE(beep_t->htim, TIM_CHANNEL_3, ccr);
            break;
        case TIM_CHANNEL_4:
            __HAL_TIM_SET_COMPARE(beep_t->htim, TIM_CHANNEL_4, ccr);
            break;
    }
}

/**
 * @brief  蜂鸣器发声
 * @param  freq: 频率(Hz)
 * @param  duration_ms: 持续时间(ms)，0表示持续发声
 * @retval 无
 */
void Buzzer_Beep(BEEP_HandleTypeDef *beep_t, float freq, uint32_t duration_ms) {
    if (beep_t->htim == NULL) return;
    
    // 设置频率并启动PWM
    Buzzer_SetFreq(beep_t,freq, 0.5f);  // 占空比50%
    HAL_TIM_PWM_Start(beep_t->htim, beep_t->channel);
    
    // 如果需要自动停止，延时后关闭
    if (duration_ms > 0) {
        HAL_Delay(duration_ms);
        Buzzer_Stop(beep_t);
    }
}

/**
 * @brief  播放简单提示音
 * @param  无
 * @retval 无
 */
void Buzzer_Alert(BEEP_HandleTypeDef *beep_t) {
    // 播放"滴滴"提示音
    Buzzer_Beep(beep_t,1000, 100);  // 1000Hz，100ms
    HAL_Delay(100);          // 间隔100ms
    Buzzer_Beep(beep_t,2000, 100);  // 2000Hz，100ms
}

/**
 * @brief  播放旋律（示例：生日快乐前四句）
 * @param  无
 * @retval 无
 */
void Buzzer_PlayMelody(BEEP_HandleTypeDef *beep_t) {
    // 音符频率(Hz)和时长(ms)
    const struct {
        float freq;
        uint16_t duration;
    } melody[] = {
        {262, 250}, {262, 250}, {294, 500}, {262, 500},
        {349, 500}, {330, 1000}, {262, 250}, {262, 250}
    };
    
    for (uint8_t i = 0; i < sizeof(melody)/sizeof(melody[0]); i++) {
        Buzzer_Beep(beep_t, melody[i].freq, melody[i].duration);
        HAL_Delay(50);  // 音符间间隔
    }
}

/**
 * @brief  停止蜂鸣器发声
 * @param  无
 * @retval 无
 */
void Buzzer_Stop(BEEP_HandleTypeDef *beep_t) {
    if (beep_t->htim != NULL) {
        HAL_TIM_PWM_Stop(beep_t->htim, beep_t->channel);
    }
}