#ifndef __ADC_H
#define __ADC_H

#include "stm32f4xx.h"

/* 初始化 ADC1 通道 6 (PA6) 电池采样 */
void Battery_ADC_Init(void);

/* 获取当前 7.4V 动力电池实际电压 (V)，并可选返回原始 12 位采样值 (0~4095) */
float Battery_GetVoltage(uint16_t *out_raw_adc);

#endif /* __ADC_H */
