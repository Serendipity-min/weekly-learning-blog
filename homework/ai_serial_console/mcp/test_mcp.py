"""以 MCP stdio 客户端验证服务注册、模式协商和真实 COM15 工具调用。"""

import asyncio
from pathlib import Path
import sys

from mcp import ClientSession
from mcp.client.stdio import StdioServerParameters, stdio_client

SERVER = Path(__file__).with_name("stm32_console_mcp.py")
EXPECTED_TOOLS = {
    "stm32_console_ping",
    "stm32_console_info",
    "stm32_console_status",
    "stm32_console_led_get",
    "stm32_console_led_set",
    "stm32_console_flash_project_firmware",
}


async def main() -> None:
    """启动独立服务进程，确保测试与 Codex 的未来注册路径一致。"""

    parameters = StdioServerParameters(command=sys.executable, args=[str(SERVER)])
    async with stdio_client(parameters) as (reader, writer):
        async with ClientSession(reader, writer) as session:
            await session.initialize()
            tools = await session.list_tools()
            actual_tools = {tool.name for tool in tools.tools}
            if actual_tools != EXPECTED_TOOLS:
                raise RuntimeError(f"MCP 工具集不匹配：{actual_tools}")

            # 用错误确认值验证 MCP 架构层会拒绝擦写请求；此调用不会进入烧录函数。
            rejected_flash = await session.call_tool(
                "stm32_console_flash_project_firmware", {"confirmation": "not-authorized"}
            )
            if not rejected_flash.isError:
                raise RuntimeError("烧录工具未拒绝错误确认值")

            for name, arguments in (
                ("stm32_console_ping", {}),
                ("stm32_console_info", {}),
                ("stm32_console_status", {}),
                ("stm32_console_led_get", {}),
                # 写入测试必须成对执行，最后恢复 LED 为关闭状态，避免测试遗留外部状态。
                ("stm32_console_led_set", {"state": "on"}),
                ("stm32_console_led_get", {}),
                ("stm32_console_led_set", {"state": "off"}),
            ):
                result = await session.call_tool(name, arguments)
                if result.isError:
                    raise RuntimeError(f"{name} 调用失败：{result.content}")
                print(f"{name}: {result.structuredContent}")


if __name__ == "__main__":
    asyncio.run(main())
