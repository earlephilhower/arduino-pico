/*
    SPI Master library for the Raspberry Pi Pico RP2040

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
#include <api/HardwareSPI.h>
#include <hardware/spi.h>
#include "SPIHelper.h"

class SPIClassRP2040 : public arduino::HardwareSPI {
public:
    SPIClassRP2040(spi_inst_t *spi, pin_size_t rx, pin_size_t cs, pin_size_t sck, pin_size_t tx);

    // Send or receive 8- or 16-bit data.  Returns read back value
    byte transfer(uint8_t data) override;
    uint16_t transfer16(uint16_t data) override;

    // Sends buffer in 8 bit chunks.  Overwrites buffer with read data
    void transfer(void *buf, size_t count) override;

    // Sends one buffer and receives into another, much faster! can set rx or txbuf to nullptr
    void transfer(const void *txbuf, void *rxbuf, size_t count) override;

    // DMA/asynchronous transfers.  Do not combime with synchronous runs or bad stuff will happen
    // All buffers must be valid for entire DMA and not touched until `finished()` returns true.
    bool transferAsync(const void *send, void *recv, size_t bytes);
    bool finishedAsync(); // Call to check if the async operations is completed and the buffer can be reused/read
    void abortAsync(); // Cancel an outstanding async operation


    // Call before/after every complete transaction
    void beginTransaction(SPISettings settings) override;
    void endTransaction(void) override;

    // Assign pins, call before begin()
    bool setRX(pin_size_t pin);
    inline bool setMISO(pin_size_t pin) {
        return setRX(pin);
    }
    bool setCS(pin_size_t pin);
    bool setSCK(pin_size_t pin);
    bool setTX(pin_size_t pin);
    inline bool setMOSI(pin_size_t pin) {
        return setTX(pin);
    }

    // Call once to init/deinit SPI class, select pins, etc.
    virtual void begin() override {
        begin(false);
    }
    void begin(bool hwCS);
    void end() override;

    // Deprecated - do not use!
    void setBitOrder(BitOrder order) __attribute__((deprecated));
    void setDataMode(uint8_t uc_mode) __attribute__((deprecated));
    void setClockDivider(uint8_t uc_div) __attribute__((deprecated));

    // List of GPIO IRQs to disable during a transaction
    virtual void usingInterrupt(int interruptNumber) override {
        _helper.usingInterrupt(interruptNumber);
    }

    virtual void notUsingInterrupt(int interruptNumber) override {
        _helper.notUsingInterrupt(interruptNumber);
    }
    virtual void attachInterrupt() override { /* noop */ }
    virtual void detachInterrupt() override { /* noop */ }

private:
    void adjustBuffer(const void *s, void *d, size_t cnt, bool by16);

    spi_inst_t *_spi;
    SPISettings _spis;
    pin_size_t _RX, _TX, _SCK, _CS;
    bool _hwCS;
    bool _running; // SPI port active
    bool _initted; // Transaction begun

    // DMA
    int _channelDMA;
    int _channelSendDMA;
    uint8_t *_dmaBuffer = nullptr;
    int _dmaBytes;
    uint8_t *_rxFinalBuffer;
    uint32_t _dummy;
    SPIHelper _helper;
};

extern SPIClassRP2040 SPI;
extern SPIClassRP2040 SPI1;
