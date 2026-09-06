# STM32F407 AI 串口控制台

该工程为 STM32F407ZGT6 开发板提供最小、可审计的串口命令接口。板载 CH340C 连接 USART1（PA9/PA10），电脑端为 `COM15`。

## 运行时协议

- 串口：115200、8N1、无流控。
- 请求：一行 ASCII 命令，以 `\n` 结束。
- 响应：一行 JSON，以 `\r\n` 结束。
- 当前命令仅包含 `help`、`info`、`status`、`ping`、`led get` 和 `led set on|off`；不提供任意内存、寄存器或 Flash 写入能力。

示例：

```text
> info
{"ok":true,"fw":"ai-console-0.1.0","chip":"STM32F407ZGT6","transport":"USART1@115200-8N1"}
```

## 构建与烧录

```powershell
python .\tools\build.py
python .\tools\flash.py --probe
python .\tools\flash.py --confirm-target
```

`flash.py --probe` 只读识别 COM15 上的 Bootloader；`flash.py --confirm-target` 会先核对 F407 芯片 ID 与本板唯一 UID，匹配后才擦除、写入并校验 `build/ai_serial_console.bin`。它使用板载 CH340 的自动下载电路，脚本不会静默写入。

烧录后的正常运行跳帽应保持 `B0=GND`、`B1=GND`。自动下载脚本会临时控制 CH340 的 DTR/RTS，无需人工切换。

## 本地 MCP

`mcp/stm32_console_mcp.py` 是本机 stdio MCP 服务，只能访问固定的 `COM15` 与固件白名单命令；它不提供任意串口透传、Bootloader 或网络监听能力。

可提供的工具为 `stm32_console_ping`、`stm32_console_info`、`stm32_console_status`、`stm32_console_led_get`、`stm32_console_led_set`，以及破坏性工具 `stm32_console_flash_project_firmware`。后者只能烧录本工程生成的 `build/ai_serial_console.bin`，固件脚本会校验目标 UID，且必须传入 `ERASE_AND_FLASH_STM32F407`；仅应在用户明确授权后调用。

MCP 审计日志写入 `logs/mcp-audit.jsonl`，单文件上限 1 MiB、保留 5 个轮转文件。日志仅包含时间、工具/命令和成功状态，不记录串口返回全文。

服务测试命令：

```powershell
python .\mcp\test_mcp.py
```
