/**
 ******************************************************************************
 * @file    task1_exti_falling.c
 * @brief   任务 1 - 下降沿触发中断实验 (EXTI_Trigger_Falling)
 * 
 * 硬件连接与原理（KEY0 -> PE4）：
 * 1. 按键 KEY0 连接到 PE4。外围电路上按键按下接 GND，松开悬空。
 *    因此 PE4 配置为内部上拉 (GPIO_PuPd_UP)：
 *      - 未按时：PE4 = 1 (高电平 3.3V)
 *      - 按下时：PE4 接 GND = 0 (1 -> 0，产生【下降沿】)
 *      - 松开时：PE4 恢复为 1 (0 -> 1，产生【上升沿】)
 * 2. 板载 LED0 连接到 PF9，低电平点亮，初始状态输出高电平熄灭。
 * 
 * 预期实验现象：
 * 1. 按下 KEY0 时：产生下降沿，触发 EXTI4 中断，LED0 状态翻转，g_exti_count 自增 1。
 * 2. 松开 KEY0 时：产生上升沿，不满足下降沿触发条件，无中断响应，LED0 状态不变。
 * 3. 结论：由于 KEY0 按下接地（低有效），下降沿触发对应的是【按键按下瞬间】！
 ******************************************************************************
 */

#include "task_config.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_syscfg.h"
#include "stm32f4xx_exti.h"
#include "misc.h"

#if (CURRENT_TASK == 2)

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
 * @brief 任务 1-2 初始化函数 (KEY0 -> PE4 -> EXTI4 下降沿触发)
 */
void Task1_Falling_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    EXTI_InitTypeDef EXTI_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    LED0_Init();

    /* 1. 使能 GPIOE 和 SYSCFG 外设时钟 */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

    /* 2. 配置 PE4 为输入引脚，内部上拉 (未按时保持高电平 3.3V) */
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_4;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP; /* KEY0 按下接地，故配置为内部上拉 */
    GPIO_Init(GPIOE, &GPIO_InitStructure);

    /* 3. 将 PE4 引脚映射到 EXTI4 外部中断线 */
    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOE, EXTI_PinSource4);

    /* 4. EXTI4 配置为下降沿触发 */
    EXTI_InitStructure.EXTI_Line    = EXTI_Line4;
    EXTI_InitStructure.EXTI_Mode    = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling; /* 下降沿触发 (对应 KEY0 按下) */
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

#endif /* (CURRENT_TASK == 2) */
