/*
    I2C Master/Slave library for the Raspberry Pi Pico RP2040

    Copyright (c) 2021 Earle F. Philhower, III <earlephilhower@yahoo.com>

    Based off of TWI/I2C library for Arduino Zero
    Copyright (c) 2015 Arduino LLC. All rights reserved.

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

    Modified May 2025 by Sven Bruns (Lorandil on GitHub) to support user defined buffer size (inspired by ESP32 code)
*/

#pragma once

#include <Arduino.h>
#include "api/HardwareI2C.h"
#include <hardware/i2c.h>

// WIRE_HAS_END means Wire has end()
#define WIRE_HAS_END 1

// WIRE_HAS_BUFFER_SIZE means Wire has setBufferSize()
#define WIRE_HAS_BUFFER_SIZE 1

#ifndef WIRE_BUFFER_SIZE
#define WIRE_BUFFER_SIZE 256    // default size, if none is set using Wire::setBuffersize(size_t)
#endif
#ifndef WIRE_BUFFER_SIZE_MIN
#define WIRE_BUFFER_SIZE_MIN 32 // minimum size for safe operation
#endif

class TwoWire : public HardwareI2C {
public:
    TwoWire(i2c_inst_t *i2c, pin_size_t sda, pin_size_t scl);

    // Start as Master
    void begin() override;
    // Start as Slave
    void begin(uint8_t address) override;
    // Shut down the I2C interface
    void end() override;

    // Select IO pins to use.  Call before ::begin()
    bool setSDA(pin_size_t sda);
    bool setSCL(pin_size_t scl);

    void setClock(uint32_t freqHz) override;

    void beginTransmission(uint8_t) override;
    uint8_t endTransmission(bool stopBit) override;
    uint8_t endTransmission(void) override;

    size_t requestFrom(uint8_t address, size_t quantity, bool stopBit) override;
    size_t requestFrom(uint8_t address, size_t quantity) override;

    size_t write(uint8_t data) override;
    size_t write(const uint8_t * data, size_t quantity) override;

    virtual int available(void) override;
    virtual int read(void) override;
    virtual int peek(void) override;
    virtual void flush(void) override;
    void onReceive(void(*)(int));
    void onRequest(void(*)(void));

    inline size_t write(unsigned long n) {
        return write((uint8_t)n);
    }
    inline size_t write(long n) {
        return write((uint8_t)n);
    }
    inline size_t write(unsigned int n) {
        return write((uint8_t)n);
    }
    inline size_t write(int n) {
        return write((uint8_t)n);
    }
    using Print::write;

    // DMA/asynchronous transfers.  Do not combime with synchronous runs or bad stuff will happen
    // All buffers must be valid for entire DMA and not touched until `finishedAsync()` returns true.
    bool writeReadAsync(uint8_t address, const void *wbuffer, size_t wbytes, const void *rbuffer, size_t rbytes, bool sendStop = true);
    bool writeAsync(uint8_t address, const void *buffer, size_t bytes, bool sendStop = true);
    bool readAsync(uint8_t address, void *buffer, size_t bytes, bool sendStop = true);
    bool busIdle();
    bool finishedAsync(); // Call to check if the async operations is completed and the buffer can be reused/read
    void abortAsync(); // Cancel an outstanding async I2C operation
    void onFinishedAsync(void(*function)(void)); // Set callback for async operation
    void _dma_irq_handler(); // "private" method, made public to call this method from low level dma irq handler

    void setTimeout(uint32_t timeout = 25, bool reset_with_timeout = false);     // sets the maximum number of milliseconds to wait
    bool getTimeoutFlag(void);
    void clearTimeoutFlag(void);

    size_t setBufferSize(size_t bSize);	// set buffer size (call prior to 'begin()')

    // IRQ callback
    void onIRQ();

private:
    i2c_inst_t *_i2c;
    pin_size_t _sda;
    pin_size_t _scl;
    int _clkHz;

    bool _running;
    bool _slave;
    uint8_t _addr;
    bool _txBegun;

    bool _timeoutFlag;
    bool _reset_with_timeout;
    void _handleTimeout(bool reset);

    uint8_t *_buff;         // pointer to i2c buffer
    size_t  _buffSize;      // current buffer size

    int _buffLen;
    int _buffOff;

    // Callback user functions
    void (*_onRequestCallback)(void);
    void (*_onReceiveCallback)(int);

    bool _slaveStartDet = false;

    // TWI clock frequency
    static const uint32_t TWI_CLOCK = 100000;

    // DMA
    bool _dmaRunning = false; // set to true after successful beginAsync() call
    int _dmaChannelReceive = -1; // dma channel to receive i2c data
    int _dmaChannelSend = -1; // dma channel to send i2c commands
    uint16_t *_dmaSendBuffer = nullptr; // dma command send buffer (dynamically allocated)
    size_t _dmaSendBufferLen = 0; // size of dma command buffer
    volatile bool _dmaFinished = true; // signals dma completion
    void (*_dmaOnFinished)(void) = nullptr; // user handler to call on dma completion
    void beginAsync(); // setup dma channels and irq, called on first use of an async read/write function
    void endAsync(); // close dma channels, irq, buffers, called from end()
};

extern TwoWire Wire;
extern TwoWire Wire1;
