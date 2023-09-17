/*
    SPI Slave library for the Raspberry Pi Pico RP2040

    Copyright (c) 2023 Earle F. Philhower, III <earlephilhower@yahoo.com>

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
#include <api/HardwareSPI.h>
#include <hardware/spi.h>
#include <functional>

typedef std::function<void(uint8_t *data, size_t len)> SPISlaveRecvHandler;
typedef std::function<void(void)> SPISlaveSentHandler;

class SPISlaveClass {
public:
    SPISlaveClass(spi_inst_t *spi, pin_size_t rx, pin_size_t cs, pin_size_t sck, pin_size_t tx);

    // Assign pins, call before begin()
    bool setRX(pin_size_t pin);
    bool setCS(pin_size_t pin);
    bool setSCK(pin_size_t pin);
    bool setTX(pin_size_t pin);

    void begin(SPISettings spis);
    void end();

    // May be set before the initial begin().  If not, then as soon as
    // begin() is called you will get a callback.
    void setData(const uint8_t * data, size_t len);
    inline void setData(const char * data) {
        setData((const uint8_t *)data, strlen(data));
    }

    // NOTE: These two callbacks are called from an IRQ context
    // NOTE: They should also be called before begin()

    // Called when bytes are available to be read from the SPI slave reception buffer
    void onDataRecv(SPISlaveRecvHandler cb) {
        _recvCB = cb;
    }
    // Called when there is space in the SPI transmission buffer
    void onDataSent(SPISlaveSentHandler cb) {
        _sentCB = cb;
    }

private:
    // Naked IRQ callbacks, will thunk to real object ones below
    static void _irq0();
    static void _irq1();

public:
    void _handleIRQ();

private:
    spi_cpol_t cpol(SPISettings _spis);
    spi_cpha_t cpha(SPISettings _spis);
    uint8_t reverseByte(uint8_t b);
    uint16_t reverse16Bit(uint16_t w);
    void adjustBuffer(const void *s, void *d, size_t cnt, bool by16);

    spi_inst_t *_spi;
    SPISettings _spis;
    pin_size_t _RX, _TX, _SCK, _CS;
    bool _running; // SPI port active
    bool _initted; // Transaction begun

    SPISlaveRecvHandler _recvCB;
    SPISlaveSentHandler _sentCB;

    // The current data to be pumped into the transmit FIFO
    const uint8_t *_dataOut;
    size_t _dataLeft;

    // Received data will be returned in small chunks directly from a local buffer in _handleIRQ()
};

extern SPISlaveClass SPISlave;
extern SPISlaveClass SPISlave1;
