#!/usr/bin/env python3
import os
import subprocess
import sys
import time
import threading

toolspath = os.path.dirname(os.path.realpath(__file__))
try:
    sys.path.insert(0, os.path.join(toolspath, ".")) # Add uf2conv dir to search path
    import uf2conv # If this fails, we can't continue and will bomb below
except ImportError:
    sys.stderr.write("uf2conv not found next to this tool.\n")
    sys.exit(1)

scannerStop = threading.Event()
dropDead = False

class ScannerDarkly(threading.Thread):

    loopTime = 0.0 # Set to 0 for 1st pass to get immediate response for arduino-cli, then bumped to 2.0 for ongoing checks

    # https://stackoverflow.com/questions/12435211/threading-timer-repeat-function-every-n-seconds
    def __init__(self, event):
        threading.Thread.__init__(self)
        self.stopped = event

    def run(self):
        global dropDead
        boards = False;
        while not self.stopped.wait(self.loopTime):
            if self.stopped.is_set() or dropDead:
                return
            self.loopTime = 2.0
            l = uf2conv.get_drives()
            if (len(l) > 0) and not boards:
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
            elif (len(l) == 0) and boards:
                boards = False
                print("""{
          "eventType": "remove",
          "port": {
            "address": "UF2_Board",
            "protocol": "uf2conv"
          }
        }""", flush = True)


def main():
    global scannerStop
    global dropDead
    try:
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
                scannerStop.set()
                print("""{
  "eventType": "stop",
  "message": "OK"
}""", flush = True)
            elif cmd == "QUIT":
                scannerStop.set()
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
                thread = ScannerDarkly(scannerStop)
                thread.start()
    except:
          dropDead = True

main()
