/*
    JoystickBLE.cpp

    Copyright (c) 2022, Benjamin Aigner <beni@asterics-foundation.org>
    Modified for BLE 2023 by Earle F. Philhower, III <earlephilhower@yahoo.com>

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

#include "JoystickBLE.h"
#include <Arduino.h>
#include <PicoBluetoothBLEHID.h>

//================================================================================
//================================================================================
//	Joystick/Gamepad

JoystickBLE_::JoystickBLE_(void) {
    _use8bit = false;
    _autosend = true;
    memset(&data, 0, sizeof(data));
    //_X_axis = _Y_axis = _Z_axis = _Zrotate = _sliderLeft = _sliderRight = _hat = data.buttons = 0;
}

/** define the mapping of axes values
    default: axes methods are accepting values from 0-1023 (compatibility to other Joystick libraries)
		and are mapped internally to int8_t
    if use8bit(true) is called, -127 to 127 values are used.*/
void JoystickBLE_::use8bit(bool mode) {
    _use8bit = mode;
}

//if set, the gamepad report is not automatically sent after an update of axes/buttons; use send_now to update
void JoystickBLE_::useManualSend(bool mode) {
    _autosend = !mode;
}

/** Maps values from 8bit signed or 10bit unsigned to report values

    Depending on the setting via use8bit(), either values from 0-1023 or -127 - 127
    are mapped.
*/
int JoystickBLE_::map8or10bit(int const value) {
    if (_use8bit) {
        if (value < -127) {
            return -127;
        } else if (value >  127) {
            return 127;
        } else {
            return value;
        }
    } else {
        if (value < 0) {
            return 0;
        }
        if (value > 1023) {
            return 1023;
        }
        return map(value, 0, 1023, -127, 127);
    }
}

#define REPORT_ID 0x01
static const uint8_t desc_joystick[] = {TUD_HID_REPORT_DESC_GAMEPAD(HID_REPORT_ID(REPORT_ID))};
void JoystickBLE_::begin(void) {
    PicoBluetoothBLEHID.startHID("PicoW BLE Joystick", "PicoW BLE Joystick", 0x03c4, desc_joystick, sizeof(desc_joystick));
}

void JoystickBLE_::end(void) {
    PicoBluetoothBLEHID.end();
}

void JoystickBLE_::setBattery(int lvl) {
    PicoBluetoothBLEHID.setBattery(lvl);
}

void JoystickBLE_::button(uint8_t button, bool val) {
    //I've no idea why, but without a second dword, it is not possible.
    //maybe something with the alignment when using bit set/clear?!?
    static uint32_t buttons_local = 0;

    if (button >= 1 && button <= 32) {
        if (val) {
            buttons_local |= (1UL << (button - 1));
        } else {
            buttons_local &= ~(1UL << (button - 1));
        }

        data.buttons = buttons_local;
        if (_autosend) {
            send_now();
        }
    }
}

void JoystickBLE_::X(int val) {
    data.x = map8or10bit(val);
    if (_autosend) {
        send_now();
    }
}
void JoystickBLE_::Y(int val) {
    data.y = map8or10bit(val);
    if (_autosend) {
        send_now();
    }
}
void JoystickBLE_::Z(int val) {
    data.z = map8or10bit(val);
    if (_autosend) {
        send_now();
    }
}
void JoystickBLE_::Zrotate(int val) {
    data.rz = map8or10bit(val);
    if (_autosend) {
        send_now();
    }
}
void JoystickBLE_::sliderLeft(int val) {
    data.rx = map8or10bit(val);
    if (_autosend) {
        send_now();
    }
}
void JoystickBLE_::sliderRight(int val) {
    data.ry = map8or10bit(val);
    if (_autosend) {
        send_now();
    }
}

void JoystickBLE_::slider(int val) {
    data.rx = map8or10bit(val);
    if (_autosend) {
        send_now();
    }
}

void JoystickBLE_::position(int X, int Y) {
    data.x = map8or10bit(X);
    data.y = map8or10bit(Y);
    if (_autosend) {
        send_now();
    }
}

//compatibility: there is only one hat implemented, num parameter is ignored
void JoystickBLE_::hat(unsigned int num, int angle) {
    (void) num;
    hat(angle);
}

//set the hat value, from 0-360. -1 is rest position
void JoystickBLE_::hat(int angle) {
    if (angle < 0) {
        data.hat = 0;
    }
    if (angle >= 0 && angle <= 360) {
        data.hat = map(angle, 0, 360, 1, 8);
    }
    if (_autosend) {
        send_now();
    }
}

//immediately send an HID report
void JoystickBLE_::send_now(void) {
    PicoBluetoothBLEHID.send(&data, sizeof(data));
}

JoystickBLE_ JoystickBLE;
