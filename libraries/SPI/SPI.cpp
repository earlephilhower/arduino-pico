/*
 * SPI Master library for the Raspberry Pi Pico RP2040
 *
 * Copyright (c) 2021 Earle F. Philhower, III <earlephilhower@yahoo.com>
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

#include "SPI.h"
#include <hardware/spi.h>
#include <hardware/gpio.h>

SPIClassRP2040::SPIClassRP2040(spi_inst_t *spi, pin_size_t rx, pin_size_t cs, pin_size_t sck, pin_size_t tx) {
    _spi = spi;
    _initted = false;
    _spis = SPISettings();
    _RX = rx;
    _TX = tx;
    _SCK = sck;
    _CS = cs;
}

inline spi_cpol_t SPIClassRP2040::cpol() {
    switch(_spis.getDataMode()) {
        case SPI_MODE0: return SPI_CPOL_0; 
        case SPI_MODE1: return SPI_CPOL_0;
        case SPI_MODE2: return SPI_CPOL_1;
        case SPI_MODE3: return SPI_CPOL_1;
    }
    // Error
    return SPI_CPOL_0;
}

inline spi_cpha_t SPIClassRP2040::cpha() {
    switch(_spis.getDataMode()) {
        case SPI_MODE0: return SPI_CPHA_0;
        case SPI_MODE1: return SPI_CPHA_1;
        case SPI_MODE2: return SPI_CPHA_0;
        case SPI_MODE3: return SPI_CPHA_1;
    }
    // Error
    return SPI_CPHA_0;
}    

inline uint8_t SPIClassRP2040::reverseByte(uint8_t b) {
    b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
    b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
    b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
    return b;
}

inline uint16_t SPIClassRP2040::reverse16Bit(uint16_t w) {
    return ( reverseByte(w & 0xff) << 8 ) | ( reverseByte(w >> 8) );
}

// The HW can't do LSB first, only MSB first, so need to bitreverse
void SPIClassRP2040::adjustBuffer(const void *s, void *d, size_t cnt, bool by16) {
    if (_spis.getBitOrder() == MSBFIRST) {
        memcpy(d, s, cnt * (by16? 2 : 1));
    } else if (!by16) {
        const uint8_t *src = (const uint8_t *)s;
        uint8_t *dst = (uint8_t *)d;
        for (auto i = 0; i < cnt; i++) {
          *(dst++) = reverseByte( *(src++) );
        }
    } else { /* by16 */
        const uint16_t *src = (const uint16_t *)s;
        uint16_t *dst = (uint16_t *)d;
        for (auto i = 0; i < cnt; i++) {
          *(dst++) = reverse16Bit( *(src++) );
        }
    }
}

byte SPIClassRP2040::transfer(uint8_t data) {
    uint8_t ret;
    if (!_initted) return 0;
    data = (_spis.getBitOrder() == MSBFIRST) ? data : reverseByte(data);
    spi_set_format(_spi, 8, cpol(), cpha(), SPI_MSB_FIRST);
    DEBUGSPI("SPI::transfer(%02x), cpol=%d, cpha=%d\n", data, cpol(), cpha());
    spi_write_read_blocking(_spi, &data, &ret, 1);
    ret = (_spis.getBitOrder() == MSBFIRST) ? ret : reverseByte(ret);
    DEBUGSPI("SPI: read back %02x\n", ret);
    return ret;
}

uint16_t SPIClassRP2040::transfer16(uint16_t data) {
    uint16_t ret;
    if (!_initted) {
        return 0;
    }
    data = (_spis.getBitOrder() == MSBFIRST) ? data : reverse16Bit(data);
    spi_set_format(_spi, 16, cpol(), cpha(), SPI_MSB_FIRST);
    DEBUGSPI("SPI::transfer16(%04x), cpol=%d, cpha=%d\n", data, cpol(), cpha());
    spi_write16_read16_blocking(_spi, &data, &ret, 1);
    ret = (_spis.getBitOrder() == MSBFIRST) ? ret : reverseByte(ret);
    DEBUGSPI("SPI: read back %02x\n", ret);
    return ret;
}

void SPIClassRP2040::transfer(void *buf, size_t count) {
    DEBUGSPI("SPI::transfer(%p, %d)\n", buf, count);
    uint8_t *buff = reinterpret_cast<uint8_t *>(buf);
    for (auto i = 0; i < count; i++) {
        *buff = transfer(*buff);
        *buff = (_spis.getBitOrder() == MSBFIRST) ? *buff : reverseByte(*buff);
        buff++;
    }
    DEBUGSPI("SPI::transfer completed\n");
}

void SPIClassRP2040::beginTransaction(SPISettings settings) {
    DEBUGSPI("SPI::beginTransaction(clk=%d, bo=%s\n", _spis.getClockFreq(), (_spis.getBitOrder() == MSBFIRST) ? "MSB":"LSB");
    _spis = settings;
    if (_initted) {
        DEBUGSPI("SPI: deinitting currently active SPI\n");
        spi_deinit(_spi);
    }
    DEBUGSPI("SPI: initting SPI\n");
    spi_init(_spi, _spis.getClockFreq());
    _initted = true;
}

void SPIClassRP2040::endTransaction(void) {
    DEBUGSPI("SPI::endTransaction()\n");
    if (_initted) {
        DEBUGSPI("SPI: deinitting currently active SPI\n");
        spi_deinit(_spi);
    }
    _initted = false;
}

bool SPIClassRP2040::setRX(pin_size_t pin) {
    const uint32_t valid[2] = { 0b10001000000000001000100000000000, 0b00000000100010000000000010001000 };
    if ( (1 << pin) & valid[spi_get_index(_spi)] ) {
        _RX = pin;
        return true;
    }
    return false;
}

bool SPIClassRP2040::setCS(pin_size_t pin) {
    const uint32_t valid[2] = { 0b01000100000000000100010000000000, 0b00000000010001000000000001000100 };
    if ( (1 << pin) & valid[spi_get_index(_spi)] ) {
        _CS = pin;
        return true;
    }
    return false;
}

bool SPIClassRP2040::setSCK(pin_size_t pin) {
    const uint32_t valid[2] = { 0b00100010000000000010001000000000, 0b00000000001000100000000000100010 };
    if ( (1 << pin) & valid[spi_get_index(_spi)] ) {
        _SCK = pin;
        return true;
    }
    return false;
}

bool SPIClassRP2040::setTX(pin_size_t pin) {
    const uint32_t valid[2] = { 0b00010001000000000001000100000000, 0b00000000000100010000000000010001 };
    if ( (1 << pin) & valid[spi_get_index(_spi)] ) {
        _TX = pin;
        return true;
    }
    return false;
}


void SPIClassRP2040::begin(bool hwCS) {
    DEBUGSPI("SPI::begin(%d), rx=%d, cs=%d, sck=%d, tx=%d\n", hwCS, _RX, _CS, _SCK, _TX);
    gpio_set_function(_RX, GPIO_FUNC_SPI);
    _hwCS = hwCS;
    if (hwCS) {
        gpio_set_function(_CS, GPIO_FUNC_SPI);
    }
    gpio_set_function(_SCK, GPIO_FUNC_SPI);
    gpio_set_function(_TX, GPIO_FUNC_SPI);
    // Give a default config in case user doesn't use beginTransaction
    beginTransaction(_spis);
}

void SPIClassRP2040::end() {
    DEBUGSPI("SPI::end()\n");
    gpio_set_function(_RX, GPIO_FUNC_SIO);
    if (_hwCS) {
        gpio_set_function(_CS, GPIO_FUNC_SIO);
    }
    gpio_set_function(_SCK, GPIO_FUNC_SIO);
    gpio_set_function(_TX, GPIO_FUNC_SIO);
}

void SPIClassRP2040::setBitOrder(BitOrder order) {
    _spis = SPISettings( _spis.getClockFreq(), order, _spis.getDataMode() );
    beginTransaction(_spis);
}

void SPIClassRP2040::setDataMode(uint8_t uc_mode) {
    _spis = SPISettings( _spis.getClockFreq(), _spis.getBitOrder(), uc_mode );
    beginTransaction(_spis);
}

void SPIClassRP2040::setClockDivider(uint8_t uc_div) {
  (void) uc_div; // no-op
}

SPIClassRP2040 SPI(spi0, 0, 1, 2, 3);
SPIClassRP2040 SPI1(spi1, 8, 9, 10, 11);
