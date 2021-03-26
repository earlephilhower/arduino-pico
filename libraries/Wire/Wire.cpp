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

#include <Arduino.h>
#include <hardware/gpio.h>
#include <hardware/i2c.h>
#include "Wire.h"

TwoWire::TwoWire(i2c_inst_t *i2c, pin_size_t sda, pin_size_t scl) {
    _sda = sda;
    _scl = scl;
    _i2c = i2c;
    _clkHz = TWI_CLOCK;
    _running = false;
    _txBegun = false;
    _buffLen = 0;
}

bool TwoWire::setSDA(pin_size_t pin) {
    constexpr uint32_t valid[2] = { __bitset({0, 4, 8, 12, 16, 20, 24, 28}) /* I2C0 */,
                                    __bitset({2, 6, 10, 14, 18, 22, 26})  /* I2C1 */};
    if (_running) {
        return false;
    } else if ((1 << pin) & valid[i2c_hw_index(_i2c)]) {
        _sda = pin;
        return true;
    } else {
        return false;
    }
}

bool TwoWire::setSCL(pin_size_t pin) {
    constexpr uint32_t valid[2] = { __bitset({1, 5, 9, 13, 17, 21, 25, 29}) /* I2C0 */,
                                    __bitset({3, 7, 11, 15, 19, 23, 27})  /* I2C1 */};
    if (_running) {
        return false;
    } else if ((1 << pin) & valid[i2c_hw_index(_i2c)]) {
        _scl = pin;
        return true;
    } else {
        return false;
    }
}

void TwoWire::setClock(uint32_t hz) {
    _clkHz = hz;
}

// Master mode
void TwoWire::begin() {
    if (_running) {
        // ERROR
        return;
    }
    _slave = false;
    i2c_init(_i2c, _clkHz);
    i2c_set_slave_mode(_i2c, false, 0);
    gpio_set_function(_sda, GPIO_FUNC_I2C);
    gpio_pull_up(_sda);
    gpio_set_function(_scl, GPIO_FUNC_I2C);
    gpio_pull_up(_scl);

    _running = true;
    _txBegun = false;
    _buffLen = 0;
}

// Slave mode
void TwoWire::begin(uint8_t addr) {
    // Slave mode isn't documented in the SDK, need to twiddle raw registers
    // and use bare interrupts.  TODO to implement, for now.
#if 0
    if (_running) {
        // ERROR
        return;
    }
    _slave = true;
    i2c_init(_i2c, _clkHz);
    i2c_set_slave_mode(_i2c, true, addr);
    gpio_set_function(_sda, GPIO_FUNC_I2C);
    gpio_pull_up(_sda);
    gpio_set_function(_scl, GPIO_FUNC_I2C);
    gpio_pull_up(_scl);
#endif
}

void TwoWire::end() {
    if (!_running) {
        // ERROR
        return;
    }
    i2c_deinit(_i2c);
    _running = false;
    _txBegun = false;
}

void TwoWire::beginTransmission(uint8_t addr) {
    if (!_running || _txBegun) {
        // ERROR
        return;
    }
    _addr = addr;
    _buffLen = 0;
    _txBegun = true;
}

size_t TwoWire::requestFrom(uint8_t address, size_t quantity, bool stopBit) {
    if (!_running || _txBegun || !quantity || (quantity > sizeof(_buff))) {
        return 0;
    }

    size_t byteRead = 0;
    _buffLen = i2c_read_blocking(_i2c, address, _buff, quantity, !stopBit);
    _buffOff = 0;
    return _buffLen;
}

size_t TwoWire::requestFrom(uint8_t address, size_t quantity) {
    return requestFrom(address, quantity, true);
}

// Errors:
//  0 : Success
//  1 : Data too long
//  2 : NACK on transmit of address
//  3 : NACK on transmit of data
//  4 : Other error
uint8_t TwoWire::endTransmission(bool stopBit) {
    if (!_running || !_txBegun || !_buffLen) {
        return 4; 
    }
    auto len = _buffLen;
    auto ret = i2c_write_blocking(_i2c, _addr, _buff, _buffLen, !stopBit);
    _buffLen = 0;
    _txBegun = false;
    return (ret == len) ? 0 : 4;
}

uint8_t TwoWire::endTransmission() {
  return endTransmission(true);
}

size_t TwoWire::write(uint8_t ucData) {
    if (!_running || !_txBegun || (_buffLen == sizeof(_buff))) {
        return 0;
    }
    _buff[_buffLen++] = ucData;
    return 1 ;
}

size_t TwoWire::write(const uint8_t *data, size_t quantity) {
    for (size_t i = 0; i < quantity; ++i) {
        if (!write(data[i])) {
            return i;
        }
    }

    return quantity;
}

int TwoWire::available(void) {
    return _running  ? _buffLen - _buffOff : 0;
}

int TwoWire::read(void) {
    if (available()) {
        return _buff[_buffOff++];
    }
    return -1; // EOF
}

int TwoWire::peek(void) {
    if (available()) {
        return _buff[_buffOff];
    }
    return -1; // EOF
}

void TwoWire::flush(void) {
    // Do nothing, use endTransmission(..) to force
    // data transfer.
}


void TwoWire::onReceive(void(*function)(int))
{
    _onReceiveCallback = function;
}

void TwoWire::onRequest(void(*function)(void))
{
    _onRequestCallback = function;
}

TwoWire Wire(i2c0, 0, 1);
TwoWire Wire1(i2c1, 4, 5);

