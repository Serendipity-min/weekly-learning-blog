#include "key.h"
#include "systick.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"

/**
 * @brief  初始化开发板 4 个物理按键的 GPIO 引脚与上下拉配置
 */
void Key_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    /* 开启 GPIOA 与 GPIOE 外设时钟 */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOE, ENABLE);

    /* 1. 配置 KEY_UP (PA0): 下拉输入模式 (默认低电平 0，按下接 3.3V 为高电平 1) */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* 2. 配置 KEY0(PE4), KEY1(PE3), KEY2(PE2): 上拉输入模式 (默认高电平 1，按下接地为低电平 0) */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOE, &GPIO_InitStructure);
}

/**
 * @brief  按键扫描函数，支持边沿检测与软件防抖消抖，防止连续误触发
 * @retval KEY_NONE(0), KEY_UP_PRES(1), KEY0_PRES(2), KEY1_PRES(3), KEY2_PRES(4)
 */
uint8_t Key_Scan(void)
{
    static uint8_t s_key_released = 1; /* 按键释放标志: 1=已松开, 0=被按住中 */

    /* 读取 4 个按键引脚的实时电平 */
    uint8_t key_up_val = GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0);
    uint8_t key0_val   = GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_4);
    uint8_t key1_val   = GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_3);
    uint8_t key2_val   = GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_2);

    /* 当按键此前处于释放状态，且检测到任一按键被按下 */
    if (s_key_released && (key_up_val == 1 || key0_val == 0 || key1_val == 0 || key2_val == 0))
    {
        Delay_ms(15);         /* 延时 15ms 滤除机械抖动 */
        s_key_released = 0;   /* 锁定按键状态，防止一次长按产生多次触发 */

        if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0) == 1)
        {
            return KEY_UP_PRES; /* KEY_UP: 高电平有效 */
        }
        else if (GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_4) == 0)
        {
            return KEY0_PRES;   /* KEY0: 低电平有效 */
        }
        else if (GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_3) == 0)
        {
            return KEY1_PRES;   /* KEY1: 低电平有效 */
        }
        else if (GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_2) == 0)
        {
            return KEY2_PRES;   /* KEY2: 低电平有效 */
        }
    }
    /* 当所有按键都已松开时，恢复释放标志 */
    else if (key_up_val == 0 && key0_val == 1 && key1_val == 1 && key2_val == 1)
    {
        s_key_released = 1;
    }

    return KEY_NONE;
}
