/*
    Joystick.cpp

    Copyright (c) 2022, Benjamin Aigner <beni@asterics-foundation.org>
    Modified for BT 2023 by Earle F. Philhower, III <earlephilhower@yahoo.com>

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

#include "JoystickBT.h"
#include <Arduino.h>
#include <PicoBluetoothHID.h>

//================================================================================
//================================================================================
//	Joystick/Gamepad

JoystickBT_::JoystickBT_() {
    // HID_Joystick sets up all the member vars
}

#define REPORT_ID 0x01
static const uint8_t desc_joystick[] = {TUD_HID_REPORT_DESC_GAMEPAD(HID_REPORT_ID(REPORT_ID))};

void JoystickBT_::begin(const char *localName, const char *hidName) {
    if (!localName) {
        localName = "PicoW BT Joystick";
    }
    if (!hidName) {
        hidName = localName;
    }
    PicoBluetoothHID.startHID(localName, hidName, 0x2508, 33, desc_joystick, sizeof(desc_joystick));
}

void JoystickBT_::end() {
    PicoBluetoothHID.end();
}

//immediately send an HID report
void JoystickBT_::send_now() {
    PicoBluetoothHID.send(REPORT_ID, &data, sizeof(data));
}

JoystickBT_ JoystickBT;
