# STM32 Console MCP 安全审查记录

审查日期：2026-08-25  
范围：`E:\stm32\ai_serial_console` 的固件、串口 MCP 服务、烧录脚本和依赖清单。

## 自动化结果

| 检查 | 结果 |
| --- | --- |
| TruffleHog 密钥扫描 | 未发现已验证或未验证的密钥 |
| pip-audit（直接依赖） | `stm32loader 0.7.1`、`mcp 1.28.1`、`pyserial 3.5` 未发现已知漏洞 |
| Semgrep `p/secrets` | 0 项发现 |
| Semgrep `p/security-audit` | 0 项发现 |
| Semgrep `p/python` | 0 项发现 |
| Semgrep `p/c` | 0 项发现 |

合并后的 SARIF 位于 `results/results.sarif`，共 0 项发现。

## 覆盖限制

第三方规则仓库（0xdea、Trail of Bits、elttam）已下载，但与本机 Semgrep 1.169.0 的规则语法不兼容，扫描退出码为 7；它们**不计入通过结果**。缓存仍保留在 `repos/`，以便后续升级规则或 Semgrep 后复跑。

`pip-audit` 仅对项目直接依赖执行，尚未生成带哈希的锁定依赖树。

## MCP 人工审查

1. 服务仅使用 stdio，不监听网络端口，也没有 HTTP、文件路径、URL 或任意 shell 输入接口。
2. 串口命令是固定白名单；LED 状态仅能是 `on` 或 `off`；返回内容限制为 512 字节。
3. 烧录工具只能调用项目内固定固件与固定 `COM15`，不能指定任意文件、端口或命令；同时校验 STM32 UID，写入前需要精确确认值 `ERASE_AND_FLASH_STM32F407`。
4. 审计日志只保存事件、命令名和成功状态，不保存串口正文或凭据；单文件达到 1 MiB 后轮转，保留 5 份备份。

## 剩余风险与操作边界

串口和本机 MCP 不是强认证边界：能够以当前 Windows 用户身份修改项目文件、替换 Python 环境或直接占用 COM15 的本地进程，仍可能影响设备。因此烧录必须继续由操作中的 AI 在调用前取得用户当次明确授权；不得把确认字符串视作身份认证。

本轮没有确认的高危漏洞。建议在引入网络、蓝牙控制、文件上传或任意串口命令前重新审查。
