#include "stm32f4xx_it.h"
#include "esp8266.h"
#include "stm32f4xx_usart.h"

extern volatile uint32_t g_ticks_ms;

void SysTick_Handler(void)
{
    g_ticks_ms++;
}

/**
  * @brief  USART3 中断服务函数（接收 ESP8266 数据）
  */
void USART3_IRQHandler(void)
{
    if (USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)
    {
        uint8_t ch = (uint8_t)USART_ReceiveData(USART3);
        if (g_esp_rx_len < ESP8266_RX_BUF_SIZE - 1)
        {
            g_esp_rx_buf[g_esp_rx_len++] = (char)ch;
            g_esp_rx_buf[g_esp_rx_len] = '\0';
        }
        USART_ClearITPendingBit(USART3, USART_IT_RXNE);
    }
}
