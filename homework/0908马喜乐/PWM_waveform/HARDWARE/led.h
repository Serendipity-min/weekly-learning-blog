#ifndef _LED_H_
#define _LED_H_

#include "stm32f4xx.h"

/* 初始化板载指示灯 (LED0: PF9, LED1: PF10) */
void LED_Init(void);

/* 设置指示灯状态: 1=点亮, 0=熄灭 */
void LED0_Set(uint8_t on);
void LED1_Set(uint8_t on);

#endif /* _LED_H_ */
