/*
    PWMAudio
    Plays a signed 16b audio stream on a user defined pin using PWM

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
#include "AudioBufferManager.h"

class PWMAudio : public Stream {
public:
    PWMAudio(pin_size_t pin = 0, bool stereo = false);
    virtual ~PWMAudio();

    bool setBuffers(size_t buffers, size_t bufferWords);
    bool setFrequency(int newFreq);
    bool setPin(pin_size_t pin);
    bool setStereo(bool stereo = true);

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
    virtual size_t write(const uint8_t *buffer, size_t size) override;
    virtual int availableForWrite() override;

    virtual size_t write(uint8_t x) override {
        return write((int16_t) x, true);
    }

    // Write 16 bit value to port, user responsible for packing/alignment, etc.
    size_t write(int16_t val, bool sync = true);
    size_t write(int val, bool sync = true) {
        return write((int16_t) val, sync);
    }

    // Note that these callback are called from **INTERRUPT CONTEXT** and hence
    // should be in RAM, not FLASH, and should be quick to execute.
    void onTransmit(void(*)(void));

private:
    pin_size_t _pin;
    bool _stereo;

    int _freq;

    size_t _buffers;
    size_t _bufferWords;

    uint32_t _pwmScale;

    bool _running;

    bool _wasHolding;
    uint32_t _holdWord;

    void (*_cb)();

    AudioBufferManager *_arb;
};
