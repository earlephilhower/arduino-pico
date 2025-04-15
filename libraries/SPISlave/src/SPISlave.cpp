/*
    SPI Slave library for the Raspberry Pi Pico RP2040

    Copyright (c) 2023 Earle F. Philhower, III <earlephilhower@yahoo.com>

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

#include "SPISlave.h"
#include <hardware/spi.h>
#include <hardware/gpio.h>
#include <hardware/structs/iobank0.h>
#include <hardware/irq.h>

#ifdef USE_TINYUSB
// For Serial when selecting TinyUSB.  Can't include in the core because Arduino IDE
// will not link in libraries called from the core.  Instead, add the header to all
// the standard libraries in the hope it will still catch some user cases where they
// use these libraries.
// See https://github.com/earlephilhower/arduino-pico/issues/167#issuecomment-848622174
#include <Adafruit_TinyUSB.h>
#endif

SPISlaveClass::SPISlaveClass(spi_inst_t *spi, pin_size_t rx, pin_size_t cs, pin_size_t sck, pin_size_t tx) {
    _spi = spi;
    _running = false;
    _initted = false;
    _spis = SPISettings();
    _RX = rx;
    _TX = tx;
    _SCK = sck;
    _CS = cs;
    _recvCB = nullptr;
    _sentCB = nullptr;
    _dataOut = nullptr;
    _dataLeft = 0;
}

inline spi_cpol_t SPISlaveClass::cpol(SPISettings _spis) {
    switch (_spis.getDataMode()) {
    case SPI_MODE0:
        return SPI_CPOL_0;
    case SPI_MODE1:
        return SPI_CPOL_0;
    case SPI_MODE2:
        return SPI_CPOL_1;
    case SPI_MODE3:
        return SPI_CPOL_1;
    }
    // Error
    return SPI_CPOL_0;
}

inline spi_cpha_t SPISlaveClass::cpha(SPISettings _spis) {
    switch (_spis.getDataMode()) {
    case SPI_MODE0:
        return SPI_CPHA_0;
    case SPI_MODE1:
        return SPI_CPHA_1;
    case SPI_MODE2:
        return SPI_CPHA_0;
    case SPI_MODE3:
        return SPI_CPHA_1;
    }
    // Error
    return SPI_CPHA_0;
}

bool SPISlaveClass::setRX(pin_size_t pin) {
#if defined(PICO_RP2350) && !PICO_RP2350A // RP2350B
    constexpr uint64_t valid[2] = { __bitset({0, 4, 16, 20, 32, 26}) /* SPI0 */,
                                    __bitset({8, 12, 24, 28, 40, 44})  /* SPI1 */
                                  };
#else
    constexpr uint64_t valid[2] = { __bitset({0, 4, 16, 20}) /* SPI0 */,
                                    __bitset({8, 12, 24, 28})  /* SPI1 */
                                  };
#endif
    if ((!_running) && ((1LL << pin) & valid[spi_get_index(_spi)])) {
        _RX = pin;
        return true;
    }

    if (_RX == pin) {
        return true;
    }

    if (_running) {
        panic("FATAL: Attempting to set SPI%s.RX while running", spi_get_index(_spi) ? "1" : "");
    } else {
        panic("FATAL: Attempting to set SPI%s.RX to illegal pin %d", spi_get_index(_spi) ? "1" : "", pin);
    }
    return false;
}

bool SPISlaveClass::setCS(pin_size_t pin) {
#if defined(PICO_RP2350) && !PICO_RP2350A // RP2350B
    constexpr uint64_t valid[2] = { __bitset({1, 5, 17, 21, 33, 37}) /* SPI0 */,
                                    __bitset({9, 13, 25, 29, 41, 45})  /* SPI1 */
                                  };
#else
    constexpr uint64_t valid[2] = { __bitset({1, 5, 17, 21}) /* SPI0 */,
                                    __bitset({9, 13, 25, 29})  /* SPI1 */
                                  };
#endif
    if ((!_running) && ((1LL << pin) & valid[spi_get_index(_spi)])) {
        _CS = pin;
        return true;
    }

    if (_CS == pin) {
        return true;
    }

    if (_running) {
        panic("FATAL: Attempting to set SPI%s.CS while running", spi_get_index(_spi) ? "1" : "");
    } else {
        panic("FATAL: Attempting to set SPI%s.CS to illegal pin %d", spi_get_index(_spi) ? "1" : "", pin);
    }
    return false;
}

bool SPISlaveClass::setSCK(pin_size_t pin) {
#if defined(PICO_RP2350) && !PICO_RP2350A // RP2350B
    constexpr uint64_t valid[2] = { __bitset({2, 6, 18, 22, 34, 38}) /* SPI0 */,
                                    __bitset({10, 14, 26, 30, 42, 46})  /* SPI1 */
                                  };
#else
    constexpr uint64_t valid[2] = { __bitset({2, 6, 18, 22}) /* SPI0 */,
                                    __bitset({10, 14, 26})  /* SPI1 */
                                  };
#endif
    if ((!_running) && ((1LL << pin) & valid[spi_get_index(_spi)])) {
        _SCK = pin;
        return true;
    }

    if (_SCK == pin) {
        return true;
    }

    if (_running) {
        panic("FATAL: Attempting to set SPI%s.SCK while running", spi_get_index(_spi) ? "1" : "");
    } else {
        panic("FATAL: Attempting to set SPI%s.SCK to illegal pin %d", spi_get_index(_spi) ? "1" : "", pin);
    }
    return false;
}

bool SPISlaveClass::setTX(pin_size_t pin) {
#if defined(PICO_RP2350) && !PICO_RP2350A // RP2350B
    constexpr uint64_t valid[2] = { __bitset({3, 7, 19, 23, 35, 39}) /* SPI0 */,
                                    __bitset({11, 15, 27, 31, 43, 47})  /* SPI1 */
                                  };
#else
    constexpr uint64_t valid[2] = { __bitset({3, 7, 19, 23}) /* SPI0 */,
                                    __bitset({11, 15, 27})  /* SPI1 */
                                  };
#endif
    if ((!_running) && ((1LL << pin) & valid[spi_get_index(_spi)])) {
        _TX = pin;
        return true;
    }

    if (_TX == pin) {
        return true;
    }

    if (_running) {
        panic("FATAL: Attempting to set SPI%s.TX while running", spi_get_index(_spi) ? "1" : "");
    } else {
        panic("FATAL: Attempting to set SPI%s.TX to illegal pin %d", spi_get_index(_spi) ? "1" : "", pin);
    }
    return false;
}

void SPISlaveClass::_handleIRQ() {
    // Attempt to read out all RX FIFO datra and return to callback
    uint8_t buff[8]; // SPI FIFO 8 deep max
    int cnt;
    for (cnt = 0; (cnt < 8) && spi_is_readable(_spi); cnt++) {
        buff[cnt] = spi_get_hw(_spi)->dr;
    }
    if (cnt && _recvCB) {
        _recvCB(buff, cnt);
    }
    // Attempt to send as many ytes to the TX FIFO as we have/are free
    while (spi_is_writable(_spi)) {
        for (; _dataLeft && spi_is_writable(_spi); _dataLeft--) {
            spi_get_hw(_spi)->dr = *(_dataOut++);
        }
        if (!_dataLeft && _sentCB) {
            _sentCB();
        }
    }
    // Disable the TX FIFO IRQ if there is still no data to send or we'd always be stuck in an IRQ
    // Will be re-enabled once user does a setData
    if (!_dataLeft) {
        spi_get_hw(_spi)->imsc = 2 | 4; // RTIM + RXIM
    }
}

void SPISlaveClass::_irq0() {
    SPISlave._handleIRQ();
}

void SPISlaveClass::_irq1() {
    SPISlave1._handleIRQ();
}

void SPISlaveClass::setData(const uint8_t *data, size_t len) {
    _dataOut = data;
    _dataLeft = len;
    if (_initted) {
        spi_get_hw(_spi)->imsc = 2 | 4 | 8;
    }
}


void SPISlaveClass::begin(SPISettings spis) {
    DEBUGSPI("SPISlave::begin(), rx=%d, cs=%d, sck=%d, tx=%d\n", _RX, _CS, _SCK, _TX);
    gpio_set_function(_RX, GPIO_FUNC_SPI);
    gpio_set_function(_CS, GPIO_FUNC_SPI);
    gpio_set_function(_SCK, GPIO_FUNC_SPI);
    gpio_set_function(_TX, GPIO_FUNC_SPI);
    if (_initted) {
        DEBUGSPI("SPISlave: deinitting currently active SPI\n");
        spi_deinit(_spi);
    }
    DEBUGSPI("SPISlave: initting SPI\n");
    spi_init(_spi, _spis.getClockFreq());
    DEBUGSPI("SPISlave: actual baudrate=%u\n", spi_get_baudrate(_spi));
    spi_set_slave(_spi, true);
    spi_set_format(_spi, 8, cpol(spis),	cpha(spis), SPI_MSB_FIRST);

    // Install our IRQ handler
    if (_spi == spi0) {
        irq_set_exclusive_handler(SPI0_IRQ, _irq0);
    } else {
        irq_set_exclusive_handler(SPI1_IRQ, _irq1);
    }

    // Set to get IRQs on transmit and receive
    spi_get_hw(_spi)->imsc = 2 | 4 | 8 ; // RTIM + RXIM + TXIM (not RORIM)
    _initted = true;
    irq_set_enabled(_spi == spi0 ? SPI0_IRQ : SPI1_IRQ, true);
}

void SPISlaveClass::end() {
    DEBUGSPI("SPISlave::end()\n");
    if (_initted) {
        DEBUGSPI("SPISlave: deinitting currently active SPI\n");
        if (_spi == spi0) {
            irq_remove_handler(SPI0_IRQ, _irq0);
        } else {
            irq_remove_handler(SPI1_IRQ, _irq1);
        }
        spi_deinit(_spi);
        _initted = false;
    }
    gpio_set_function(_RX, GPIO_FUNC_SIO);
    gpio_set_function(_CS, GPIO_FUNC_SIO);
    gpio_set_function(_SCK, GPIO_FUNC_SIO);
    gpio_set_function(_TX, GPIO_FUNC_SIO);
}

#ifndef __SPI0_DEVICE
#define __SPI0_DEVICE spi0
#endif
#ifndef __SPI1_DEVICE
#define __SPI1_DEVICE spi1
#endif

SPISlaveClass SPISlave(__SPI0_DEVICE, PIN_SPI0_MISO, PIN_SPI0_SS, PIN_SPI0_SCK, PIN_SPI0_MOSI);
SPISlaveClass SPISlave1(__SPI1_DEVICE, PIN_SPI1_MISO, PIN_SPI1_SS, PIN_SPI1_SCK, PIN_SPI1_MOSI);
