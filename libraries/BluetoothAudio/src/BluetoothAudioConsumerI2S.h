/*
    Bluetooth A2DP audio stream consumer - I2S

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

#include <I2S.h>
#include "BluetoothAudioConsumer.h"

class BluetoothAudioConsumerI2S : public BluetoothAudioConsumer_ {
public:
    BluetoothAudioConsumerI2S(I2S &i2s) : BluetoothAudioConsumer_() {
        _i2s = &i2s;
    }

    virtual bool init(uint8_t channels, uint32_t samplerate, A2DPSink *a2dpSink) override;
    virtual void setVolume(uint8_t gain) override;
    virtual void startStream() override;
    virtual void stopStream() override;
    virtual void close() override;

private:
    I2S *_i2s;
    void fill();
};
