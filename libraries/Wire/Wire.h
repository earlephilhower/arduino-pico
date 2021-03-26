/*
 * I2C Master/Slave library for the Raspberry Pi Pico RP2040
 *
 * Copyright (c) 2021 Earle F. Philhower, III <earlephilhower@yahoo.com>
 *
 * Based off of TWI/I2C library for Arduino Zero
 * Copyright (c) 2015 Arduino LLC. All rights reserved.
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

#ifndef TwoWire_h
#define TwoWire_h

#include <Arduino.h>
#include "api/HardwareI2C.h"
#include <hardware/i2c.h>

 // WIRE_HAS_END means Wire has end()
#define WIRE_HAS_END 1

#ifndef WIRE_BUFFER_SIZE
    #define WIRE_BUFFER_SIZE 128
#endif

class TwoWire : public HardwareI2C {
public:
    TwoWire(i2c_inst_t *i2c, pin_size_t sda, pin_size_t scl);

    // Start as Master
    void begin();
    // Start as Slave
    void begin(uint8_t address);
    // Shut down the I2C interface
    void end();

    // Select IO pins to use.  Call before ::begin()
    bool setSDA(pin_size_t sda);
    bool setSCL(pin_size_t scl);

    void setClock(uint32_t freqHz) override;

    void beginTransmission(uint8_t);
    uint8_t endTransmission(bool stopBit);
    uint8_t endTransmission(void);

    size_t requestFrom(uint8_t address, size_t quantity, bool stopBit);
    size_t requestFrom(uint8_t address, size_t quantity);

    size_t write(uint8_t data);
    size_t write(const uint8_t * data, size_t quantity);

    virtual int available(void);
    virtual int read(void);
    virtual int peek(void);
    virtual void flush(void);
    void onReceive(void(*)(int));
    void onRequest(void(*)(void));

    inline size_t write(unsigned long n) { return write((uint8_t)n); }
    inline size_t write(long n) { return write((uint8_t)n); }
    inline size_t write(unsigned int n) { return write((uint8_t)n); }
    inline size_t write(int n) { return write((uint8_t)n); }
    using Print::write;

private:
    i2c_inst_t *_i2c;
    pin_size_t _sda;
    pin_size_t _scl;
    int _clkHz;

    bool _running;
    bool _slave;
    uint8_t _addr;
    bool _txBegun;

    uint8_t _buff[WIRE_BUFFER_SIZE];
    int _buffLen;
    int _buffOff;

    // Callback user functions
    void (*_onRequestCallback)(void);
    void (*_onReceiveCallback)(int);

    // TWI clock frequency
    static const uint32_t TWI_CLOCK = 100000;
};

extern TwoWire Wire;
extern TwoWire Wire1;

#endif

