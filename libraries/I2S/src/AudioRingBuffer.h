/*
    AudioRingBuffer for Rasperry Pi Pico
    Implements a ring buffer for PIO DMA for I2S read or write

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
#include <vector>

class AudioRingBuffer {
public:
    AudioRingBuffer(size_t bufferCount, size_t bufferWords, int32_t silenceSample, PinMode direction = OUTPUT);
    ~AudioRingBuffer();

    void setCallback(void (*fn)());

    bool begin(int dreq, volatile void *pioFIFOAddr);

    bool write(uint32_t v, bool sync = true);
    bool read(uint32_t *v, bool sync = true);
    void flush();

    bool getOverUnderflow();
    int available();

private:
    void _dmaIRQ(int channel);
    static void _irq();

    typedef struct {
        uint32_t *buff;
        volatile bool empty;
    } AudioBuffer;

    bool _running = false;
    std::vector<AudioBuffer*> _buffers;
    volatile int _curBuffer;
    volatile int _nextBuffer;
    size_t _chunkSampleCount;
    int _bitsPerSample;
    size_t _wordsPerBuffer;
    size_t _bufferCount;
    bool _isOutput;
    int32_t _silenceSample;
    int _channelDMA[2];
    void (*_callback)();

    bool _overunderflow;

    // User buffer pointer
    int _userBuffer = -1;
    size_t _userOff = 0;
};
