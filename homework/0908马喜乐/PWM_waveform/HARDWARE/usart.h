#ifndef __USART_H
#define __USART_H

#include "stm32f4xx.h"
#include <stdio.h>

void USART1_Init(uint32_t baudrate);
void USART1_SendString(const char *str);

#endif /* __USART_H */
