#include "radar_ui.h"
#include <math.h>

/* 正弦函数查表（0~180度，放大1000倍）
 * 不用单片机算浮点三角函数，直接查表最快
 */
const int16_t SinTable[91] = {
    0, 17, 35, 52, 70, 87, 105, 122, 139, 156,
    174, 191, 208, 225, 242, 259, 276, 292, 309, 326,
    342, 358, 375, 391, 407, 423, 438, 454, 469, 485,
    500, 515, 530, 545, 559, 574, 588, 602, 616, 629,
    643, 656, 669, 682, 695, 707, 719, 731, 743, 755,
    766, 777, 788, 799, 809, 819, 829, 839, 848, 857,
    866, 875, 883, 891, 899, 906, 914, 921, 927, 934,
    940, 946, 951, 956, 961, 966, 970, 974, 978, 982,
    985, 988, 990, 993, 995, 996, 998, 999, 999, 1000, 1000
};

/**
 * @brief 快速正弦函数（查表法）
 * 
 * @param angle 角度 0~180度
 * @return int16_t sin值 × 1000
 */
static int16_t fast_sin(uint8_t angle)
{
    if(angle > 180) angle = 180;
    if(angle <= 90)
        return SinTable[angle];
    else
        return SinTable[180 - angle];
}

/**
 * @brief 快速余弦函数（查表法）
 * 
 * @param angle 角度 0~180度
 * @return int16_t cos值 × 1000
 */
static int16_t fast_cos(uint8_t angle)
{
    if(angle > 180) angle = 180;
    if(angle <= 90)
        return SinTable[90 - angle];
    else
        return -SinTable[angle - 90];
}

/**
 * @brief 雷达UI初始化
 * 
 * 清空屏幕和缓冲区
 */
void RadarUI_Init(void)
{
    OLED_ClearBuffer();
    OLED_Clear();
}

/**
 * @brief 绘制雷达背景（半圆形刻度线）
 * 
 * 效果：底部中心为原点，向上的半圆形雷达
 * 有三个同心圆作为距离刻度，还有角度刻度线
 */
void RadarUI_DrawBackground(void)
{
    uint8_t i;
    int16_t x, y;
    
    /* -------- 绘制三个同心圆刻度 -------- */
    /* 内圈：20像素 = 10cm */
    for(i = 0; i <= 180; i += 2)
    {
        x = RADAR_CENTER_X + (20 * fast_cos(i)) / 1000;
        y = RADAR_CENTER_Y - (20 * fast_sin(i)) / 1000;
        OLED_DrawPixel(x, y, 1);
    }
    
    /* 中圈：40像素 = 25cm */
    for(i = 0; i <= 180; i += 2)
    {
        x = RADAR_CENTER_X + (40 * fast_cos(i)) / 1000;
        y = RADAR_CENTER_Y - (40 * fast_sin(i)) / 1000;
        OLED_DrawPixel(x, y, 1);
    }
    
    /* 外圈：60像素 = 50cm */
    for(i = 0; i <= 180; i++)
    {
        x = RADAR_CENTER_X + (58 * fast_cos(i)) / 1000;
        y = RADAR_CENTER_Y - (58 * fast_sin(i)) / 1000;
        OLED_DrawPixel(x, y, 1);
    }
    
    /* -------- 绘制角度刻度线 -------- */
    /* 0度 - 左水平线 */
    OLED_DrawLine(RADAR_CENTER_X - 60, RADAR_CENTER_Y, RADAR_CENTER_X + 60, RADAR_CENTER_Y);
    
    /* 45度线 */
    OLED_DrawLine(RADAR_CENTER_X, RADAR_CENTER_Y, 
                  RADAR_CENTER_X - 42, RADAR_CENTER_Y - 42);
    
    /* 90度 - 垂直线 */
    OLED_DrawLine(RADAR_CENTER_X, RADAR_CENTER_Y, 
                  RADAR_CENTER_X, RADAR_CENTER_Y - 60);
    
    /* 135度线 */
    OLED_DrawLine(RADAR_CENTER_X, RADAR_CENTER_Y, 
                  RADAR_CENTER_X + 42, RADAR_CENTER_Y - 42);
    
    /* -------- 标注刻度文字 -------- */
    /* 距离标注 */
    OLED_DrawPixel(RADAR_CENTER_X + 20, RADAR_CENTER_Y - 1, 1);
    OLED_DrawPixel(RADAR_CENTER_X + 40, RADAR_CENTER_Y - 1, 1);
}

/**
 * @brief 绘制雷达扫描线
 * 
 * @param angle 当前扫描角度 0~180度
 *              0度 = 正左，90度 = 正上，180度 = 正右
 */
void RadarUI_DrawScanLine(float angle_f)
{
    int16_t x_end, y_end;
    uint8_t angle = (uint8_t)angle_f;
    
    /* 计算扫描线终点坐标 */
    x_end = RADAR_CENTER_X + (58 * fast_cos(angle)) / 1000;
    y_end = RADAR_CENTER_Y - (58 * fast_sin(angle)) / 1000;
    
    /* 从圆心画扫描线 */
    OLED_DrawLine(RADAR_CENTER_X, RADAR_CENTER_Y, x_end, y_end);
    
    /* 扫描线头画一个亮点，更有动感 */
    OLED_DrawPixel(x_end, y_end, 1);
    if(x_end > 0) OLED_DrawPixel(x_end - 1, y_end, 1);
    if(x_end < 127) OLED_DrawPixel(x_end + 1, y_end, 1);
    if(y_end > 0) OLED_DrawPixel(x_end, y_end - 1, 1);
}

/**
 * @brief 在雷达上标记一个目标点
 * 
 * @param angle 目标角度 0~180度
 * @param distance_cm 目标距离（厘米）
 *        距离按比例显示：最大60cm对应60像素半径
 */
void RadarUI_DrawTarget(float angle_f, float distance_cm)
{
    int16_t x, y, radius;
    uint8_t angle = (uint8_t)angle_f;
    
    /* 距离超过60cm的，按60cm显示在边缘 */
    if(distance_cm > 60.0f) distance_cm = 60.0f;
    if(distance_cm < 0) distance_cm = 0;
    
    /* 距离换算成像素半径 */
    radius = (int16_t)(distance_cm * 1.0f);  /* 1cm = 1像素 */
    if(radius > 58) radius = 58;
    
    /* 计算目标点坐标 */
    x = RADAR_CENTER_X + (radius * fast_cos(angle)) / 1000;
    y = RADAR_CENTER_Y - (radius * fast_sin(angle)) / 1000;
    
    /* 画一个3x3的大方块标记目标，醒目！ */
    OLED_DrawPixel(x, y, 1);
    if(x > 0) OLED_DrawPixel(x - 1, y, 1);
    if(x < 127) OLED_DrawPixel(x + 1, y, 1);
    if(y > 0) OLED_DrawPixel(x, y - 1, 1);
    if(y < 63) OLED_DrawPixel(x, y + 1, 1);
}

/**
 * @brief 在雷达右侧显示信息
 * 
 * @param distance 当前距离值
 * @param alarm_state 报警状态 1=报警 0=正常
 * @param servo_state 舵机状态 1=运行 0=停止
 */
void RadarUI_ShowInfo(float distance, uint8_t alarm_state, uint8_t servo_state)
{
    /* 这个函数用原来的字符显示，放在雷达图的右上角空白处
     * 因为绘图和字符是两套显示系统，交替使用会有冲突
     * 所以我们把字符信息也画到缓冲区，最后一起刷新
     * 这里简化，直接用原来的字符显示函数显示在第4行
     */
    
    /* 显示距离值 */
    if(distance < 400.0f)
    {
        uint16_t int_part = (uint16_t)distance;
        OLED_ShowNum(4, 2, int_part, 3);
        OLED_ShowString(4, 5, "cm");
    }
    else
    {
        OLED_ShowString(4, 2, "---cm");
    }
    
    /* 报警状态 */
    if(alarm_state)
    {
        OLED_ShowString(4, 7, "ALM");
    }
    else
    {
        OLED_ShowString(4, 7, "OK!");
    }
    
    /* 舵机状态 */
    if(servo_state)
    {
        OLED_ShowString(4, 11, "RUN");
    }
    else
    {
        OLED_ShowString(4, 11, "STP");
    }
}
