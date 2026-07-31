#!/usr/bin/env python3
"""
==============================================================================
AI AGENT SIMULATOR & DEBUGGING TOOL
==============================================================================
Provides automated headless inspection, UI state extraction, touch injection,
screenshot capture, and live Lua evaluation for AI coding agents and tests.
==============================================================================
"""

import sys
import os
import subprocess
import argparse

# Path to built desktop_linux binary
PROJECT_DIR = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
SIM_BIN = os.path.join(PROJECT_DIR, ".pio", "build", "desktop_linux", "program")

def ensure_built():
    """Ensure native Linux simulator binary is built."""
    if not os.path.exists(SIM_BIN):
        print("[AGENT SIM] Building native desktop simulator target...")
        res = subprocess.run(["pio", "run", "-e", "desktop_linux"], cwd=PROJECT_DIR, capture_output=True, text=True)
        if res.returncode != 0:
            print(f"[AGENT SIM ERROR] Build failed:\n{res.stderr}")
            sys.exit(1)

def run_simulator(args_list):
    """Run simulator binary with CLI arguments."""
    ensure_built()
    cmd = [SIM_BIN] + args_list
    res = subprocess.run(cmd, capture_output=True, text=True)
    return res.stdout, res.stderr, res.returncode

def main():
    parser = argparse.ArgumentParser(description="AI Agent ESP32 Simulator & Inspection CLI Tool")
    parser.add_argument("--dump-ui", action="store_true", help="Dump UI component tree & state as JSON")
    parser.add_argument("--screenshot", type=str, help="Capture framebuffer and save screenshot image (PNG or PPM)")
    parser.add_argument("--touch", nargs=2, type=int, metavar=("X", "Y"), help="Inject touch press event at canvas coordinates (X, Y)")
    parser.add_argument("--eval", type=str, help="Execute arbitrary Lua code inside runtime environment")
    parser.add_argument("--frames", type=int, default=10, help="Number of animation frames to run headlessly (default: 10)")
    parser.add_argument("--gui", action="store_true", help="Launch interactive GUI window mode instead of headless mode")

    args = parser.parse_args()

    cli_flags = []
    if not args.gui:
        cli_flags.append("--headless")

    cli_flags.extend(["--frames", str(args.frames)])

    if args.dump_ui:
        cli_flags.append("--dump-ui")

    ppm_temp_path = None
    target_screenshot = None

    if args.screenshot:
        target_screenshot = args.screenshot
        if target_screenshot.endswith(".png"):
            ppm_temp_path = target_screenshot + ".ppm"
            cli_flags.extend(["--screenshot", ppm_temp_path])
        else:
            cli_flags.extend(["--screenshot", target_screenshot])

    if args.touch:
        cli_flags.extend(["--touch", str(args.touch[0]), str(args.touch[1])])

    if args.eval:
        cli_flags.extend(["--eval", args.eval])

    stdout, stderr, retcode = run_simulator(cli_flags)

    if retcode != 0:
        print(f"[AGENT SIM ERROR] Execution failed:\n{stderr}")
        sys.exit(retcode)

    # Convert PPM to PNG if requested
    if ppm_temp_path and os.path.exists(ppm_temp_path):
        try:
            from PIL import Image
            img = Image.open(ppm_temp_path)
            img.save(target_screenshot)
            os.remove(ppm_temp_path)
            print(f"[AGENT SIM] Successfully saved PNG screenshot to {target_screenshot}")
        except Exception as e:
            print(f"[AGENT SIM WARNING] PIL conversion failed ({e}), kept PPM at {ppm_temp_path}")

    # Output stdout
    print(stdout)

if __name__ == "__main__":
    main()
