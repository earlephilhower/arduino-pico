#!/usr/bin/env python3
# Burn Bootloader shim: write the iLabs Hearth firmware to the ESP32-C6
# co-processor of a Challenger board.
#
# The platform owns this shim and the library owns the flasher. The whole job
# here is to put three pure-Python dependencies on sys.path and hand off to
# libraries/iLabs_Hearth/fw/flash.py, so that the library never has to know
# anything about platform layout and the flasher stays the one implementation.

import argparse
import os
import os.path
import runpy
import sys

toolspath = os.path.dirname(os.path.realpath(__file__)).replace('\\', '/')
platformpath = os.path.dirname(toolspath)

ap = argparse.ArgumentParser(description="flash iLabs Hearth firmware to the ESP32-C6")
ap.add_argument("--variant", default="", help="wifi, thread or combined")
ap.add_argument("--port", default="", help="serial port of the board")
args, extra = ap.parse_known_args()

if args.variant not in ("wifi", "thread", "combined"):
    sys.stderr.write(
        "No Hearth firmware variant is selected, so there is nothing to burn.\n"
        "Pick one under Tools, ESP Wifi Type:\n"
        "  Hearth (Matter, WiFi)         the usual choice\n"
        "  Hearth (Matter, Thread)       if you have a Thread border router\n"
        "  Hearth (Matter, WiFi+Thread)  one image, either transport\n"
        "then run Burn Bootloader again.\n")
    sys.exit(1)

flasher = platformpath + "/libraries/iLabs_Hearth/fw/flash.py"
if not os.path.isfile(flasher):
    sys.stderr.write(
        "the iLabs_Hearth library is missing from " + flasher + "\n"
        "Run: git submodule update --init libraries/iLabs_Hearth\n")
    sys.exit(1)

# pyserial ships with this core already. esptool is the iLabs fork, which
# carries the RP2040 reset profile the stock tool has no idea about, and
# intelhex is imported at module scope by esptool.cmds. These go ahead of
# site-packages on purpose: a stock pip esptool cannot flash these boards and
# must not win the import.
for dep in ("pyserial", "esptool", "intelhex"):
    path = toolspath + "/" + dep
    if not os.path.isdir(path):
        sys.stderr.write(
            path + " is missing.\n"
            "Run: git submodule update --init tools/" + dep + "\n")
        sys.exit(1)
    sys.path.insert(0, path)

sys.argv = [flasher, "--variant", args.variant]
if args.port:
    sys.argv += ["--port", args.port]
sys.argv += extra
runpy.run_path(flasher, run_name="__main__")
