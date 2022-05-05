/*
    AudioRingBuffer for Raspnerry Pi Pico RP2040
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

#include <Arduino.h>
#include <vector>
#include "hardware/dma.h"
#include "hardware/irq.h"
#include "hardware/pio.h"
#include "pio_i2s.pio.h"
#include "AudioRingBuffer.h"

static int              __channelCount = 0;    // # of channels left.  When we hit 0, then remove our handler
static AudioRingBuffer* __channelMap[12];      // Lets the IRQ handler figure out where to dispatch to

AudioRingBuffer::AudioRingBuffer(size_t bufferCount, size_t bufferWords, int32_t silenceSample, PinMode direction) {
    _running = false;
    _silenceSample = silenceSample;
    _bufferCount = bufferCount;
    _wordsPerBuffer = bufferWords;
    _isOutput = direction == OUTPUT;
    _overunderflow = false;
    _callback = nullptr;
    _userBuffer = -1;
    _userOff = 0;
    for (size_t i = 0; i < bufferCount; i++) {
        auto ab = new AudioBuffer;
        ab->buff = new uint32_t[_wordsPerBuffer];
        ab->empty = true;
        _buffers.push_back(ab);
    }
}

AudioRingBuffer::~AudioRingBuffer() {
    if (_running) {
        for (auto i = 0; i < 2; i++) {
            dma_channel_set_irq0_enabled(_channelDMA[i], false);
            dma_channel_unclaim(_channelDMA[i]);
            __channelMap[_channelDMA[i]] = nullptr;
        }
        while (_buffers.size()) {
            auto ab = _buffers.back();
            _buffers.pop_back();
            delete[] ab->buff;
            delete ab;
        }
        __channelCount--;
        if (!__channelCount) {
            irq_set_enabled(DMA_IRQ_0, false);
            // TODO - how can we know if there are no other parts of the core using DMA0 IRQ??
            irq_remove_handler(DMA_IRQ_0, _irq);
        }
    }
}

void AudioRingBuffer::setCallback(void (*fn)()) {
    _callback = fn;
}

bool AudioRingBuffer::begin(int dreq, volatile void *pioFIFOAddr) {
    _running = true;
    // Set all buffers to silence, empty
    for (auto buff : _buffers) {
        buff->empty = true;
        if (_isOutput) {
            for (uint32_t x = 0; x < _wordsPerBuffer; x++) {
                buff->buff[x] = _silenceSample;
            }
        }
    }
    // Get ping and pong DMA channels
    for (auto i = 0; i < 2; i++) {
        _channelDMA[i] = dma_claim_unused_channel(true);
        if (_channelDMA[i] == -1) {
            if (i == 1) {
                dma_channel_unclaim(_channelDMA[0]);
            }
            return false;
        }
    }
    bool needSetIRQ = __channelCount == 0;
    // Need to know both channels to set up ping-pong, so do in 2 stages
    for (auto i = 0; i < 2; i++) {
        dma_channel_config c = dma_channel_get_default_config(_channelDMA[i]);
        channel_config_set_transfer_data_size(&c, DMA_SIZE_32); // 32b transfers into PIO FIFO
        if (_isOutput) {
            channel_config_set_read_increment(&c, true); // Reading incrementing addresses
            channel_config_set_write_increment(&c, false); // Writing to the same FIFO address
        } else {
            channel_config_set_read_increment(&c, false); // Reading same FIFO address
            channel_config_set_write_increment(&c, true); // Writing to incrememting buffers
        }
        channel_config_set_dreq(&c, dreq); // Wait for the PIO TX FIFO specified
        channel_config_set_chain_to(&c, _channelDMA[i ^ 1]); // Start other channel when done
        channel_config_set_irq_quiet(&c, false); // Need IRQs

        if (_isOutput) {
            dma_channel_configure(_channelDMA[i], &c, pioFIFOAddr, _buffers[i]->buff, _wordsPerBuffer, false);
        } else {
            dma_channel_configure(_channelDMA[i], &c, _buffers[i]->buff, pioFIFOAddr, _wordsPerBuffer, false);
        }
        dma_channel_set_irq0_enabled(_channelDMA[i], true);
        __channelMap[_channelDMA[i]] = this;
        __channelCount++;
    }
    if (needSetIRQ) {
        irq_add_shared_handler(DMA_IRQ_0, _irq, PICO_SHARED_IRQ_HANDLER_DEFAULT_ORDER_PRIORITY);
        irq_set_enabled(DMA_IRQ_0, true);
    }
    _curBuffer = 0;
    _nextBuffer = 2 % _bufferCount;
    dma_channel_start(_channelDMA[0]);
    return true;
}

bool AudioRingBuffer::write(uint32_t v, bool sync) {
    if (!_running || !_isOutput) {
        return false;
    }
    if (_userBuffer == -1) {
        // First write or overflow, pick spot 2 buffers out
        _userBuffer = (_nextBuffer + 2) % _bufferCount;
        _userOff = 0;
    }
    if (!_buffers[_userBuffer]->empty) {
        if (!sync) {
            return false;
        } else {
            while (!_buffers[_userBuffer]->empty) {
                /* noop busy wait */
            }
        }
    }
    if (_userBuffer == _curBuffer) {
        if (!sync) {
            return false;
        } else {
            while (_userBuffer == _curBuffer) {
                /* noop busy wait */
            }
        }
    }
    _buffers[_userBuffer]->buff[_userOff++] = v;
    if (_userOff == _wordsPerBuffer) {
        _buffers[_userBuffer]->empty = false;
        _userBuffer = (_userBuffer + 1) % _bufferCount;
        _userOff = 0;
    }
    return true;
}

bool AudioRingBuffer::read(uint32_t *v, bool sync) {
    if (!_running || _isOutput) {
        return false;
    }
    if (_userBuffer == -1) {
        // First write or overflow, pick last filled buffer
        _userBuffer = (_curBuffer - 1 + _bufferCount) % _bufferCount;
        _userOff = 0;
    }
    if (_buffers[_userBuffer]->empty) {
        if (!sync) {
            return false;
        } else {
            while (_buffers[_userBuffer]->empty) {
                /* noop busy wait */
            }
        }
    }
    if (_userBuffer == _curBuffer) {
        if (!sync) {
            return false;
        } else {
            while (_userBuffer == _curBuffer) {
                /* noop busy wait */
            }
        }
    }
    auto ret = _buffers[_userBuffer]->buff[_userOff++];
    if (_userOff == _wordsPerBuffer) {
        _buffers[_userBuffer]->empty = true;
        _userBuffer = (_userBuffer + 1) % _bufferCount;
        _userOff = 0;
    }
    *v = ret;
    return true;
}

bool AudioRingBuffer::getOverUnderflow() {
    bool hold = _overunderflow;
    _overunderflow = false;
    return hold;
}

int AudioRingBuffer::available() {
    if (!_running) {
        return 0;
    }
    int avail;
    avail = _wordsPerBuffer - _userOff;
    avail += ((_bufferCount + _curBuffer - _userBuffer) % _bufferCount) * _wordsPerBuffer;
    return avail;
}

void AudioRingBuffer::flush() {
    while (_curBuffer != _userBuffer) {
        // busy wait
    }
}

void __not_in_flash_func(AudioRingBuffer::_dmaIRQ)(int channel) {
    if (_isOutput) {
        for (uint32_t x = 0; x < _wordsPerBuffer; x++) {
            _buffers[_curBuffer]->buff[x] = _silenceSample;
        }
        _buffers[_curBuffer]-> empty = true;
        _overunderflow = _overunderflow | _buffers[_nextBuffer]->empty;
        dma_channel_set_read_addr(channel, _buffers[_nextBuffer]->buff, false);
    } else {
        _buffers[_curBuffer]-> empty = false;
        _overunderflow = _overunderflow | !_buffers[_nextBuffer]->empty;
        dma_channel_set_write_addr(channel, _buffers[_nextBuffer]->buff, false);
    }
    dma_channel_set_trans_count(channel, _wordsPerBuffer, false);
    _curBuffer = (_curBuffer + 1) % _bufferCount;
    _nextBuffer = (_nextBuffer + 1) % _bufferCount;
    dma_channel_acknowledge_irq0(channel);
    if (_callback) {
        _callback();
    }
}

void __not_in_flash_func(AudioRingBuffer::_irq)() {
    for (size_t i = 0; i < sizeof(__channelMap); i++) {
        if (dma_channel_get_irq0_status(i) && __channelMap[i]) {
            __channelMap[i]->_dmaIRQ(i);
        }
    }
}
