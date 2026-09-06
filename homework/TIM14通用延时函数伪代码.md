## 一、 计算过程与分析

TIM14 挂载于 APB1 低速外设总线上。

1. 定时器输入时钟 (f_TIM_CLK)：
   - STM32F407 系统主频 f_SYSCLK = 168 MHz。
   - APB1 预分频器分频系数通常为 4，故 f_APB1 = 42 MHz。
   - 根据 STM32F4 时钟树规范，当 APB 预分频系数 != 1 时，定时器时钟倍频为 2，因此：
     f_TIM14_CLK = f_APB1 * 2 = 84 MHz

2. 时间单位（数一个数 1ms）的预分频器（PSC）计算：
   - 目标计数频率：f_CNT = 1 kHz = 1000 Hz（即每计一次数耗时 T = 1 ms）。
   - 计数器时钟计算公式：
     f_CNT = f_TIM_CLK / (PSC + 1)
   - 代入数值计算：
     1000 = 84000000 / (PSC + 1)
     PSC + 1 = 84000
     PSC = 83999

3. 自动重装载值（ARR）设置：
   - 延时采用轮询计数器（CNT）方式，为防止延时期间计数器溢出重置（从0重新计数造成死循环或逻辑错误），ARR 应设置足够大。
   - TIM14 为 16 位定时器，最大值为 65535。因此设置：
     ARR = 65535 (0xFFFF)

---

## 三、 伪代码设计 (Pseudocode)

### 1. TIM14 初始化模块伪代码 (TIM14_Init_1ms)

```text
    1. 开启 TIM14 外设时钟 (使能 RCC APB1 对应时钟门控)
    
    2. 配置定时器时基参数结构体:
       - 预分频系数 Prescaler   <-- 83999  (使计数器频率为 84MHz / 84000 = 1kHz，即 1ms/次)
       - 计数模式 CounterMode   <-- 向上计数 (Up)
       - 重装载值 Period (ARR)  <-- 65535  (设置最大周期，避免短时间内溢出)
       - 时钟分频 ClockDivision <-- 0
    
    3. 调用底层的 TimeBaseInit 接口，将上述配置写入 TIM14 寄存器
    
    4. 启动 TIM14 计数器 (使能 TIM14)
END
```

### 2. 通用延时函数伪代码 (delay_ms)

```text
	 // 步骤 1：清空定时器的计数值，保证从 0 开始计时
    TIM14 计数寄存器 (CNT) <-- 0
    
    // 步骤 2：轮询读取计数器的当前值，并与目标 count 比较
    WHILE (TIM14 当前计数器的值 < count) DO
        // 空操作，等待硬件计数器自主累加
        NOP
    END WHILE
    
    // 达到目标值后函数返回，完成延时
END
```

---

## 四、 STM32 标准库 (C语言) 落地实现

### 1. timer.c 源码实现

```c
#include "timer.h"
#include "stm32f4xx.h"

/**
 * @brief  初始化 TIM14，配置为每 1ms 计数一次
 * @note   基于 84MHz APB1 定时器时钟
 */
void TIM14_Init_1ms(void)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;

    // 1. 开启 TIM14 时钟 (TIM14 位于 APB1 总线)
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM14, ENABLE);

    // 2. 配置时基结构体
    // APB1 定时器频率 84MHz，经过 83999+1=84000 分频后，计数频率为 1kHz (周期 1ms)
    TIM_TimeBaseStructure.TIM_Prescaler = 83999; 
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; // 向上计数
    TIM_TimeBaseStructure.TIM_Period = 65535;                   // ARR 设为最大，防溢出
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;

    TIM_TimeBaseInit(TIM14, &TIM_TimeBaseStructure);

    // 3. 使能 TIM14
    TIM_Cmd(TIM14, ENABLE);
}

/**
 * @brief  毫秒级通用阻塞延时函数
 * @param  count: 需要延时的毫秒数 (图中形参为 uint8_t count，支持 0~255ms；
 *                若需支持更长时间可将类型改为 uint16_t)
 */
void delay_ms(uint8_t count)
{
    // 1、先将 tim 的计数器清空 (0)
    TIM_SetCounter(TIM14, 0);

    // 2、去获取计数器的值与 count 进行比较
    // 等计数值达到 count
    while (TIM_GetCounter(TIM14) < count);
}
```

### 2. timer.h 头文件声明

```c
#ifndef __TIMER_H
#define __TIMER_H

#include "stm32f4xx.h"

void Tim6_Init(void);
void TIM14_Init_1ms(void);
void delay_ms(uint8_t count);

#endif /* __TIMER_H */
```

### 3. 在 main.c 中的调用示例

```c
#include "stm32f4xx.h"
#include "timer.h"
#include "led.h"

int main(void)
{
    LED_Init();
    
    // 初始化 TIM14 (1 个数 = 1ms)
    TIM14_Init_1ms();

    while (1)
    {
        // 翻转 LED
        GPIO_ToggleBits(GPIOF, GPIO_Pin_9);

        // 调用通用延时，如延时 20ms
        delay_ms(20); 
    }
}
```

---

## 五、 重点说明与拓展建议

1. 变量名称与类型说明：
   - 笔记截图中参数写为 count (图中笔误写作 conut)。
   - uint8_t 的有效数值范围为 0 ~ 255。若延时参数超过 255ms（例如需要延时 500ms 或 1000ms），uint8_t 会发生整数溢出，因此工程中更建议将形参声明为 uint16_t count。

2. 为什么需要清空计数器？
   - 每次进入 delay_ms 时，TIM14 的硬件计数器可能停留在一个非零的历史值。如果不先清零（TIM_SetCounter(TIM14, 0)），判断条件 TIM_GetCounter(TIM14) < count 会因为初始值已经大于或接近 count 而立即退出，导致延时严重缩水。

3. 对比软件 for 循环延时的优势：
   - 原代码中 for (; nCount != 0; nCount--); 依赖 CPU 执行指令周期，会受编译器优化级别（-O0/-O2）及中断抢占的影响，延时极不精确。
   - 硬件定时器直接由内部总线时钟驱动，计数周期严格等于 1ms，延时精度极高。