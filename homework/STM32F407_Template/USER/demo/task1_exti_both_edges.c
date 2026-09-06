/**
 ******************************************************************************
 * @file    task1_exti_both_edges.c
 * @brief   任务 1 - 双边沿触发中断与按键抖动现象 (EXTI_Trigger_Rising_Falling)
 * 
 * 硬件连接与原理（KEY0 -> PE4）：
 * 1. 按键 KEY0 连接到 PE4，内部上拉 (GPIO_PuPd_UP)：
 *      - 按下产生下降沿 (1 -> 0)
 *      - 松开产生上升沿 (0 -> 1)
 * 2. 板载 LED0 连接到 PF9，低电平点亮，初始状态输出高电平熄灭。
 * 
 * 预期实验现象：
 * 1. 理论预期：
 *    - 按下瞬间：下降沿触发中断，LED0 翻转 (灭 -> 亮)，g_exti_count + 1。
 *    - 松开瞬间：上升沿触发中断，LED0 再次翻转 (亮 -> 灭)，g_exti_count + 1。
 *    - 完整按一下松开，理论上触发 2 次中断，LED 最终回到初始状态。
 * 2. 实际抖动现象：
 *    - 机械弹片接触和断开瞬间产生微秒~毫秒级毛刺脉冲（1->0->1->0->1）。
 *    - 双边沿均敏感，导致按一次按键 g_exti_count 增加多次（如 +4, +6...），
 *      LED 高速闪烁后最终状态似乎“随机”，直观验证机械按键抖动与消抖的必要性。
 ******************************************************************************
 */

#include "task_config.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_syscfg.h"
#include "stm32f4xx_exti.h"
#include "misc.h"

#if (CURRENT_TASK == 3)

/* 中断计数器：可在 Keil Watch 窗口实时监测 */
volatile uint32_t g_exti_count = 0;

static void LED0_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);

    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOF, &GPIO_InitStructure);

    GPIO_SetBits(GPIOF, GPIO_Pin_9);
}

static void LED0_Toggle(void)
{
    GPIOF->ODR ^= GPIO_Pin_9;
}

/**
 * @brief 任务 1-3 初始化函数 (KEY0 -> PE4 -> EXTI4 双边沿触发)
 */
void Task1_BothEdges_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    EXTI_InitTypeDef EXTI_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    LED0_Init();

    /* 1. 使能 GPIOE 和 SYSCFG 外设时钟 */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

    /* 2. 配置 PE4 为输入引脚，内部上拉 */
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_4;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
    GPIO_Init(GPIOE, &GPIO_InitStructure);

    /* 3. 将 PE4 引脚映射到 EXTI4 外部中断线 */
    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOE, EXTI_PinSource4);

    /* 4. EXTI4 配置为双边沿触发 */
    EXTI_InitStructure.EXTI_Line    = EXTI_Line4;
    EXTI_InitStructure.EXTI_Mode    = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling; /* 双边沿触发 */
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    /* 5. 清除可能存在的初始挂起标志 */
    EXTI_ClearITPendingBit(EXTI_Line4);

    /* 6. NVIC 中断配置使能 EXTI4 */
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    NVIC_InitStructure.NVIC_IRQChannel                   = EXTI4_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

/**
 * @brief EXTI4 中断服务函数
 */
void EXTI4_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line4) != RESET)
    {
        g_exti_count++;
        LED0_Toggle();
        EXTI_ClearITPendingBit(EXTI_Line4);
    }
}

#endif /* (CURRENT_TASK == 3) */
