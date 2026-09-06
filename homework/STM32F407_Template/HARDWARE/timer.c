#include "timer.h"
#include "stm32f4xx.h"

//TIM6--》中断
void Tim6_Init(void)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE);

    /*
		定时1S
    */
    TIM_TimeBaseStructure.TIM_Prescaler = 8399;//PSC 8400-1
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;//计数模式
    TIM_TimeBaseStructure.TIM_Period = 9999;//ARR  10000-1
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;

    //初始化配置
    TIM_TimeBaseInit(TIM6, &TIM_TimeBaseStructure);
	
		NVIC_InitStructure.NVIC_IRQChannel = TIM6_DAC_IRQn;//选择中断编号
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
		NVIC_InitStructure.NVIC_IRQChannelCmd= ENABLE;
		NVIC_Init(&NVIC_InitStructure);
	
		//开启定时器中断，更新中断
		TIM_ITConfig(TIM6, TIM_IT_Updata,ENABLE);
		
		TIM_Cmd(TIM6,ENABLE);
		
}
