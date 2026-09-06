#ifndef __KEY_H
#define __KEY_H

#include "stm32f4xx.h"

#define KEY_NONE     0
#define KEY_UP_PRES  1   /* PA0, 按下高电平 */
#define KEY0_PRES    2   /* PE4, 按下低电平 */
#define KEY1_PRES    3   /* PE3, 按下低电平 */
#define KEY2_PRES    4   /* PE2, 按下低电平 */

void Key_Init(void);
uint8_t Key_Scan(void);

#endif /* __KEY_H */
