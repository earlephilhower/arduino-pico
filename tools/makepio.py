#!/usr/bin/env python3

import os
import subprocess

PIOASM="system/pioasm/pioasm"

def recursivepioasm(path):
    for root, dirs, files in os.walk(path):
        for f in files:
            if f.endswith(".pio"):
                subprocess.run([PIOASM, "-o", "c-sdk", os.path.join(root, f), os.path.join(root, f) + ".h"])
                print(os.path.join(root, f))


def main():
    recursivepioasm("cores")
    recursivepioasm("libraries")

if __name__ == "__main__":
    main()
