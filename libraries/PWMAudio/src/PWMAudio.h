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
#include <AudioOutputBase.h>
#include <AudioBufferManager.h>

class PWMAudio : public Stream, public AudioOutputBase {
public:
    PWMAudio(pin_size_t pin = 0, bool stereo = false);
    virtual ~PWMAudio();

    virtual bool setBuffers(size_t buffers, size_t bufferWords, int32_t silenceSample = 0) override;
    /*Sets the frequency of the PWM in hz*/
    bool setPWMFrequency(int newFreq);
    /*Sets the sample rate frequency in hz*/
    virtual bool setFrequency(int frequency) override;
    bool setPin(pin_size_t pin);
    virtual bool setStereo(bool stereo = true) override;
    virtual bool setBitsPerSample(int bits) override {
        return bits == 16;
    }

    bool begin(long sampleRate) {
        _sampleRate = sampleRate;
        return begin();
    }

    bool begin(long sampleRate, long PWMfrequency) {
        setPWMFrequency(PWMfrequency);
        _sampleRate = sampleRate;
        return begin();
    }

    virtual bool begin() override;
    virtual bool end() override;

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
    void onTransmit(void(*)(void *), void *data);

    bool getUnderflow() {
        if (!_running) {
            return false;
        } else {
            return _arb->getOverUnderflow();
        }
    }

private:
    pin_size_t _pin;
    bool _stereo;

    int _freq;
    int _sampleRate;

    int _pacer;

    size_t _buffers;
    size_t _bufferWords;

    uint32_t _pwmScale;

    bool _running;

    bool _wasHolding;
    uint32_t _holdWord;

    void (*_cb)();
    void (*_cbd)(void *);
    void *_cbdata;

    AudioBufferManager *_arb;

    /*An accurate but brute force method to find 16bit numerator and denominator.*/
    void find_pacer_fraction(int target, uint16_t *numerator, uint16_t *denominator);
};
