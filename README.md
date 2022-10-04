# Arduino-Pico
[![Release](https://img.shields.io/github/v/release/earlephilhower/arduino-pico?style=plastic)](https://github.com/earlephilhower/arduino-pico/releases)
[![Gitter](https://img.shields.io/gitter/room/earlephilhower/arduino-pico?style=plastic)](https://gitter.im/arduino-pico/community)

Raspberry Pi Pico Arduino core, for all RP2040 boards

This is a port of the RP2040 (Raspberry Pi Pico processor) to the Arduino ecosystem. It uses the bare Raspberry Pi Pico SDK and a custom GCC 10.3/Newlib 4.0 toolchain.

# Documentation
See https://arduino-pico.readthedocs.io/en/latest/ along with the examples for more detailed usage information.

# Supported Boards
* Raspberry Pi Pico
* Raspberry Pi Pico W
* Adafruit Feather RP2040
* Adafruit ItsyBitsy RP2040
* Adafruit KB2040
* Adafruit Macropad RP2040
* Adafruit QTPy RP2040
* Adafruit STEMMA Friend RP2040
* Adafruit Trinkey RP2040 QT
* Arduino Nano RP2040 Connect
* Cytron Maker Pi RP2040
* Cytron Maker Nano RP2040
* DeRuiLab FlyBoard2040 Core
* DFRobot Beetle RP2040
* ElectronicCats Hunter Cat NFC
* ExtremeElectronics RC2040
* Invector Labs Challenger RP2040 WiFi
* Invector Labs Challenger RP2040 WiFi/BLE
* Invector Labs Challenger NB RP2040 WiFi
* Invector Labs Challenger RP2040 LTE
* Invector Labs Challenger RP2040 LoRa
* Invector Labs Challenger RP2040 SubGHz
* Invector Labs Challenger RP2040 SD/RTC
* Invector Labs RPICO32
* Melopero Shake RP2040
* Seeed XIAO RP2040
* Solder Party RP2040 Stamp
* SparkFun ProMicro RP2040
* SparkFun Thing Plus RP2040
* uPesy RP2040 DevKit
* WIZnet W5100S-EVB-Pico
* WIZnet W5500-EVB-Pico
* WIZnet WizFi360-EVB-Pico
* Generic (configurable flash, I/O pins)

# Installing via Arduino Boards Manager
**Windows Users**: Please do not use the Windows Store version of the actual Arduino application
because it has issues detecting attached Pico boards.  Use the "Windows ZIP" or plain "Windows"
executable (EXE)  download direct from https://arduino.cc. and allow it to install any device
drivers it suggests.  Otherwise the Pico board may not be detected.  Also, if trying out the
2.0 beta Arduino please install the release 1.8 version beforehand to ensure needed device drivers
are present.  (See #20 for more details.)

Open up the Arduino IDE and go to File->Preferences.

In the dialog that pops up, enter the following URL in the "Additional Boards Manager URLs" field:

https://github.com/earlephilhower/arduino-pico/releases/download/global/package_rp2040_index.json

![image](https://user-images.githubusercontent.com/11875/111917251-3c57f400-8a3c-11eb-8120-810a8328ab3f.png)

Hit OK to close the dialog.

Go to Tools->Boards->Board Manager in the IDE

Type "pico" in the search box and select "Add":

![image](https://user-images.githubusercontent.com/11875/111917223-12063680-8a3c-11eb-8884-4f32b8f0feb1.png)

# Installing via GIT
To install via GIT (for latest and greatest versions):
````
mkdir -p ~/Arduino/hardware/pico
git clone https://github.com/earlephilhower/arduino-pico.git ~/Arduino/hardware/pico/rp2040
cd ~/Arduino/hardware/pico/rp2040
git submodule update --init
cd pico-sdk
git submodule update --init
cd ../tools
python3 ./get.py
`````

# Installing both Arduino and CMake
Tom's Hardware presented a very nice writeup on installing `arduino-pico` on both Windows and Linux, available at https://www.tomshardware.com/how-to/program-raspberry-pi-pico-with-arduino-ide

If you follow Les' step-by-step you will also have a fully functional `CMake`-based environment to build Pico apps on if you outgrow the Arduino ecosystem.

# Uploading Sketches
To upload your first sketch, you will need to hold the BOOTSEL button down while plugging in the Pico to your computer.
Then hit the upload button and the sketch should be transferred and start to run.

After the first upload, this should not be necessary as the `arduino-pico` core has auto-reset support.
Select the appropriate serial port shown in the Arduino Tools->Port->Serial Port menu once (this setting will stick and does not need to be
touched for multiple uploads).   This selection allows the auto-reset tool to identify the proper device to reset.
Them hit the upload button and your sketch should upload and run.

In some cases the Pico will encounter a hard hang and its USB port will not respond to the auto-reset request.  Should this happen, just
follow the initial procedure of holding the BOOTSEL button down while plugging in the Pico to enter the ROM bootloader.

# Uploading Filesystem Images
The onboard flash filesystem for the Pico, LittleFS, lets you upload a filesystem image from the sketch directory for your sketch to use.  Download the needed plugin from
* https://github.com/earlephilhower/arduino-pico-littlefs-plugin/releases

To install, follow the directions in
* https://github.com/earlephilhower/arduino-pico-littlefs-plugin/blob/master/README.md

For detailed usage information, please check the ESP8266 repo documentation (ignore SPIFFS related notes) available at
* https://arduino-esp8266.readthedocs.io/en/latest/filesystem.html

# Uploading Sketches with Picoprobe
If you have built a Raspberry Pi Picoprobe, you can use OpenOCD to handle your sketch uploads and for debugging with GDB.

Under Windows a local admin user should be able to access the Picoprobe port automatically, but under Linux `udev` must be told about the device and to allow normal users access.

To set up user-level access to Picoprobes on Ubuntu (and other OSes which use `udev`):
````
echo 'SUBSYSTEMS=="usb", ATTRS{idVendor}=="2e8a", ATTRS{idProduct}=="0004", GROUP="users", MODE="0666"' | sudo tee -a /etc/udev/rules.d/98-PicoProbe.rules
sudo udevadm control --reload
````

The first line creates a file with the USB vendor and ID of the Picoprobe and tells UDEV to give users full access to it.  The second causes `udev` to load this new rule.  Note that you will need to unplug and re-plug in your device the first time you create this file, to allow udev to make the device node properly.

Once Picoprobe permissions are set up properly, then select the board "Raspberry Pi Pico (Picoprobe)" in the Tools menu and upload as normal.

# Uploading Sketches with pico-debug
[pico-debug](https://github.com/majbthrd/pico-debug/) differs from Picoprobe in that pico-debug is a virtual debug pod that runs side-by-side on the same RP2040 that you run your code on; so, you only need one RP2040 board instead of two.  pico-debug also differs from Picoprobe in that pico-debug is standards-based; it uses the CMSIS-DAP protocol, which means even software not specially written for the Raspberry Pi Pico can support it.  pico-debug uses OpenOCD to handle your sketch uploads, and debugging can be accomplished with CMSIS-DAP capable debuggers including GDB.

Under Windows and macOS, any user should be able to access pico-debug automatically, but under Linux `udev` must be told about the device and to allow normal users access.

To set up user-level access to all CMSIS-DAP adapters on Ubuntu (and other OSes which use `udev`):
````
echo 'ATTRS{product}=="*CMSIS-DAP*", MODE="664", GROUP="plugdev"' | sudo tee -a /etc/udev/rules.d/98-CMSIS-DAP.rules
sudo udevadm control --reload
````

The first line creates a file that recognizes all CMSIS-DAP adapters and tells UDEV to give users full access to it.  The second causes `udev` to load this new rule.  Note that you will need to unplug and re-plug in your device the first time you create this file, to allow udev to make the device node properly.

Once CMSIS-DAP permissions are set up properly, then select the board "Raspberry Pi Pico (pico-debug)" in the Tools menu.

When first connecting the USB port to your PC, you must copy [pico-debug-gimmecache.uf2](https://github.com/majbthrd/pico-debug/releases/) to the Pi Pico to load pico-debug into RAM; after this, upload as normal.

# Debugging with Picoprobe/pico-debug, OpenOCD, and GDB
The installed tools include a version of OpenOCD (in the pqt-openocd directory) and GDB (in the pqt-gcc directory).  These may be used to run GDB in an interactive window as documented in the Pico Getting Started manuals from the Raspberry Pi Foundation.  For [pico-debug](https://github.com/majbthrd/pico-debug/), replace the raspberrypi-swd and picoprobe example OpenOCD arguments of "-f interface/raspberrypi-swd.cfg -f target/rp2040.cfg" or "-f interface/picoprobe.cfg -f target/rp2040.cfg" respectively in the Pico Getting Started manual with "-f board/pico-debug.cfg".

# Features
* Adafruit TinyUSB Arduino (USB mouse, keyboard, flash drive, generic HID, CDC Serial, MIDI, WebUSB, others)
* Generic Arduino USB Serial, Keyboard, and Mouse emulation
* WiFi (Pico W)
* HTTP client and server (WebServer)
* SSL/TLS/HTTPS
* Over-the-Air (OTA) upgrades
* Filesystems (LittleFS and SD/SDFS)
* Multicore support (setup1() and loop1())
* FreeRTOS SMP support
* Overclocking and underclocking from the menus
* digitalWrite/Read, shiftIn/Out, tone, analogWrite(PWM)/Read, temperature
* Peripherals:  SPI master, Wire(I2C) master/slave, dual UART, emulated EEPROM, I2S audio input, I2S audio output, Servo
* printf (i.e. debug) output over USB serial

The RP2040 PIO state machines (SMs) are used to generate jitter-free:
* Servos
* Tones
* I2S Input
* I2S Output
* Software UARTs (Serial ports)

# Tutorials from Across the Web
Here are some links to coverage and additional tutorials for using `arduino-pico`
* The File:: class is taken from the ESP8266.  See https://arduino-esp8266.readthedocs.io/en/latest/filesystem.html
* Arduino Support for the Pi Pico available! And how fast is the Pico? - https://youtu.be/-XHh17cuH5E
* Pre-release Adafruit QT Py RP2040 - https://www.youtube.com/watch?v=sfC1msqXX0I
* Adafruit Feather RP2040 running LCD + TMP117 - https://www.youtube.com/watch?v=fKDeqZiIwHg
* Demonstration of Servos and I2C in Korean - https://cafe.naver.com/arduinoshield/1201
* Home Assistant Pico W integration starter project using Arduino - https://github.com/daniloc/PicoW_HomeAssistant_Starter

# Contributing
If you want to contribute or have bugfixes, drop me a note at <earlephilhower@yahoo.com> or open an issue/PR here.

# Licensing and Credits
* The [Arduino IDE and ArduinoCore-API](https://arduino.cc) are developed and maintained by the Arduino team. The IDE is licensed under GPL.
* The [RP2040 GCC-based toolchain](https://github.com/earlephilhower/pico-quick-toolchain) is licensed under under the GPL.
* The [Pico-SDK](https://github.com/raspberrypi/pico-sdk) is by Raspberry Pi (Trading) Ltd and licensed under the BSD 3-Clause license.
* [Arduino-Pico](https://github.com/earlephilhower/arduino-pico) core files are licensed under the LGPL.
* [LittleFS](https://github.com/ARMmbed/littlefs) library written by ARM Limited and released under the [BSD 3-clause license](https://github.com/ARMmbed/littlefs/blob/master/LICENSE.md).
* [UF2CONV.PY](https://github.com/microsoft/uf2) is by Microsoft Corporation and licensed under the MIT license.
* Networking and filesystem code taken from the [ESP8266 Arduino Core](https://github.com/esp8266/Arduino) and licensed under the LGPL.
* DHCP server for AP host mode from the [Micropython Project](https://micropython.org), distributed under the MIT License.
* [FreeRTOS](https://freertos.org) is Copyright Amazon.com, Inc. or its affiliates, and distributed under the MIT license.
* [lwIP](https://savannah.nongnu.org/projects/lwip/) is (c) the Swedish Institute of Computer Science and licenced under the BSD license.
* [BearSSL](https://bearssl.org) library written by Thomas Pornin, is distributed under the [MIT License](https://bearssl.org/#legal-details).
* [UZLib](https://github.com/pfalcon/uzlib) is copyright (c) 2003 Joergen Ibsen and distributed under the zlib license.
* [LEAmDNS](https://github.com/LaborEtArs/ESP8266mDNS) is copyright multiple authors and distributed under the MIT license.
* [http-parser](https://github.com/nodejs/http-parser) is copyright Joyent, Inc. and other Node contributors.
* WebServer code modified from the [ESP32 WebServer](https://github.com/espressif/arduino-esp32/tree/master/libraries/WebServer) and is copyright (c) 2015 Ivan Grokhotkov and others


-Earle F. Philhower, III  
 earlephilhower@yahoo.com
