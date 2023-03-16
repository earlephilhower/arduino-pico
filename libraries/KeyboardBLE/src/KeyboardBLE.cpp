/*
    KeyboardBLE.cpp

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

#include "KeyboardBLE.h"
#include "KeyboardLayout.h"
#include <PicoBluetoothBLEHID.h>

//================================================================================
//================================================================================
//  Keyboard

KeyboardBLE_::KeyboardBLE_(void) {
    // Base class clears the members we care about
}

#define REPORT_ID 0x01

static const uint8_t desc_keyboard[] = {TUD_HID_REPORT_DESC_KEYBOARD(HID_REPORT_ID(REPORT_ID))};

void KeyboardBLE_::begin(const char *localName, const char *hidName, const uint8_t *layout) {
    if (!localName) {
        localName = "PicoW BLE Keyboard";
    }
    if (!hidName) {
        hidName = localName;
    }
    _asciimap = layout;
    PicoBluetoothBLEHID.startHID(localName, hidName, 0x03c1, desc_keyboard, sizeof(desc_keyboard));
}

void KeyboardBLE_::end(void) {
    PicoBluetoothBLEHID.end();
}

void KeyboardBLE_::setBattery(int lvl) {
    PicoBluetoothBLEHID.setBattery(lvl);
}

void KeyboardBLE_::sendReport(KeyReport* keys) {
    hid_keyboard_report_t data;
    data.modifier = keys->modifiers;
    data.reserved = 0;
    memcpy(data.keycode, keys->keys, sizeof(data.keycode));
    PicoBluetoothBLEHID.send(&data, sizeof(data));
}

void KeyboardBLE_::sendConsumerReport(uint16_t key) {
    (void) key;
    // TODO - Need some BLE-specific code to send 2nd report
}

KeyboardBLE_ KeyboardBLE;
