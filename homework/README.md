# 🛠️ 课后作业与实验工程归档 · Homework & Labs

本目录用于集中归档嵌入式学习过程中的**课后作业、硬件外设实战工程源码、模块驱动分析与实验技术文档**。

---

## 📁 作业与实验工程全景

### 1. 嵌入式实战工程代码 (`projects`)

| 工程目录 | 核心主控 | 核心功能与实验内容 | 说明文档 / 关键代码 |
| :--- | :--- | :--- | :--- |
| **[`PWM_LED/`](PWM_LED/)** | STM32F407ZGT6 | **TIM3 PWM 呼吸灯 + TB6612 电机双向调速系统**<br>• TIM3_CH3/CH4 输出 10kHz 硬件 PWM<br>• 按键状态机交互：KEY_UP(+10%)、KEY0(-10%)、KEY1(正转/刹车)、KEY2(反转/刹车)<br>• 硬件短路制动刹车控制 (`Motor_Stop`) 与 LED0/1 指示<br>• 0.96寸 OLED 实时状态监控与编码器接口 | [`USER/main.c`](PWM_LED/USER/main.c)<br>[`HARDWARE/pwm.c`](PWM_LED/HARDWARE/pwm.c) |
| **[`STM32F407_KeyUpLED/`](STM32F407_KeyUpLED/)** | STM32F407ZGT6 | **GPIO 输入输出实战：按键扫描与 LED 翻转**<br>• PA0 (KEY_UP, 高电平有效，硬件下拉)<br>• PE2/3/4 (KEY2/1/0, 低电平有效，软件上拉)<br>• PF9/PF10 (LED0/LED1 推挽输出点亮) | [`USER/main.c`](STM32F407_KeyUpLED/USER/main.c)<br>[`HARDWARE/key.c`](STM32F407_KeyUpLED/HARDWARE/key.c) |
| **[`STM32_TIME/`](STM32_TIME/)** | STM32F407ZGT6 | **定时器中断与精密时基实战**<br>• TIM 基本/通用定时器更新中断配置<br>• APB1 84MHz 定时器时钟计算与 1s 周期心跳中断<br>• 精准微秒/毫秒非阻塞与阻塞延时 | [`USER/main.c`](STM32_TIME/USER/main.c)<br>[`HARDWARE/timer.c`](STM32_TIME/HARDWARE/timer.c) |
| **[`STM32F407_Template/`](STM32F407_Template/)** | STM32F407ZGT6 | **标准外设库纯净工程模板**<br>• 预配置完整 Cortex-M4 启动汇编、时钟树分频与中断向量表<br>• 集成 SysTick 滴答时钟与 USART1 调试串口打印输出 | [`USER/main.c`](STM32F407_Template/USER/main.c) |
| **[`ai_serial_console/`](ai_serial_console/)** | STM32F407ZGT6 | **AI 串口控制台与自动化流水线**<br>• USART 字符/JSON 交互协议<br>• 自动化测试与串口指令解析 | [`README.md`](ai_serial_console/README.md) |
| **[`C语言/`](C语言/)** | C 语言 | **嵌入式 C 语言底层进阶教案与习题**<br>• 共 14 篇教案笔记（`.lake` 格式）<br>• 涵盖指针初阶/进阶、内存动态管理、结构体位段、预处理宏等 | [`C语言/`](C语言/) |

---

### 2. 实验报告与核心技术文档 (`docs`)

| 文档名称 | 核心主题 | 关键知识点与内容概要 |
| :--- | :--- | :--- |
| **[TB6612电机驱动与MG310电机商家代码分析及STM32F4对比文档.md](TB6612电机驱动与MG310电机商家代码分析及STM32F4对比文档.md)** | **电机驱动综合对比** | • 商家 F103 例程引脚、驱动真值表、10kHz PWM 频率与 41.67% 初始占空比推导<br>• STM32F407ZGT6 引脚避让与板载 SPI Flash/LCD 背光冲突分析<br>• F1 vs F4 定时器时钟树（72MHz vs 84MHz）与 ARR 换算 |
| **[STM32F407开发板硬件资源与模块引脚说明手册.md](STM32F407开发板硬件资源与模块引脚说明手册.md)** | **硬件手册与引脚速查** | • 裸板实测 Chip ID (`0x413`) 与 1MB Flash / 192KB SRAM 资源<br>• 板载按键、LED、蜂鸣器、SRAM、W25Q128、LAN8720A 全映射全表<br>• P8 / P9 双排 60-Pin 扩展引脚完整定义与复用查询 |
| **[0904通用延时函数与电机信息.md](0904通用延时函数与电机信息.md)** | **延时函数与电机参数** | • MG310 霍尔电机电气规格（7.4V, 13ppr, 减速比 1:20）<br>• TB6612 稳压版端子定义与 6-Pin 线序对应关系 |
| **[TIM14通用延时函数伪代码.md](TIM14通用延时函数伪代码.md)** | **定时器延时算法** | • 基于 APB1 84MHz 定时器配置 1ms/1μs 精密硬件延时伪代码 |
| **[0825马喜乐.md](0825马喜乐.md)** | **强电/弱电驱动规范** | • 为什么单片机不能直驱舵机/电机<br>• 独立动力电源供电与**必须严格共地**的红线原则 |
| **[STM32_Keil_to_MCP_Flash_Workflow.md](STM32_Keil_to_MCP_Flash_Workflow.md)** | **开发工具链** | • Keil MDK 编译产物自动导出与 MCP 工具链自动化烧录流水线 |
| **[01-电学基础.lake](01-电学基础.lake)** | **电路基础** | • 电压、电流、阻抗与欧姆定律基石 |
| **[模块连接.txt](模块连接.txt)** | **器件与物料清单** | • 外接传感器、OLED、蓝牙、TB6612、MG310 物料采购与选型链接 |

---

## ⚠️ 嵌入式硬件安全红线说明

1. **信号与动力独立，地线必须严格相连**：
   - 电机/舵机等大功率负载必须使用独立动力电池供电（如 7.4V 2S 锂电池组）；
   - 单片机开发板 GND 与电机驱动板 GND **必须使用杜邦线牢固连通（严格共地）**，严禁悬空。
2. **严禁 GPIO 直连感性负载**：
   - STM32 GPIO 输出极限仅 25mA，直流电机启动/堵转电流可达 1A~2A，直连必烧 MCU；必须通过 TB6612 等驱动芯片进行功率放大。
3. **严禁带电热插拔电机动力线**：
   - 直流电机绕组具有显著感抗（MG310 内部电感约 30.7mH），带电插拔瞬间产生的几十伏反电动势感应尖峰会瞬间击穿 TB6612 H 桥 MOS 管。
