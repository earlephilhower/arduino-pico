/*
    Copyright (c) 2019 Arduino LLC.  All right reserved.

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
    See the GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#pragma once

#include <Arduino.h>
//#include <pinDefinitions.h>

#include "utility/PDMDoubleBuffer.h"

class PDMClass {
public:
    PDMClass(int dinPin, int clkPin, int pwrPin);
    virtual ~PDMClass();

    int begin(int channels, int sampleRate);
    void end();

    virtual int available();
    virtual int read(void* buffer, size_t size);

    void onReceive(void(*)(void));

    //PORTENTA_H7 min -12 max 51
    //NANO 33 BLE SENSe min 0 max 80
    void setGain(int gain);
    void setBufferSize(int bufferSize);
    size_t getBufferSize();

    // private:
    void IrqHandler(bool halftranfer);

private:
    int _dinPin;
    int _clkPin;
    int _pwrPin;

    int _channels;
    int _samplerate;

    int _gain;
    int _init;

    // Hardware peripherals used
    uint _dmaChannel;
    PIO _pio;
    int _smIdx;
    int _pgmOffset;

    PDMDoubleBuffer _doubleBuffer;

    void (*_onReceive)(void);
};

#ifdef PIN_PDM_DIN
extern PDMClass PDM;
#endif
