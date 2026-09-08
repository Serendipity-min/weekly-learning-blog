#include "pwm.h"
#include "stm32f4xx.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_tim.h"

/* 记录当前电机的运行方向与速度百分比 (0~100) */
static MotorDir_t s_motor_dir = MOTOR_DIR_STOP;
static uint16_t s_motor_speed = 0;

/**
 * @brief  初始化 TB6612 电机驱动的 GPIO 与 PWM 定时器
 * @note   - 转向控制: PE0, PE1 (推挽输出)
 *         - PWM 调速: PB0 (TIM3_CH3), PB1 (TIM3_CH4), 频率 10kHz
 */
void Motor_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_OCInitTypeDef TIM_OCInitStructure;

    /* 1. 开启外设时钟: GPIOB、GPIOE 以及通用定时器 TIM3 */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOE, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

    /* 2. 配置方向引脚 PE0 和 PE1 为推挽输出模式 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOE, &GPIO_InitStructure);

    /* 初始设置为低电平刹车制动状态 (0, 0) */
    GPIO_ResetBits(GPIOE, GPIO_Pin_0 | GPIO_Pin_1);

    /* 3. 将 PB0 和 PB1 引脚复用映射为 TIM3 */
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource0, GPIO_AF_TIM3); /* PB0 -> TIM3_CH3 */
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource1, GPIO_AF_TIM3); /* PB1 -> TIM3_CH4 */

    /* 配置 PB0 与 PB1 为复用推挽输出 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    /* 4. 配置 TIM3 时基单元
     *    APB1 定时器时钟为 84MHz
     *    不分频 (PSC=0), 自动重装载值 ARR = 8399
     *    PWM 频率 = 84MHz / (0 + 1) / 8400 = 10,000 Hz (10 kHz)
     */
    TIM_TimeBaseStructure.TIM_Prescaler = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; /* 向上计数 */
    TIM_TimeBaseStructure.TIM_Period = 8400 - 1;                /* ARR = 8399 */
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

    /* 5. 配置 TIM3 的输出比较通道 (PWM 模式 1，高电平有效) */
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_Pulse = 0;                        /* 初始占空比为 0 */
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High; /* 高电平有效 */

    /* 初始化通道 3 (PB0) 与通道 4 (PB1) */
    TIM_OC3Init(TIM3, &TIM_OCInitStructure);
    TIM_OC3PreloadConfig(TIM3, TIM_OCPreload_Enable);

    TIM_OC4Init(TIM3, &TIM_OCInitStructure);
    TIM_OC4PreloadConfig(TIM3, TIM_OCPreload_Enable);

    /* 使能 ARR 预装载并启动 TIM3 计数 */
    TIM_ARRPreloadConfig(TIM3, ENABLE);
    TIM_Cmd(TIM3, ENABLE);

    s_motor_dir = MOTOR_DIR_STOP;
    s_motor_speed = 0;
}

/**
 * @brief  控制电机旋转方向与输出速度
 * @param  dir: MOTOR_DIR_STOP(停止), MOTOR_DIR_FORWARD(正转), MOTOR_DIR_REVERSE(反转)
 * @param  speed_percent: 速度百分比 (0 ~ 100)
 */
void Motor_SetState(MotorDir_t dir, uint16_t speed_percent)
{
    uint32_t compare_val;

    /* 限制速度最大值为 100% */
    if (speed_percent > 100)
    {
        speed_percent = 100;
    }

    /* 如果为停止模式或速度为 0，则执行刹车制动 */
    if (dir == MOTOR_DIR_STOP || speed_percent == 0)
    {
        GPIO_ResetBits(GPIOE, GPIO_Pin_0 | GPIO_Pin_1); /* PE0=0, PE1=0 刹车 */
        TIM_SetCompare3(TIM3, 0);                       /* 占空比清零 */
        TIM_SetCompare4(TIM3, 0);
        s_motor_dir = MOTOR_DIR_STOP;
        s_motor_speed = 0;
        return;
    }

    s_motor_dir = dir;
    s_motor_speed = speed_percent;

    /* 计算 PWM 比较值: 84 * 速度百分比 (最大 8400 对应 100%) */
    compare_val = (uint32_t)speed_percent * 84;
    TIM_SetCompare3(TIM3, compare_val);
    TIM_SetCompare4(TIM3, compare_val);

    /* 根据方向设置方向引脚电平 */
    if (dir == MOTOR_DIR_FORWARD)
    {
        /* 正转模式: PE0=1, PE1=0 */
        GPIO_SetBits(GPIOE, GPIO_Pin_0);
        GPIO_ResetBits(GPIOE, GPIO_Pin_1);
    }
    else if (dir == MOTOR_DIR_REVERSE)
    {
        /* 反转模式: PE0=0, PE1=1 */
        GPIO_ResetBits(GPIOE, GPIO_Pin_0);
        GPIO_SetBits(GPIOE, GPIO_Pin_1);
    }
}

/**
 * @brief  刹车停止电机
 */
void Motor_Stop(void)
{
    Motor_SetState(MOTOR_DIR_STOP, 0);
}

/**
 * @brief  获取电机当前运行方向
 */
MotorDir_t Motor_GetDir(void)
{
    return s_motor_dir;
}

/**
 * @brief  获取电机当前速度百分比
 */
uint16_t Motor_GetSpeed(void)
{
    return s_motor_speed;
}
