/*
    Serial-over-USB for the Raspberry Pi Pico RP2040

    Copyright (c) 2021 Earle F. Philhower, III <earlephilhower@yahoo.com>

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
#include "api/HardwareSerial.h"
#include <stdarg.h>

class SerialUSB : public arduino::HardwareSerial {
public:
    SerialUSB();
    void begin(unsigned long baud = 115200) override;
    void begin(unsigned long baud, uint16_t config) override {
        (void) config;
        begin(baud);
    };
    void end() override;

    virtual int peek() override;
    virtual int read() override;
    virtual int available() override;
    virtual int availableForWrite() override;
    virtual void flush() override;
    virtual size_t write(uint8_t c) override;
    virtual size_t write(const uint8_t *p, size_t len) override;
    using Print::write;
    operator bool() override;
    bool dtr();
    bool rts();

    void ignoreFlowControl(bool ignore = true);

    // ESP8266 compat
    void setDebugOutput(bool unused) {
        (void) unused;
    }

    // TUSB callbacks
    void tud_cdc_line_state_cb(uint8_t itf, bool dtr, bool rts);
    void tud_cdc_line_coding_cb(uint8_t itf, void const *p_line_coding); // Can't use cdc_line_coding_t const* p_line_coding, TinyUSB and BTStack conflict when we include tusb.h + BTStack.h

private:
    bool _running = false;
    uint8_t _id;
    uint8_t _epIn;
    uint8_t _epOut;

    typedef struct {
        unsigned int rebooting : 1;
        unsigned int ignoreFlowControl : 1;
        unsigned int dtr : 1;
        unsigned int rts : 1;
        unsigned int bps : 28;
    } SyntheticState;
    SyntheticState _ss = { 0, 0, 0, 0, 115200};

    void checkSerialReset();
};

extern SerialUSB Serial;

namespace arduino {
extern void serialEventRun(void) __attribute__((weak));
};
