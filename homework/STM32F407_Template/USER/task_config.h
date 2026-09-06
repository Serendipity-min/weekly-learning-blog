#ifndef __TASK_CONFIG_H__
#define __TASK_CONFIG_H__

#include <stdint.h>
#include "stm32f4xx.h"

/*
 * ==============================================================================
 * 【实验选择宏】方式 A：改这一个数字，一秒切换实验（1 ~ 5）
 * 硬件连接：按键 KEY0 -> PE4（按下接 GND，配置内部上拉 GPIO_PuPd_UP）
 *          未按下 = 1 (高电平 3.3V)
 *          按下   = 0 (低电平 0V，1->0 下降沿)
 *          松开   = 1 (高电平 3.3V，0->1 上升沿)
 * 
 * 1 = 任务 1-1：上升沿触发中断 (task1_exti_rising.c)
 *     现象：KEY0 按下无反应；【松手瞬间】产生上升沿，LED0 状态翻转，g_exti_count + 1。
 * 
 * 2 = 任务 1-2：下降沿触发中断 (task1_exti_falling.c)
 *     现象：【按下 KEY0 瞬间】产生下降沿，LED0 状态翻转，g_exti_count + 1；松手无反应。
 * 
 * 3 = 任务 1-3：双边沿触发与按键抖动 (task1_exti_both_edges.c)
 *     现象：按下翻转一次、松开翻转一次；
 *           实际因机械弹片抖动可能单次按压引起多次中断，LED 状态随机。
 * 
 * 4 = 任务 2-1：正常清除挂起标志位对照 (task2_exti_clear_flag.c)
 *     现象：中断服务程序正常调用 EXTI_ClearITPendingBit 清除 PR4；
 *           g_exti_count 单次递增，g_main_count 持续高速累加，CPU 正常返回主循环。
 * 
 * 5 = 任务 2-2：故意不清除标志位/中断风暴 (task2_exti_storm_no_clear.c)
 *     现象：故意不清除 PR4 挂起位；首次按下后触发【中断风暴】，
 *           g_exti_count 疯狂飙升（每秒数十万次），g_main_count 几乎停滞，
 *           LED0 以极高频率翻转呈现微亮常亮。
 * ==============================================================================
 */
#define CURRENT_TASK    4

/* 全局观察变量声明 (可在 Keil Watch 窗口观察) */
extern volatile uint32_t g_exti_count;
extern volatile uint32_t g_main_count;

/* 各实验初始化函数声明 */
void Task1_Rising_Init(void);
void Task1_Falling_Init(void);
void Task1_BothEdges_Init(void);
void Task2_ClearFlag_Init(void);
void Task2_StormNoClear_Init(void);

#endif /* __TASK_CONFIG_H__ */
