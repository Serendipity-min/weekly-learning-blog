#include "led.h"
#include "stm32f4xx.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"

/**
 * @brief  初始化板载用户指示灯 LED0 (PF9) 与 LED1 (PF10)
 */
void LED_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    /* 开启 GPIOF 外设时钟 */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);

    /* 配置 PF9 与 PF10 为推挽输出模式 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOF, &GPIO_InitStructure);

    /* 初始状态全灭 (LED 阳极接 3.3V，输出高电平为熄灭) */
    GPIO_SetBits(GPIOF, GPIO_Pin_9 | GPIO_Pin_10);
}

/**
 * @brief  控制 LED0 (PF9) 状态
 * @param  on: 1=点亮 (输出低电平), 0=熄灭 (输出高电平)
 */
void LED0_Set(uint8_t on)
{
    if (on)
        GPIO_ResetBits(GPIOF, GPIO_Pin_9);
    else
        GPIO_SetBits(GPIOF, GPIO_Pin_9);
}

/**
 * @brief  控制 LED1 (PF10) 状态
 * @param  on: 1=点亮 (输出低电平), 0=熄灭 (输出高电平)
 */
void LED1_Set(uint8_t on)
{
    if (on)
        GPIO_ResetBits(GPIOF, GPIO_Pin_10);
    else
        GPIO_SetBits(GPIOF, GPIO_Pin_10);
}
