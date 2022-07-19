#!/usr/bin/env python3
import sys
import struct
import subprocess
import re
import os
import os.path
import argparse
import time
import shutil

def main():
    parser = argparse.ArgumentParser(description='Version updater')
    parser.add_argument('-v', '--version', action='store', required=True, help='Version in X.Y.Z form')
    args = parser.parse_args()

    major, minor, sub = args.version.split(".")
    # Silly way to check for integer x.y.z
    major = int(major)
    minor = int(minor)
    sub = int(sub)

    # platform.txt
    with open("platform.txt", "r") as fin:
        with open("platform.txt.new", "w") as fout:
            for l in fin:
                if l.startswith("version="):
                    l = "version=" + str(args.version) + "\n"
                fout.write(l);
    shutil.move("platform.txt.new", "platform.txt")

    # package.json
    with open("package.json", "r") as fin:
        with open("package.json.new", "w") as fout:
            for l in fin:
                if l.startswith('  "version": '):
                    l = l.split(":")[0]
                    l = l + ': "1.' + str(major) + "{:02d}".format(minor) + "{:02d}".format(sub) + '.0",' + "\n"
                fout.write(l);
    shutil.move("package.json.new", "package.json")

    # cores/rp2040/RP2040Version.h
    with open("cores/rp2040/RP2040Version.h", "w") as fout:
        fout.write("#pragma once\n")
        fout.write("#define ARDUINO_PICO_MAJOR " + str(major) + "\n")
        fout.write("#define ARDUINO_PICO_MINOR " + str(minor) + "\n")
        fout.write("#define ARDUINO_PICO_REVISION " + str(sub) + "\n")
        fout.write('#define ARDUINO_PICO_VERSION_STR "' + str(args.version) + '"' + "\n")

    # docs/conf.py
    with open("docs/conf.py", "r") as fin:
        with open("docs/conf.py.new", "w") as fout:
            for l in fin:
                if l.startswith("version = "):
                    l = "version = u'" + str(args.version) + "'\n"
                if l.startswith("release = "):
                    l = "release = u'" + str(args.version) + "'\n"
                fout.write(l);
    shutil.move("docs/conf.py.new", "docs/conf.py")

main()
