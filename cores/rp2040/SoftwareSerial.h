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

/**
    @brief Implements a UART port using PIO for input and output
*/
class SoftwareSerial : public SerialPIO {
public:
    /**
        @brief Constructs a PIO-based UART

        @param [in] rx GPIO for RX pin or -1 for transmit-only
        @param [in] tx GPIO for TX pin or -1 for receive-only
        @param [in] invert True to invert the receive and transmit lines
    */
    SoftwareSerial(pin_size_t rx, pin_size_t tx, bool invert = false) : SerialPIO(tx, rx) {
        _invert = invert;
    }

    ~SoftwareSerial() {
    }

    /**
        @brief Starts the PIO UART

        @param [in] baud Serial bit rate
    */
    virtual void begin(unsigned long baud = 115200) override {
        begin(baud, SERIAL_8N1);
    };

    /**
        @brief Starts the PIO UART

        @param [in] baud Serial bit rate
        @param [in] config Start/Stop/Len configuration (i.e. SERIAL_8N1 or SERIAL_7E2)
    */
    void begin(unsigned long baud, uint16_t config) override {
        setInvertTX(_invert);
        setInvertRX(_invert);
        SerialPIO::begin(baud, config);
    }

    /**
        @brief No-op on this core
    */
    void listen() { /* noop */ }

    /**
        @brief No-op on this core

        @returns True always
    */
    bool isListening() {
        return true;
    }

private:
    bool _invert;
};
