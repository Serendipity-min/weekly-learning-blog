#ifndef _KEY_H_
#define _KEY_H_

#include <stdbool.h>
#include "stm32f4xx.h"

void Key_Init(void);
bool GetStaKey(void);
uint8_t Key_Scan(void);

#define KEY0 	GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_4)
#define KEY1 	GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_3)
#define KEY2 	GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_2)

#endif
