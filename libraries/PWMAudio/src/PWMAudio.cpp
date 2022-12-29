/*
    PWMAudio
    Plays a 16b audio stream on a user defined pin using PWM

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
#include "PWMAudio.h"
#include <hardware/pwm.h>


PWMAudio::PWMAudio(pin_size_t pin) {
    _running = false;
    _writtenHalf = false;
    _pin = pin;
    _freq = 48000;
    _arb = nullptr;
    _cb = nullptr;
    _buffers = 8;
    _bufferWords = 0;
}

PWMAudio::~PWMAudio() {
}

bool PWMAudio::setBuffers(size_t buffers, size_t bufferWords) {
    if (_running || (buffers < 3) || (bufferWords < 8)) {
        return false;
    }
    _buffers = buffers;
    _bufferWords = bufferWords;
    return true;
}

bool PWMAudio::setFrequency(int newFreq) {
    if (_running) {
        return false;
    }
    _freq = newFreq;
    return true;
}

void PWMAudio::onTransmit(void(*fn)(void)) {
    _cb = fn;
    if (_running) {
        _arb->setCallback(_cb);
    }
}

bool PWMAudio::begin() {
    _running = true;

    if (!_bufferWords) {
        _bufferWords = 16;
    }

    // Figure out the scale factor for PWM values
    float fPWM = 65535.0 * _freq; // ideal

    if (fPWM > clock_get_hz(clk_sys)) {
        // Need to downscale the range to hit the frequency target
        float pwmMax = (float) clock_get_hz(clk_sys) /(float) _freq;
        _pwmScale = pwmMax;
        fPWM = clock_get_hz(clk_sys);
    } else {
        _pwmScale = 1 << 16;
    }

    pwm_config c = pwm_get_default_config();
    pwm_config_set_clkdiv(&c, clock_get_hz(clk_sys) / fPWM);
    pwm_config_set_wrap(&c, _pwmScale);
    pwm_init(pwm_gpio_to_slice_num(_pin), &c, true);
    gpio_set_function(_pin, GPIO_FUNC_PWM);
    pwm_set_gpio_level(_pin, (0x8000 * _pwmScale) >> 16);

    uint32_t ccAddr = PWM_BASE + PWM_CH0_CC_OFFSET + pwm_gpio_to_slice_num(_pin) * 20;

    _arb = new AudioBufferManager(_buffers, _bufferWords, 0x80008000, OUTPUT);
    _arb->begin(pwm_get_dreq(pwm_gpio_to_slice_num(_pin)), (volatile void*)ccAddr);
    _arb->setCallback(_cb);

    return true;
}

void PWMAudio::end() {
    _running = false;
    delete _arb;
    _arb = nullptr;
}

int PWMAudio::available() {
    return availableForWrite(); // Do what I mean, not what I say
}

int PWMAudio::read() {
    return -1;
}

int PWMAudio::peek() {
    return -1;
}

void PWMAudio::flush() {
    if (_running) {
        _arb->flush();
    }
}

int PWMAudio::availableForWrite() {
    if (!_running) {
        return 0;
    }
    return _arb->available();
}

size_t PWMAudio::write(int16_t val, bool sync) {
    if (!_running) {
        return 0;
    }
    // Go from signed -32K...32K to unsigned 0...64K
    uint32_t sample = (uint32_t) (val + 0x8000); 
    // Adjust to the real range
    sample *= _pwmScale;
    sample >>= 16;

    if (!_writtenHalf) {
        _holdWord = sample & 0xffff;
        _writtenHalf = true;
        return 1;
    }
    _holdWord = (_holdWord & 0xffff) | (sample << 16);
    auto ret = _arb->write(_holdWord, sync);
    if (ret) {
        _writtenHalf = false;
    }
    return ret;
}

size_t PWMAudio::write(const uint8_t *buffer, size_t size) {
    // We can only write 16-bit chunks here
    if (size & 0x1) {
        return 0;
    }
    size_t writtenSize = 0;
    int16_t *p = (int16_t *)buffer;
    while (size) {
        if (!write((int16_t)*p)) {
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
