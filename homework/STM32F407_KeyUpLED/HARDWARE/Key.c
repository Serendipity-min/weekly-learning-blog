#include "Key.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"

extern void delay_ms(uint32_t ms);

void Key_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOE, ENABLE);

    /* 1. KEY_UP (PA0) 下拉输入，按下为高电平(1) */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* 2. KEY0(PE4), KEY1(PE3), KEY2(PE2) 上拉输入，按下为低电平(0) */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOE, &GPIO_InitStructure);
}

/**
 * @brief  独立按键边沿检测扫描函数（4路按键互不干扰）
 * @retval KEY_UP_PRES(1), KEY0_PRES(2), KEY1_PRES(3), KEY2_PRES(4), KEY_NONE(0)
 */
uint8_t Key_Scan(void)
{
    static uint8_t s_last_key_up = 0;
    static uint8_t s_last_key0 = 1;
    static uint8_t s_last_key1 = 1;
    static uint8_t s_last_key2 = 1;

    uint8_t curr_key_up = GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0);
    uint8_t curr_key0   = GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_4);
    uint8_t curr_key1   = GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_3);
    uint8_t curr_key2   = GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_2);

    /* 1. KEY_UP (PA0, 0->1 上升沿触发) */
    if (curr_key_up == 1 && s_last_key_up == 0)
    {
        delay_ms(15);
        if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0) == 1)
        {
            s_last_key_up = 1;
            return KEY_UP_PRES;
        }
    }
    s_last_key_up = curr_key_up;

    /* 2. KEY0 (PE4, 1->0 下降沿触发) */
    if (curr_key0 == 0 && s_last_key0 == 1)
    {
        delay_ms(15);
        if (GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_4) == 0)
        {
            s_last_key0 = 0;
            return KEY0_PRES;
        }
    }
    s_last_key0 = curr_key0;

    /* 3. KEY1 (PE3, 1->0 下降沿触发) */
    if (curr_key1 == 0 && s_last_key1 == 1)
    {
        delay_ms(15);
        if (GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_3) == 0)
        {
            s_last_key1 = 0;
            return KEY1_PRES;
        }
    }
    s_last_key1 = curr_key1;

    /* 4. KEY2 (PE2, 1->0 下降沿触发) */
    if (curr_key2 == 0 && s_last_key2 == 1)
    {
        delay_ms(15);
        if (GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_2) == 0)
        {
            s_last_key2 = 0;
            return KEY2_PRES;
        }
    }
    s_last_key2 = curr_key2;

    return KEY_NONE;
}
