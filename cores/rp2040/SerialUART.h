/*
 * Serial-over-UART for the Raspberry Pi Pico RP2040
 *
 * Copyright (c) 2021 Earle F. Philhower, III <earlephilhower@yahoo.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef __SERIALUART_H__
#define __SERIALUART_H__

#include <Arduino.h>
#include "api/HardwareSerial.h"
#include <stdarg.h>
#include "CoreMutex.h"

extern "C" typedef struct uart_inst uart_inst_t;

class SerialUART : public HardwareSerial {
public:
    SerialUART(uart_inst_t *uart, pin_size_t tx, pin_size_t rx);
    
    // Select the pinout.  Call before .begin()
    bool setRX(pin_size_t pin);
    bool setTX(pin_size_t pin);
    bool setPinout(pin_size_t tx, pin_size_t rx) { bool ret = setRX(rx); ret &= setTX(tx); return ret; }

    void begin(unsigned long baud = 115200) override { begin(baud, SERIAL_8N1); };
    void begin(unsigned long baud, uint16_t config) override;
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

    // By all rights, this should be in Print.h...
    size_t printf(const char *format, ...) {
        va_list arg;
        va_start(arg, format);
        char temp[64];
        char* buffer = temp;
        size_t len = vsnprintf(temp, sizeof(temp), format, arg);
        va_end(arg);
        if (len > sizeof(temp) - 1) {
            buffer = new char[len + 1];
            if (!buffer) {
                return 0;
            }
            va_start(arg, format);
            vsnprintf(buffer, len + 1, format, arg);
            va_end(arg);
        }
        len = write((const uint8_t*) buffer, len);
        if (buffer != temp) {
            delete[] buffer;
        }
        return len;
    }
private:
    bool _running = false;
    uart_inst_t *_uart;
    pin_size_t _tx, _rx;
    int _baud;
    int _peek;
    mutex_t _mutex;
};

extern SerialUART Serial1; // HW UART 0
extern SerialUART Serial2; // HW UART 1

namespace arduino {
    extern void serialEvent1Run(void) __attribute__((weak));
    extern void serialEvent2Run(void) __attribute__((weak));
};

#endif
