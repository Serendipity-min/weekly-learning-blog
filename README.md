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

> 详细工程结构、外设接线与源码说明请参阅：**[🛠️ 0907 作业与实验工程总览 (homework/README.md)](homework/README.md)**

| 分类 | 项目 / 文档 | 核心功能与学习成果 | 链接 |
|:---|:---|:---|:---:|
| **零基础教程** | **STM32F407 电机调速与测速工程小白从零入门全解** | 针对初学者系统拆解：为什么不能直插电机、168MHz 时钟树 10kHz PWM 推导、TB6612 短路刹车、TIM4 编码器 4 倍频 1040PPR 转速公式、ADC 10k:1k 分压原理与状态机架构 | [零基础精讲](homework/STM32F407电机调速与测速监控工程小白从零入门全解.md) |
| **实战工程** | **电机测速、OLED 监控与 ADC 采样综合系统** | TIM3 10kHz PWM、PE0/1 防外设冲突正反转、TIM4 硬件正交编码器 4 倍频测速、TIM6 100ms 周期 RPM 换算、0.96 寸 I2C OLED 动态多维监控、ADC1 母线电压采样 | [进入工程源码](homework/Motor_Encoder_OLED_Monitor/) |
| **深度复盘** | **TB6612 电机测速/OLED/ADC 实战深度复盘报告** | 破除“电流 4095”硬件限制认知、PA6 板载 SPI Flash 冲突、碰触 GND 工频感应 4~10V 跳动解析、复位误入 ROM DFU Bootloader 排查及系统 4 大局限缺陷清单 | [阅读复盘报告](homework/TB6612电机测速与OLED监控系统实战复盘报告.md) |
| **基础工程** | **PWM 呼吸灯与 TB6612 电机调速工程** | TIM3 10kHz 硬件 PWM、按键加减速调速状态机 (0~100%)、硬件短路刹车制动 (`Motor_Stop`)、板载 LED 运行状态指示 | [进入工程源码](homework/PWM_LED/) |
| **技术报告** | **TB6612 电机驱动商家例程分析与 F4 对比文档** | 深入剖析商家 F103 例程引脚/驱动真值/10kHz PWM/41.67% 初始占空比，并针对 STM32F407 给出规避 SPI Flash/LCD 冲突的完整移植方案与安全红线 | [阅读技术报告](homework/TB6612电机驱动与MG310电机商家代码分析及STM32F4对比文档.md) |

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
└── homework/               # 课后作业归档【0907 电机驱动与实验作业】
    ├── README.md           # 0907 作业总览与实验工程详细索引
    ├── STM32F407电机调速...md       # 零基础精讲：小白从零理解 PWM/编码器/ADC/状态机全逻辑
    ├── Motor_Encoder_OLED_Monitor/  # 综合工程：电机调速 + 编码器测速 + OLED + ADC 监控
    ├── TB6612电机测速与OLED...md    # 深度复盘报告：硬件冲突/物理限制/故障排查/BUG清单
    ├── PWM_LED/            # 基础工程源码：TIM3 PWM 呼吸灯与 TB6612 电机驱动系统
    └── TB6612电机驱动...md # 实验报告：TB6612 与 MG310 驱动例程剖析及 F4 移植对比文档
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
