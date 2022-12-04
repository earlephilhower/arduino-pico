#!/usr/bin/env python3
import os
import subprocess
import sys
import time
import threading


toolspath = os.path.dirname(os.path.realpath(__file__))
try:
    sys.path.insert(0, os.path.join(toolspath, ".")) # Add pyserial dir to search path
    import uf2conv # If this fails, we can't continue and will bomb below
except ImportError:
    sys.stderr.write("uf2conv not found next to this tool.\n")
    sys.exit(1)


scannerGo = False

def scanner():
    global scannerGo
    scannerGo = True
    boards = False
    while scannerGo:
        l = uf2conv.get_drives()
        if (len(l) > 0) and scannerGo and not boards:
            boards = True
            print ("""{
  "eventType": "add",
  "port": {
    "address": "UF2_Board",
    "label": "UF2 Board",
    "protocol": "uf2conv",
    "protocolLabel": "UF2 Devices",
    "properties": {
        "mac": "ffffffffffff",
        "pid" : "0x2e8a",
        "vid" : "0x000a"
    }
  }
}""", flush=True)
        elif (len(l) == 0) and scannerGo and boards:
            boards = False
            print("""{
  "eventType": "remove",
  "port": {
    "address": "UF2_Board",
    "protocol": "uf2conv"
  }
}""", flush = True)
        n = time.time() + 2
        while scannerGo and (time.time() < n):
            time.sleep(.1)
    scannerGo = True

def main():
    global scannerGo
    while True:
        cmdline = input()
        cmd = cmdline.split()[0]
        if cmd == "HELLO":
            print(""" {
  "eventType": "hello",
  "message": "OK",
  "protocolVersion": 1
}""", flush = True)
        elif cmd == "START":
            print("""{
  "eventType": "start",
  "message": "OK"
}""", flush = True);
        elif cmd == "STOP":
            scannerGo = False
            while not scannerGo:
                time.sleep(.1)
            print("""{
  "eventType": "stop",
  "message": "OK"
}""", flush = True)
        elif cmd == "QUIT":
            scannerGo = False
            print("""{
  "eventType": "quit",
  "message": "OK"
}""", flush = True)
            return
        elif cmd == "LIST":
            l = uf2conv.get_drives()
            if len(l) > 0:
                        print ("""{
  "eventType": "list",
  "ports": [
   {
    "address": "UF2_Board",
    "label": "UF2 Board",
    "protocol": "uf2conv",
    "protocolLabel": "UF2 Devices",
    "properties": {
        "mac": "ffffffffffff",
        "pid" : "0x2e8a",
        "vid" : "0x000a"
    }
   }
 ]
}""", flush=True)
            else:
                print ("""{
  "eventType": "list",
  "ports": [ ]
}""", flush=True)
        elif cmd == "START_SYNC":
            print("""{
  "eventType": "start_sync",
  "message": "OK"
}""", flush = True)
            scannerGo = True
            threading.Thread(target = scanner).start()
    time.sleep(.5)

main()
