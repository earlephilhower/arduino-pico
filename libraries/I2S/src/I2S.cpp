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


I2S::I2S(PinMode direction) {
    _running = false;
    _bps = 16;
    _writtenHalf = false;
    _pinBCLK = 26;
    _pinDOUT = 28;
    _freq = 48000;
    _arb = nullptr;
    _isOutput = direction == OUTPUT;
    _cb = nullptr;
    _buffers = 8;
    _bufferWords = 16;
    _silenceSample = 0;
}

I2S::~I2S() {
}

bool I2S::setBCLK(pin_size_t pin) {
    if (_running || (pin > 28)) {
        return false;
    }
    _pinBCLK = pin;
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
        float bitClk = _freq * _bps * 2.0 /* channels */ * 2.0 /* edges per clock */;
        pio_sm_set_clkdiv(_pio, _sm, (float)clock_get_hz(clk_sys) / bitClk);
    }
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

bool I2S::begin() {
    _running = true;
    _hasPeeked = false;
    int off = 0;
    _i2s = new PIOProgram(_isOutput ? &pio_i2s_out_program : &pio_i2s_in_program);
    _i2s->prepare(&_pio, &_sm, &off);
    if (_isOutput) {
        pio_i2s_out_program_init(_pio, _sm, off, _pinDOUT, _pinBCLK, _bps);
    } else {
        pio_i2s_in_program_init(_pio, _sm, off, _pinDOUT, _pinBCLK, _bps);
    }
    setFrequency(_freq);
    if (_bps == 8) {
        uint8_t a = _silenceSample & 0xff;
        _silenceSample = (a << 24) | (a << 16) | (a << 8) | a;
    } else if (_bps == 16) {
        uint16_t a = _silenceSample & 0xffff;
        _silenceSample = (a << 16) | a;
    }
    _arb = new AudioRingBuffer(_buffers, _bufferWords, _silenceSample, _isOutput ? OUTPUT : INPUT);
    _arb->begin(pio_get_dreq(_pio, _sm, _isOutput), _isOutput ? &_pio->txf[_sm] : (volatile void*)&_pio->rxf[_sm]);
    _arb->setCallback(_cb);
    pio_sm_set_enabled(_pio, _sm, true);

    return true;
}

void I2S::end() {
    _running = false;
    delete _arb;
    _arb = nullptr;
    delete _i2s;
    _i2s = nullptr;
}

int I2S::available() {
    if (!_running || _isOutput) {
        return 0;
    }
    return _arb->available();
}

int I2S::read() {
    if (!_running || _isOutput) {
        return 0;
    }

    if (_hasPeeked) {
        _hasPeeked = false;
        return _peekSaved;
    }

    if (_wasHolding <= 0) {
        read(&_holdWord, true);
        _wasHolding = 32;
    }

    int ret;
    switch (_bps) {
    case 8:
        ret = _holdWord >> 24;
        _holdWord <<= 8;
        _wasHolding -= 8;
        return ret;
    case 16:
        ret = _holdWord >> 16;
        _holdWord <<=  16;
        _wasHolding -= 32;
        return ret;
    case 24:
    case 32:
    default:
        ret = _holdWord;
        _wasHolding = 0;
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
        if (_wasHolding >= 24) {
            auto ret = write(_holdWord, true);
            _holdWord = 0;
            _wasHolding = 0;
            return ret;
        } else {
            _holdWord <<= 8;
            _wasHolding += 8;
            return 1;
        }
    case 16:
        _holdWord |= s & 0xffff;
        if (_wasHolding) {
            auto ret = write(_holdWord, true);
            _holdWord = 0;
            _wasHolding = 0;
            return ret;
        } else {
            _holdWord <<= 16;
            _wasHolding = 16;
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
    if (_wasHolding) {
        *l = (_holdWord >> 8) & 0xff;
        *r = (_holdWord >> 0) & 0xff;
        _wasHolding = 0;
    } else {
        read(&_holdWord, true);
        _wasHolding = 16;
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
    if (size & 0x3) {
        return 0;
    }
    size_t writtenSize = 0;
    int32_t *p = (int32_t *)buffer;
    while (size) {
        if (!write((int32_t)*p)) {
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
    return _arb->available();
}
