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
#include <AudioBufferManager.h>
#include <AudioOutputBase.h>

class I2S : public Stream, public AudioOutputBase {
public:
    I2S(PinMode direction = OUTPUT, pin_size_t bclk = 26, pin_size_t data = 28, pin_size_t mclk = 25, pin_size_t data_rx = 22);
    virtual ~I2S();

    bool setBCLK(pin_size_t pin);
    bool setDATA(pin_size_t pin);
    bool setDOUT(pin_size_t pin);
    bool setDIN(pin_size_t pin);
    bool setMCLK(pin_size_t pin);
    virtual bool setBitsPerSample(int bps) override;
    virtual bool setBuffers(size_t buffers, size_t bufferWords, int32_t silenceSample = 0) override;
    virtual bool setFrequency(int newFreq) override;
    virtual bool setStereo(bool stereo = true) override {
        return stereo;
    }
    bool setLSBJFormat();
    bool setTDMFormat();
    bool setTDMChannels(int channels);
    bool swapClocks();
    bool setMCLKmult(int mult);
    bool setSysClk(int samplerate);

    bool begin(long sampleRate) {
        setFrequency(sampleRate);
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

    // From the AR
    bool getOverUnderflow() {
        if (!_running) {
            return false;
        } else {
            return _isOutput ? _arbOutput->getOverUnderflow() : _arbInput->getOverUnderflow();
        }
    }
    bool getOverflow() {
        if (!_running || !_isInput) {
            return false;
        } else {
            return _arbInput->getOverUnderflow();
        }
    }
    virtual bool getUnderflow() override {
        if (!_running || !_isOutput) {
            return false;
        } else {
            return _arbOutput->getOverUnderflow();
        }
    }

    // Try and make I2S::write() do what makes sense, namely write
    // one sample (L or R) at the I2S configured bit width
    virtual size_t write(uint8_t s) override {
        return _writeNatural(s & 0xff);
    }
    size_t write(int8_t s) {
        return write((uint8_t)s);
    }
    size_t write(uint16_t s) {
        return _writeNatural(s & 0xffff);
    }
    size_t write(int16_t s) {
        return write((uint16_t)s);
    }
    size_t write(uint32_t s) {
        return _writeNatural(s);
    }
    size_t write(int32_t s) {
        return write((uint32_t)s);
    }

    // Write 32 bit value to port, user responsible for packing/alignment, etc.
    size_t write(int32_t val, bool sync);

    // Write sample to I2S port, will block until completed
    size_t write8(int8_t l, int8_t r);
    size_t write16(int16_t l, int16_t r);
    size_t write24(int32_t l, int32_t r); // Note that 24b must have values left-aligned (i.e. 0xABCDEF00)
    size_t write32(int32_t l, int32_t r);

    // Read 32 bit value to port, user responsible for packing/alignment, etc.
    size_t read(int32_t *val, bool sync);

    // Read samples from I2S port, will block until data available
    bool read8(int8_t *l, int8_t *r);
    bool read16(int16_t *l, int16_t *r);
    bool read24(int32_t *l, int32_t *r); // Note that 24b reads will be left-aligned (see above)
    bool read32(int32_t *l, int32_t *r);

    // Read samples into buffer
    size_t read(uint8_t *buffer, size_t size);

    // Note that these callback are called from **INTERRUPT CONTEXT** and hence
    // should be in RAM, not FLASH, and should be quick to execute.
    void onTransmit(void(*)(void));
    void onTransmit(void(*)(void *), void *);
    void onReceive(void(*)(void));
    void onReceive(void(*)(void *), void *);

private:
    pin_size_t _pinBCLK;
    pin_size_t _pinDOUT;
    pin_size_t _pinDIN;
    pin_size_t _pinMCLK;
    int _bps;
    int _freq;
    int _multMCLK;
    size_t _buffers;
    size_t _bufferWords;
    int32_t _silenceSample;
    bool _isLSBJ;
    bool _isTDM;
    int _tdmChannels;
    bool _isInput;
    bool _isOutput;
    bool _swapClocks;
    bool _MCLKenabled;

    bool _running;

    bool _hasPeeked;
    int32_t _peekSaved;

    size_t _writeNatural(int32_t s);
    uint32_t _writtenData;
    bool _writtenHalf;

    int32_t _holdWord = 0;
    int _isHolding = 0;

    void (*_cbInput)();
    void (*_cbdInput)(void *);
    void *_cbdataInput;

    void (*_cbOutput)();
    void (*_cbdOutput)(void *);
    void *_cbdataOutput;

    void MCLKbegin();

    AudioBufferManager *_arbInput;
    AudioBufferManager *_arbOutput;
    PIOProgram *_i2s;
    PIOProgram *_i2sMCLK;
    PIO _pio, _pioMCLK;
    int _sm, _smMCLK;

    static const int I2SSYSCLK_44_1 = 135600; // 44.1, 88.2 kHz sample rates
    static const int I2SSYSCLK_8 = 153600;  // 8k, 16, 32, 48, 96, 192 kHz
};
