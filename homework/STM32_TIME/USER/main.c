#include "stm32f4xx.h"
#include "task_config.h"
#include "timer.h"
#include "led.h"
#include "key.h"

void Delay(uint32_t nCount)
{
    for (; nCount != 0; nCount--)
        ;
}
int main(void)
{
		Key_Init();
		LED_Init();
	
		Tim6_Init();
	
    while (1)
    {
       
    }
}
