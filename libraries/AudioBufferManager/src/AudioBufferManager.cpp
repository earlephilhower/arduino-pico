/*
    AudioBufferManager for Raspnerry Pi Pico RP2040
    Implements a DMA controlled linked-list series of buffers

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
#include <hardware/dma.h>
#include <hardware/irq.h>
#include "AudioBufferManager.h"

static int                 __channelCount = 0;    // # of channels left.  When we hit 0, then remove our handler
static AudioBufferManager* __channelMap[12];      // Lets the IRQ handler figure out where to dispatch to

AudioBufferManager::AudioBufferManager(size_t bufferCount, size_t bufferWords, int32_t silenceSample, PinMode direction, enum dma_channel_transfer_size dmaSize) {
    _running = false;

    // Need at least 2 DMA buffers and 1 user or this isn't going to work at all
    if (bufferCount < 3) {
        bufferCount = 3;
    }

    _bufferCount = bufferCount;
    _wordsPerBuffer = bufferWords;
    _isOutput = direction == OUTPUT;
    _dmaSize = dmaSize;
    _overunderflow = false;
    _callback = nullptr;
    _userOff = 0;

    // Create the silence buffer, fill with appropriate value
    _silence = new AudioBuffer;
    _silence->next = nullptr;
    _silence->buff = new uint32_t[_wordsPerBuffer];
    for (uint32_t x = 0; x < _wordsPerBuffer; x++) {
        _silence->buff[x] = silenceSample;
    }

    // No filled buffers yet
    _filled = nullptr;

    // Create all buffers on the empty chain
    _empty = nullptr;
    for (size_t i = 0; i < bufferCount; i++) {
        auto ab = new AudioBuffer;
        ab->buff = new uint32_t[_wordsPerBuffer];
        bzero(ab->buff, _wordsPerBuffer * 4);
        ab->next = nullptr;
        _addToList(&_empty, ab);
    }

    _active[0] = _silence;
    _active[1] = _silence;
}

AudioBufferManager::~AudioBufferManager() {
    noInterrupts();
    if (_running) {
        _running = false;
        for (auto i = 0; i < 2; i++) {
            dma_channel_set_irq0_enabled(_channelDMA[i], false);
            __channelMap[_channelDMA[i]] = nullptr;
            dma_channel_abort(_channelDMA[i]);
            dma_channel_unclaim(_channelDMA[i]);
            dma_channel_acknowledge_irq0(_channelDMA[i]);
            __channelCount--;
        }
        if (!__channelCount) {
            irq_set_enabled(DMA_IRQ_0, false);
            // TODO - how can we know if there are no other parts of the core using DMA0 IRQ??
            irq_remove_handler(DMA_IRQ_0, _irq);
        }
    }
    interrupts();
    for (int i = 0; i < 2; i++) {
        if (_active[i] != _silence) {
            _deleteAudioBuffer(_active[i]);
        }
    }
    while (_filled) {
        auto x = _filled->next;
        _deleteAudioBuffer(_filled);
        _filled = x;
    }
    while (_empty) {
        auto x = _empty->next;
        _deleteAudioBuffer(_empty);
        _empty = x;
    }
    _deleteAudioBuffer(_silence);
}

void AudioBufferManager::setCallback(void (*fn)()) {
    _callback = fn;
}

bool AudioBufferManager::begin(int dreq, volatile void *pioFIFOAddr) {
    _running = true;

    // Get ping and pong DMA channels
    for (auto i = 0; i < 2; i++) {
        _channelDMA[i] = dma_claim_unused_channel(false);
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
        channel_config_set_transfer_data_size(&c, _dmaSize); // 16b/32b transfers into PIO FIFO
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
            dma_channel_configure(_channelDMA[i], &c, pioFIFOAddr, _silence->buff, _wordsPerBuffer * (_dmaSize == DMA_SIZE_16 ? 2 : 1), false);
        } else {
            _active[i] = _takeFromList(&_empty);
            dma_channel_configure(_channelDMA[i], &c, _active[i]->buff, pioFIFOAddr, _wordsPerBuffer * (_dmaSize == DMA_SIZE_16 ? 2 : 1), false);
        }
        dma_channel_set_irq0_enabled(_channelDMA[i], true);
        __channelMap[_channelDMA[i]] = this;
        __channelCount++;
    }
    if (needSetIRQ) {
        irq_add_shared_handler(DMA_IRQ_0, _irq, PICO_SHARED_IRQ_HANDLER_DEFAULT_ORDER_PRIORITY);
        irq_set_enabled(DMA_IRQ_0, true);
    }

    dma_channel_start(_channelDMA[0]);
    return true;
}

// Following 2 routines use volatile because the IRQ may update the "this"
// pointer and change the list head while we are waiting.  Volatile will
// cause GCC to keep re-reading from memory and not use cached value read
// on the first pass.

bool AudioBufferManager::write(uint32_t v, bool sync) {
    if (!_running || !_isOutput) {
        return false;
    }
    AudioBuffer ** volatile p = (AudioBuffer ** volatile)&_empty;
    if (!*p) {
        if (!sync) {
            return false;
        } else {
            while (!*p) {
                /* noop busy wait */
            }
        }
    }
    (*p)->buff[_userOff++] = v;
    if (_userOff == _wordsPerBuffer) {
        _addToList(&_filled, _takeFromList(p));
        _userOff = 0;
    }
    return true;
}

bool AudioBufferManager::read(uint32_t *v, bool sync) {
    if (!_running || _isOutput) {
        return false;
    }

    AudioBuffer ** volatile p = (AudioBuffer ** volatile)&_filled;
    if (!*p) {
        if (!sync) {
            return false;
        } else {
            while (!*p) {
                /* noop busy wait */
            }
        }
    }
    auto ret = (*p)->buff[_userOff++];
    if (_userOff == _wordsPerBuffer) {
        _addToList(&_empty, _takeFromList(p));
        _userOff = 0;
    }
    *v = ret;
    return true;
}

bool AudioBufferManager::getOverUnderflow() {
    bool hold = _overunderflow;
    _overunderflow = false;
    return hold;
}

int AudioBufferManager::available() {
    AudioBuffer *p = _isOutput ? _empty : _filled;

    if (!_running || !p) {
        // No buffers available...
        return 0;
    }

    int avail = _wordsPerBuffer - _userOff; // Currently available in this buffer

    // Each add'l buffer has wpb spaces...
    auto x = p->next;
    while (x) {
        avail += _wordsPerBuffer;
        x = x->next;
    }
    return avail;
}

void AudioBufferManager::flush() {
    AudioBuffer ** volatile a = (AudioBuffer ** volatile)&_active[0];
    AudioBuffer ** volatile b = (AudioBuffer ** volatile)&_active[1];
    AudioBuffer ** volatile c = (AudioBuffer ** volatile)&_filled;
    while (*c && (*b != (AudioBuffer * volatile)_silence) && (*a != (AudioBuffer * volatile)_silence)) {
        // busy wait until all user written data enroute
    }
}

void __not_in_flash_func(AudioBufferManager::_dmaIRQ)(int channel) {
    if (!_running) {
        return;
    }
    if (_isOutput) {
        if (_active[0] != _silence) {
            _addToList(&_empty, _active[0]);
        }
        _active[0] = _active[1];
        if (!_filled) {
            _active[1] = _silence;
        } else {
            _active[1] = _takeFromList(&_filled);
        }
        _overunderflow = _overunderflow | (_active[1] == _silence);
        dma_channel_set_read_addr(channel, _active[1]->buff, false);
    } else {
        if (_empty) {
            _addToList(&_filled, _active[0]);
            _active[0] = _active[1];
            _active[1] = _takeFromList(&_empty);
        } else {
            _overunderflow = true;
        }
        dma_channel_set_write_addr(channel, _active[1]->buff, false);
    }
    dma_channel_set_trans_count(channel, _wordsPerBuffer * (_dmaSize == DMA_SIZE_16 ? 2 : 1), false);
    dma_channel_acknowledge_irq0(channel);
    if (_callback) {
        _callback();
    }
}

void __not_in_flash_func(AudioBufferManager::_irq)() {
    for (size_t i = 0; i < sizeof(__channelMap); i++) {
        if (dma_channel_get_irq0_status(i) && __channelMap[i]) {
            __channelMap[i]->_dmaIRQ(i);
        }
    }
}
