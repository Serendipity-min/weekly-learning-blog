#include "SysTick.h"
#include "stm32f4xx.h" 

static uint16_t delay_us = 0; 
static uint16_t delay_ms = 0; 

void SysTick_Init(void)
{
	SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8);
	delay_us = 168/8;
	delay_ms = delay_us * 1000;
}


void Delay_us(uint32_t nTime)
{

	
	uint32_t temp;
	
	SysTick->LOAD = nTime * delay_us ;   //重装载寄存器，最大计数值
	SysTick->VAL = 0x00	;				//清空计数值
	SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk ; // 最低位为1时使能，开启倒数；core_cm4.h的642行
	
	do
	{
		temp=SysTick->CTRL;//先读出来状态值
		//先判断第一位是否使能；检测CTRL的第16位如果是1（移位后是65536），则倒数计数结束
	}while((temp&0x01)&& !(temp & (1<<16)));
	
	SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk ; // 关闭计数器
	SysTick->VAL = 0x00	;	//清空计数值
	
}


static void Delay_xms(uint32_t nTime)
{

	uint32_t temp;
	
	
	SysTick->LOAD = nTime * delay_ms ;   //重装载寄存器，最大计数值
	SysTick->VAL = 0x00	;				//清空计数值
	SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk ; // 开启倒数
	
	do
	{
		temp=SysTick->CTRL;
		
	}while((temp&0x01)&& !(temp & (1<<16))); //等待倒数到0为止
	
	SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk ; // 关闭计数器
	SysTick->VAL = 0x00	;	//清空计数值
	
}


void Delay_ms(uint32_t nTime)
{
	uint8_t repeat =  nTime/798;//重复调用次数--》取商
	uint8_t remain =  nTime%798;//取余
	while(repeat)
	{
		Delay_xms(798);
		repeat--;
	}
	
	if(remain)
		Delay_xms(remain);
	//例如2000ms--》2个798余若干
}

