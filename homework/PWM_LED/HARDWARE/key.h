#ifndef _KEY_H_
#define _KEY_H_

#include "stm32f4xx.h"

/* 按键扫描返回值定义 */
#define KEY_NONE     0   /* 无按键按下 */
#define KEY_UP_PRES  1   /* KEY_UP (PA0) 按下: 加速 (高电平有效) */
#define KEY0_PRES    2   /* KEY0 (PE4) 按下: 减速 (低电平有效) */
#define KEY1_PRES    3   /* KEY1 (PE3) 按下: 正转/启停 (低电平有效) */
#define KEY2_PRES    4   /* KEY2 (PE2) 按下: 反转/启停 (低电平有效) */

/* 初始化按键 GPIO */
void Key_Init(void);

/* 按键扫描检测函数 (单次触发模式) */
uint8_t Key_Scan(void);

#endif /* _KEY_H_ */
