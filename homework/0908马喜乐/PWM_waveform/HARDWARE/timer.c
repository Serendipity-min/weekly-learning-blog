#include "timer.h"
#include "stm32f4xx.h"

/**
 * @brief  初始化 TIM6 为 100ms 定时中断 (用于高精度编码器测速采样)
 */
void Tim6_Init(void)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE);

    /*
     * APB1 定时器时钟为 84MHz
     * 84MHz / 8400 = 10000Hz (0.1ms 计数一次)
     * 1000 次计数 = 100ms 定时周期
     */
    TIM_TimeBaseStructure.TIM_Prescaler = 8400 - 1;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseStructure.TIM_Period = 1000 - 1;
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseInit(TIM6, &TIM_TimeBaseStructure);

    NVIC_InitStructure.NVIC_IRQChannel = TIM6_DAC_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    /* 开启更新中断并启动 TIM6 */
    TIM_ITConfig(TIM6, TIM_IT_Update, ENABLE);
    TIM_Cmd(TIM6, ENABLE);
}
