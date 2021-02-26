# arduino-pico
Raspberry Pi Pico Arduino core, for all RP2040 boards

This is an under-development port of the RP2040 (Raspberry Pi Pico processor) to the Arduino ecosystem.

It uses a custom toolset with GCC 10.2 and Newlib 4.0.0, not depending on system-installed prerequisites.  https://github.com/earlephilhower/pico-quick-toolchain

Only delay, analogWrite, and digitalWrite are implemented now.  The biggest hurdle was getting a working build system outside of the pick-sdk CMake environment.
Presently, a manually build `pico.a` file is generated using objects compiled by `cmake` in the pico-examples repo, but in the future this will be scripted and automated.

There is automated discovery of boards in bootloader mode, so they show up in the IDE, and the upload command works using the Microsoft UF2 tool (included).

To install:
````
mkdir -p ~/Arduino/hardware/pico
git clone https://github.com/earlephilhower/arduino-pico.git ~/Arduino/hardware/pico/rp2040
cd ~/Arduino/hardware/pico/rp2040
git submodule init
git submodule update
cd pico-sdk
git submodule init
git submodule update
cd ../system
python3 ./get.py
`````

