#ifndef __ENCODER_H
#define __ENCODER_H

#include "stm32f4xx.h"

/* 初始化 TIM4 硬件编码器接口 (PB6=TIM4_CH1/E2A, PB7=TIM4_CH2/E2B) */
void Encoder_Init(void);

/* 100ms 定时采样更新函数 (由 TIM6 定时中断自动调用) */
void Encoder_100ms_Update(void);

/* 获取最新测得的电机输出轴转速 (RPM) 与 100ms 采样脉冲数 */
int16_t Encoder_GetSpeedRPM(int16_t *out_count);

#endif /* __ENCODER_H */
