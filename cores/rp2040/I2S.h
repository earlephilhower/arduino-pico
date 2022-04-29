/*
    I2SIn and I2SOut for Raspberry Pi Pico
    Implements one or more I2S interfaces using DMA

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
#include <Arduino.h>
#include <vector>
#include "AudioRingBuffer.h"

class I2S : public Stream {
public:
    I2S(PinMode direction);
    virtual ~I2S();

    bool setBCLK(pin_size_t pin);
    bool setDATA(pin_size_t pin);
    bool setBitsPerSample(int bps);
    bool setFrequency(int newFreq);

    bool begin(long sampleRate) {
        setFrequency(sampleRate);
        return begin();
    }

    bool begin();
    void end();

    // from Stream
    virtual int available() override;
    virtual int read() override;
    virtual int peek() override;
    virtual void flush() override;

    // from Print (see notes on write() methods below)
    virtual size_t write(uint8_t) override;
    virtual size_t write(const uint8_t *buffer, size_t size) override;
    virtual int availableForWrite() override;

    // Write 32 bit value to port, user responsbile for packing/alignment, etc.
    size_t write(uint32_t val, bool sync = true);

    // Write sample to I2S port, will block until completed
    size_t write8(uint8_t l, uint8_t r);
    size_t write16(uint16_t l, uint16_t r);
    size_t write24(uint32_t l, uint32_t r); // Note that 24b must have values left-aligned (i.e. 0xABCDEF00)
    size_t write32(uint32_t l, uint32_t r);

    // Read 32 bit value to port, user responsbile for packing/alignment, etc.
    size_t read(uint32_t *val, bool sync = true);

    // Read samples from I2S port, will block until data available
    bool read8(uint8_t *l, uint8_t *r);
    bool read16(uint16_t *l, uint16_t *r);
    bool read24(uint32_t *l, uint32_t *r); // Note that 24b reads will be left-aligned (see above)
    bool read32(uint32_t *l, uint32_t *r);

    // Note that these callback are called from **INTERRUPT CONTEXT** and hence
    // must be both stored in IRAM and not perform anything that's not legal in
    // an interrupt
    //void onTransmit(void(*)(void)); -- Not yet implemented, need to edit pico-extra to get callback
    //void onReceive(void(*)(void)); -- no I2S input yet

private:
    pin_size_t _pinBCLK;
    pin_size_t _pinDOUT;
    int _bps;
    int _freq;
    bool _isOutput;

    bool _running;

    // Support for ::write(x) on 16b quantities
    uint32_t _writtenData;
    bool _writtenHalf;

    uint32_t _holdWord = 0;
    bool _wasHolding = false;


    AudioRingBuffer *_arb;
    PIOProgram *_i2s;
    PIO _pio;
    int _sm;
};
