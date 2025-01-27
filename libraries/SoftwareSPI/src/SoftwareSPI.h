/*
    PIO-based SPI Master library for the Raspberry Pi Pico RP2040

    Copyright (c) 2025 Earle F. Philhower, III <earlephilhower@yahoo.com>

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
#include <map>

class SoftwareSPI : public arduino::HardwareSPI {
public:
/**
@brief Create a PIO-based SPI instance

@param [in] sck SCK GPIO
@param [in] miso MISO GPIO
@param [in] mosi MOSI GPIO
@param [in] cs Optional CS pin for HW CS, must be SCK+1
*/
    SoftwareSPI(pin_size_t sck, pin_size_t miso, pin_size_t mosi, pin_size_t cs = -1);

    // Send or receive 8- or 16-bit data.  Returns read back value
    byte transfer(uint8_t data) override;
    uint16_t transfer16(uint16_t data) override;

    // Sends buffer in 8 bit chunks.  Overwrites buffer with read data
    void transfer(void *buf, size_t count) override;

    // Sends one buffer and receives into another, much faster! can set rx or txbuf to nullptr
    void transfer(const void *txbuf, void *rxbuf, size_t count) override;

    // Call before/after every complete transaction
    void beginTransaction(SPISettings settings) override;
    void endTransaction(void) override;

    // Assign pins, call before begin()
    bool setMISO(pin_size_t pin);
    inline bool setRX(pin_size_t pin) {
        return setMISO(pin);
    }
    bool setCS(pin_size_t pin);
    bool setSCK(pin_size_t pin);
    bool setMOSI(pin_size_t pin);
    inline bool setTX(pin_size_t pin) {
        return setMOSI(pin);
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
        _usingIRQs.insert({interruptNumber, 0});
    }
    virtual void notUsingInterrupt(int interruptNumber) override {
        _usingIRQs.erase(interruptNumber);
    }
    virtual void attachInterrupt() override { /* noop */ }
    virtual void detachInterrupt() override { /* noop */ }

private:
    spi_cpol_t cpol();
    spi_cpha_t cpha();
    uint8_t reverseByte(uint8_t b);
    uint16_t reverse16Bit(uint16_t w);
    void adjustBuffer(const void *s, void *d, size_t cnt, bool by16);
    void _adjustPIO(int bits);

    PIOProgram *_spi;
    PIO _pio;
    int _sm;
    int _off;

    SPISettings _spis;
    pin_size_t _sck, _miso, _mosi, _cs;
    bool _hwCS;
    bool _running; // SPI port active
    bool _initted; // Transaction begun
    int _bits;

    std::map<int, int> _usingIRQs;
};
