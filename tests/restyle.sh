#!/bin/bash

for dir in ./cores/rp2040 ./libraries/EEPROM ./libraries/I2S ./libraries/SingleFileDrive \
           ./libraries/LittleFS/src ./libraries/LittleFS/examples ./libraries/PWMAudio \
           ./libraries/rp2040 ./libraries/SD ./libraries/ADCInput \
           ./libraries/Servo ./libraries/SPI ./libraries/Wire ./libraries/PDM \
           ./libraries/WiFi ./libraries/lwIP_Ethernet ./libraries/lwIP_CYW43 \
           ./libraries/FreeRTOS/src ./libraries/LEAmDNS ./libraries/MD5Builder \
           ./libraries/PicoOTA ./libraries/SDFS ./libraries/ArduinoOTA \
           ./libraries/Updater ./libraries/HTTPClient ./libraries/HTTPUpdate \
           ./libraries/WebServer ./libraries/HTTPUpdateServer ./libraries/DNSServer \
           ./libraries/Joystick ./libraries/Keyboard ./libraries/Mouse \
           ./libraries/JoystickBT ./libraries/KeyboardBT ./variants ./libraries/BTstackLib \
           ./libraries/MouseBT ./libraries/SerialBT ./libraries/HID_Bluetooth \
           ./libraries/JoystickBLE ./libraries/KeyboardBLE ./libraries/MouseBLE \
           ./libraries/lwIP_w5500 ./libraries/lwIP_w5100 ./libraries/lwIP_enc28j60 \
           ./libraries/SPISlave ./libraries/lwIP_ESPHost ./libraries/FatFS\
           ./libraries/FatFSUSB ./libraries/BluetoothAudio ./libraries/BluetoothHCI \
           ./libraries/BluetoothHIDMaster ./libraries/NetBIOS ./libraries/Ticker; do
    find $dir -type f \( -name "*.c" -o -name "*.h" -o -name "*.cpp" \) -a  \! -path '*api*' -exec astyle --suffix=none --options=./tests/astyle_core.conf \{\} \;
    find $dir -type f -name "*.ino" -exec astyle --suffix=none --options=./tests/astyle_examples.conf \{\} \;
done

