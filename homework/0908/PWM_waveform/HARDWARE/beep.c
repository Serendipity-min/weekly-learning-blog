#include "beep.h"
#include "systick.h"

/* 静态运行状态与当前设置 */
static uint8_t  s_beep_state = 0;      /* 0: 关闭(静音), 1: 开启 */
static uint32_t s_current_freq = BEEP_FREQ_DEFAULT;
static uint8_t  s_current_duty = 50;

/**
 * @brief  初始化蜂鸣器硬件引脚与 TIM13 定时器 PWM 通道
 * @note   PF8 -> TIM13_CH1 (AF9)
 */
void BEEP_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_OCInitTypeDef TIM_OCInitStructure;

    /* 1. 使能 GPIOF 与 TIM13 外设时钟 */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM13, ENABLE);

    /* 2. 将 PF8 引脚复用映射为 TIM13_CH1 (AF9) */
    GPIO_PinAFConfig(GPIOF, GPIO_PinSource8, GPIO_AF_TIM13);

    /* 3. 配置 PF8 引脚为复用功能、推挽输出、带下拉 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN; /* 默认下拉防止悬空误触发 */
    GPIO_Init(GPIOF, &GPIO_InitStructure);

    /* 4. 配置 TIM13 时基单元
     *    APB1 定时器输入时钟 = 84MHz
     *    预分频系数 PSC = 84 - 1 = 83，计数基准频率 = 84MHz / 84 = 1MHz (1us 计数一次)
     *    默认重装载值 ARR = (1000000 / 2000) - 1 = 499 (对应 2000Hz 频率)
     */
    TIM_TimeBaseStructure.TIM_Prescaler = 84 - 1;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseStructure.TIM_Period = (1000000 / BEEP_FREQ_DEFAULT) - 1;
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInit(TIM13, &TIM_TimeBaseStructure);

    /* 5. 配置 TIM13 通道 1 为 PWM 模式 1
     *    向上计数时：CNT < CCR1 输出高电平（使三极管导通发声）
     *    初始 Pulse (CCR1) 设为 0，即占空比 0%，蜂鸣器保持静音
     */
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_Pulse = 0;                        /* 初始静音 */
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High; /* 高电平有效 */
    TIM_OC1Init(TIM13, &TIM_OCInitStructure);

    /* 使能输出比较通道与自动重装载寄存器的预装载 */
    TIM_OC1PreloadConfig(TIM13, TIM_OCPreload_Enable);
    TIM_ARRPreloadConfig(TIM13, ENABLE);

    /* 6. 启动定时器 TIM13 */
    TIM_Cmd(TIM13, ENABLE);

    s_beep_state = 0;
    s_current_freq = BEEP_FREQ_DEFAULT;
    s_current_duty = 50;
}

/**
 * @brief  设置蜂鸣器 PWM 频率与占空比
 * @param  freq: 目标发声频率 (Hz)，建议 100Hz ~ 20000Hz
 * @param  duty_percent: 占空比百分比 (0 ~ 100)，无源蜂鸣器推荐 50%
 */
void BEEP_SetFreq(uint32_t freq, uint8_t duty_percent)
{
    uint32_t arr_val;
    uint32_t ccr_val;

    if (freq == 0 || duty_percent == 0)
    {
        /* 静音：将 CCR 设为 0，输出恒定低电平，三极管截止 */
        TIM_SetCompare1(TIM13, 0);
        s_beep_state = 0;
        return;
    }

    if (freq > 20000)
    {
        freq = 20000;
    }
    if (duty_percent > 100)
    {
        duty_percent = 100;
    }

    s_current_freq = freq;
    s_current_duty = duty_percent;

    /* 计数基准为 1MHz (1us 计数一次)，周期为 1000000 / freq (us) */
    arr_val = (1000000 / freq) - 1;
    if (arr_val > 65535)
    {
        arr_val = 65535;
    }

    /* 根据占空比计算 CCR1 */
    ccr_val = ((arr_val + 1) * duty_percent) / 100;

    /* 动态重置自动重装载值与比较捕获值 */
    TIM_SetAutoreload(TIM13, arr_val);
    TIM_SetCompare1(TIM13, ccr_val);

    s_beep_state = 1;
}

/**
 * @brief  以 50% 最佳对称占空比开启指定频率发声
 * @param  freq: 发声频率 (Hz)
 */
void BEEP_On(uint32_t freq)
{
    BEEP_SetFreq(freq, 50);
}

/**
 * @brief  关闭蜂鸣器（完全静音）
 */
void BEEP_Off(void)
{
    TIM_SetCompare1(TIM13, 0);
    s_beep_state = 0;
}

/**
 * @brief  发声指定时长后自动关闭
 * @param  freq: 频率 (Hz)
 * @param  duration_ms: 持续时间 (毫秒)
 */
void BEEP_PlayTone(uint32_t freq, uint32_t duration_ms)
{
    BEEP_On(freq);
    Delay_ms(duration_ms);
    BEEP_Off();
}

/**
 * @brief  发出指定次数的警报滴滴声
 * @param  count: 鸣叫次数
 * @param  duration_ms: 鸣叫与静音间隔时间 (毫秒)
 */
void BEEP_Alarm(uint8_t count, uint32_t duration_ms)
{
    uint8_t i;
    for (i = 0; i < count; i++)
    {
        BEEP_On(BEEP_FREQ_ALARM);
        Delay_ms(duration_ms);
        BEEP_Off();
        if (i < count - 1)
        {
            Delay_ms(duration_ms);
        }
    }
}

/**
 * @brief  获取当前蜂鸣器开关状态
 * @return 0: 关闭, 1: 开启
 */
uint8_t BEEP_GetState(void)
{
    return s_beep_state;
}

/**
 * @brief  获取当前设定的发声频率
 * @return 频率值 (Hz)
 */
uint32_t BEEP_GetFreq(void)
{
    return s_current_freq;
}

/**
 * @brief  获取当前设定的发声占空比
 * @return 占空比百分比 (0 ~ 100)
 */
uint8_t BEEP_GetDuty(void)
{
    return s_current_duty;
}
