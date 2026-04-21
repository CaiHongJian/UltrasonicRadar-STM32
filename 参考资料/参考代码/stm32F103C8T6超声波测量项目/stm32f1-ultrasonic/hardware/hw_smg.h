#ifndef __HW_SMG_H
#define __HW_SMG_H

#include "main.h"

// º¯ÊıÉùÃ÷
void Seg7_SetDigit(uint8_t digit, uint8_t position, FunctionalState dp_state);
void Seg7_DisplayNumber(uint16_t number);
void Seg7_DisplayFloatAuto(float value);
void Seg7_Refresh(void);
void clear_show(void);
#endif

