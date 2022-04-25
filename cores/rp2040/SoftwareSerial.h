/*
    SoftwareSerial wrapper for SerialPIO

    Copyright (c) 2022 Earle F. Philhower, III <earlephilhower@yahoo.com>

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

#include "SerialPIO.h"

class SoftwareSerial : public SerialPIO {
public:
    // Note the rx/tx pins are swapped in PIO vs SWSerial
    SoftwareSerial(pin_size_t rx, pin_size_t tx, bool invert = true) : SerialPIO(tx, rx) {
        if (invert) {
            panic("SoftwareSerial inverted operation not supported\n");
        }
    }
    void listen() { /* noop */ }
    bool isListening() {
        return true;
    }
};
