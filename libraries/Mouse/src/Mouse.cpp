/*
    Mouse.cpp

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

#include "Mouse.h"
#include <RP2040USB.h>

#include "tusb.h"
#include "class/hid/hid_device.h"

//================================================================================
//================================================================================
//	Mouse

/*  This function is for limiting the input value for x and y
    axis to -127 <= x/y <= 127 since this is the allowed value
    range for a USB HID device.
*/

static const uint8_t desc_hid_report_mouse[] = { TUD_HID_REPORT_DESC_MOUSE(HID_REPORT_ID(1)) };
Mouse_::Mouse_(void) {
    _id = usbRegisterHIDDevice(desc_hid_report_mouse, sizeof(desc_hid_report_mouse), 20, 0x0002);
}

void Mouse_::move(int x, int y, signed char wheel) {
    CoreMutex m(&__usb_mutex);
    tud_task();
    if (__USBHIDReady()) {
        tud_hid_mouse_report(usbFindHIDReportID(_id), _buttons, limit_xy(x), limit_xy(y), wheel, 0);
    }
    tud_task();
}

Mouse_ Mouse;
