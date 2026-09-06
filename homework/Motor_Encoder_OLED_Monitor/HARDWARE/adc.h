#ifndef __ADC_H
#define __ADC_H

#include "stm32f4xx.h"

/* 初始化 ADC1 (PA5: P9排针Pin51, PA6: P8排针Pin51)，并拉高 PB14 释放板载 Flash */
void Battery_ADC_Init(void);

/* 读取指定通道的单次原始 ADC 值 */
uint16_t ADC1_ReadChannel(uint8_t channel);

/*
 * 采集电池分压并换算为实际电压值 (V)
 * out_raw: 传出当前有效引脚的 12 位 ADC 原始采样值
 * out_pin: 传出当前生效的引脚编号 (5 表示 PA5/P9-51, 6 表示 PA6/P8-51)
 */
float Battery_GetVoltage(uint16_t *out_raw, uint8_t *out_pin);

#endif /* __ADC_H */
