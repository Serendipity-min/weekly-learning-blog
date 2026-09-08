#ifndef __BEEP_H
#define __BEEP_H

#include "stm32f4xx.h"

/**
 * @brief  蜂鸣器硬件与定时器配置说明
 *         - 物理引脚: PF8 (板载 BEEP 信号，连接 S8050 NPN 三极管基极)
 *         - 定时器通道: TIM13_CH1 (复用功能 AF9, GPIO_AF_TIM13)
 *         - 定时器时钟: APB1 定时器时钟 = 84MHz (SYSCLK 168MHz, APB1 DIV4, 定时器 x2)
 *         - 预分频器 PSC: 84 - 1 = 83 (计数基准频率 1MHz, 即每 1us 计数 1 次)
 *         - 占空比: 蜂鸣器发声最佳方波占空比为 50%
 *         - 静音控制: 将占空比设为 0 (CCR1 = 0)，输出低电平，三极管截止
 */

/* 常用音阶频率宏定义 (Hz) */
#define BEEP_NOTE_C4   262   /* Do */
#define BEEP_NOTE_D4   294   /* Re */
#define BEEP_NOTE_E4   330   /* Mi */
#define BEEP_NOTE_F4   349   /* Fa */
#define BEEP_NOTE_G4   392   /* Sol */
#define BEEP_NOTE_A4   440   /* La */
#define BEEP_NOTE_B4   494   /* Si */
#define BEEP_NOTE_C5   523   /* 高音 Do */

/* 常用提示音频率宏定义 */
#define BEEP_FREQ_DEFAULT 2000  /* 默认共振频率 2000Hz (2kHz) */
#define BEEP_FREQ_ALARM   2700  /* 高频报警音 2700Hz */

/* 函数接口声明 */
void BEEP_Init(void);
void BEEP_SetFreq(uint32_t freq, uint8_t duty_percent);
void BEEP_On(uint32_t freq);
void BEEP_Off(void);
void BEEP_PlayTone(uint32_t freq, uint32_t duration_ms);
void BEEP_Alarm(uint8_t count, uint32_t duration_ms);
uint8_t BEEP_GetState(void);
uint32_t BEEP_GetFreq(void);
uint8_t BEEP_GetDuty(void);

#endif /* __BEEP_H */
