/*
    JoystickBT.h

    Copyright (c) 2022, Benjamin Aigner <beni@asterics-foundation.org>
    Modified for BT 2023 by Earle F. Philhower, III <earlephilhower@yahoo.com>

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

#pragma once

#include <Arduino.h>
#include "class/hid/hid.h"

//======================================================================
class JoystickBT_ {
private:
    bool _autosend;
    bool _use8bit;
    hid_gamepad_report_t data;
    int map8or10bit(int const value);
public:
    JoystickBT_(void);
    void begin(void);
    void end(void);

    //set a selected button to pressed/released
    void button(uint8_t button, bool val);
    //set axis values
    void X(int val);
    void Y(int val);
    void position(int X, int Y);
    void Z(int val);
    void Zrotate(int val);
    void sliderLeft(int val);
    void sliderRight(int val);
    //note: not implemented in TinyUSB gamepad, is mapped to sliderLeft.
    void slider(int val);

    //set the hat value, from 0-360. -1 is rest position
    void hat(int angle);
    //compatibility: there is only one hat implemented, num parameter is ignored
    void hat(unsigned int num, int angle);


    //if set, the gamepad report is not automatically sent after an update of axes/buttons; use send_now to update
    void useManualSend(bool mode);
    //immediately send an HID report
    void send_now(void);
    //define the mapping of axes values
    //default: axes methods are accepting values from 0-1023 (compatibility to other Joystick libraries)
    // and are mapped internally to int8_t
    //if use8bit(true) is called, -127 to 127 values are used.
    void use8bit(bool mode);
};
extern JoystickBT_ JoystickBT;
