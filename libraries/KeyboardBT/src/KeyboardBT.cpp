/*
    KeyboardBT.cpp

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

#include "KeyboardBT.h"
#include "KeyboardLayout.h"
#include <PicoBluetoothHID.h>

//================================================================================
//================================================================================
//  Keyboard

KeyboardBT_::KeyboardBT_(void) {
    // Base class clears the members we care about
}

#define REPORT_ID 0x01

static const uint8_t desc_keyboard[] = {TUD_HID_REPORT_DESC_KEYBOARD(HID_REPORT_ID(REPORT_ID)), TUD_HID_REPORT_DESC_CONSUMER(HID_REPORT_ID(REPORT_ID + 1))};

static void _hidReportCB(uint16_t cid, hid_report_type_t report_type, uint16_t report_id, int report_size, uint8_t *report) {
    (void) cid;
    (void) report_id;
    if ((report_type == HID_REPORT_TYPE_OUTPUT) && (report_size > 0) && (KeyboardBT._ledCB)) {
        uint8_t const kbd_leds = report[0];
        KeyboardBT._ledCB(kbd_leds & KEYBOARD_LED_NUMLOCK, kbd_leds & KEYBOARD_LED_CAPSLOCK, kbd_leds & KEYBOARD_LED_SCROLLLOCK, kbd_leds & KEYBOARD_LED_COMPOSE, kbd_leds & KEYBOARD_LED_KANA, KeyboardBT._ledCBdata);
    }
}

void KeyboardBT_::begin(const char *localName, const char *hidName, const uint8_t *layout) {
    if (!localName) {
        localName = "PicoW BT Keyboard";
    }
    if (!hidName) {
        hidName = localName;
    }
    _asciimap = layout;
    // Required because the hid_report_type_t overlap in BTStack and TUSB
    auto *fcn = (void (*)(short unsigned int, hid_report_type_t_bt, short unsigned int, int, unsigned char*))_hidReportCB;
    hid_device_register_report_data_callback(fcn);
    PicoBluetoothHID.startHID(localName, hidName, 0x2540, 33, desc_keyboard, sizeof(desc_keyboard));
}

void KeyboardBT_::end(void) {
    PicoBluetoothHID.end();
}

void KeyboardBT_::sendReport(KeyReport* keys) {
    hid_keyboard_report_t data;
    data.modifier = keys->modifiers;
    data.reserved = 0;
    memcpy(data.keycode, keys->keys, sizeof(data.keycode));
    PicoBluetoothHID.send(REPORT_ID, &data, sizeof(data));
}

void KeyboardBT_::sendConsumerReport(uint16_t key) {
    PicoBluetoothHID.send(REPORT_ID + 1, &key, sizeof(key));
}

KeyboardBT_ KeyboardBT;
