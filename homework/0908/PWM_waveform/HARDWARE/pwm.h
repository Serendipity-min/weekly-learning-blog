#ifndef _PWM_H_
#define _PWM_H_

#include "stm32f4xx.h"

/* ================= PB6 (TIM4_CH1) PWM 输出比较接口 ================= */

/**
 * @brief  初始化 PB6 为 PWM 输出引脚 (使用 TIM4_CH1 输出比较功能)
 * @param  period_us: PWM 周期，单位微秒 (us)，例如 2000 表示 2000us (500Hz)
 * @param  duty_percent: 占空比百分比 (0 ~ 100)，例如 40 表示 40%
 */
void PWM_PB6_Init(uint32_t period_us, uint16_t duty_percent);

/**
 * @brief  动态设置 PB6 的 PWM 占空比
 * @param  duty_percent: 占空比百分比 (0 ~ 100)
 */
void PWM_PB6_SetDuty(uint16_t duty_percent);

/**
 * @brief  获取当前 PB6 的 PWM 周期 (us)
 */
uint32_t PWM_PB6_GetPeriod(void);

/**
 * @brief  获取当前 PB6 的 PWM 占空比 (%)
 */
uint16_t PWM_PB6_GetDuty(void);

/* ================= TB6612 电机驱动接口 (保留兼容) ================= */

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
