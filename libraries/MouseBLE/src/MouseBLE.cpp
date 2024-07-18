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
#include <HID_Bluetooth.h>
#include <PicoBluetoothBLEHID.h>

// Weak function override to add our descriptor to the list
void __BLEInstallMouse() { /* noop */ }

MouseBLE_::MouseBLE_(bool absolute) : HID_Mouse(absolute) {
    _running = false;
}

uint8_t *desc_mouseBLE;
uint16_t desc_mouseBLE_length;

void MouseBLE_::begin(const char *localName, const char *hidName) {
    if (!localName) {
        localName = "PicoW BLE Mouse";
    }
    if (!hidName) {
        hidName = localName;
    }

    __SetupHIDreportmap(__BLEInstallMouse, __BLEInstallKeyboard, __BLEInstallJoystick, _absolute, &desc_mouseBLE_length, &desc_mouseBLE);

    PicoBluetoothBLEHID.startHID(localName, hidName, __BLEGetAppearance(), desc_mouseBLE, desc_mouseBLE_length);
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
    static uint8_t report[sizeof(hid_abs_mouse_report_t) + 1];

    if (!_absolute) {
        hid_mouse_report_t data;
        data.buttons = _buttons;
        data.x = limit_xy(x);
        data.y = limit_xy(y);
        data.wheel = wheel;
        data.pan = 0;

        report[0] = __BLEGetMouseReportID();
        memcpy(&report[1], (uint8_t*)&data, sizeof(data));
        PicoBluetoothBLEHID.send(report, sizeof(data) + 1);
    } else {
        hid_abs_mouse_report_t data;
        data.buttons = _buttons;
        data.x = limit_xy(x);
        data.y = limit_xy(y);
        data.wheel = wheel;
        data.pan = 0;

        report[0] = __BLEGetMouseReportID();
        memcpy(&report[1], (uint8_t*)&data, sizeof(data));
        PicoBluetoothBLEHID.send(report, sizeof(data) + 1);
    }
}

MouseBLE_ MouseBLE;
