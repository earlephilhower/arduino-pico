#!/usr/bin/env python3
# Host side of the BLENotifyStress hardware test: subscribe to the counter, write the LED
# characteristic once a second, and require a steady, in-order notification stream while
# loop() keeps printing on serial.  Needs: pip install bleak pyserial
#
# usage: ble_notify_test.py [serial-port]     exit 0 = PASS, 1 = FAIL (hang/disconnect)
import asyncio, sys, threading, time
import serial
from bleak import BleakClient, BleakScanner

NAME = "PicoWnotify"
COUNTER = "c3feed72-b50c-400a-836c-c8981beb0b1c"
LED = "c3feed71-b50c-400a-836c-c8981beb0b1c"
SECONDS = 20
MIN_NOTES = 200  # The sketch sends ~100/s; a hang stops them dead

port = sys.argv[1] if len(sys.argv) > 1 else None
alive = []

def read_serial():
    s = serial.Serial(port, 115200, timeout=1)
    while True:
        if s.readline().startswith(b"Alive:"):
            alive.append(time.monotonic())

async def main():
    if port:
        threading.Thread(target=read_serial, daemon=True).start()
    # Match the advertised name: it may be truncated, and macOS may cache an old device name
    dev = await BleakScanner.find_device_by_filter(
        lambda d, adv: NAME.startswith(adv.local_name or "\0"), timeout=30)
    if not dev:
        print("FAIL: device not found")
        return 1
    notes = []
    async with BleakClient(dev) as c:
        await c.start_notify(COUNTER, lambda _, d: notes.append(int(d.decode())))
        start = time.monotonic()
        for i in range(SECONDS):
            await asyncio.sleep(1)
            await c.write_gatt_char(LED, bytes([i & 1]), response=True)
        await c.stop_notify(COUNTER)
    late = [t for t in alive if t > start + SECONDS - 3]
    print(f"notifications={len(notes)} first={notes[:3]} last={notes[-3:]} late_serial={len(late)}")
    ok = len(notes) >= MIN_NOTES and all(a < b for a, b in zip(notes, notes[1:]))
    if port:
        ok = ok and len(late) >= 2
    print("PASS" if ok else "FAIL")
    return 0 if ok else 1

try:
    sys.exit(asyncio.run(asyncio.wait_for(main(), SECONDS + 60)))
except Exception as e:
    print(f"FAIL: {type(e).__name__}: {e}")
    sys.exit(1)
