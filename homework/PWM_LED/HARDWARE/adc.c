#include "adc.h"
#include "stm32f4xx_adc.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"

/**
 * @brief  初始化 ADC1 通道 6 (PA6) 采集电池电压
 */
void Battery_ADC_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    ADC_CommonInitTypeDef ADC_CommonInitStructure;
    ADC_InitTypeDef ADC_InitStructure;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);

    /* PA6: 模拟输入 (ADC1_IN6) */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* ADC 通用参数配置: 独立模式，时钟 4 分频 (84MHz / 4 = 21MHz) */
    ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;
    ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div4;
    ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;
    ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;
    ADC_CommonInit(&ADC_CommonInitStructure);

    /* ADC1 参数配置: 12 位分辨率，单通道单次转换 */
    ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;
    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
    ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_NbrOfConversion = 1;
    ADC_Init(ADC1, &ADC_InitStructure);

    /* 开启每次转换完成产生 EOC 标志并使能 ADC1 */
    ADC_EOCOnEachRegularChannelCmd(ADC1, ENABLE);
    ADC_Cmd(ADC1, ENABLE);
}

/**
 * @brief  采集电池分压并换算为实际电压值
 * @param  out_raw_adc: 可选输出 12 位 ADC 采样原始值 (0~4095)
 * @return 动力电池实际电压 (V)
 */
float Battery_GetVoltage(uint16_t *out_raw_adc)
{
    uint32_t sum = 0;
    /* 8 次均值滤波，消除瞬态抖动 */
    for (int i = 0; i < 8; i++)
    {
        ADC_RegularChannelConfig(ADC1, ADC_Channel_6, 1, ADC_SampleTime_480Cycles);
        ADC_SoftwareStartConv(ADC1);
        while (!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC));
        sum += ADC_GetConversionValue(ADC1);
    }
    uint16_t avg = (uint16_t)(sum / 8);
    if (out_raw_adc != 0)
    {
        *out_raw_adc = avg;
    }

    /*
     * 换算说明:
     * ADC 满刻度 4095 对应 3.3V
     * 驱动板 TB6612 板载 10k 与 1k 电阻构成的 1/11 分压电路:
     * Vbatt = (avg / 4095.0) * 3.3 * 11.0 = (avg / 4095.0) * 36.3
     */
    float raw_v = (float)avg * 3.3f / 4095.0f;
    float batt_v = raw_v * 11.0f;
    return batt_v;
}
