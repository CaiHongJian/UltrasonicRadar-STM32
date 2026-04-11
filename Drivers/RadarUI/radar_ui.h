#ifndef __RADAR_UI_H
#define __RADAR_UI_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include "oled.h"

/* 雷达UI坐标系 */
#define RADAR_CENTER_X    64    /* 雷达中心X坐标 */
#define RADAR_CENTER_Y    63    /* 雷达中心Y坐标（底部） */
#define RADAR_RADIUS      60    /* 雷达最大半径 */

/**
 * @brief 雷达UI初始化
 * 
 * 清空屏幕，准备绘制雷达
 */
void RadarUI_Init(void);

/**
 * @brief 绘制雷达背景（刻度圆弧、基准线）
 * 
 * 绘制三个同心圆作为距离刻度
 * 绘制0°、45°、90°、135°、180°刻度线
 */
void RadarUI_DrawBackground(void);

/**
 * @brief 更新雷达扫描线位置
 * 
 * @param angle 当前扫描角度 0~180度
 */
void RadarUI_DrawScanLine(float angle);

/**
 * @brief 在雷达上标记一个检测点
 * 
 * @param angle 目标角度 0~180度
 * @param distance_cm 目标距离（厘米），将按比例显示
 */
void RadarUI_DrawTarget(float angle, float distance_cm);

/**
 * @brief 在雷达右上角显示数值信息
 * 
 * @param distance 当前距离值
 * @param alarm_state 报警状态 1=报警 0=正常
 * @param servo_state 舵机状态 1=运行 0=停止
 */
void RadarUI_ShowInfo(float distance, uint8_t alarm_state, uint8_t servo_state);

#ifdef __cplusplus
}
#endif

#endif
