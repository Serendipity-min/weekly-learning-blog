#include "encoder.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_tim.h"

/* 全局变量: 100ms 脉冲计数值与换算出的实际转速 */
static volatile int16_t s_encoder_count = 0;
static volatile int16_t s_encoder_rpm = 0;

/**
 * @brief  初始化 TIM4 为编码器模式 (4倍频模式)
 * @note   硬件接线:
 *         驱动板 Pin 1 (E2B) -> STM32 PB7 (TIM4_CH2, P8 排针 Pin 47)
 *         驱动板 Pin 2 (E2A) -> STM32 PB6 (TIM4_CH1, P8 排针 Pin 48)
 */
void Encoder_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_ICInitTypeDef TIM_ICInitStructure;

    /* 开启 GPIOB 与 TIM4 外设时钟 */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);

    /* 配置 PB6 与 PB7 引脚复用为 TIM4 */
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_TIM4);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource7, GPIO_AF_TIM4);

    /* 配置 PB6、PB7 为复用功能，开启内部弱上拉，确保霍尔开漏信号电平正常 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    /* 定时器基本参数配置: 16位计数器 0~65535，不分频 */
    TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
    TIM_TimeBaseStructure.TIM_Prescaler = 0;
    TIM_TimeBaseStructure.TIM_Period = 65535;
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);

    /* 编码器接口模式: TI1 与 TI2 双通道双边沿计数 (4倍频) */
    TIM_EncoderInterfaceConfig(TIM4, TIM_EncoderMode_TI12, TIM_ICPolarity_Rising, TIM_ICPolarity_Rising);

    /* 初始化输入捕获通道 1 与通道 2 的输入滤波 (滤波值 10，有效消除电机电刷与供电毛刺) */
    TIM_ICStructInit(&TIM_ICInitStructure);
    TIM_ICInitStructure.TIM_ICFilter = 10;

    TIM_ICInitStructure.TIM_Channel = TIM_Channel_1;
    TIM_ICInit(TIM4, &TIM_ICInitStructure);

    TIM_ICInitStructure.TIM_Channel = TIM_Channel_2;
    TIM_ICInit(TIM4, &TIM_ICInitStructure);

    /* 开启通道捕获使能位 */
    TIM4->CCER |= (TIM_CCER_CC1E | TIM_CCER_CC2E);

    /* 清除标志位并使能定时器 */
    TIM_ClearFlag(TIM4, TIM_FLAG_Update);
    TIM_SetCounter(TIM4, 0);
    TIM_Cmd(TIM4, ENABLE);
}

/**
 * @brief  每 100ms 由 TIM6 中断自动调用一次，计算瞬时转速
 */
void Encoder_100ms_Update(void)
{
    /* 读取 100ms 周期内计数值并清零，重新开始下一轮累加 */
    int16_t count = (int16_t)TIM_GetCounter(TIM4);
    TIM_SetCounter(TIM4, 0);
    s_encoder_count = count;

    /*
     * MG310 电机编码器参数:
     * 减速比 1:20，霍尔码盘 13 线，4 倍频后输出轴转 1 圈对应 13 * 4 * 20 = 1040 个脉冲。
     * 100ms (0.1秒) 采样周期:
     * 转速 RPM = (count / 1040) * (60s / 0.1s) = count * 600 / 1040 = count * 15 / 26
     */
    int32_t rpm = (int32_t)count * 15 / 26;
    if (rpm < 0)
    {
        rpm = -rpm; // 转速大小取绝对值，转向在第2行已独立标明
    }
    s_encoder_rpm = (int16_t)rpm;
}

/**
 * @brief  获取最新测得的电机输出轴转速 (RPM)
 * @param  out_count: 可选输出 100ms 内计数的脉冲原始值
 * @return 输出轴转速 (RPM)
 */
int16_t Encoder_GetSpeedRPM(int16_t *out_count)
{
    if (out_count != 0)
    {
        *out_count = s_encoder_count;
    }
    return s_encoder_rpm;
}
