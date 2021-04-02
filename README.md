# Arduino-Pico [![Join the chat at https://gitter.im/arduino-pico/community](https://badges.gitter.im/arduino-pico/community.svg)](https://gitter.im/arduino-pico/community?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)


Raspberry Pi Pico Arduino core, for all RP2040 boards

This is a port of the RP2040 (Raspberry Pi Pico processor) to the Arduino ecosystem.

It uses a custom toolset with GCC 10.2 and Newlib 4.0.0, not depending on system-installed prerequisites.  https://github.com/earlephilhower/pico-quick-toolchain

There is automated discovery of boards in bootloader mode, so they show up in the IDE, and the upload command works using the Microsoft UF2 tool (included).

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
cd pico-extras
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

# Uploading with Picoprobe
If you have built a Raspberry Pi Picoprobe, you can use OpenOCD to handle your sketch uploads and for debugging with GDB.

Under Windows a local admin user should be able to access the Picoprobe port automatically, but under Linux `udev` must be told about the device and to allow normal users access.

To set up user-level access to Picoprobes on Ubuntu (and other OSes which use `udev`):
````
echo 'SUBSYSTEMS=="usb", ATTRS{idVendor}=="2e8a", ATTRS{idProduct}=="0004", GROUP="users", MODE="0666"' | sudo tee -a /etc/udev/rules.d/98-PicoProbe.rules
sudo udevadm control --reload
````

The first line creates a file with the USB vendor and ID of the Picoprobe and tells UDEV to give users full access to it.  The second causes `udev` to load this new rule.  Note that you will need to unplug and re-plug in your device the first time you create this file, to allow udev to make the device node properly.

Once Picoprobe permissions are set up properly, then select the board "Raspberry Pi Pico (Picoprobe)" in the Tools menu and upload as normal.

# Debugging with Picoprobe, OpenOCD, and GDB
The installed tools include a version of OpenOCD (in the pqt-openocd directory) and GDB (in the pqt-gcc directory).  These may be used to run GDB in an interactive window as documented in the Pico Getting Started manuals from the Raspberry Pi Foundation.

# Status of Port
Relatively stable and very functional, but bug reports and PRs always accepted.
* digitalWrite/Read
* shiftIn/Out
* SPI master (tested using SdFat 2.0 https://github.com/greiman/SdFat ... note that the Pico voltage regulator can't reliably supply enough power for a SD Card so use external power, and adjust the `USE_SIMPLE_LITTLE_ENDIAN` define in `src/sdfat.h` to 0)
* analogWrite/PWM
* tone/noTone
* Wire/I2C Master and Slave (tested using DS3231 https://github.com/rodan/ds3231)
* EEPROM
* USB Serial(ACM) w/automatic reboot-to-UF2 upload)
* Hardware UART
* Servo
* Overclocking and underclocking from the menus
* printf (i.e. debug) output over USB serial 

The RP2040 PIO state machines (SMs) are used to generate jitter-free:
* Servos
* Tones
* I2S Output

# Todo
Some major features I want to add are:
* Installable filesystem support (SD, LittleFS, etc.)
* Updated debug infrastructure

# Tutorials from Across the Web
Here are some links to coverage and additional tutorials for using `arduino-pico`
* Arduino Support for the Pi Pico available! And how fast is the Pico? - https://youtu.be/-XHh17cuH5E
* Pre-release Adafruit QT Py RP2040 - https://www.youtube.com/watch?v=sfC1msqXX0I
* Adafruit Feather RP2040 running LCD + TMP117 - https://www.youtube.com/watch?v=fKDeqZiIwHg

# Contributing
If you want to contribute or have bugfixes, drop me a note at <earlephilhower@yahoo.com> or open an issue/PR here.

-Earle F. Philhower, III
 earlephilhower@yahoo.com
