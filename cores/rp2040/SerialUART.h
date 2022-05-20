/*
    Serial-over-UART for the Raspberry Pi Pico RP2040

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
#include <queue>
#include "CoreMutex.h"

extern "C" typedef struct uart_inst uart_inst_t;

#define UART_PIN_NOT_DEFINED      (255u)
class SerialUART : public HardwareSerial {
public:
    SerialUART(uart_inst_t *uart, pin_size_t tx, pin_size_t rx, pin_size_t rts = UART_PIN_NOT_DEFINED, pin_size_t cts = UART_PIN_NOT_DEFINED);

    // Select the pinout.  Call before .begin()
    bool setRX(pin_size_t pin);
    bool setTX(pin_size_t pin);
    bool setRTS(pin_size_t pin);
    bool setCTS(pin_size_t pin);
    bool setPinout(pin_size_t tx, pin_size_t rx) {
        bool ret = setRX(rx);
        ret &= setTX(tx);
        return ret;
    }
    bool setFIFOSize(size_t size);
    bool setPollingMode(bool mode = true);

    void begin(unsigned long baud = 115200) override {
        begin(baud, SERIAL_8N1);
    };
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
    bool overflow();
    operator bool() override;

    // Not to be called by users, only from the IRQ handler.  In public so that the C-language IQR callback can access it
    void _handleIRQ(bool inIRQ = true);

private:
    bool _running = false;
    uart_inst_t *_uart;
    pin_size_t _tx, _rx;
    pin_size_t _rts, _cts;
    int _baud;
    mutex_t _mutex;
    bool _polling = false;
    bool _overflow;

    // Lockless, IRQ-handled circular queue
    uint32_t _writer;
    uint32_t _reader;
    size_t   _fifoSize = 32;
    uint8_t *_queue;
    mutex_t  _fifoMutex; // Only needed when non-IRQ updates _writer
    void _pumpFIFO(); // User space FIFO transfer
};

extern SerialUART Serial1; // HW UART 0
extern SerialUART Serial2; // HW UART 1

namespace arduino {
extern void serialEvent1Run(void) __attribute__((weak));
extern void serialEvent2Run(void) __attribute__((weak));
};
