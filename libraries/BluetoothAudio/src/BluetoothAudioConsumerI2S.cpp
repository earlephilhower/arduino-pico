/*
    Bluetooth A2DP audio stream consumer - PWMAudio

    Copyright (c) 2024 Earle F. Philhower, III <earlephilhower@yahoo.com>

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

#include "BluetoothAudioConsumerI2S.h"
#include "A2DPSink.h"
#include <functional>

#define CCALLBACKNAME _BTA2DPI2S
#include <ctocppcallback.h>
#define NOPARAMCB(class, cbFcn) \
  (CCALLBACKNAME<void(void), __COUNTER__>::func = std::bind(&class::cbFcn, this), \
   static_cast<void (*)(void)>(CCALLBACKNAME<void(void), __COUNTER__ - 1>::callback))

bool BluetoothAudioConsumerI2S::init(uint8_t channels, uint32_t samplerate, A2DPSink *a2dpSink) {
    _channels = channels;
    _samplerate = samplerate;
    _a2dpSink = a2dpSink;
    _i2s->setBuffers(16, 64);
    _i2s->onTransmit(NOPARAMCB(BluetoothAudioConsumerI2S, fill));
    if (_i2s->begin(samplerate)) {
        _state = STATE_INITIALIZED;
        _gain = 64;
        return true;
    }
    return false;
}

void BluetoothAudioConsumerI2S::setVolume(uint8_t gain) {
    _gain = gain;
}

void BluetoothAudioConsumerI2S::startStream() {
    if (_state != STATE_INITIALIZED) {
        return;
    }
    _state = STATE_STREAMING;
}

void BluetoothAudioConsumerI2S::stopStream() {
    if (_state != STATE_STREAMING) {
        return;
    }
    _state = STATE_INITIALIZED;
}

void BluetoothAudioConsumerI2S::close() {
    if (_state == STATE_STREAMING) {
        stopStream();
    }
    _state = STATE_OFF;
    _i2s->end();
}

void BluetoothAudioConsumerI2S::fill() {
    int num_samples = _i2s->availableForWrite() / 2;
    int16_t buff[32 * 2];
    while (num_samples > 63) {
        _a2dpSink->playback_handler((int16_t *) buff, 32);
        num_samples -= 64;
        for (int i = 0; i < 64; i++) {
            buff[i] = (((int32_t)buff[i]) * _gain) >> 8;
        }
        _i2s->write((uint8_t *)buff, 64 * 2);
    }
}
