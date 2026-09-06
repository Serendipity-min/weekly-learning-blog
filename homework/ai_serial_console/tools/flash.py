"""通过板载 CH340 自动进入 Bootloader 后烧录并校验固件。"""

import argparse
from importlib.metadata import PackageNotFoundError, version
from pathlib import Path
import re
import subprocess
import sys
import time

PROJECT = Path(__file__).resolve().parents[1]
FIRMWARE = PROJECT / "build" / "ai_serial_console.bin"
PORT = "COM15"
STM32LOADER_VERSION = "0.7.1"
EXPECTED_CHIP_ID = "0x413"
EXPECTED_UID = "0033-0054-324D5006-20313156"


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="识别已连接目标，或在明确确认后烧录固件。")
    action = parser.add_mutually_exclusive_group(required=True)
    action.add_argument("--probe", action="store_true", help="只读识别 COM15 上的芯片，不改写 Flash")
    action.add_argument("--confirm-target", action="store_true", help="确认目标 UID 后擦除、写入并校验 Flash")
    return parser.parse_args()


def check_uploader_version() -> None:
    try:
        installed = version("stm32loader")
    except PackageNotFoundError as error:
        raise RuntimeError("未安装 stm32loader；请执行 python -m pip install -r requirements.txt") from error

    if installed != STM32LOADER_VERSION:
        raise RuntimeError(f"stm32loader 版本应为 {STM32LOADER_VERSION}，当前为 {installed}；拒绝烧录以避免下载时序漂移")


def probe_target() -> None:
    # 先只读查询 ROM Bootloader，确认端口没有被重枚举到另一块开发板。
    command = [
        sys.executable, "-m", "stm32loader", "--port", PORT, "--family", "F4",
        "--reset-active-high", "--boot0-active-low", "--no-progress",
    ]
    result = subprocess.run(command, capture_output=True, text=True, check=False)
    output = result.stdout + result.stderr
    if result.returncode != 0:
        raise RuntimeError(f"无法读取 {PORT} 上的 Bootloader：\n{output}")

    chip_id = re.search(r"Chip id:\s*(0x[0-9A-Fa-f]+)", output)
    uid = re.search(r"Device UID:\s*([0-9A-Fa-f-]+)", output)
    if chip_id is None or uid is None:
        raise RuntimeError(f"Bootloader 输出缺少芯片标识，拒绝继续：\n{output}")
    if chip_id.group(1).lower() != EXPECTED_CHIP_ID or uid.group(1).upper() != EXPECTED_UID:
        raise RuntimeError(
            f"目标不匹配：检测到芯片 {chip_id.group(1)}、UID {uid.group(1)}；"
            f"期望芯片 {EXPECTED_CHIP_ID}、UID {EXPECTED_UID}。未改写 Flash。"
        )
    print(f"目标校验通过：{PORT}，芯片 {EXPECTED_CHIP_ID}，UID {EXPECTED_UID}")


def main() -> int:
    args = parse_args()
    check_uploader_version()

    if args.probe:
        probe_target()
        return 0

    if not FIRMWARE.is_file():
        print(f"未找到固件：{FIRMWARE}；请先运行 tools/build.py。", file=sys.stderr)
        return 1

    # 已在本机实测该板 CH340 的 DTR/RTS 极性：复位高有效、BOOT0 低有效。
    # 单次事务中擦除、写入并校验 Flash，同时校验芯片型号与 UID。
    command = [
        sys.executable, "-m", "stm32loader", "--port", PORT, "--family", "F4",
        "--reset-active-high", "--boot0-active-low", "--erase", "--write", "--verify",
        "--no-progress", str(FIRMWARE),
    ]
    print("+", " ".join(command))
    result = subprocess.run(command, stdin=subprocess.DEVNULL, capture_output=True, text=True, check=False)
    output = result.stdout + result.stderr
    print(output)

    if result.returncode != 0:
        raise RuntimeError(f"烧录过程失败：\n{output}")

    chip_id = re.search(r"Chip id:\s*(0x[0-9A-Fa-f]+)", output)
    uid = re.search(r"Device UID:\s*([0-9A-Fa-f-]+)", output)
    if chip_id is None or uid is None:
        raise RuntimeError(f"Bootloader 输出缺少芯片标识：\n{output}")
    if chip_id.group(1).lower() != EXPECTED_CHIP_ID or uid.group(1).upper() != EXPECTED_UID:
        raise RuntimeError(
            f"目标不匹配：检测到芯片 {chip_id.group(1)}、UID {uid.group(1)}；"
            f"期望芯片 {EXPECTED_CHIP_ID}、UID {EXPECTED_UID}。"
        )
    if "Verification OK" not in output:
        raise RuntimeError(f"Flash 回读校验未通过：\n{output}")

    print("烧录和校验完成；工具已将 BOOT0 恢复到正常启动状态。")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except RuntimeError as error:
        # 识别阶段失败时不会进入写入命令；统一返回非零状态而非泄露 Python 回溯。
        print(f"烧录失败：{error}", file=sys.stderr)
        raise SystemExit(1)
    except subprocess.CalledProcessError as error:
        print(f"烧录失败：{error}", file=sys.stderr)
        raise SystemExit(error.returncode)
