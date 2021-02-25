# arduino-pico
Raspberry Pi Pico Arduino core, for all RP2040 boards

This is an under-development port of the RP2040 (Raspberry Pi Pico processor) to the Arduino ecosystem.

It uses a custom toolset with GCC 10.2 and Newlib 4.0.0, not depending on system-installed prerequisites.  https://github.com/earlephilhower/pico-quick-toolchain
A `package.json`file needs to be built to enable full integration, so for now some paths are just hardcoded.

Only delay, analogWrite, and digitalWrite are implemented now.  The biggest hurdle was getting a working build system outside of the pick-sdk CMake environment.
Presently, a manually build `pico.a` file is generated using objects compiled by `cmake` in the pico-examples repo, but in the future this will be scripted and automated.

There is automated discovery of boards in bootloader mode, so they show up in the IDE, and the upload command works using the Microsoft UF2 tool (included).
