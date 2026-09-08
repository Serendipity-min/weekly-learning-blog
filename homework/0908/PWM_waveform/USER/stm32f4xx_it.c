#include "stm32f4xx_it.h"
#include "encoder.h"

/**
 * @brief  TIM6 DAC 定时中断服务函数 (每 100ms 触发一次)
 */
void TIM6_DAC_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM6, TIM_IT_Update) != RESET)
    {
        TIM_ClearITPendingBit(TIM6, TIM_IT_Update);
        Encoder_100ms_Update();
    }
}
