/**
 ******************************************************************************
 * @file    task2_exti_clear_flag.c
 * @brief   任务 2 - 正常清除挂起标志位对照实验 (CLEAR_EXTI_FLAG = 1)
 * 
 * 硬件与机制（KEY0 -> PE4，EXTI4）：
 *   EXTI_PR 是 W1C (Write 1 to Clear) 类型。
 *   在 EXTI4_IRQHandler 中调用 EXTI_ClearITPendingBit(EXTI_Line4) 清除 PR4。
 * 
 * 现象：
 *   按下 KEY0 时 g_exti_count 正常单次递增；
 *   CPU 正常返回主循环，g_main_count 持续高速累加（每秒数十万次以上）。
 ******************************************************************************
 */

#include "task_config.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_syscfg.h"
#include "stm32f4xx_exti.h"
#include "misc.h"

#if (CURRENT_TASK == 4)

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
 * @brief 任务 2-1 初始化函数 (KEY0 -> PE4 -> EXTI4，按下下降沿触发)
 */
void Task2_ClearFlag_Init(void)
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

    /* 4. EXTI4 配置为下降沿触发 (按键按下瞬间) */
    EXTI_InitStructure.EXTI_Line    = EXTI_Line4;
    EXTI_InitStructure.EXTI_Mode    = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    EXTI_ClearITPendingBit(EXTI_Line4);

    /* 5. NVIC 中断配置使能 EXTI4 */
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

        /*
         * 【核心点】：正常清除 EXTI4 中断挂起标志位 (向 EXTI_PR 的 PR4 写 1)
         */
        EXTI_ClearITPendingBit(EXTI_Line4);
    }
}

#endif /* (CURRENT_TASK == 4) */
