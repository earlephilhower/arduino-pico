/*
 * I2S Master library for the Raspberry Pi Pico RP2040
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

#include <Arduino.h>
#include "I2S.h"

I2SClass::I2SClass() {
    _running = false;
    _pool = nullptr;
    _curBuff = nullptr;
    _bps = 0;
    _writtenHalf = false;
}

bool I2SClass::begin(long sampleRate, pin_size_t sck, pin_size_t data) {
    if (_running) {
        return false;
    }

    bzero(&_audio_format, sizeof(_audio_format));
    _audio_format.sample_freq = (uint32_t)sampleRate;
    _audio_format.format = AUDIO_BUFFER_FORMAT_PCM_S16;
    _audio_format.channel_count = 2;

    bzero(&_producer_format, sizeof(_producer_format));
    _producer_format.format = &_audio_format;
    _producer_format.sample_stride = 4;

    if (!_pool) {
        _pool = audio_new_producer_pool(&_producer_format, 3, 256);
    }

    bzero(&_config, sizeof(_config));
    _config.data_pin = data;
    _config.clock_pin_base = sck;
    _config.dma_channel = 0;
    _config.pio_sm = 1;

    if (!audio_i2s_setup(&_audio_format, &_config)) {
        return false;
    }

    if (!audio_i2s_connect(_pool)) {
        return false;
    }

    audio_i2s_set_enabled(true);

    _curBuff = take_audio_buffer(_pool, true);
    _curBuff->sample_count = 0;

    _bps = 16;
    _running = true;
    return true;
}

void I2SClass::end() {
    if (_running) {
        audio_i2s_set_enabled(false);
        if (_curBuff) {
            release_audio_buffer(_pool, _curBuff);
            _curBuff = nullptr;
        }
    }
    _running = false;
    _bps = 0;
    _writtenHalf = false;
}

int I2SClass::availableForWrite() {
    if (!_running) {
        return 0;
    }
    // Can we get a whole new buffer to work with?
    if (!_curBuff) {
        _curBuff = take_audio_buffer(_pool, false);
        _curBuff->sample_count = 0;
    }
    if (!_curBuff) {
        return false;
    }
    return _curBuff->max_sample_count - _curBuff->sample_count;
}

void I2SClass::flush() {
    if (!_curBuff || !_curBuff->sample_count) {
        return;
    }
    give_audio_buffer(_pool, _curBuff);
    _curBuff = nullptr;
}

size_t I2SClass::write(uint8_t s) {
    return write((int16_t)s);
}

size_t I2SClass::write(const uint8_t *buffer, size_t size) {
    return write((const void *)buffer, size);
}

size_t I2SClass::write(int16_t s) {
    if (!_running) {
        return 0;
    }

    // Because our HW really wants 32b writes, store any 16b writes until another
    // 16b write comes in and then send the combined write down.
    if (_bps == 16) {
        if (_writtenHalf) {
            _writtenData <<= 16;
            _writtenData |= 0xffff & s;
            _writtenHalf = false;
            if (!_curBuff) {
                _curBuff = take_audio_buffer(_pool, true);
                _curBuff->sample_count = 0;
            }
            int32_t *samples = (int32_t *)_curBuff->buffer->bytes;
            samples[_curBuff->sample_count++] = _writtenData;
            if (_curBuff->sample_count == _curBuff->max_sample_count) {
                give_audio_buffer(_pool, _curBuff);
                _curBuff = nullptr;
            }
        } else {
            _writtenHalf = true;
            _writtenData = s & 0xffff;
        }
    }
    return 1;
}

// Mostly non-blocking
size_t I2SClass::write(const void *buffer, size_t size) {
    if (!_running) {
        return 0;
    }
    // We have no choice here because we need to write at least 1 byte...
    if (!_curBuff) {
        _curBuff = take_audio_buffer(_pool, true);
        _curBuff->sample_count = 0;
    }

    int32_t *inSamples = (int32_t *)_curBuff->buffer->bytes;
    int written = 0;
    int wantToWrite = size / 4;
    while (wantToWrite) {
        if (!_curBuff) {
            _curBuff = take_audio_buffer(_pool, false);
            if (_curBuff) {
                _curBuff->sample_count = 0;
            } else {
                break;
            }
        }

        int avail = _curBuff->max_sample_count - _curBuff->sample_count;
        int writeSize = (avail > wantToWrite) ? wantToWrite : avail;
        int32_t *samples = (int32_t *)_curBuff->buffer->bytes;
        memcpy(samples + _curBuff->sample_count, inSamples, writeSize * 4);
        _curBuff->sample_count += writeSize;
        inSamples += writeSize;
        written += writeSize;
        if (_curBuff->sample_count == _curBuff->max_sample_count) {
            give_audio_buffer(_pool, _curBuff);
            _curBuff = nullptr;
        }
    }
    return written;
}

I2SClass I2S;
