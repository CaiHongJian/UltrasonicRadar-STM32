#include "oled.h"
#include "oled_font.h"
#include <string.h>

#define OLED_W_SCL(x)   HAL_GPIO_WritePin(OLED_SCL_GPIO_Port, OLED_SCL_Pin, (x) ? GPIO_PIN_SET : GPIO_PIN_RESET)
#define OLED_W_SDA(x)   HAL_GPIO_WritePin(OLED_SDA_GPIO_Port, OLED_SDA_Pin, (x) ? GPIO_PIN_SET : GPIO_PIN_RESET)

/* OLED显存缓冲区 128x64 */
uint8_t OLED_Buffer[8][128] = {0};

void OLED_I2C_Start(void)
{
    OLED_W_SDA(1);
    OLED_W_SCL(1);
    OLED_W_SDA(0);
    OLED_W_SCL(0);
}

void OLED_I2C_Stop(void)
{
    OLED_W_SDA(0);
    OLED_W_SCL(1);
    OLED_W_SDA(1);
}

void OLED_I2C_SendByte(uint8_t Byte)
{
    uint8_t i;
    for (i = 0; i < 8; i++)
    {
        OLED_W_SDA(Byte & (0x80 >> i));
        OLED_W_SCL(1);
        OLED_W_SCL(0);
    }
    OLED_W_SCL(1);
    OLED_W_SCL(0);
}

void OLED_WriteCommand(uint8_t Command)
{
    OLED_I2C_Start();
    OLED_I2C_SendByte(0x78);
    OLED_I2C_SendByte(0x00);
    OLED_I2C_SendByte(Command);
    OLED_I2C_Stop();
}

void OLED_WriteData(uint8_t Data)
{
    OLED_I2C_Start();
    OLED_I2C_SendByte(0x78);
    OLED_I2C_SendByte(0x40);
    OLED_I2C_SendByte(Data);
    OLED_I2C_Stop();
}

void OLED_SetCursor(uint8_t Y, uint8_t X)
{
    OLED_WriteCommand(0xB0 | Y);
    OLED_WriteCommand(0x10 | ((X & 0xF0) >> 4));
    OLED_WriteCommand(0x00 | (X & 0x0F));
}

void OLED_Clear(void)
{
    uint8_t i, j;
    for (j = 0; j < 8; j++)
    {
        OLED_SetCursor(j, 0);
        for (i = 0; i < 128; i++)
        {
            OLED_WriteData(0x00);
        }
    }
    OLED_ClearBuffer();
}

/**
 * @brief 清空绘图缓冲区
 */
void OLED_ClearBuffer(void)
{
    memset(OLED_Buffer, 0, sizeof(OLED_Buffer));
}

/**
 * @brief 将缓冲区内容刷新到OLED屏幕
 * 
 * 所有画图操作都是先写到缓冲区，最后调用此函数显示
 */
void OLED_Refresh(void)
{
    uint8_t page, column;
    for(page = 0; page < 8; page++)
    {
        OLED_SetCursor(page, 0);
        for(column = 0; column < 128; column++)
        {
            OLED_WriteData(OLED_Buffer[page][column]);
        }
    }
}

/**
 * @brief 在缓冲区画一个点
 * 
 * @param x 横坐标 0~127
 * @param y 纵坐标 0~63
 * @param pixel 1=点亮，0=熄灭
 */
void OLED_DrawPixel(uint8_t x, uint8_t y, uint8_t pixel)
{
    if(x >= 128 || y >= 64) return;  /* 超出范围 */
    
    uint8_t page = y / 8;
    uint8_t bit = y % 8;
    
    if(pixel)
    {
        OLED_Buffer[page][x] |= (1 << bit);   /* 置1点亮 */
    }
    else
    {
        OLED_Buffer[page][x] &= ~(1 << bit);  /* 清0熄灭 */
    }
}

/**
 * @brief 画线函数（Bresenham算法）
 * 
 * @param x1,y1 起点
 * @param x2,y2 终点
 */
void OLED_DrawLine(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2)
{
    int dx = x2 - x1;
    int dy = y2 - y1;
    int ux = ((dx > 0) << 1) - 1;
    int uy = ((dy > 0) << 1) - 1;
    int x = x1, y = y1, eps = 0;
    
    dx = (dx > 0) ? dx : -dx;
    dy = (dy > 0) ? dy : -dy;
    
    if(dx > dy)
    {
        for(x = x1; x != x2; x += ux)
        {
            OLED_DrawPixel(x, y, 1);
            eps += dy;
            if((eps << 1) >= dx)
            {
                y += uy;
                eps -= dx;
            }
        }
    }
    else
    {
        for(y = y1; y != y2; y += uy)
        {
            OLED_DrawPixel(x, y, 1);
            eps += dx;
            if((eps << 1) >= dy)
            {
                x += ux;
                eps -= dy;
            }
        }
    }
    OLED_DrawPixel(x2, y2, 1);
}

void OLED_ShowChar(uint8_t Line, uint8_t Column, char Char)
{
    uint8_t i;
    OLED_SetCursor((Line - 1) * 2, (Column - 1) * 8);
    for (i = 0; i < 8; i++)
    {
        OLED_WriteData(OLED_F8x16[Char - ' '][i]);
    }
    OLED_SetCursor((Line - 1) * 2 + 1, (Column - 1) * 8);
    for (i = 0; i < 8; i++)
    {
        OLED_WriteData(OLED_F8x16[Char - ' '][i + 8]);
    }
}

void OLED_ShowString(uint8_t Line, uint8_t Column, char *String)
{
    uint8_t i;
    for (i = 0; String[i] != '\0'; i++)
    {
        OLED_ShowChar(Line, Column + i, String[i]);
    }
}

uint32_t OLED_Pow(uint32_t X, uint32_t Y)
{
    uint32_t Result = 1;
    while (Y--)
    {
        Result *= X;
    }
    return Result;
}

void OLED_ShowNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length)
{
    uint8_t i;
    for (i = 0; i < Length; i++)
    {
        OLED_ShowChar(Line, Column + i, Number / OLED_Pow(10, Length - i - 1) % 10 + '0');
    }
}

void OLED_ShowSignedNum(uint8_t Line, uint8_t Column, int32_t Number, uint8_t Length)
{
    uint8_t i;
    uint32_t Number1;
    if (Number >= 0)
    {
        OLED_ShowChar(Line, Column, '+');
        Number1 = Number;
    }
    else
    {
        OLED_ShowChar(Line, Column, '-');
        Number1 = -Number;
    }
    for (i = 0; i < Length; i++)
    {
        OLED_ShowChar(Line, Column + i + 1, Number1 / OLED_Pow(10, Length - i - 1) % 10 + '0');
    }
}

void OLED_ShowHexNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length)
{
    uint8_t i, SingleNumber;
    for (i = 0; i < Length; i++)
    {
        SingleNumber = Number / OLED_Pow(16, Length - i - 1) % 16;
        if (SingleNumber < 10)
        {
            OLED_ShowChar(Line, Column + i, SingleNumber + '0');
        }
        else
        {
            OLED_ShowChar(Line, Column + i, SingleNumber - 10 + 'A');
        }
    }
}

void OLED_ShowBinNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length)
{
    uint8_t i;
    for (i = 0; i < Length; i++)
    {
        OLED_ShowChar(Line, Column + i, Number / OLED_Pow(2, Length - i - 1) % 10 + '0');
    }
}

void OLED_DrawBMP(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, const uint8_t *BMP)
{
    uint16_t j = 0;
    uint8_t x, y;
    
    if (y1 % 8 == 0)
    {
        y = y1 / 8;
    }
    else
    {
        y = y1 / 8 + 1;
    }
    
    for (y = y0; y < y1; y++)
    {
        OLED_SetCursor(y, x0);
        for (x = x0; x < x1; x++)
        {
            OLED_WriteData(BMP[j++]);
        }
    }
}

void OLED_Init(void)
{
    uint32_t i, j;
    
    for (i = 0; i < 1000; i++)
    {
        for (j = 0; j < 1000; j++);
    }
    
    OLED_WriteCommand(0xAE);
    OLED_WriteCommand(0xD5);
    OLED_WriteCommand(0x80);
    OLED_WriteCommand(0xA8);
    OLED_WriteCommand(0x3F);
    OLED_WriteCommand(0xD3);
    OLED_WriteCommand(0x00);
    OLED_WriteCommand(0x40);
    OLED_WriteCommand(0xA1);
    OLED_WriteCommand(0xC8);
    OLED_WriteCommand(0xDA);
    OLED_WriteCommand(0x12);
    OLED_WriteCommand(0x81);
    OLED_WriteCommand(0xCF);
    OLED_WriteCommand(0xD9);
    OLED_WriteCommand(0xF1);
    OLED_WriteCommand(0xDB);
    OLED_WriteCommand(0x30);
    OLED_WriteCommand(0xA4);
    OLED_WriteCommand(0xA6);
    OLED_WriteCommand(0x8D);
    OLED_WriteCommand(0x14);
    OLED_WriteCommand(0xAF);
    OLED_Clear();
}
