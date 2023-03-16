/*
    Keyboard.cpp

    Modified by Earle F. Philhower, III <earlephilhower@yahoo.com>
    Main Arduino Library Copyright (c) 2015, Arduino LLC
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

#include "Keyboard.h"
#include <RP2040USB.h>

#include "tusb.h"
#include "class/hid/hid_device.h"

// Weak function override to add our descriptor to the TinyUSB list
void __USBInstallKeyboard() { /* noop */ }

//================================================================================
//================================================================================
//  Keyboard

Keyboard_::Keyboard_(void) {
    // Base class clears the members we care about
}

void Keyboard_::sendReport(KeyReport* keys) {
    CoreMutex m(&__usb_mutex);
    tud_task();
    if (tud_hid_ready()) {
        tud_hid_keyboard_report(__USBGetKeyboardReportID(), keys->modifiers, keys->keys);
    }
    tud_task();
}

void Keyboard_::sendConsumerReport(uint16_t key) {
    CoreMutex m(&__usb_mutex);
    tud_task();
    if (tud_hid_ready()) {
        tud_hid_report(__USBGetKeyboardReportID() + 1, &key, sizeof(key));
    }
    tud_task();
}


extern "C" void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize) {
    (void) report_id;
    (void) instance;

    if ((report_type == HID_REPORT_TYPE_OUTPUT) && (bufsize > 0) && (Keyboard._ledCB)) {
        uint8_t const kbd_leds = buffer[0];
        Keyboard._ledCB(kbd_leds & KEYBOARD_LED_NUMLOCK, kbd_leds & KEYBOARD_LED_CAPSLOCK, kbd_leds & KEYBOARD_LED_SCROLLLOCK, kbd_leds & KEYBOARD_LED_COMPOSE, kbd_leds & KEYBOARD_LED_KANA, Keyboard._ledCBdata);
    }
}

Keyboard_ Keyboard;
