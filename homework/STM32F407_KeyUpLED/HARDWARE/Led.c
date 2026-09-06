#include "Led.h"
#include "stm32f4xx.h"

void LED_Init(void)
	{
    GPIO_InitTypeDef GPIO_InitStructure;

    /* 使能 GPIOF 外设时钟 */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);

    /* 配置 PF9 和 PF10 为推挽输出模式 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10;//
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//引脚高响应
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//上拉模式
    GPIO_Init(GPIOF, &GPIO_InitStructure);

    /* 初始状态：全部熄灭 (高电平熄灭) */
    GPIO_SetBits(GPIOF, GPIO_Pin_9 | GPIO_Pin_10);
	}
