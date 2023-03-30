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
#include <sdkoverride/tusb_absmouse.h>
#include <PicoBluetoothBLEHID.h>

MouseBLE_::MouseBLE_(bool absolute) : HID_Mouse(absolute) {
    _running = false;
}

#define REPORT_ID 0x01
const uint8_t desc_mouse[] = {TUD_HID_REPORT_DESC_MOUSE(HID_REPORT_ID(REPORT_ID))};
const uint8_t desc_absmouse[] = {TUD_HID_REPORT_DESC_ABSMOUSE(HID_REPORT_ID(REPORT_ID))};
void MouseBLE_::begin(const char *localName, const char *hidName) {
    if (!localName) {
        localName = "PicoW BLE Mouse";
    }
    if (!hidName) {
        hidName = localName;
    }
    PicoBluetoothBLEHID.startHID(localName, hidName, 0x03c2, _absolute ? desc_absmouse : desc_mouse, _absolute ? sizeof(desc_absmouse) : sizeof(desc_mouse));
    _running = true;
}

void MouseBLE_::end(void) {
    if (_running) {
        PicoBluetoothBLEHID.end();
    }
    _running = false;
}

void MouseBLE_::setBattery(int lvl) {
    if (_running) {
        PicoBluetoothBLEHID.setBattery(lvl);
    }
}

void MouseBLE_::setAbsolute(bool absolute) {
    if (!_running) {
        _absolute = absolute;
    }
}

void MouseBLE_::move(int x, int y, signed char wheel) {
    if (!_absolute) {
        hid_mouse_report_t data;
        data.buttons = _buttons;
        data.x = limit_xy(x);
        data.y = limit_xy(y);
        data.wheel = wheel;
        data.pan = 0;
        PicoBluetoothBLEHID.send(&data, sizeof(data));
    } else {
        hid_abs_mouse_report_t data;
        data.buttons = _buttons;
        data.x = limit_xy(x);
        data.y = limit_xy(y);
        data.wheel = wheel;
        data.pan = 0;
        PicoBluetoothBLEHID.send(&data, sizeof(data));
    }
}

MouseBLE_ MouseBLE;
