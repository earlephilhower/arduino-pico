/*
    WiFiMutex.h - Ensure the timer-driven sys_check_timeouts doesn't
                  get executed while we're in the user-level TCP stack.
    Copyright (c) 2022 Earle F. Philhower, III.  All rights reserved.

    Implements the API defined by the Arduino WiFiNINA library,
    copyright (c) 2018 Arduino SA. All rights reserved.

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

extern "C" volatile bool __inLWIP;

class LWIPMutex {
public:
    LWIPMutex() {
        __inLWIP = true;
        _ref++;
    }
    ~LWIPMutex() {
        if (0 == --_ref) {
            __inLWIP = false;
        }
    }
private:
    static int _ref;
};
