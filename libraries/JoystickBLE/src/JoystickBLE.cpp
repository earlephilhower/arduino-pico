/*
    JoystickBLE.cpp

    Copyright (c) 2022, Benjamin Aigner <beni@asterics-foundation.org>
    Modified for BLE 2023 by Earle F. Philhower, III <earlephilhower@yahoo.com>

    Implementation loosely based on:
    Mouse library from https://github.com/earlephilhower/arduino-pico
    Joystick functions from Teensyduino https://github.com/PaulStoffregen

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "JoystickBLE.h"
#include <Arduino.h>
#include <HID_Bluetooth.h>
#include <PicoBluetoothBLEHID.h>

//================================================================================
//================================================================================
//	Joystick/Gamepad

// Weak function override to add our descriptor to the list
void __BLEInstallJoystick() { /* noop */ }

JoystickBLE_::JoystickBLE_(void) {
    // Member vars set in base constructor
}

uint8_t *desc_joystickBLE;
uint16_t desc_joystickBLE_length;

void JoystickBLE_::begin(const char *localName, const char *hidName) {
    if (!localName) {
        localName = "PicoW BLE Joystick";
    }
    if (!hidName) {
        hidName = localName;
    }

    __SetupHIDreportmap(__BLEInstallMouse, __BLEInstallKeyboard, __BLEInstallJoystick, false, &desc_joystickBLE_length, &desc_joystickBLE);

    PicoBluetoothBLEHID.startHID(localName, hidName, __BLEGetAppearance(), desc_joystickBLE, desc_joystickBLE_length);
}

void JoystickBLE_::end() {
    PicoBluetoothBLEHID.end();
}

void JoystickBLE_::setBattery(int lvl) {
    PicoBluetoothBLEHID.setBattery(lvl);
}

void JoystickBLE_::send_now() {
    //insert report ID; not part of the hid_gamepad_report_t
    uint8_t *report = (uint8_t *)malloc(sizeof(hid_gamepad16_report_t) + 1);
    if (report) {
        report[0] = __BLEGetJoystickReportID();
        memcpy(&report[1], (uint8_t*)&data, sizeof(data));
        PicoBluetoothBLEHID.send(report, sizeof(data) + 1);
    }
}

JoystickBLE_ JoystickBLE;
