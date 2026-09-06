#include "encoder.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_tim.h"

/* 编码器全局采样结果 */
static volatile int16_t s_encoder_count = 0;
static volatile int16_t s_encoder_rpm = 0;

/**
 * @brief  初始化 TIM4 为正交编码器接口模式 (4倍频)
 * @note   PB6 -> TIM4_CH1 (编码器 A 相)
 *         PB7 -> TIM4_CH2 (编码器 B 相)
 */
void Encoder_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_ICInitTypeDef TIM_ICInitStructure;

    /* 开启 GPIOB 和 TIM4 外设时钟 */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);

    /* 配置 PB6 和 PB7 引脚复用为 TIM4 */
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_TIM4);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource7, GPIO_AF_TIM4);

    /* 配置 PB6、PB7 为复用功能，开启内置上拉电阻 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    /* 定时器基础配置: 16位最大量程 0~65535，不分频 */
    TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
    TIM_TimeBaseStructure.TIM_Prescaler = 0;
    TIM_TimeBaseStructure.TIM_Period = 65535;
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);

    /* 配置为 TI1 和 TI2 双边沿计数 (4 倍频编码器模式) */
    TIM_EncoderInterfaceConfig(TIM4, TIM_EncoderMode_TI12, TIM_ICPolarity_Rising, TIM_ICPolarity_Rising);

    /* 初始化输入捕获通道 1 和通道 2，并设置数字滤波值 (滤波值 10，滤除电机电刷火花干扰) */
    TIM_ICStructInit(&TIM_ICInitStructure);
    TIM_ICInitStructure.TIM_ICFilter = 10;

    TIM_ICInitStructure.TIM_Channel = TIM_Channel_1;
    TIM_ICInit(TIM4, &TIM_ICInitStructure);

    TIM_ICInitStructure.TIM_Channel = TIM_Channel_2;
    TIM_ICInit(TIM4, &TIM_ICInitStructure);

    /* 关键: 确保硬件捕获使能位 CC1E 和 CC2E 置 1，允许脉冲进入计数器 */
    TIM4->CCER |= (TIM_CCER_CC1E | TIM_CCER_CC2E);

    /* 清除标志，清零计数值并使能定时器 */
    TIM_ClearFlag(TIM4, TIM_FLAG_Update);
    TIM_SetCounter(TIM4, 0);
    TIM_Cmd(TIM4, ENABLE);
}

/**
 * @brief  每 100ms 由 TIM6 中断自动调用一次，计算瞬时转速
 */
void Encoder_100ms_Update(void)
{
    /* 读取 100ms 内计数值并立即清零，重新开始下一周期累加 */
    int16_t count = (short)TIM_GetCounter(TIM4);
    TIM_SetCounter(TIM4, 0);
    s_encoder_count = count;

    /*
     * MG310 电机参数:
     * 减速比 1:20，编码器单圈 13 线，4 倍频后输出轴转 1 圈对应 13 * 4 * 20 = 1040 个脉冲。
     * 100ms (0.1秒) 采样周期下:
     * 转速 RPM = (count / 1040) * (60s / 0.1s) = count * 600 / 1040
     */
    int32_t rpm = (int32_t)count * 600 / 1040;
    if (rpm < 0)
    {
        rpm = -rpm;
    }
    s_encoder_rpm = (int16_t)rpm;
}

/**
 * @brief  获取当前测得的转速 (RPM)
 * @param  out_count: 可选输出 100ms 的实际脉冲计数值 (若不需要传 0 即可)
 * @return 输出轴转速 (r/min)
 */
int16_t Encoder_GetSpeedRPM(int16_t *out_count)
{
    if (out_count != 0)
    {
        *out_count = s_encoder_count;
    }
    return s_encoder_rpm;
}
