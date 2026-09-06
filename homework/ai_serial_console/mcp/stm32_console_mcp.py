"""STM32F407 串口控制台的最小权限本地 MCP 服务。"""

import asyncio
from enum import Enum
import json
import logging
from logging.handlers import RotatingFileHandler
from pathlib import Path
import subprocess
import sys
from threading import Lock
import time
from typing import Any, Literal

from mcp.server.fastmcp import FastMCP
from mcp.types import ToolAnnotations
from pydantic import BaseModel, ConfigDict, Field
import serial

MCP_NAME = "stm32_console_mcp"
SERIAL_PORT = "COM15"
BAUDRATE = 115200
READ_TIMEOUT_SECONDS = 1.0
RESPONSE_LIMIT_BYTES = 512
PROJECT_ROOT = Path(__file__).resolve().parents[1]
FLASH_SCRIPT = PROJECT_ROOT / "tools" / "flash.py"
PROJECT_FIRMWARE = PROJECT_ROOT / "build" / "ai_serial_console.bin"
FLASH_APPROVAL = "ERASE_AND_FLASH_STM32F407"
LOG_PATH = PROJECT_ROOT / "logs" / "mcp-audit.jsonl"

# 串口设备一次只能处理一条请求；锁覆盖打开、发送、接收和关闭，避免并发 MCP 调用串扰。
SERIAL_LOCK = Lock()
ALLOWED_COMMANDS = frozenset({"ping", "info", "status", "led get", "led set on", "led set off"})

mcp = FastMCP(MCP_NAME)


def _create_audit_logger() -> logging.Logger:
    """建立仅写本地文件的轮转日志，stdio stdout 始终保留给 MCP JSON-RPC。"""

    logger = logging.getLogger(MCP_NAME)
    if logger.handlers:
        return logger
    LOG_PATH.parent.mkdir(parents=True, exist_ok=True)
    handler = RotatingFileHandler(LOG_PATH, maxBytes=1_000_000, backupCount=5, encoding="utf-8")
    handler.setFormatter(logging.Formatter("%(message)s"))
    logger.addHandler(handler)
    logger.setLevel(logging.INFO)
    logger.propagate = False
    return logger


AUDIT_LOGGER = _create_audit_logger()


class LedState(str, Enum):
    """允许写入开发板的唯一状态枚举。"""

    ON = "on"
    OFF = "off"


class ConsoleReply(BaseModel):
    """所有工具统一返回的结构化串口响应。"""

    model_config = ConfigDict(extra="forbid")

    ok: bool = Field(description="本机串口事务是否成功完成")
    command: str = Field(description="服务器实际发送的白名单命令")
    response: dict[str, Any] | None = Field(default=None, description="开发板返回的 JSON 对象")
    error: str | None = Field(default=None, description="失败时可操作的简短说明")


def _audit(event: str, command: str, success: bool) -> None:
    """写入最小审计记录；不记录完整串口内容，避免把未来敏感业务数据落盘。"""

    AUDIT_LOGGER.info(json.dumps({"timestamp": time.time(), "event": event, "command": command, "success": success}, ensure_ascii=False))


_PORT: serial.Serial | None = None


def _get_console() -> serial.Serial:
    """获取或初始化持久串口连接，设置 DTR/RTS 为低以保持 MCU 正常运行。"""
    global _PORT
    if _PORT is not None and _PORT.is_open:
        return _PORT
    port = serial.Serial(port=SERIAL_PORT, baudrate=BAUDRATE, timeout=READ_TIMEOUT_SECONDS, write_timeout=READ_TIMEOUT_SECONDS)
    port.dtr = False
    port.rts = False
    time.sleep(0.1)
    port.reset_input_buffer()
    _PORT = port
    return _PORT


def _close_console() -> None:
    """关闭当前串口连接（在烧录前或出错时释放端口）。"""
    global _PORT
    if _PORT is not None:
        try:
            if _PORT.is_open:
                _PORT.close()
        except Exception:
            pass
        _PORT = None


def _send_command(command: str) -> dict[str, Any]:
    """在单次受锁定的串口事务中发送固定命令，并验证一行 JSON 回复。"""

    if command not in ALLOWED_COMMANDS:
        raise ValueError("拒绝非白名单串口命令")

    with SERIAL_LOCK:
        try:
            port = _get_console()
            port.reset_input_buffer()
            port.write((command + "\n").encode("ascii"))
            port.flush()
            raw = port.readline(RESPONSE_LIMIT_BYTES)
        except (serial.SerialException, OSError):
            _close_console()
            raise

    if not raw:
        _close_console()
        raise TimeoutError("开发板未在 1 秒内响应；请检查 USB 线、COM15 和正常启动跳帽")
    try:
        decoded = raw.decode("ascii", errors="strict").strip()
        response = json.loads(decoded)
    except (UnicodeDecodeError, json.JSONDecodeError) as error:
        _close_console()
        raise ValueError("开发板返回的不是预期 JSON；请确认运行 ai-console-0.1.0 固件") from error
    if not isinstance(response, dict):
        raise ValueError("开发板 JSON 响应必须为对象")
    return response


async def _execute(command: str) -> ConsoleReply:
    """将阻塞式 pyserial 操作移出 MCP 事件循环，并以一致结构返回错误。"""

    try:
        response = await asyncio.to_thread(_send_command, command)
        _audit("console_command", command, True)
        return ConsoleReply(ok=True, command=command, response=response)
    except (OSError, ValueError, TimeoutError, serial.SerialException) as error:
        _audit("console_command", command, False)
        return ConsoleReply(ok=False, command=command, error=str(error))


def _flash_project_firmware() -> None:
    """调用固定烧录脚本；不接受路径、端口或命令参数，避免扩大为本机命令执行入口。"""

    if not FLASH_SCRIPT.is_file() or not PROJECT_FIRMWARE.is_file():
        raise FileNotFoundError("未找到已构建固件；请先在工程中完成构建")
    with SERIAL_LOCK:
        _close_console()
        time.sleep(0.2)
        result = subprocess.run(
            [sys.executable, str(FLASH_SCRIPT), "--confirm-target"],
            cwd=PROJECT_ROOT,
            stdin=subprocess.DEVNULL,
            capture_output=True,
            text=True,
            timeout=60,
            check=False,
        )
        time.sleep(0.5)
    if result.returncode != 0 or "Verification OK" not in (result.stdout + result.stderr):
        raise RuntimeError("烧录或回读校验失败；请检查 COM15、USB 线和 Bootloader 连接")


@mcp.tool(
    name="stm32_console_ping",
    title="测试 STM32 串口连通性",
    annotations=ToolAnnotations(readOnlyHint=True, destructiveHint=False, idempotentHint=True, openWorldHint=False),
)
async def stm32_console_ping() -> ConsoleReply:
    """发送 ping 并返回 pong，用于确认 COM15 和开发板固件都在线。"""

    return await _execute("ping")


@mcp.tool(
    name="stm32_console_info",
    title="读取 STM32 固件信息",
    annotations=ToolAnnotations(readOnlyHint=True, destructiveHint=False, idempotentHint=True, openWorldHint=False),
)
async def stm32_console_info() -> ConsoleReply:
    """读取芯片型号、固件版本和串口协议；不会改变开发板状态。"""

    return await _execute("info")


@mcp.tool(
    name="stm32_console_status",
    title="读取 STM32 运行状态",
    annotations=ToolAnnotations(readOnlyHint=True, destructiveHint=False, idempotentHint=True, openWorldHint=False),
)
async def stm32_console_status() -> ConsoleReply:
    """读取运行时间、最近复位原因和 LED 状态；不会改变开发板状态。"""

    return await _execute("status")


@mcp.tool(
    name="stm32_console_led_get",
    title="读取 STM32 板载 LED 状态",
    annotations=ToolAnnotations(readOnlyHint=True, destructiveHint=False, idempotentHint=True, openWorldHint=False),
)
async def stm32_console_led_get() -> ConsoleReply:
    """读取 PF9/PF10 板载 LED 的逻辑状态；不会改变开发板状态。"""

    return await _execute("led get")


@mcp.tool(
    name="stm32_console_led_set",
    title="设置 STM32 板载 LED 状态",
    annotations=ToolAnnotations(readOnlyHint=False, destructiveHint=False, idempotentHint=True, openWorldHint=False),
)
async def stm32_console_led_set(
    state: LedState = Field(description="目标 LED 状态，只允许 on 或 off"),
) -> ConsoleReply:
    """设置板载 LED 开或关；仅映射到固件已实现的两条精确白名单命令。"""

    return await _execute(f"led set {state.value}")


@mcp.tool(
    name="stm32_console_flash_project_firmware",
    title="烧录当前 STM32 工程固件",
    annotations=ToolAnnotations(readOnlyHint=False, destructiveHint=True, idempotentHint=True, openWorldHint=False),
)
async def stm32_console_flash_project_firmware(
    confirmation: Literal[FLASH_APPROVAL] = Field(
        description="必须精确填写 ERASE_AND_FLASH_STM32F407；调用会擦除本板 Flash，仅可在用户明确授权后使用"
    ),
) -> ConsoleReply:
    """擦除、烧录并回读校验当前工程的固定固件；不会接受其他文件、端口或任意下载命令。"""

    # Literal 已在 MCP 输入层拒绝其他值；保留变量以确保确认字段不会被优化为未使用参数。
    if confirmation != FLASH_APPROVAL:
        raise ValueError("缺少有效的擦除确认")
    try:
        await asyncio.to_thread(_flash_project_firmware)
        _audit("flash_project_firmware", PROJECT_FIRMWARE.name, True)
        return ConsoleReply(ok=True, command="flash_project_firmware", response={"firmware": PROJECT_FIRMWARE.name, "verified": True})
    except (FileNotFoundError, OSError, RuntimeError, subprocess.TimeoutExpired) as error:
        _audit("flash_project_firmware", PROJECT_FIRMWARE.name, False)
        return ConsoleReply(ok=False, command="flash_project_firmware", error=str(error))


if __name__ == "__main__":
    # stdio 是本机 MCP 协议通道；不要向 stdout 输出日志，以免破坏 JSON-RPC 帧。
    mcp.run(transport="stdio")
