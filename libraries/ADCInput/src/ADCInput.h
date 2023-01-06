/*
    ADCInput
    Records ADC values (i.e. microphone, sensors) and presents an I2S-like
    callback/immediate read interface

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
#include "AudioBufferManager.h"

class ADCInput : public Stream {
public:
    ADCInput(pin_size_t pin0, pin_size_t pin1 = 255, pin_size_t pin2 = 255, pin_size_t pin3 = 255);
    virtual ~ADCInput();

    bool setBuffers(size_t buffers, size_t bufferWords);
    bool setFrequency(int newFreq);
    bool setPins(pin_size_t pin0, pin_size_t pin1 = 255, pin_size_t pin2 = 255, pin_size_t pin3 = 255);

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

    // from Print, not supported
    virtual size_t write(const uint8_t *buffer, size_t size) override {
        (void) buffer;
        (void) size;
        return -1;
    }
    virtual size_t write(uint8_t x) override {
        (void) x;
        return -1;
    }
    virtual int availableForWrite() override {
        return 0;
    }

    // Note that these callback are called from **INTERRUPT CONTEXT** and hence
    // should be in RAM, not FLASH, and should be quick to execute.
    void onReceive(void(*)(void));

private:
    uint32_t _pinMask;

    int _freq;

    size_t _buffers;
    size_t _bufferWords;

    bool _running;
    void (*_cb)();

    bool _hasPeeked;
    uint32_t _peekSaved;
    uint32_t _holdWord = 0;
    int _isHolding = 0;

    int _mask(pin_size_t pin);

    AudioBufferManager *_arb;
};
