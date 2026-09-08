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

## 📝 课后作业文档 (Homework Documents)

> 完整作业文档归档与专题目录请参阅：**[🛠️ 作业文档总索引 (homework/README.md)](homework/README.md)**  
> *注：索引列表仅展示核心技术沉淀与系统解析文档，各期实战工程源码完整保留在对应作业目录下。*

| 日期 | 作业专题 | 核心技术沉淀文档 | 要点概述 |
|:---:|:---|:---|:---|
| **09-08** | **[定时器PWM驱动蜂鸣器](homework/0908_定时器PWM驱动蜂鸣器/)** | [STM32F407 蜂鸣器 PWM 硬件驱动与八音阶演奏原理全解析](homework/0908_定时器PWM驱动蜂鸣器/STM32F407蜂鸣器PWM硬件驱动与八音阶演奏原理全解析.md) | S8050 NPN 放大驱动、TIM13_CH1 唯一定时器选型、十二平均律八音阶 ARR/CCR 精确计算表、发声包络与顿音消振 |
| **09-07** | **[电机驱动与技术对比](homework/0907_电机驱动与技术对比/)** | 1. [STM32F407 电机调速与测速工程原理与代码](homework/0907_电机驱动与技术对比/STM32F407电机调速与测速工程原理与代码.md)<br>2. [TB6612 电机测速与 OLED 监控系统实战复盘报告](homework/0907_电机驱动与技术对比/TB6612电机测速与OLED监控系统实战复盘报告.md)<br>3. [TB6612 驱动商家例程分析与 F4 对比文档](homework/0907_电机驱动与技术对比/TB6612电机驱动与MG310电机商家代码分析及STM32F4对比文档.md) | 电机隔离驱动与 10kHz PWM、TB6612 短路制动、TIM4 编码器 4 倍频测速、PA6 Flash 冲突与 DFU 误入排查、商家例程对比移植方案 |

---

## 📁 仓库结构

```
weekly-learning-blog/
├── README.md                  # 博客首页导航与全局索引
├── LICENSE                    # MIT 开源协议
├── .gitignore                 # Git 忽略配置（已过滤 Keil MDK 编译临时文件）
├── weekly/                    # 每周文章归档（按 ISO 周目录组织）
│   ├── 2026-W35/              # 第 35 周（电路基础与 PCB 制作）
│   │   ├── index.md           # 周记总结
│   │   └── issues/            # 问题与知识沉淀
│   └── 2026-W36/              # 第 36 周（STM32驱动与MQTT物联网）
│       ├── index.md           # 周记总结
│       └── issues/            # 问题与知识沉淀
└── homework/                  # 课后作业归档（按 MMDD_作业专题 组织）
    ├── README.md              # 作业文档总索引
    ├── 0907_电机驱动与技术对比/  # 2026-09-07 电机驱动、测速与技术对比（含文档与工程源码）
    └── 0908_定时器PWM驱动蜂鸣器/ # 2026-09-08 定时器PWM驱动蜂鸣器（含文档与工程源码）
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
