#include "key.h"
#include "stm32f4xx.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"

extern void Delay(uint32_t nCount);

void Key_Init(void)
{
	//定义GPIO外设结构体变量
	GPIO_InitTypeDef  GPIO_InitStructure;
	//开关--》时钟开关、时钟使能
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
 
   //结构体成员的配置
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;//指定引脚号
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;//输入模式
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//高速响应
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;//下拉模式
	
	//初始化端口
    GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4;//指定引脚号
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;//输入模式
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//高速响应
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//上拉模式
	
	//初始化端口
    GPIO_Init(GPIOE, &GPIO_InitStructure);
	
}
bool GetStaKey(void)
{
	//按键被按下
	if(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_0) == 1)
	{
		//进行抖动处理
		Delay(0xCC6);
		while(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_0) == 1);
		Delay(0xCC6);
		
		return true;
	}
	
	else
	{
		return false;
	}
	
}

//按键扫描函数
uint8_t Key_Scan(void)
{
	//进行按键锁存，防止重复识别,1表示没有按下，0表示按下
	static uint8_t flag = 1;
	//有任何一个按键被按下
	if(flag && (KEY0 == 0|| KEY1 == 0 || KEY2 == 0))
	{
		flag = 0;//不会重复返回
		Delay(0xCC6);
		if(KEY0 == 0)
			return 1;
		if(KEY1 == 0)
			return 2;
		if(KEY2 == 0)
			return 3;
		
	}
	else if(KEY0 == 1 && KEY1 == 1 && KEY2 == 1)
	{
		//没有被按下
		flag = 1;
	}
	return 0;
	
}
