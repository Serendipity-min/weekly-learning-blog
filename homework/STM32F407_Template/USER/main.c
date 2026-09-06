#include "stm32f4xx.h"
#include "task_config.h"

/* 原工程 HARDWARE/key.c 中 extern 引用的简单软件延时函数 */
void Delay(uint32_t nCount)
{
    for (; nCount != 0; nCount--)
        ;
}

/* 主循环计数器：在 Keil Watch 窗口观察其变化速率 */
volatile uint32_t g_main_count = 0;

int main(void)
{
    /* 
     * ==============================================================================
     * 【KEY0 实验（PE4，按下接 GND，内部上拉）】
     * 只需要在 task_config.h 中将 CURRENT_TASK 改为 1 ~ 5 即可自动匹配，
     * 编译时其他 4 个文件自动被预处理器屏蔽，绝不产生符号冲突！
     * ==============================================================================
     */
#if (CURRENT_TASK == 1)
    Task1_Rising_Init();        /* 任务 1-1：上升沿触发 (按下无反应，松手产生上升沿翻转) */
#elif (CURRENT_TASK == 2)
    Task1_Falling_Init();       /* 任务 1-2：下降沿触发 (按下瞬间产生下降沿翻转，松手无反应) */
#elif (CURRENT_TASK == 3)
    Task1_BothEdges_Init();     /* 任务 1-3：双边沿触发与按键抖动现象 */
#elif (CURRENT_TASK == 4)
    Task2_ClearFlag_Init();      /* 任务 2-1：正常清除标志位 (main 高速运行) */
#elif (CURRENT_TASK == 5)
    Task2_StormNoClear_Init();   /* 任务 2-2：故意不清除标志位 (引发中断风暴) */
#endif

    /* 主循环 */
    while (1)
    {
        /*
         * 主循环持续自增 g_main_count。
         * 在 Keil Watch 窗口观察：
         *   - 正常情况下高速狂飙递增
         *   - 中断风暴 (任务 2-2) 触发后几乎完全停滞！
         */
        g_main_count++;
    }
}
