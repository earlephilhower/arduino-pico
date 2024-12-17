/*
    MouseBT.cpp

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

#include "MouseBT.h"
#include <HID_Bluetooth.h>
#include <PicoBluetoothHID.h>

// Weak function override to add our descriptor to the list
void __BTInstallMouse() { /* noop */ }

MouseBT_::MouseBT_(bool absolute) : HID_Mouse(absolute) {
    _running = false;
}

uint8_t *desc_mouseBT;
uint16_t desc_mouseBT_length;

void MouseBT_::begin(const char *localName, const char *hidName) {
    if (!localName) {
        localName = "PicoW Mouse 00:00:00:00:00:00";
    }
    if (!hidName) {
        hidName = localName;
    }

    __SetupHIDreportmap(__BTInstallMouse, __BTInstallKeyboard, __BTInstallJoystick, _absolute, &desc_mouseBT_length, &desc_mouseBT);

    PicoBluetoothHID.startHID(localName, hidName, __BTGetCOD(), 33, desc_mouseBT, desc_mouseBT_length);
    _running = true;
}

void MouseBT_::end(void) {
    if (_running) {
        PicoBluetoothHID.end();
    }
    _running = false;
}

void MouseBT_::setAbsolute(bool absolute) {
    if (!_running) {
        _absolute = absolute;
    }
}

void MouseBT_::move(int x, int y, signed char wheel) {
    if (!_absolute) {
        hid_mouse_report_t data;
        data.buttons = _buttons;
        data.x = limit_xy(x);
        data.y = limit_xy(y);
        data.wheel = wheel;
        data.pan = 0;
        PicoBluetoothHID.send(__BTGetMouseReportID(), &data, sizeof(data));
    } else {
        hid_abs_mouse_report_t data;
        data.buttons = _buttons;
        data.x = limit_xy(x);
        data.y = limit_xy(y);
        data.wheel = wheel;
        data.pan = 0;
        PicoBluetoothHID.send(__BTGetMouseReportID(), &data, sizeof(data));
    }
}

MouseBT_ MouseBT;
