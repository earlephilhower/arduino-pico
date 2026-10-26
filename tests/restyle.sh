#!/bin/bash

base=$(git rev-parse --show-toplevel)
cd "$base/tests"

# Build our own astyle because version creep could cause issues
if ! [ -e astyle ]; then
    git clone https://gitlab.com/saalen/astyle.git astyle
    cd astyle
    git checkout 3.1
    cd AStyle
    mkdir build
    cd build/
    cmake ..
    make -j
fi
astyle="$base/tests/astyle/AStyle/build/astyle"

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
           ./libraries/lwIP_w6300 ./libraries/lwIP_w55rp20 ./libraries/lwIP_w5500 ./libraries/lwIP_w5100 ./libraries/lwIP_enc28j60 \
           ./libraries/SPISlave ./libraries/lwIP_ESPHost ./libraries/lwIP_USB_NCM ./libraries/FatFS\
           ./libraries/FatFSUSB ./libraries/BluetoothAudio ./libraries/BluetoothHCI \
           ./libraries/BluetoothHIDMaster ./libraries/NetBIOS ./libraries/Ticker \
           ./libraries/VFS ./libraries/rp2350 ./libraries/SimpleMDNS ./libraries/SoftwareSPI \
           ./libraries/MIDIUSB ./libraries/BLE; do
    find "$base"/$dir -type f \( -name "*.c" -o -name "*.h" -o -name "*.cpp" \) -a  \! -path '*api*' -exec "$astyle" --suffix=none --options="$base"/tests/astyle_core.conf \{\} \;
    find "$base"/$dir -type f -name "*.ino" -exec "$astyle" --suffix=none --options="$base"/tests/astyle_examples.conf \{\} \;
done

