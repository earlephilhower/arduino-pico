/*
 * SPI Master library for the Raspberry Pi Pico RP2040
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

#ifndef _SPI_H_INCLUDED
#define _SPI_H_INCLUDED

#include <Arduino.h>
#include <api/HardwareSPI.h>
#include <hardware/spi.h>

class SPIClassRP2040 : public arduino::HardwareSPI {
public:
    SPIClassRP2040(spi_inst_t *spi);

    // Send or receive data in 8- or 16-bit chunks  Returns readback value
    byte transfer(uint8_t data);
    uint16_t transfer16(uint16_t data);
    // Sends buffer in 8 bit chunks.  Overwrites buffer with read data
    void transfer(void *buf, size_t count);

    // Call before/after every complete transaction
    void beginTransaction(SPISettings settings);
    void endTransaction(void);

    // Call once to init/deinit SPI class, select pins, etc.
    virtual void begin() { begin(false, D0); }
    void begin(bool hwCS, pin_size_t spiRX);
    void end();

    // Deprecated - do not use!
    void setBitOrder(BitOrder order) __attribute__((deprecated));
    void setDataMode(uint8_t uc_mode) __attribute__((deprecated));
    void setClockDivider(uint8_t uc_div) __attribute__((deprecated));

    // Unimplemented
    virtual void usingInterrupt(int interruptNumber) { (void) interruptNumber; }
    virtual void notUsingInterrupt(int interruptNumber) { (void) interruptNumber; }
    virtual void attachInterrupt() { /* noop */ }
    virtual void detachInterrupt() { /* noop */ }

private:
    spi_cpol_t cpol();
    spi_cpha_t cpha();
    uint8_t reverseByte(uint8_t b);
    uint16_t reverse16Bit(uint16_t w);
    void adjustBuffer(const void *s, void *d, size_t cnt, bool by16);

    spi_inst_t *_spi;
    SPISettings _spis;
    pin_size_t _pin;
    bool _hwCS;
    bool _initted;
};

extern SPIClassRP2040 SPI;
extern SPIClassRP2040 SPI1;

#endif

