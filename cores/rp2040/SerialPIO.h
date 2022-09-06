/*
    Serial-over-PIO for the Raspberry Pi Pico RP2040

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
#include <hardware/uart.h>
#include "CoreMutex.h"

extern "C" typedef struct uart_inst uart_inst_t;

class SerialPIO : public HardwareSerial {
public:
    static const pin_size_t NOPIN = 0xff; // Use in constructor to disable RX or TX unit
    SerialPIO(pin_size_t tx, pin_size_t rx, size_t fifoSize = 32);
    ~SerialPIO();

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
    bool overflow();
    using Print::write;
    operator bool() override;

    // Not to be called by users, only from the IRQ handler.  In public so that the C-language IQR callback can access it
    void _handleIRQ();

protected:
    bool _running = false;
    pin_size_t _tx, _rx;
    int _baud;
    int _bits;
    uart_parity_t _parity;
    int _stop;
    bool _overflow;
    mutex_t _mutex;

    PIOProgram *_txPgm;
    PIO _txPIO;
    int _txSM;
    int _txBits;

    PIOProgram *_rxPgm;
    PIO _rxPIO;
    int _rxSM;
    int _rxBits;

    // Lockless, IRQ-handled circular queue
    size_t   _fifoSize;
    uint32_t _writer;
    uint32_t _reader;
    uint8_t  *_queue;
};

#ifdef ARDUINO_NANO_RP2040_CONNECT
// NINA updates
extern SerialPIO Serial3;
#endif
