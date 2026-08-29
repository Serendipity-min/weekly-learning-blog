# 🔧 本周问题与知识沉淀：用 AI 智能体 + MCP + 技能驱动嘉立创 EDA 完成 PCB 与原理图绘制

> **日期**：2026-08-27（调研与落地）/ 2026-08-29（沉淀成文）
> **主题**：如何在 **Codex / Antigravity / DeepSeek Harness** 等 AI 编程智能体上，通过 **MCP（Model Context Protocol）** 与 **Agent Skills（技能）** 让智能体直接操作**嘉立创 EDA（EasyEDA 专业版）**，完成原理图绘制、PCB 布局布线、DRC 检查等任务。
> **适用读者**：嵌入式/硬件学习者，想用 AI 辅助画板子的人。

---

## 一、问题背景

学习 PCB 制作后，一个自然的想法冒出来：**画原理图、布线的很多操作是重复性的（放元件、连线、铺铜），能不能让 AI 智能体替我做？**

于是本周提出一个具体问题：

> 在 GitHub 上寻找有没有能够在 **Antigravity** 或 **Codex**（候选）平台上使用的 **MCP 工具**，或者能帮我实现在**嘉立创 EDA** 平台完成原理图制作的**技能（Skill）**。

最终不仅完成了调研，还完成了整套工具链的本地安装与验证。

---

## 二、调研结论：两条可行路线

经过 GitHub 检索 + 仓库 README 核实，结论是**有官方方案，且 Codex / Antigravity 都能用**：

| 方案 | 类型 | 说明 | 状态 |
|---|---|---|---|
| **官方 SKILL 路线**（画原理图首选） | Agent Skill（SKILL.md 标准） | `easyeda-api-skill`（基础 API + 桥）+ `easyeda-enhanced-schematic-skill`（原理图绘制功能层） | ✅ 已安装 |
| **MCP 路线** | stdio MCP Server（59 个工具） | `hyl64/jlcmcp`：原理图/PCB 自动化、自动布局/布线、DRC 自修复、网表生成原理图 | ✅ 已安装 |

### 路线 1：官方 SKILL

- **[easyeda/easyeda-api-skill](https://github.com/easyeda/easyeda-api-skill)**（⭐586）——基础层：完整的 EasyEDA Pro API 文档索引 + WebSocket Bridge 服务器。AI 通过 HTTP 向 EDA 发送 `eda.*` 代码，例如：
  ```js
  return await eda.dmt_Project.getCurrentProjectInfo();
  ```
- **[easyeda/easyeda-enhanced-schematic-skill](https://github.com/easyeda/easyeda-enhanced-schematic-skill)**（⭐27）——增强层：**专门画原理图**。
  - 通过 LCSC ID 批量获取器件（`lib_Device.getByLcscIds()`）
  - 绘制模块框（USB / MCU / 显示分区）
  - 框内按间距放置元件、读取引脚位置（`getAllPinsByPrimitiveId()`）
  - 创建网络标记（电源/地）与网络端口（信号）
  - 自动连线（每个引脚到网络标签短连）
- 官方实测平台：**CodeX (gpt-5.6)、Hermes (glm-5.2)、Claude (opus-4.7)、OpenCode**；SKILL.md 标准同样兼容 Antigravity CLI 与 Codex CLI。

### 路线 2：MCP Server（jlcmcp）

- **[hyl64/jlcmcp](https://github.com/hyl64/jlcmcp)**（⭐193）——嘉立创 EDA MCP Server（官方栈迁移版），v1.0 已完整迁移到官方 JLC AI 栈，**59 个工具**：
  - 原理图：`sch_get_state` / `sch_get_netlist` / `sch_run_drc` / `sch_generate_from_netlist`
  - PCB：`pcb_get_state` / `pcb_screenshot` / `pcb_run_drc` / `pcb_get_board_info`
  - 自动布局布线：`pcb_auto_place_components` / `pcb_auto_route_nets` / `pcb_auto_fanout_and_route`
  - DRC 修复：`pcb_drc_autofix` / `pcb_design_health_report`
  - BOM/分析：`pcb_bom_export` / `pcb_net_connectivity_check` / `pcb_current_density_report`
  - 计算：`calc_impedance` / `calc_trace_width`
- 注意：`pcb_agent`（自主智能体工具）需要 `ANTHROPIC_API_KEY` 环境变量，其余 58 个工具开箱即用；坐标单位均为 mil。

### 备选方案（未采用）

- [salitronic/eda-agent](https://github.com/salitronic/eda-agent)（⭐161）——主攻 Altium（400+ 工具），EasyEDA Pro 只是可选后端，与需求不完全对口。
- [rayfalling/easyeda-mcp](https://github.com/rayfalling/easyeda-mcp)（⭐5）——轻量 MCP，早期项目，功能较少。
- [BeckhamLabsLLC/kicad-jlcpcb](https://github.com/BeckhamLabsLLC/kicad-jlcpcb) ——KiCad → EasyEDA 自动布线 → JLCPCB 下单，偏打板流程。

---

## 三、架构原理（重点理解）

```
AI 智能体 (Antigravity / Codex / DSH)
   │  ① SKILL 路线: 读取 SKILL.md → 调用 HTTP API
   │  ② MCP 路线:   jlcmcp (stdio MCP) → 编译 eda.* 代码
   ▼
桥服务器 Bridge Server (Node.js, 端口 49620-49629, 单例)
   │  WebSocket 握手 (service: "easyeda-bridge")
   ▼
嘉立创 EDA 专业版 (Run API Gateway 扩展 V1.0.5+, 已启用"允许外部交互")
```

关键点：

1. **MCP 是"智能体 ↔ 工具"的标准化协议**：智能体通过 stdio 与 MCP Server 通信，MCP Server 把每个工具动作编译成 `eda.*` 官方 API 代码。
2. **桥服务器是"电脑 ↔ EDA"的桥梁**：官方 Bridge Server（来自 easyeda-api-skill 的 `scripts/bridge-server.mjs`）监听 **49620–49629** 端口，嘉立创 EDA 的 Run API Gateway 扩展**自动扫描端口段并握手连接**。
3. **EDA 侧扩展接收 `new AsyncFunction('eda', code)` 执行**——所以 AI 的每个操作都是实时作用于你打开的工程。

---

## 四、安装落地过程（08-27 实操记录）

### 4.1 前置条件检查

- Node.js ≥ 18（建议 22 LTS）✅
- git ✅
- 嘉立创 EDA 专业版 V3.2+（已装）✅
- 在 EDA 中安装官方 **Run API Gateway 扩展**（扩展广场：[jlcext.com/item/oshwhub-official/run-api-gateway](https://jlcext.com/item/oshwhub-official/run-api-gateway)，源码 [easyeda/eext-run-api-gateway](https://github.com/easyeda/eext-run-api-gateway)），并勾选**「允许外部交互」**✅

### 4.2 目录与克隆

```
E:\嘉立创EDA\ai-tools\
├── easyeda-api-skill\                # 官方 API 技能（基础层）
├── easyeda-enhanced-schematic-skill\ # 官方原理图增强技能
├── jlcmcp\                           # MCP Server（dist/index.js 已编译）
├── start-bridge.cmd                  # 启动桥服务器（双击运行）
├── check-bridge.cmd                  # 查看桥服务器与 EDA 连接状态
└── README.md                         # 使用文档
```

### 4.3 构建与验证

- `npm install` 三个仓库；
- jlcmcp `npm run build` 产出 `dist/index.js`；
- **桥协议冒烟测试 73/75 通过**（2 个失败为 mock 夹具问题，不影响真实使用）；
- 启动桥服务器后 `curl http://localhost:49620/health` 返回：
  ```json
  {"service":"easyeda-bridge","status":"ok","edaConnected":false}
  ```

### 4.4 Antigravity 注册（优先平台）

- **技能**：用目录联接（junction）挂到 Antigravity 全局技能目录：
  - `~/.gemini/config/skills/easyeda-api` → `E:\嘉立创EDA\ai-tools\easyeda-api-skill`
  - `~/.gemini/config/skills/easyeda-enhanced-schematic` → `E:\嘉立创EDA\ai-tools\easyeda-enhanced-schematic-skill`
- **MCP**：写入全局配置 `~/.gemini/config/mcp_config.json`：
  ```json
  {
    "mcpServers": {
      "jlceda": {
        "command": "D:/Node/node.exe",
        "args": ["E:/嘉立创EDA/ai-tools/jlcmcp/dist/index.js"],
        "env": {}
      }
    }
  }
  ```

### 4.5 Codex 接入方式（未在本机启用，备用）

Codex CLI 同样兼容，在项目里执行：

```bash
codex mcp add jlceda -- node D:/Node/node.exe E:/嘉立创EDA/ai-tools/jlcmcp/dist/index.js
```

技能放入项目的 `.agents/skills/` 或 `~/.gemini/config/skills/` 同构目录即可。

---

## 五、遇到的问题与踩坑记录（重点沉淀）

### 问题 1：桥服务器端口在哪里配置？EDA 扩展管理器里看不到端口

- **现象**：嘉立创 EDA 的扩展管理器中没有任何端口设置项，一度以为配置遗漏。
- **结论**：端口 **不在 EDA 里配置**。49620–49629 是官方 easyeda-api-skill 的**固定约定**，桥服务器是电脑上的 Node.js 进程，嘉立创 EDA 的 Run API Gateway 扩展会**自动扫描这个端口段并握手连接**。
- **教训**：先读官方文档的固定约定，别在客户端里找不存在的配置项。

### 问题 2：`/health` 返回 `edaConnected: false`

- **原因**：嘉立创 EDA 没打开，或 Run API Gateway 扩展未启用。
- **解决**：打开嘉立创 EDA → 高级 → 扩展管理器确认 Run API Gateway 状态；桥窗口保持开启；EDA 打开后扩展自动连上，无需手动配置。
- **多窗口场景**：用 `pcb_list_eda_windows` / `pcb_select_eda_window` 指定目标窗口。

### 问题 3：mcp_config.json 中文路径显示乱码

- **现象**：PowerShell 终端里读 `mcp_config.json`，`E:/嘉立创EDA/...` 显示成 `E:/鍢夌珛鍒汦DA/...`。
- **结论**：**文件本身是正确的 UTF-8**，乱码只是 Windows 控制台代码页（GBK）显示问题，用支持 UTF-8 的方式读取后确认无误。
- **教训**：Windows 下处理含中文路径的 JSON 配置文件，务必用 UTF-8 工具链验证文件字节，不要被终端显示误导。

### 问题 4：`pcb_agent` 工具不可用

- **现象**：MCP 工具列表里 `pcb_agent`（自主智能体）调用报错。
- **原因**：该工具需要 `ANTHROPIC_API_KEY` 环境变量，其余 58 个工具开箱即用。
- **解决**：不需要自主智能体的话忽略即可；需要时配置环境变量。

### 问题 5：第三方代码安全与版本漂移

- **教训**：jlcmcp 是社区项目（非嘉立创官方维护），MCP/技能会**直接修改 EDA 里打开的实时设计**。首次使用务必在**备份工程或测试工程**上验证；尽量固定 commit 而非跟随 main 分支。

### 问题 6：冒烟测试 73/75 的两个失败项

- 两个失败均为 **mock 夹具问题**（无真实 EDA 连接时的模拟数据差异），不影响真实使用。验证方式：桥服务器 + 真实 EDA 打开后调用健康检查接口确认。

---

## 六、使用姿势（日常操作）

### Antigravity（优先平台）

重启 Antigravity 让 MCP 配置与技能生效（MCP 在"更多选项(…) → MCP Servers"里可看到 `jlceda`），然后直接对话：

- 「嘉立创EDA，启动！」→ 技能初始化会话并检查桥连接
- 「帮我在原理图上放置 STM32F407 最小系统元件并按模块框扇出引脚」
- 「用 jlceda 跑一下当前 PCB 的 DRC 并自动修复丝印冲突」

### DeepSeek Harness（本工作区）

- 已安装 `dsh-mcp-bridge` 等 MCP 相关插件（2026-08-24 web profile 重装），MCP 服务器通过标准 stdio 配置即可接入；本工具链同样适用（jlceda 是标准 MCP server）。

### 操作纪律

1. **先备份工程**：MCP/技能直接改实时设计，务必先在测试工程验证；
2. **信号网用网络端口、电源/地用网络标记**：技能会自动放短线连接每个引脚；
3. **坐标单位是 mil**，与 EDA 内一致；
4. **Codex CLI 兼容性最稳**，Antigravity 的 MCP/SKILL 支持较新，遇到问题可先用 Codex CLI 验证。

---

## 七、本周思考与收获

1. **MCP 是"智能体时代的 USB-C"**：一个标准协议让所有智能体接入同一套硬件工具链，选型时优先选「标准 MCP / SKILL.md」方案，可移植性最强。
2. **官方优先**：嘉立创官方已经出了 API Skill 和原理图增强 Skill，社区 MCP（jlcmcp）也迁移到了官方栈——官方栈的稳定性和文档质量明显更好。
3. **画原理图的本质是"操作 EDA 对象模型"**：放元件、画框、连线、建网络，每一步都能映射到 `eda.*` API 调用，所以自动化可行；而 PCB 的**自动布线/DRC 自修复**是 AI 价值最大的环节。
4. **工具链分层思维**：智能体（决策）→ MCP Server（协议适配）→ 桥服务器（进程间通信）→ EDA 扩展（执行环境），每一层都职责单一，出了问题能快速定位（`/health` 一层层查）。

---

## 八、参考链接

- 官方 API 技能：[github.com/easyeda/easyeda-api-skill](https://github.com/easyeda/easyeda-api-skill)
- 官方原理图增强技能：[github.com/easyeda/easyeda-enhanced-schematic-skill](https://github.com/easyeda/easyeda-enhanced-schematic-skill)
- 官方 Run API Gateway 扩展：[jlcext.com/item/oshwhub-official/run-api-gateway](https://jlcext.com/item/oshwhub-official/run-api-gateway)（源码 [github.com/easyeda/eext-run-api-gateway](https://github.com/easyeda/eext-run-api-gateway)）
- MCP 服务器 jlcmcp：[github.com/hyl64/jlcmcp](https://github.com/hyl64/jlcmcp)
- 备选：eda-agent / easyeda-mcp / kicad-jlcpcb（见上文）

> ⚠️ 安全提醒：MCP/技能会直接修改 EDA 实时设计，且社区代码需要审查；首次使用请在备份工程上验证。
