# 📚 每周学习博客 · Weekly Learning Blog

<div align="center">

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![STM32](https://img.shields.io/badge/MCU-STM32F407ZGT6-blue.svg)](https://www.st.com/)
[![IoT](https://img.shields.io/badge/IoT-ESP8266%20%7C%20MQTT%203.1.1-green.svg)](https://mqtt.org/)
[![EDA](https://img.shields.io/badge/EDA-EasyEDA%20Pro%20%7C%20MCP-orange.svg)](https://lceda.cn/)

<p align="center">
  <b>嵌入式（STM32）与硬件设计系统学习记录</b><br>
  每周一篇周记总结 + 深度问题沉淀 · 见证从电路基础到完整物联网项目的成长轨迹
</p>

</div>

---

## 🗓️ 文章索引

| 周次 | 日期 | 核心主题 | 周记总结 | 深度问题沉淀 |
|:---|:---|:---|:---:|:---|
| **[2026-W36](weekly/2026-W36/)** | 08-31 ~ 09-06 | STM32F407 驱动体系 · 硬件调试三剑客 · 定时器/PWM电机驱动 · 自研 MQTT 协议栈 | [周记总结](weekly/2026-W36/index.md) | [🔧 STM32F407+ESP8266 MQTT 物联网实战与按键解耦](weekly/2026-W36/issues/stm32f407-esp8266-mqtt-iot.md) |
| **[2026-W35](weekly/2026-W35/)** | 08-24 ~ 08-30 | 电路基础 · 模拟/数字电路 · PCB 制作 · AI×嘉立创EDA 工具链 | [周记总结](weekly/2026-W35/index.md) | [🔧 AI 智能体 + MCP + 技能驱动嘉立创 EDA](weekly/2026-W35/issues/ai-agent-mcp-jlceda.md) |

---

## 📝 课后作业与实验工程 (Homework & Labs)

> 详细工程结构、外设接线与源码说明请参阅：**[🛠️ 课后作业与实验工程总览 (homework/README.md)](homework/README.md)**

| 分类 | 项目 / 文档 | 核心功能与学习成果 | 链接 |
|:---|:---|:---|:---:|
| **实战工程** | **PWM 呼吸灯与 TB6612 电机驱动系统** | TIM3 10kHz 硬件 PWM、按键加减速调速状态机、硬件短路刹车制动、OLED 运行状态监控 | [进入工程](homework/PWM_LED/) |
| **实战工程** | **GPIO 按键输入与 LED 状态翻转** | 硬件下拉 (KEY_UP) 与软件上拉 (KEY0~2) 输入检测，按键消抖，PF9/10 推挽输出控制 | [进入工程](homework/STM32F407_KeyUpLED/) |
| **实战工程** | **TIM 基本/通用定时器中断与精密时基** | APB1 84MHz 定时器时钟计算，1s 定时中断心跳，微秒/毫秒非阻塞硬件延时 | [进入工程](homework/STM32_TIME/) |
| **工程模板** | **STM32F407ZGT6 标准库纯净模板** | 汇编启动文件、SystemInit 时钟树分频、中断向量表、SysTick 滴答时钟与 USART1 串口 | [进入工程](homework/STM32F407_Template/) |
| **工具框架** | **AI 串口交互控制台** | 基于 USART 的字符/JSON 交互协议与自动化流水线框架 | [进入工程](homework/ai_serial_console/) |
| **底层基础** | **嵌入式 C 语言底层进阶教案 (14篇)** | 指针初阶/进阶、动态内存管理、结构体位段、预处理与模块化工程组织笔记 | [查看教案](homework/C语言/) |
| **技术报告** | **TB6612 电机驱动商家例程分析与 F4 对比** | 深度解析商家 F103 例程引脚/驱动真值/10kHz PWM/初始占空比，并针对 STM32F407 给出规避 SPI Flash/LCD 冲突的完整移植方案 | [阅读文档](homework/TB6612电机驱动与MG310电机商家代码分析及STM32F4对比文档.md) |
| **硬件手册** | **STM32F407 开发板硬件资源全映射手册** | 裸板 Chip ID 探测、片上 Flash/SRAM 规格、板载外设 GPIO 映射表与 P8/P9 60-Pin 排针速查 | [阅读文档](homework/STM32F407开发板硬件资源与模块引脚说明手册.md) |
| **实验笔记** | **通用延时函数与 MG310 电机接线规范** | MG310 霍尔减速电机电气参数、TB6612 驱动板端子定义与弱电/强电必须严格共地的红线原则 | [阅读文档](homework/0904通用延时函数与电机信息.md) |

---

## 📁 仓库结构

```
weekly-learning-blog/
├── README.md               # 博客首页导航与全局索引
├── LICENSE                 # MIT 开源协议
├── .gitignore              # Git 忽略配置（已过滤 Keil MDK 编译垃圾文件）
├── weekly/                 # 每周文章归档（按 ISO 周目录组织）
│   ├── 2026-W35/           # 第 35 周（电路基础与 PCB 制作）
│   │   ├── index.md        # 周记总结
│   │   └── issues/         # 问题与知识沉淀
│   │       └── ai-agent-mcp-jlceda.md
│   └── 2026-W36/           # 第 36 周（STM32驱动与MQTT物联网）
│       ├── index.md        # 周记总结
│       └── issues/         # 问题与知识沉淀
│           └── stm32f407-esp8266-mqtt-iot.md
└── homework/               # 课后作业与实验工程归档【作业专用目录】
    ├── README.md           # 作业总览与实验工程详细索引
    ├── PWM_LED/            # 作业：TIM3 PWM 呼吸灯与 TB6612 电机驱动系统
    ├── STM32F407_KeyUpLED/ # 作业：GPIO 输入输出（按键扫描与 LED 控制）
    ├── STM32_TIME/         # 作业：TIM 定时器中断与时基延时系统
    ├── STM32F407_Template/ # 作业：STM32F407 标准外设库基础工程模板
    ├── ai_serial_console/  # 作业：AI 串口自动化交互控制台
    ├── C语言/              # 作业：嵌入式 C 语言底层进阶教案与习题（14篇）
    ├── TB6612电机驱动...md # 实验报告：TB6612 与 MG310 驱动例程剖析及 F4 移植对比
    ├── STM32F407开发板...md# 实验参考：开发板硬件资源与 60-Pin 排针映射手册
    ├── 0904通用延时函数...md# 实验参考：TIM14 延时与 MG310/TB6612 硬件参数
    └── ...                 # 更多电路原理图截图与笔记
```

---

## 🎯 专栏定位与技术栈

- **嵌入式软件与系统**：STM32F407 (ARM Cortex-M4)、Keil MDK 5、标准外设库、SysTick 定时器、USART 中断驱动、自研轻量协议栈；
- **物联网与网络通信**：ESP8266 WiFi 模组、MQTT 3.1.1 二进制变长编码协议栈、腾讯云 Mosquitto、EMQX Broker、MQTTX (GUI/CLI)；
- **硬件设计与自动化**：嘉立创 EDA 专业版、原理图/PCB 设计、DRC 规则检查、AI 智能体 (Antigravity/Codex) × MCP 自动化工具链。

---

## 🔧 维护指南（每周例行）

1. 新建目录 `weekly/<ISO周>/`（如 `weekly/2026-W36/`）；
2. 编写 `index.md`（周记总结）与 `issues/` 下的专项问题沉淀文档；
3. 更新 `README.md` 的索引表格；
4. 提交并推送至 GitHub：
   ```bash
   git add -A
   git commit -m "docs: 2026-W36 周记与问题沉淀"
   git push origin main
   ```

---

## 📜 许可证

本项目遵循 [MIT](LICENSE) 开源许可证 © 2026 Serendipity-min
