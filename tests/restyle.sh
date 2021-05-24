#!/bin/bash

for dir in ./cores/rp2040 ./libraries/EEPROM ./libraries/I2S \
           ./libraries/LittleFS/src ./libraries/LittleFS/examples \
           ./libraries/rp2040 ./libraries/SD ./libraries/SdFat \
           ./libraries/Servo ./libraries/SPI ./libraries/Wire; do
    find $dir  \( -name "*.c" -o -name "*.h" -o -name "*.cpp" \) -exec astyle --suffix=none --options=./tests/astyle_core.conf \{\} \;
    find $dir -name "*.ino" -exec astyle --suffix=none --options=./tests/astyle_examples.conf \{\} \;
done
