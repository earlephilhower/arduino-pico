/*
    JoystickBLE.h

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

#include <HID_Joystick.h>
#include "class/hid/hid.h"

//======================================================================
class JoystickBLE_ : public HID_Joystick {
public:
    JoystickBLE_();
    void begin(const char *localName = nullptr, const char *hidName = nullptr);
    void end();
    virtual void send_now() override;
    void setBattery(int lvl);
};
extern JoystickBLE_ JoystickBLE;
