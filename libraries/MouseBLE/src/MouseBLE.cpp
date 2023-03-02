/*
    MouseBLE.cpp

    Copyright (c) 2015, Arduino LLC
    Original code (pre-library): Copyright (c) 2011, Peter Barrett

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

#include "MouseBLE.h"
#include <PicoBluetoothBLEHID.h>

//================================================================================
//================================================================================
//	Mouse

/*  This function is for limiting the input value for x and y
    axis to -127 <= x/y <= 127 since this is the allowed value
    range for a USB HID device.
*/
static signed char limit_xy(int const xy) {
    if (xy < -127) {
        return -127;
    } else if (xy >  127) {
        return 127;
    } else {
        return xy;
    }
}

MouseBLE_::MouseBLE_(void) : _buttons(0) {
    /* noop */
}

#define REPORT_ID 0x01
const uint8_t desc_mouse[] = {TUD_HID_REPORT_DESC_MOUSE(HID_REPORT_ID(REPORT_ID))};
void MouseBLE_::begin(void) {
    PicoBluetoothBLEHID.startHID("PicoW BLE Mouse", "PicoW BLE Mouse", 0x03c2, desc_mouse, sizeof(desc_mouse));
}

void MouseBLE_::end(void) {
    PicoBluetoothBLEHID.end();
}

void MouseBLE_::click(uint8_t b) {
    _buttons = b;
    move(0, 0, 0);
    delay(10);
    _buttons = 0;
    move(0, 0, 0);
    delay(10);
}

void MouseBLE_::move(int x, int y, signed char wheel) {
    hid_mouse_report_t data;
    data.buttons = _buttons;
    data.x = limit_xy(x);
    data.y = limit_xy(y);
    data.wheel = wheel;
    data.pan = 0;
    PicoBluetoothBLEHID.send(&data, sizeof(data));
}

void MouseBLE_::buttons(uint8_t b) {
    if (b != _buttons) {
        _buttons = b;
        move(0, 0, 0);
    }
}

void MouseBLE_::press(uint8_t b) {
    buttons(_buttons | b);
}

void MouseBLE_::release(uint8_t b) {
    buttons(_buttons & ~b);
}

bool MouseBLE_::isPressed(uint8_t b) {
    if ((b & _buttons) > 0) {
        return true;
    } else {
        return false;
    }
}

MouseBLE_ MouseBLE;
