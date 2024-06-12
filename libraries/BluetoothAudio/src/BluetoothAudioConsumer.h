/*
    Bluetooth A2DP audio stream consumer

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

#pragma once

class A2DPSink;

class BluetoothAudioConsumer_ {
public:
    BluetoothAudioConsumer_() {
        _state = STATE_OFF;
    }
    virtual ~BluetoothAudioConsumer_() {
        /* noop */
    }
    virtual bool init(uint8_t channels, uint32_t samplerate, A2DPSink *a2dpSink) = 0;
    virtual void setVolume(uint8_t gain) = 0;
    virtual void startStream() = 0;
    virtual void stopStream() = 0;
    virtual void close() = 0;
protected:
    typedef enum { STATE_OFF = 0, STATE_INITIALIZED, STATE_STREAMING } State;
    A2DPSink *_a2dpSink;
    State _state;
    uint8_t _gain;
    int _channels;
    int _samplerate;
};
