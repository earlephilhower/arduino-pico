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

#include "MouseAbsolute.h"
#include <RP2040USB.h>

#include "tusb.h"
#include "class/hid/hid_device.h"
#include <sdkoverride/tusb_absmouse.h>

// Weak function override to add our descriptor to the TinyUSB list
void __USBInstallAbsoluteMouse() { /* noop */ }

MouseAbsolute_::MouseAbsolute_(void) : HID_Mouse(true) {
    /* noop */
}

void MouseAbsolute_::move(int x, int y, signed char wheel) {
    CoreMutex m(&__usb_mutex);
    tud_task();
    if (tud_hid_ready()) {
        tud_hid_abs_mouse_report(__USBGetMouseReportID(), _buttons, limit_xy(x), limit_xy(y), wheel, 0);
    }
    tud_task();
}

MouseAbsolute_ MouseAbsolute;
