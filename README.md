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
| **[2026-W36](weekly/2026-W36/)** | 08-31 ~ 09-06 | STM32F407 GPIO · ESP8266 WiFi · 自研轻量 MQTT 3.1.1 协议栈 · 双向物联网云控 | [周记总结 (待定)](weekly/2026-W36/index.md) | [🔧 STM32F407+ESP8266 MQTT 物联网实战与按键解耦](weekly/2026-W36/issues/stm32f407-esp8266-mqtt-iot.md) |
| **[2026-W35](weekly/2026-W35/)** | 08-24 ~ 08-30 | 电路基础 · 模拟/数字电路 · PCB 制作 · AI×嘉立创EDA 工具链 | [周记总结](weekly/2026-W35/index.md) | [🔧 AI 智能体 + MCP + 技能驱动嘉立创 EDA](weekly/2026-W35/issues/ai-agent-mcp-jlceda.md) |

---

## 📁 仓库结构

```
weekly-learning-blog/
├── README.md               # 博客首页导航与全局索引
├── LICENSE                 # MIT 开源协议
├── .gitignore
└── weekly/                 # 每周文章归档（按 ISO 周目录组织）
    ├── 2026-W35/           # 第 35 周（电路基础与 PCB 制作）
    │   ├── index.md        # 周记总结
    │   └── issues/         # 问题与知识沉淀
    │       └── ai-agent-mcp-jlceda.md
    └── 2026-W36/           # 第 36 周（STM32驱动与MQTT物联网）
        ├── index.md        # 周记总结（待定）
        └── issues/         # 问题与知识沉淀
            └── stm32f407-esp8266-mqtt-iot.md
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
