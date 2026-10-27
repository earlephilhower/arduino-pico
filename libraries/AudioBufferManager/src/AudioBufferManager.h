/*
    AudioBufferManager for Rasperry Pi Pico
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

#pragma once
#include <Arduino.h>
#include <hardware/dma.h>

class AudioBufferManager {
public:
    AudioBufferManager(size_t bufferCount, size_t bufferWords, int32_t silenceSample, PinMode direction = OUTPUT, enum dma_channel_transfer_size dmaSize = DMA_SIZE_32);
    ~AudioBufferManager();

    void setCallback(void (*fn)());
    void setCallback(void (*fn)(void *), void *cbData);

    bool begin(int dreq, volatile void *pioFIFOAddr);

    bool write(uint32_t v, bool sync = true);
    size_t write(const uint32_t *v, size_t words, bool sync = true);
    bool read(uint32_t *v, bool sync = true);
    size_t read(uint32_t *v, size_t words, bool sync = true);
    void flush();

    bool getOverUnderflow();
    int available();

private:
    void _dmaIRQ(int channel);
    static void _irq();

    typedef struct AudioBuffer {
        struct AudioBuffer *next;
        uint32_t *buff;
    } AudioBuffer;

    bool _running = false;

    AudioBuffer *_silence = nullptr; // A single silence buffer to be looped on underflow
    AudioBuffer *_filled = nullptr;  // List of buffers ready to be played
    AudioBuffer *_empty = nullptr;   // List of buffers waiting to be filled. *_empty = currently writing
    AudioBuffer *_active[2] = { nullptr, nullptr }; // The 2 buffers currently in use for DMA, by data channel

    // The address each data channel starts on next.  When one data channel
    // finishes, its control channel copies the other's slot into that
    // channel's address register, which starts it.  Only the IRQ changes the
    // slots, so if it's late the DMA reuses a buffer it owns, never more RAM.
    volatile uint32_t _slot[2] = { 0, 0 };

    // Can't use std::list because we need to put in RAM for IRQ use, so roll our own
    void __not_in_flash_func(_addToList)(AudioBuffer **list, AudioBuffer *element) {
        noInterrupts();
        // Find end of list, if any
        while ((*list) && ((*list)->next != nullptr)) {
            list = &(*list)->next;
        }
        if (*list) {
            (*list)->next = element;
        } else {
            *list = element;
        }
        element->next = nullptr; // Belt and braces
        interrupts();
    }

    void __not_in_flash_func(_addToFront)(AudioBuffer **list, AudioBuffer *element) {
        noInterrupts();
        element->next = *list;
        *list = element;
        interrupts();
    }

    AudioBuffer *__not_in_flash_func(_takeFromList)(AudioBuffer **list) {
        noInterrupts();
        auto ret = *list;
        if (ret) {
            *list = ret->next;
        }
        interrupts();
        return ret;
    }

    void _deleteAudioBuffer(AudioBuffer *ab) {
        delete[] ab->buff;
        delete ab;
    }

    size_t _wordsPerBuffer;
    size_t _bufferCount;
    enum dma_channel_transfer_size _dmaSize;
    bool _isOutput;

    int _channelDMA[2];  // Data: PIO FIFO <-> buffers
    int _channelCtrl[2]; // Control: _channelCtrl[i] starts data channel i ^ 1 at its slot

    bool _useData;
    void (*_callback)();
    void (*_callbackCB)(void *);
    void *_callbackData;

    bool _overunderflow;

    // User buffer pointer
    size_t _userOff = 0;
};
