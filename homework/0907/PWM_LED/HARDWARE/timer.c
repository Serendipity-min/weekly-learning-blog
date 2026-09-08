#include "timer.h"
#include "stm32f4xx.h"


//TIM6---》1S中断
void Tim6_Init(void)
{

    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE);

	/*
	APB1总线时钟频率--》42--》定时器时钟频率是42*2 = 84
	84Mhz/8400=10000hz
	1s--》10000个数--》数1个数--》0.0001s--》0.1ms
	数一个数是0.1--》数多少个数---》10000
	---》1s定时
	*/
    TIM_TimeBaseStructure.TIM_Prescaler = 8400-1;//PSC
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;//计数模式
    TIM_TimeBaseStructure.TIM_Period = 10000-1;//ARR
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
 
	//初始化配置
    TIM_TimeBaseInit(TIM6, &TIM_TimeBaseStructure);
	
	NVIC_InitStructure.NVIC_IRQChannel = TIM6_DAC_IRQn ;//选择中断编号
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;//设置抢占优先级
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;//响应优先级
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;//开启中断
    NVIC_Init(&NVIC_InitStructure); //初始化
	
	//开启定时器中断--》更新中断
	TIM_ITConfig(TIM6, TIM_IT_Update, ENABLE);
	
	//启动定时器6
	TIM_Cmd(TIM6,ENABLE);
	


	
	
}
