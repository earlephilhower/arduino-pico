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
    SoftwareSerial(pin_size_t rx, pin_size_t tx, bool invert = false) : SerialPIO(tx, rx) {
        _invert = invert;
    }

    ~SoftwareSerial() {
        if (_invert) {
            gpio_set_outover(_tx, 0);
            gpio_set_outover(_rx, 0);
        }
    }

    virtual void begin(unsigned long baud = 115200) override {
        begin(baud, SERIAL_8N1);
    };

    void begin(unsigned long baud, uint16_t config) override {
        SerialPIO::begin(baud, config);
        if (_invert) {
            gpio_set_outover(_tx, GPIO_OVERRIDE_INVERT);
            gpio_set_inover(_rx, GPIO_OVERRIDE_INVERT);
        }
    }

    void listen() { /* noop */ }

    bool isListening() {
        return true;
    }

private:
    bool _invert;
};
