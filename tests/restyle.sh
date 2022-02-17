#!/bin/bash

for dir in ./cores/rp2040 ./libraries/EEPROM ./libraries/I2S \
           ./libraries/LittleFS/src ./libraries/LittleFS/examples \
           ./libraries/rp2040 ./libraries/SD ./libraries/ESP8266SdFat \
           ./libraries/Servo ./libraries/SPI ./libraries/Wire; do
    find $dir -type f \( -name "*.c" -o -name "*.h" -o -name "*.cpp" \) -a  \! -path '*api*' -exec astyle --suffix=none --options=./tests/astyle_core.conf \{\} \;
    find $dir -type f -name "*.ino" -exec astyle --suffix=none --options=./tests/astyle_examples.conf \{\} \;
done
