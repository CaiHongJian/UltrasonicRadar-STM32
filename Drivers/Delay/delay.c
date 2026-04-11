#include "delay.h"
void HAL_Delay_us(uint32_t us)
{
    uint32_t ticks = us * (SystemCoreClock / 1000000U) / 9;
    while(ticks--);
}
