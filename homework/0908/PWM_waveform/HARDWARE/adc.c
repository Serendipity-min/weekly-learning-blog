#include "adc.h"
#include "stm32f4xx_adc.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"

/**
 * @brief  初始化 ADC1 采集电池分压，支持双引脚输入 (PA5 与 PA6)
 *         - PA5: P9 排针 Pin 51 (靠近电机方向控制引脚 PE0/PE1)
 *         - PA6: P8 排针 Pin 51 (靠近 OLED 引脚 PB8/PB9)
 *         - PB14: 输出高电平，禁用板载 W25Q128 Flash，确保 PA6(MISO) 处于高阻态不被驱动
 */
void Battery_ADC_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    ADC_CommonInitTypeDef ADC_CommonInitStructure;
    ADC_InitTypeDef ADC_InitStructure;

    /* 开启 GPIOA, GPIOB 与 ADC1 外设时钟 */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);

    /* 1. 禁用板载 SPI Flash: PB14 输出推挽高电平 (F_CS = 1) */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    GPIO_SetBits(GPIOB, GPIO_Pin_14);

    /* 2. 配置 PA5 (ADC1_IN5) 与 PA6 (ADC1_IN6) 为模拟输入 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* 3. ADC 通用参数初始化: 独立模式，时钟 4 分频 (84MHz / 4 = 21MHz) */
    ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;
    ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div4;
    ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;
    ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;
    ADC_CommonInit(&ADC_CommonInitStructure);

    /* 4. ADC1 基本参数初始化 */
    ADC_StructInit(&ADC_InitStructure);
    ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;
    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
    ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_NbrOfConversion = 1;
    ADC_Init(ADC1, &ADC_InitStructure);

    ADC_EOCOnEachRegularChannelCmd(ADC1, ENABLE);
    ADC_Cmd(ADC1, ENABLE);
}

/**
 * @brief  单次读取 ADC1 指定通道 (480 周期充分采样)
 */
uint16_t ADC1_ReadChannel(uint8_t channel)
{
    ADC_ClearFlag(ADC1, ADC_FLAG_EOC);
    ADC_RegularChannelConfig(ADC1, channel, 1, ADC_SampleTime_480Cycles);
    ADC_SoftwareStartConv(ADC1);
    while (!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC));
    return ADC_GetConversionValue(ADC1);
}

/**
 * @brief  采集电池电压并自动识别 PA5 / PA6 接线
 */
float Battery_GetVoltage(uint16_t *out_raw, uint8_t *out_pin)
{
    uint32_t sum5 = 0, sum6 = 0;
    int i;
    /* 8 次采样均值滤波 */
    for (i = 0; i < 8; i++)
    {
        sum5 += ADC1_ReadChannel(ADC_Channel_5);
        sum6 += ADC1_ReadChannel(ADC_Channel_6);
    }
    uint16_t avg5 = (uint16_t)(sum5 / 8);
    uint16_t avg6 = (uint16_t)(sum6 / 8);

    uint16_t chosen_raw = 0;
    uint8_t chosen_pin = 6;

    /*
     * 智能识别接线引脚:
     * 7.4V 锂电池分压后在 7.0V~8.4V 时，ADC 采样值约为 790~950
     * 若其中某个引脚在 200~3000 范围内，说明该引脚接了驱动板电池分压
     */
    if (avg5 >= 200 && avg5 <= 3000)
    {
        chosen_raw = avg5;
        chosen_pin = 5;
    }
    else if (avg6 >= 200 && avg6 <= 3000)
    {
        chosen_raw = avg6;
        chosen_pin = 6;
    }
    else
    {
        /* 若未接电池 (例如测试接地)，优先选择偏向 0V 的接地点 */
        if (avg5 < 200 && avg5 <= avg6)
        {
            chosen_raw = avg5;
            chosen_pin = 5;
        }
        else
        {
            chosen_raw = avg6;
            chosen_pin = 6;
        }
    }

    if (out_raw != 0) *out_raw = chosen_raw;
    if (out_pin != 0) *out_pin = chosen_pin;

    /* 11 倍分压换算: Vbatt = avg * 3.3 * 11 / 4095 */
    float batt_v = (float)chosen_raw * 3.3f * 11.0f / 4095.0f;
    return batt_v;
}
