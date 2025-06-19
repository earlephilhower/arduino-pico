# Arduino-Pico
[![Release](https://img.shields.io/github/v/release/earlephilhower/arduino-pico?style=plastic)](https://github.com/earlephilhower/arduino-pico/releases)
[![Gitter](https://img.shields.io/gitter/room/earlephilhower/arduino-pico?style=plastic)](https://gitter.im/arduino-pico/community)

Raspberry Pi Pico Arduino core, for all RP2040 and RP2350 boards

This is a port of Arduino to the RP2040 (Raspberry Pi Pico processor) and RP2350 (Raspberry Pi Pico 2 processor). It uses the bare Raspberry Pi Pico SDK and a custom GCC 14.2/Newlib 4.3 toolchain and supports ARM and RISC-V cores.

# Documentation
See https://arduino-pico.readthedocs.io/en/latest/ along with the examples for more detailed usage information.

# Contributing
Read the [Contributing Guide](https://github.com/earlephilhower/arduino-pico/blob/master/docs/contrib.rst) for more information on submitting pull requests and porting libraries or sketches to this core.

# Supported Boards
* Raspberry Pi Pico
* Raspberry Pi Pico W
* Raspberry Pi Pico 2
* Raspberry Pi Pico 2W
* 0xCB Helios
* Adafruit Feather RP2040
* Adafruit Feather RP2040 SCORPIO
* Adafruit Floppsy RP2040
* Adafruit ItsyBitsy RP2040
* Adafruit KB2040
* Adafruit Macropad RP2040
* Adafruit Metro RP2040
* Adafruit Metro RP2350
* Adafruit QTPy RP2040
* Adafruit STEMMA Friend RP2040
* Adafruit Trinkey RP2040 QT
* Amken Bunny 
* Amken Revelop 
* Amken Revelop Plus 
* Amken Revelop eS
* Architeuthis Flux Jumperless
* Architeuthis Flux Jumperless V5
* Arduino Nano RP2040 Connect
* ArtronShop RP2 Nano
* BIGTREETECH SKR-Pico
* Breadstick Raspberry
* BridgeTek IDM2040-7A
* BridgeTek IDM2040-43A
* Cytron IRIV IO Controller
* Cytron Maker Pi RP2040
* Cytron Maker Nano RP2040
* Cytron Maker Uno RP2040
* Cytron Motion 2350 Pro
* Datanoise PicoADK v1
* Datanoise PicoADK v2 (RP2350)
* Degz Suibo RP2040
* DeRuiLab FlyBoard2040 Core
* DFRobot Beetle RP2040
* ElectronicCats Hunter Cat NFC
* EVN Alpha
* ExtremeElectronics RC2040
* GroundStudio Marble Pico
* Invector Labs Challenger RP2040 WiFi
* Invector Labs Challenger RP2040 WiFi/BLE
* Invector Labs Challenger RP2040 WiFi6/BLE
* Invector Labs Challenger NB RP2040 WiFi
* Invector Labs Challenger RP2040 LTE
* Invector Labs Challenger RP2040 LoRa
* Invector Labs Challenger RP2040 SubGHz
* Invector Labs Challenger RP2040 SD/RTC
* Invector Labs Challenger RP2040 UWB
* Invector Labs Challenger RP2350 BConnect
* Invector Labs Challenger RP2350 WiFi/BLE
* Invector Labs RPICO32
* Melopero Cookie RP2040
* Melopero Shake RP2040
* METE HOCA Akana R1
* Makerbase MKSTHR36
* Makerbase MKSTHR42
* MyMakers RP2040
* Neko Systems BL2040 Mini
* Newsan Archi
* nullbits Bit-C PRO
* Olimex Pico2XL
* Olimex Pico2XXL
* Olimex RP2040-Pico30
* Pimoroni PGA2040
* Pimoroni Pico Plus 2
* Pimoroni Pico Plus 2W
* Pimoroni Plasma2040
* Pimoroni Plasma2350
* Pimoroni Servo2040
* Pimoroni Tiny2040
* Pimoroni Tiny2350
* Pintronix PinMax
* RAKwireless RAK11300
* Redscorp RP2040-Eins
* Redscorp RP2040-ProMini
* Sea-Picro
* Seeed Indicator RP2040
* Seeed XIAO RP2040
* Seeed XIAO RP2350
* Silicognition RP2040-Shim
* Solder Party RP2040 Stamp
* Solder Party RP2350 Stamp
* Solder Party RP2350 Stamp XL
* SparkFun IoT RedBoard RP2350
* SparkFun MicroMod RP2040
* SparkFun ProMicro RP2040
* SparkFun ProMicro RP2350
* SparkFun Thing Plus RP2040
* SparkFun Thing Plus RP2350
* SparkFun XRP Controller
* uPesy RP2040 DevKit
* VCC-GND YD-RP2040
* Viyalab Mizu RP2040
* Waveshare RP2040 Zero
* Waveshare RP2040 One
* Waveshare RP2040 Plus
* Waveshare RP2040 LCD 0.96
* Waveshare RP2040 LCD 1.28
* Waveshare RP2040 Matrix
* Waveshare RP2040 PiZero
* Waveshare RP2350 Plus
* Waveshare RP2350 LCD 0.96
* WIZnet W5100S-EVB-Pico
* WIZnet W5100S-EVB-Pico2
* WIZnet W5500-EVB-Pico
* WIZnet W5500-EVB-Pico2
* WIZnet W55RP20-EVB-Pico
* WIZnet W6300-EVB-Pico
* WIZnet W6300-EVB-Pico2
* WIZnet WizFi360-EVB-Pico
* Generic RP2040 (configurable flash, I/O pins)
* Generic RP2350 (configurable flash, I/O pins)

# Features
* Adafruit TinyUSB Arduino (USB mouse, keyboard, flash drive, generic HID, CDC Serial, MIDI, WebUSB, others)
* Bluetooth on the PicoW (Classic and BLE) with Keyboard, Mouse, Joystick, and Virtual Serial
* Bluetooth Classic and BLE HID master mode (connect to BT keyboard, mouse, or joystick)
* Generic Arduino USB Serial, Keyboard, Joystick, and Mouse emulation
* WiFi (Pico W, ESP32-based ESPHost, Atmel WINC1500)
* Ethernet (Wired WizNet W6300, WizNet W6100, WizNet W5500, WizNet W5100, ENC28J60)
* HTTP client and server (WebServer)
* SSL/TLS/HTTPS
* Over-the-Air (OTA) upgrades
* Filesystems (LittleFS and SD/SDFS)
* Multicore support (setup1() and loop1())
* FreeRTOS SMP support
* Overclocking and underclocking from the menus
* digitalWrite/Read, shiftIn/Out, tone, analogWrite(PWM)/Read, temperature
* Analog stereo audio in using DMA and the built-in ADC
* Analog stereo audio out using PWM hardware
* Bluetooth A2DP audio source (output) and sink (input) on the PicoW
* USB drive mode for data loggers (SingleFileDrive, FatFSUSB)
* Peripherals:  SPI master/slave, Wire(I2C) master/slave, dual UART, emulated EEPROM, I2S audio input/output, Servo
* printf (i.e. debug) output over USB serial
* Transparent use of PSRAM globals and heap (RP2350 only)
* ARM or RISC-V (Hazard3) support for the RP2350
* Semihosted serial and file system access
* GPROF profiling support

The RP2040 PIO state machines (SMs) are used to generate jitter-free:
* Servos
* Tones
* I2S Input
* I2S Output
* Software UARTs (Serial ports)
* Software SPIs

# Installing via Arduino Boards Manager
## Windows-specific Notes
Please do not use the Windows Store version of the actual Arduino application
because it has issues detecting attached Pico boards.  Use the "Windows ZIP" or plain "Windows"
executable (EXE)  download direct from https://arduino.cc. and allow it to install any device
drivers it suggests.  Otherwise the Pico board may not be detected.  Also, if trying out the
2.0 beta Arduino please install the release 1.8 version beforehand to ensure needed device drivers
are present.  (See #20 for more details.)

## Linux-specific Notes
Installing Arduino using flatpak (often used by "App Stores" in various Linux
distributions) will mean it has restricted access to the host. This might cause uploads to fail
with error messages such as the following:

```
Scanning for RP2040 devices
...
No drive to deploy.
```

If you encounter this, you will need to either install Arduino in a different manner, or override
the flatpak sandboxing feature using the following command, then restarting Arduino.

```
flatpak override --user --filesystem=host:ro cc.arduino.IDE2
```

## Installation
Open up the Arduino IDE and go to File->Preferences.

In the dialog that pops up, enter the following URL in the "Additional Boards Manager URLs" field:

https://github.com/earlephilhower/arduino-pico/releases/download/global/package_rp2040_index.json

![image](https://user-images.githubusercontent.com/11875/111917251-3c57f400-8a3c-11eb-8120-810a8328ab3f.png)

Hit OK to close the dialog.

Go to Tools->Boards->Board Manager in the IDE

Type "pico" in the search box and select "Add":

![image](https://user-images.githubusercontent.com/11875/111917223-12063680-8a3c-11eb-8884-4f32b8f0feb1.png)

# Installing via GIT

**Windows Users:**  Before installing via `git` on Windows, please read and follow the directions in
[this link](https://arduino-pico.readthedocs.io/en/latest/platformio.html#important-steps-for-windows-users-before-installing).
If Win32 long paths are not enabled, and `git` not configured to use them then there
may be errors when attempting to clone the submodules.

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
* https://arduino-pico.readthedocs.io/en/latest/fs.html

# Uploading Sketches with Picoprobe/Debugprobe
If you have built a Raspberry Pi Picoprobe, you can use OpenOCD to handle your sketch uploads and for debugging with GDB.

Under Windows a local admin user should be able to access the Picoprobe port automatically, but under Linux `udev` must be told about the device and to allow normal users access.

To set up user-level access to Picoprobes on Ubuntu (and other OSes which use `udev`):
````
echo 'SUBSYSTEM=="usb", ATTRS{idVendor}=="2e8a", ATTRS{idProduct}=="0004", MODE="660", GROUP-"plugdev"' | sudo tee -a /etc/udev/rules.d/98-PicoProbe.rules
echo 'SUBSYSTEM=="usb", ATTRS{idVendor}=="2e8a", ATTRS{idProduct}=="000a", MODE="660", GROUP="plugdev"' | sudo tee -a /etc/udev/rules.d/98-PicoProbe.rules
echo 'SUBSYSTEM=="usb", ATTRS{idVendor}=="2e8a", ATTRS{idProduct}=="000f", MODE="660", GROUP="plugdev"' | sudo tee -a /etc/udev/rules.d/98-PicoProbe.rules
sudo udevadm control --reload
sudo udevadm trigger -w -s usb
````

The first line creates a device file in `/dev` matching the USB vendor and product ID of the Picoprobe, and it enables global read+write permissions. The second line causes `udev` to load this new rule. The third line requests the kernel generate "device change" events that will cause our new `udev` rule to run. 

If for some reason the device file does not appear, manually unplug and re-plug the USB connection and check again. The output from `dmesg` can reveal useful diagnostics if the device file remains absent.

Once Picoprobe permissions are set up properly, then select the board "Raspberry Pi Pico (Picoprobe)" in the Tools menu and upload as normal.

# Debugging with Picoprobe, OpenOCD, and GDB
The installed tools include a version of OpenOCD (in the pqt-openocd directory) and GDB (in the pqt-gcc directory).  These may be used to run GDB in an interactive window as documented in the Pico Getting Started manuals from the Raspberry Pi Foundation.  Use the command line `./system/openocd/bin/openocd -f ./lib/rp2040/picoprobe_cmsis_dap.tcl` or `./system/openocd/bin/openocd -f ./lib/rp2350/picoprobe_cmsis_dap.tcl` from the `git` installation directory.

# Licensing and Credits
* The [Arduino IDE and ArduinoCore-API](https://arduino.cc) are developed and maintained by the Arduino team. The IDE is licensed under GPL.
* The [RP2040 GCC-based toolchain](https://github.com/earlephilhower/pico-quick-toolchain) is licensed under under the GPL.
* The [Pico-SDK](https://github.com/raspberrypi/pico-sdk) is by Raspberry Pi (Trading) Ltd and licensed under the BSD 3-Clause license.
* [Arduino-Pico](https://github.com/earlephilhower/arduino-pico) core files are licensed under the LGPL.
* [LittleFS](https://github.com/ARMmbed/littlefs) library written by ARM Limited and released under the [BSD 3-clause license](https://github.com/ARMmbed/littlefs/blob/master/LICENSE.md).
* [UF2CONV.PY](https://github.com/microsoft/uf2) is by Microsoft Corporation and licensed under the MIT license.
* Networking and filesystem code taken from the [ESP8266 Arduino Core](https://github.com/esp8266/Arduino) and licensed under the LGPL.
* DHCP server for AP host mode from the [Micropython Project](https://micropython.org), distributed under the MIT License.
* [FreeRTOS](https://freertos.org) is copyright Amazon.com, Inc. or its affiliates, and distributed under the MIT license.
* [lwIP](https://savannah.nongnu.org/projects/lwip/) is (c) the Swedish Institute of Computer Science and licenced under the BSD license.
* [BearSSL](https://bearssl.org) library written by Thomas Pornin, is distributed under the [MIT License](https://bearssl.org/#legal-details).
* [UZLib](https://github.com/pfalcon/uzlib) is copyright (c) 2003 Joergen Ibsen and distributed under the zlib license.
* [LEAmDNS](https://github.com/LaborEtArs/ESP8266mDNS) is copyright multiple authors and distributed under the MIT license.
* [http-parser](https://github.com/nodejs/http-parser) is copyright Joyent, Inc. and other Node contributors.
* WebServer code modified from the [ESP32 WebServer](https://github.com/espressif/arduino-esp32/tree/master/libraries/WebServer) and is copyright (c) 2015 Ivan Grokhotkov and others.
* [Xoshiro-cpp](https://github.com/Reputeless/Xoshiro-cpp) is copyright (c) 2020 Ryo Suzuki and distributed under the MIT license.
* [FatFS low-level filesystem](http://elm-chan.org/fsw/ff/) code is Copyright (C) 2024, ChaN, all rights reserved.
* [TLSF memory manager for PSRAM from Espressif fork](https://github.com/espressif/tlsf) of [original](https://github.com/mattconte/tlsf) by Matthew Conte is copyright Matthew Conte and licensed under the MIT license.
* [ESPHost library](https://github.com/Networking-for-Arduino/ESPHost) is LGPL licensed by its maintainers.

-Earle F. Philhower, III  
 earlephilhower@yahoo.com
