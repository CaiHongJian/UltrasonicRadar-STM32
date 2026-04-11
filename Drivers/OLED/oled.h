#ifndef __OLED_H
#define __OLED_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

void OLED_Init(void);
void OLED_Clear(void);
void OLED_ShowChar(uint8_t Line, uint8_t Column, char Char);
void OLED_ShowString(uint8_t Line, uint8_t Column, char *String);
void OLED_ShowNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length);
void OLED_ShowSignedNum(uint8_t Line, uint8_t Column, int32_t Number, uint8_t Length);
void OLED_ShowHexNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length);
void OLED_ShowBinNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length);
void OLED_DrawBMP(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, const uint8_t *BMP);

/* ============ 新增绘图功能 ============ */

/**
 * @brief OLED画点函数
 * 
 * @param x 横坐标 0~127
 * @param y 纵坐标 0~63
 * @param pixel 1=点亮，0=熄灭
 */
void OLED_DrawPixel(uint8_t x, uint8_t y, uint8_t pixel);

/**
 * @brief OLED画线函数
 * 
 * @param x1,y1 起点坐标
 * @param x2,y2 终点坐标
 */
void OLED_DrawLine(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2);

/**
 * @brief 更新OLED显示缓冲区到屏幕
 * 
 * 所有画图操作都是先写到缓冲区，最后调用此函数刷新显示
 */
void OLED_Refresh(void);

/**
 * @brief 清空绘图缓冲区
 */
void OLED_ClearBuffer(void);

#ifdef __cplusplus
}
#endif

#endif
