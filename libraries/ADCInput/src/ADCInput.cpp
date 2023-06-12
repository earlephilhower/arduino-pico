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

#include <Arduino.h>
#include "ADCInput.h"
#include <hardware/adc.h>

ADCInput::ADCInput(pin_size_t p0, pin_size_t p1, pin_size_t p2, pin_size_t p3) {
    _running = false;
    setPins(p0, p1, p2, p3);
    _freq = 48000;
    _arb = nullptr;
    _cb = nullptr;
    _buffers = 8;
    _bufferWords = 0;
}

ADCInput::~ADCInput() {
    end();
}

bool ADCInput::setBuffers(size_t buffers, size_t bufferWords) {
    if (_running || (buffers < 3) || (bufferWords < 8)) {
        return false;
    }
    _buffers = buffers;
    _bufferWords = bufferWords;
    return true;
}

int ADCInput::_mask(pin_size_t p) {
    switch (p) {
    case 26: return 1;
    case 27: return 2;
    case 28: return 4;
    case 29: return 8;
    default: return 0;
    }
}

bool ADCInput::setPins(pin_size_t pin0, pin_size_t pin1, pin_size_t pin2, pin_size_t pin3) {
    if (_running) {
        return false;
    }
    _pinMask = _mask(pin0) | _mask(pin1) | _mask(pin2) | _mask(pin3);
    return true;
}

bool ADCInput::setFrequency(int newFreq) {
    _freq = newFreq * __builtin_popcount(_pinMask); // Want to sample all channels at given frequency
    adc_set_clkdiv(48000000.0f / _freq - 1.0f);
    return true;
}

void ADCInput::onReceive(void(*fn)(void)) {
    _cb = fn;
    if (_running) {
        _arb->setCallback(_cb);
    }
}

bool ADCInput::begin() {
    if (_running) {
        return false;
    }

    _running = true;

    _isHolding = 0;

    if (!_bufferWords) {
        _bufferWords = 16;
    }

    // Set up the GPIOs to go to ADC
    adc_init();
    int cnt = 0;
    for (int mask = 1, pin = 26; pin <= 29; mask <<= 1, pin++) {
        if (_pinMask & mask) {
            if (!cnt) {
                adc_select_input(pin - 26);
            }
            cnt++;
            adc_gpio_init(pin);
        }
    }
    adc_set_round_robin(_pinMask);
    adc_fifo_setup(true, true, 1, false, false);

    setFrequency(_freq);

    _arb = new AudioBufferManager(_buffers, _bufferWords, 0, INPUT, DMA_SIZE_16);
    if (!_arb->begin(DREQ_ADC, (volatile void*)&adc_hw->fifo)) {
        delete _arb;
        _arb = nullptr;
        return false;
    }
    _arb->setCallback(_cb);

    adc_fifo_drain();

    adc_run(true);

    return true;
}

void ADCInput::end() {
    if (_running) {
        _running = false;
        delete _arb;
        _arb = nullptr;
    }
    adc_run(false);
    adc_fifo_drain();
}

int ADCInput::available() {
    if (!_running) {
        return 0;
    } else {
        return _arb->available();
    }
}

int ADCInput::read() {
    if (!_running) {
        return -1;
    }

    if (_hasPeeked) {
        _hasPeeked = false;
        return _peekSaved;
    }

    if (_isHolding <= 0) {
        _arb->read(&_holdWord, true);
        _isHolding = 32;
    }

    int ret = _holdWord & 0x0fff;
    _holdWord >>= 16;
    _isHolding -= 16;
    return ret;
}

int ADCInput::peek() {
    if (!_running) {
        return -1;
    }
    if (!_hasPeeked) {
        _peekSaved = read();
        _hasPeeked = true;
    }
    return _peekSaved;
}

void ADCInput::flush() {
    if (_running) {
        _arb->flush();
    }
}
