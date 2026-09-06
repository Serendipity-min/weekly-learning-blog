/**
 ******************************************************************************
 * @file    task2_exti_storm_no_clear.c
 * @brief   任务 2 - 故意不清除标志位导致中断风暴实验 (CLEAR_EXTI_FLAG = 0)
 * 
 * 硬件与机制（KEY0 -> PE4，EXTI4）：
 *   故意不调用 EXTI_ClearITPendingBit(EXTI_Line4) 清除 PR4 挂起位。
 *   第一次按下按键产生下降沿后，PR4 持续为 1。
 *   CPU 退出 ISR 后 NVIC 立即再次触发中断（中断风暴 Interrupt Storm）！
 * 
 * 现象：
 *   首次按下后手完全离开按键，g_exti_count 依然疯狂飙升（数十万次/秒）；
 *   g_main_count 几乎完全停滞；
 *   LED0 极速翻转人眼无法分辨，看起来呈现微弱常亮状态。
 ******************************************************************************
 */

#include "task_config.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_syscfg.h"
#include "stm32f4xx_exti.h"
#include "misc.h"

#if (CURRENT_TASK == 5)

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
 * @brief 任务 2-2 初始化函数 (KEY0 -> PE4 -> EXTI4，按下下降沿触发)
 */
void Task2_StormNoClear_Init(void)
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
         * 【实验核心】：故意不清除挂起标志位！
         * 
         * // EXTI_ClearITPendingBit(EXTI_Line4);
         * 
         * 不向 PR4 写 1，硬件挂起标志一直为 1，CPU 一退出 ISR 立刻再次被 NVIC 捕获！
         */
    }
}

#endif /* (CURRENT_TASK == 5) */
