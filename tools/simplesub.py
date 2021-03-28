#!/usr/bin/env python3
import sys
import struct
import subprocess
import re
import os
import os.path
import argparse
import time

def main():
    parser = argparse.ArgumentParser(description='Simple text substitution')
    parser.add_argument('-i', '--input', action='store', required=True, help='Path to the source file')
    parser.add_argument('-o', '--out', action='store', required=True, help='Path to the output file')
    parser.add_argument('-s', '--sub', action='append', nargs=2, metavar=('find', 'replace'), required=True, help='Substition')
    args = parser.parse_args()

    with open(args.input, "r") as fin:
        data = fin.read()

    for f, r in args.sub:
        data = re.sub(f, r, data)

    with open(args.out, "w") as fout:
        fout.write(data)


main()
