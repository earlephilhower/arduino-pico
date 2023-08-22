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
#include <Arduino.h>
#include "I2S.h"
#include "pio_i2s.pio.h"
#include <pico/stdlib.h>


I2S::I2S(PinMode direction) {
    _running = false;
    _bps = 16;
    _writtenHalf = false;
    _isOutput = direction == OUTPUT;
    _pinBCLK = 26;
    _pinDOUT = 28;
    _pinMCLK = 25;
    _MCLKenabled = false;
#ifdef PIN_I2S_BCLK
    _pinBCLK = PIN_I2S_BCLK;
#endif

#ifdef PIN_I2S_DOUT
    if (_isOutput) {
        _pinDOUT = PIN_I2S_DOUT;
    }
#endif

#ifdef PIN_I2S_DIN
    if (!_isOutput) {
        _pinDOUT = PIN_I2S_DIN;
    }
#endif
    _freq = 48000;
    _arb = nullptr;
    _cb = nullptr;
    _buffers = 6;
    _bufferWords = 0;
    _silenceSample = 0;
    _isLSBJ = false;
    _swapClocks = false;
    _multMCLK = 256;
}

I2S::~I2S() {
    end();
}

bool I2S::setBCLK(pin_size_t pin) {
    if (_running || (pin > 28)) {
        return false;
    }
    _pinBCLK = pin;
    return true;
}


bool I2S::setMCLK(pin_size_t pin) {
    if (_running || (pin > 28)) {
        return false;
    }
    _pinMCLK = pin;
    return true;
}
bool I2S::setDATA(pin_size_t pin) {
    if (_running || (pin > 29)) {
        return false;
    }
    _pinDOUT = pin;
    return true;
}

bool I2S::setBitsPerSample(int bps) {
    if (_running || ((bps != 8) && (bps != 16) && (bps != 24) && (bps != 32))) {
        return false;
    }
    _bps = bps;
    return true;
}

bool I2S::setBuffers(size_t buffers, size_t bufferWords, int32_t silenceSample) {
    if (_running || (buffers < 3) || (bufferWords < 8)) {
        return false;
    }
    _buffers = buffers;
    _bufferWords = bufferWords;
    _silenceSample = silenceSample;
    return true;
}

bool I2S::setFrequency(int newFreq) {
    _freq = newFreq;
    if (_running) {
        if (_MCLKenabled) {
            int bitClk = _freq * _bps * 2.0 /* channels */ * 2.0 /* edges per clock */;
            pio_sm_set_clkdiv_int_frac(_pio, _sm, clock_get_hz(clk_sys) / bitClk, 0);
        } else {
            float bitClk = _freq * _bps * 2.0 /* channels */ * 2.0 /* edges per clock */;
            pio_sm_set_clkdiv(_pio, _sm, (float)clock_get_hz(clk_sys) / bitClk);
        }
    }
    return true;
}

bool I2S::setSysClk(int samplerate) { // optimise sys_clk for desired samplerate
    if (samplerate % 11025 == 0) {
        set_sys_clock_khz(I2SSYSCLK_44_1, false); // 147.6 unsuccessful - no I2S no USB
        return true;
    }
    if (samplerate % 8000 == 0) {
        set_sys_clock_khz(I2SSYSCLK_8, false);
        return true;
    }
    return false;
}

bool I2S::setMCLKmult(int mult) {
    if (_running || !_isOutput) {
        return false;
    }
    if ((mult % 64) == 0) {
        _MCLKenabled = true;
        _multMCLK = mult;
        return true;
    }
    return false;
}

bool I2S::setLSBJFormat() {
    if (_running || !_isOutput) {
        return false;
    }
    _isLSBJ = true;
    return true;
}

bool I2S::swapClocks() {
    if (_running || !_isOutput) {
        return false;
    }
    _swapClocks = true;
    return true;
}

void I2S::onTransmit(void(*fn)(void)) {
    if (_isOutput) {
        _cb = fn;
        if (_running) {
            _arb->setCallback(_cb);
        }
    }
}

void I2S::onReceive(void(*fn)(void)) {
    if (!_isOutput) {
        _cb = fn;
        if (_running) {
            _arb->setCallback(_cb);
        }
    }
}

void I2S::MCLKbegin() {
    int off = 0;
    _i2sMCLK = new PIOProgram(&pio_i2s_mclk_program);
    _i2sMCLK->prepare(&_pioMCLK, &_smMCLK, &off); // not sure how to use the same PIO
    pio_i2s_MCLK_program_init(_pioMCLK, _smMCLK, off, _pinMCLK);
    int mClk = _multMCLK * _freq * 2.0 /* edges per clock */;
    pio_sm_set_clkdiv_int_frac(_pioMCLK, _smMCLK, clock_get_hz(clk_sys) / mClk, 0);
    pio_sm_set_enabled(_pioMCLK, _smMCLK, true);
}

bool I2S::begin() {
    _running = true;
    _hasPeeked = false;
    _isHolding = 0;
    int off = 0;
    if (!_swapClocks) {
        _i2s = new PIOProgram(_isOutput ? (_isLSBJ ? &pio_lsbj_out_program : &pio_i2s_out_program) : &pio_i2s_in_program);
    } else {
        _i2s = new PIOProgram(_isOutput ? (_isLSBJ ? &pio_lsbj_out_swap_program : &pio_i2s_out_swap_program) : &pio_i2s_in_swap_program);
    }
    if (!_i2s->prepare(&_pio, &_sm, &off)) {
        _running = false;
        delete _i2s;
        _i2s = nullptr;
        return false;
    }
    if (_isOutput) {
        if (_isLSBJ) {
            pio_lsbj_out_program_init(_pio, _sm, off, _pinDOUT, _pinBCLK, _bps, _swapClocks);
        } else {
            pio_i2s_out_program_init(_pio, _sm, off, _pinDOUT, _pinBCLK, _bps, _swapClocks);
        }
    } else {
        pio_i2s_in_program_init(_pio, _sm, off, _pinDOUT, _pinBCLK, _bps, _swapClocks);
    }
    setFrequency(_freq);
    if (_MCLKenabled) {
        MCLKbegin();
    }
    if (_bps == 8) {
        uint8_t a = _silenceSample & 0xff;
        _silenceSample = (a << 24) | (a << 16) | (a << 8) | a;
    } else if (_bps == 16) {
        uint16_t a = _silenceSample & 0xffff;
        _silenceSample = (a << 16) | a;
    }
    if (!_bufferWords) {
        _bufferWords = 64 * (_bps == 32 ? 2 : 1);
    }
    _arb = new AudioBufferManager(_buffers, _bufferWords, _silenceSample, _isOutput ? OUTPUT : INPUT);
    if (!_arb->begin(pio_get_dreq(_pio, _sm, _isOutput), _isOutput ? &_pio->txf[_sm] : (volatile void*)&_pio->rxf[_sm])) {
        _running = false;
        delete _arb;
        _arb = nullptr;
        delete _i2s;
        _i2s = nullptr;
        return false;
    }
    _arb->setCallback(_cb);
    pio_sm_set_enabled(_pio, _sm, true);

    return true;
}

void I2S::end() {
    if (_running) {
        if (_MCLKenabled) {
            pio_sm_set_enabled(_pioMCLK, _smMCLK, false);
            delete _i2sMCLK;
            _i2sMCLK = nullptr;
        }
        pio_sm_set_enabled(_pio, _sm, false);
        _running = false;
        delete _arb;
        _arb = nullptr;
        delete _i2s;
        _i2s = nullptr;
    }
}

int I2S::available() {
    if (!_running) {
        return 0;
    } else {
        auto avail = _arb->available();
        avail *= 4; // 4 samples per 32-bits
        if (_bps < 24 && !_isOutput) {
            avail += _isHolding / 8;
        }
        return avail;
    }
}

int I2S::read() {
    if (!_running || _isOutput) {
        return 0;
    }

    if (_hasPeeked) {
        _hasPeeked = false;
        return _peekSaved;
    }

    if (_isHolding <= 0) {
        read(&_holdWord, true);
        _isHolding = 32;
    }

    int ret;
    switch (_bps) {
    case 8:
        ret = _holdWord >> 24;
        _holdWord <<= 8;
        _isHolding -= 8;
        return ret;
    case 16:
        ret = _holdWord >> 16;
        _holdWord <<=  16;
        _isHolding -= 16;
        return ret;
    case 24:
    case 32:
    default:
        ret = _holdWord;
        _isHolding = 0;
        return ret;
    }
}

int I2S::peek() {
    if (!_running || _isOutput) {
        return 0;
    }
    if (!_hasPeeked) {
        _peekSaved = read();
        _hasPeeked = true;
    }
    return _peekSaved;
}

void I2S::flush() {
    if (_running) {
        _arb->flush();
    }
}

size_t I2S::_writeNatural(int32_t s) {
    if (!_running || !_isOutput) {
        return 0;
    }
    switch (_bps) {
    case 8:
        _holdWord |= s & 0xff;
        if (_isHolding >= 24) {
            auto ret = write(_holdWord, true);
            _holdWord = 0;
            _isHolding = 0;
            return ret;
        } else {
            _holdWord <<= 8;
            _isHolding += 8;
            return 1;
        }
    case 16:
        _holdWord |= s & 0xffff;
        if (_isHolding) {
            auto ret = write(_holdWord, true);
            _holdWord = 0;
            _isHolding = 0;
            return ret;
        } else {
            _holdWord <<= 16;
            _isHolding = 16;
            return 1;
        }
    case 24:
    case 32:
    default:
        return write(s, true);
    }
}

size_t I2S::write(int32_t val, bool sync) {
    if (!_running || !_isOutput) {
        return 0;
    }
    return _arb->write(val, sync);
}

size_t I2S::write8(int8_t l, int8_t r) {
    if (!_running || !_isOutput) {
        return 0;
    }
    int16_t o = (l << 8) | (r & 0xff);
    return write((int16_t) o);
}

size_t I2S::write16(int16_t l, int16_t r) {
    if (!_running || !_isOutput) {
        return 0;
    }
    int32_t o = (l << 16) | (r & 0xffff);
    return write((int32_t)o, true);
}

size_t I2S::write24(int32_t l, int32_t r) {
    return write32(l, r);
}

size_t I2S::write32(int32_t l, int32_t r) {
    if (!_running || !_isOutput) {
        return 0;
    }
    write((int32_t)l);
    write((int32_t)r);
    return 1;
}

size_t I2S::read(int32_t *val, bool sync) {
    if (!_running || _isOutput) {
        return 0;
    }
    return _arb->read((uint32_t *)val, sync);
}

bool I2S::read8(int8_t *l, int8_t *r) {
    if (!_running || _isOutput) {
        return false;
    }
    if (_isHolding) {
        *l = (_holdWord >> 8) & 0xff;
        *r = (_holdWord >> 0) & 0xff;
        _isHolding = 0;
    } else {
        read(&_holdWord, true);
        _isHolding = 16;
        *l = (_holdWord >> 24) & 0xff;
        *r = (_holdWord >> 16) & 0xff;
    }
    return true;
}

bool I2S::read16(int16_t *l, int16_t *r) {
    if (!_running || _isOutput) {
        return false;
    }
    int32_t o;
    read(&o, true);
    *l = (o >> 16) & 0xffff;
    *r = (o >> 0) & 0xffff;
    return true;
}

bool I2S::read24(int32_t *l, int32_t *r) {
    if (!_running || _isOutput) {
        return false;
    }
    read32(l, r);
    // 24-bit samples are read right-aligned, so left-align them to keep the binary point between 33.32
    *l <<= 8;
    *r <<= 8;
    return true;
}

bool I2S::read32(int32_t *l, int32_t *r) {
    if (!_running || _isOutput) {
        return false;
    }
    read(l, true);
    read(r, true);
    return true;
}

size_t I2S::write(const uint8_t *buffer, size_t size) {
    // We can only write 32-bit chunks here
    if (size & 0x3 || !_running || !_isOutput) {
        return 0;
    }

    size_t writtenSize = 0;
    uint32_t *p = (uint32_t *)buffer;
    while (size) {
        if (!_arb->write(*p, false)) {
            // Blocked, stop write here
            return writtenSize;
        } else {
            p++;
            size -= 4;
            writtenSize += 4;
        }
    }
    return writtenSize;
}

int I2S::availableForWrite() {
    if (!_running || !_isOutput) {
        return 0;
    }
    return available();
}
