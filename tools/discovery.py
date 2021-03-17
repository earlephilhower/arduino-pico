#!/usr/bin/env python3
import time
import subprocess
import uf2conv

boards = False
while True:
    l = uf2conv.get_drives()
    if (len(l) > 0) and (not boards):
        print ("""
{
  "eventType": "add",
  "port": {
    "address": "1",
    "label": "Board",
    "boardName": "RPI 2040",
    "protocol": "uf2",
    "protocolLabel": "UF2 Devices",
    "prefs": {},
    "identificationPrefs": {}
  }
}""", flush=True)
        boards = True
    elif (len(l) == 0) and boards:
        print ("""
{
  "eventType": "remove",
  "port": {
    "address": "1",
    "label": "Board",
    "boardName": "RPI 2040",
    "protocol": "uf2",
    "protocolLabel": "UF2 Devices",
    "prefs": {},
    "identificationPrefs": {}
  }
}""", flush=True)
        boards = False
    time.sleep(1)
