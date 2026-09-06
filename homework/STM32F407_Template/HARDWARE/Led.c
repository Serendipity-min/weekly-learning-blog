#include "led.h"
#include "stm32f4xx.h"

//PF9--》LED0--》低亮
void LED_Init(void)
{
	
	//定义GPIO外设结构体变量
	GPIO_InitTypeDef  GPIO_InitStructure;
	//开关--》时钟开关、时钟使能
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);
 
   //结构体成员的配置
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;//指定引脚号
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//输出工作模式
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//引脚高速响应
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//上拉模式
	
	//初始化端口
    GPIO_Init(GPIOF, &GPIO_InitStructure);
	
	
}
