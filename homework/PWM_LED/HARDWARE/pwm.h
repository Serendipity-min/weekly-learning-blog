#ifndef _PWM_H_
#define _PWM_H_

#include "stm32f4xx.h"

/* 电机运行方向定义 */
typedef enum {
    MOTOR_DIR_STOP    = 0,  /* 刹车制动/停止 (AIN1/BIN1=0, AIN2/BIN2=0, PWM=0) */
    MOTOR_DIR_FORWARD = 1,  /* 正转 (AIN1/BIN1=1, AIN2/BIN2=0) */
    MOTOR_DIR_REVERSE = 2   /* 反转 (AIN1/BIN1=0, AIN2/BIN2=1) */
} MotorDir_t;

/* 初始化电机驱动引脚及定时器 PWM */
void Motor_Init(void);

/* 设置电机方向与速度百分比 (0 ~ 100%) */
void Motor_SetState(MotorDir_t dir, uint16_t speed_percent);

/* 刹车停止电机 */
void Motor_Stop(void);

/* 获取当前电机方向与速度 */
MotorDir_t Motor_GetDir(void);
uint16_t Motor_GetSpeed(void);

#endif /* _PWM_H_ */
