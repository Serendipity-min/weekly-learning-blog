#ifndef __OLED_H
#define __OLED_H

#include "stm32f4xx.h"

/* 初始化 OLED 显示屏 (I2C: PB8=SCL, PB9=SDA) */
void OLED_Init(void);

/* 清屏函数 */
void OLED_Clear(void);

/* 设置显示光标坐标: x(0~127), y(页地址 0~7) */
void OLED_Set_Pos(uint8_t x, uint8_t y);

/* 显示单个 8x16 字符 */
void OLED_ShowChar(uint8_t x, uint8_t y, char chr);

/* 显示 8x16 字符串 */
void OLED_ShowString(uint8_t x, uint8_t y, const char *str);

#endif /* __OLED_H */
