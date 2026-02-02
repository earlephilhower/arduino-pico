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
#include <hardware/uart.h>
#include "CoreMutex.h"
#include "LocklessQueue.h"

extern "C" typedef struct uart_inst uart_inst_t;

class SerialPIO : public arduino::HardwareSerial {
public:
    SerialPIO(pin_size_t tx, pin_size_t rx, size_t fifoSize = 32);
    ~SerialPIO();

    void begin(unsigned long baud = 115200) override {
        begin(baud, SERIAL_8N1);
    };
    void begin(unsigned long baud, uint16_t config) override;
    void end() override;

    void setInverted(bool invTx = true, bool invRx = true) {
        setInvertTX(invTx);
        setInvertRX(invRx);
    }
    bool setInvertTX(bool invert = true) {
        if (!_running) {
            _invertTX = invert;
        }
        return !_running;
    }
    bool setInvertRX(bool invert = true) {
        if (!_running) {
            _invertRX = invert;
        }
        return !_running;
    }

    virtual int peek() override;
    virtual int read() override;
    virtual int available() override;
    virtual int availableForWrite() override;
    virtual void flush() override;
    virtual size_t write(uint8_t c) override;
    bool overflow();
    using Print::write;
    operator bool() override;

protected:
    static void _fifoIRQ();
    void _handleIRQ();

    bool _running = false;
    pin_size_t _tx, _rx;
    int _baud;
    int _bits;
    uart_parity_t _parity;
    int _stop;
    bool _overflow;
    mutex_t _mutex;
    bool _invertTX;
    bool _invertRX;

    PIOProgram *_txPgm;
    PIO _txPIO;
    int _txSM;
    int _txBits;

    PIOProgram *_rxPgm;
    PIO _rxPIO;
    int _rxSM;
    int _rxBits;

    LocklessQueue<uint8_t> *_queue;
    size_t   _fifoSize;

    uint _onCore;
};

#ifdef ARDUINO_NANO_RP2040_CONNECT
// NINA updates
extern SerialPIO Serial3;
#endif
