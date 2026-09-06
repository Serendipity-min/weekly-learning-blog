#include "stm32f4xx.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "Led.h"
#include "key.h"

/* 全局毫秒计数器，由 SysTick_Handler 每 1ms 自增 */
volatile uint32_t g_ticks_ms = 0;

void delay_ms(uint32_t ms)
{
    uint32_t start = g_ticks_ms;
    while ((uint32_t)(g_ticks_ms - start) < ms)
    {
        /* 等待 SysTick 计时达到指定毫秒 */
    }
}

static void SysTick_Init(void)
{
    /* 更新 SystemCoreClock 并配置 SysTick 为 1ms 产生一次中断 */
    SystemCoreClockUpdate();
    SysTick_Config(SystemCoreClock / 1000);
}

int main(void)
{
    uint8_t key = 0;

    /* 1. 初始化 SysTick 定时器 (1ms 时基) */
    SysTick_Init();

    /* 2. 初始化板载 LED */
    LED_Init();

    /* 3. 初始化按键 (KEY_UP -> PA0) */
    KEY_Init();

    /* 4. 主循环*/
 /*   while (1)
    {
        key = KEY_Scan(0); // 单次触发模式
        if (key == KEY0_PRES)
        {
            LED_Toggle_0(); // 翻转 PF9 
        }
				else if(key == KEY1_PRES)
				{
					LED_Toggle_1();
				}
				else if(key == KEY2_PRES)
				{
					LED_Toggle_2();
				}
    }
*/
	while(1)
	{
		key = KEY_Scan(0);
			if(key)
			{
				switch(key)
				{
					case 1:
						LED_Toggle_0();
					break;
					case 2:
						LED_Toggle_1();
					break;
					case 3:
						LED_Toggle_2();
					break;
				}
		
		}
	}
	
}
