/*
    SPI Master library for the Raspberry Pi Pico RP2040

    Copyright (c) 2021 Earle F. Philhower, III <earlephilhower@yahoo.com>

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

#include "SPI.h"
#include <hardware/dma.h>
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

SPIClassRP2040::SPIClassRP2040(spi_inst_t *spi, pin_size_t rx, pin_size_t cs, pin_size_t sck, pin_size_t tx) {
    _spi = spi;
    _running = false;
    _initted = false;
    _spis = SPISettings(0, LSBFIRST, SPI_MODE0); // Ensure spi_init called by setting current freq to 0
    _RX = rx;
    _TX = tx;
    _SCK = sck;
    _CS = cs;
}

// The HW can't do LSB first, only MSB first, so need to bitreverse
void SPIClassRP2040::adjustBuffer(const void *s, void *d, size_t cnt, bool by16) {
    if (_spis.getBitOrder() == MSBFIRST) {
        memcpy(d, s, cnt * (by16 ? 2 : 1));
    } else if (!by16) {
        const uint8_t *src = (const uint8_t *)s;
        uint8_t *dst = (uint8_t *)d;
        for (size_t i = 0; i < cnt; i++) {
            *(dst++) = _helper.reverseByte(*(src++));
        }
    } else { /* by16 */
        const uint16_t *src = (const uint16_t *)s;
        uint16_t *dst = (uint16_t *)d;
        for (size_t i = 0; i < cnt; i++) {
            *(dst++) = _helper.reverse16Bit(*(src++));
        }
    }
}

byte SPIClassRP2040::transfer(uint8_t data) {
    uint8_t ret;
    if (!_initted) {
        return 0;
    }
    data = (_spis.getBitOrder() == MSBFIRST) ? data : _helper.reverseByte(data);
    DEBUGSPI("SPI::transfer(%02x), cpol=%d, cpha=%d\n", data, _helper.cpol(_spis), _helper.cpha(_spis));
    hw_write_masked(&spi_get_hw(_spi)->cr0, (8 - 1) << SPI_SSPCR0_DSS_LSB, SPI_SSPCR0_DSS_BITS); // Fast set to 8-bits
    spi_write_read_blocking(_spi, &data, &ret, 1);
    ret = (_spis.getBitOrder() == MSBFIRST) ? ret : _helper.reverseByte(ret);
    DEBUGSPI("SPI: read back %02x\n", ret);
    return ret;
}

uint16_t SPIClassRP2040::transfer16(uint16_t data) {
    uint16_t ret;
    if (!_initted) {
        return 0;
    }
    data = (_spis.getBitOrder() == MSBFIRST) ? data : _helper.reverse16Bit(data);
    DEBUGSPI("SPI::transfer16(%04x), cpol=%d, cpha=%d\n", data, _helper.cpol(_spis), _helper.cpha(_spis));
    hw_write_masked(&spi_get_hw(_spi)->cr0, (16 - 1) << SPI_SSPCR0_DSS_LSB, SPI_SSPCR0_DSS_BITS); // Fast set to 16-bits
    spi_write16_read16_blocking(_spi, &data, &ret, 1);
    ret = (_spis.getBitOrder() == MSBFIRST) ? ret : _helper.reverse16Bit(ret);
    DEBUGSPI("SPI: read back %02x\n", ret);
    return ret;
}

void SPIClassRP2040::transfer(void *buf, size_t count) {
    DEBUGSPI("SPI::transfer(%p, %d)\n", buf, count);
    uint8_t *buff = reinterpret_cast<uint8_t *>(buf);
    for (size_t i = 0; i < count; i++) {
        *buff = transfer(*buff);
        buff++;
    }
    DEBUGSPI("SPI::transfer completed\n");
}

void SPIClassRP2040::transfer(const void *txbuf, void *rxbuf, size_t count) {
    if (!_initted) {
        return;
    }

    hw_write_masked(&spi_get_hw(_spi)->cr0, (8 - 1) << SPI_SSPCR0_DSS_LSB, SPI_SSPCR0_DSS_BITS); // Fast set to 8-bits

    DEBUGSPI("SPI::transfer(%p, %p, %d)\n", txbuf, rxbuf, count);
    const uint8_t *txbuff = reinterpret_cast<const uint8_t *>(txbuf);
    uint8_t *rxbuff = reinterpret_cast<uint8_t *>(rxbuf);

    // MSB version is easy!
    if (_spis.getBitOrder() == MSBFIRST) {
        if (rxbuf == nullptr) { // transmit only!
            spi_write_blocking(_spi, txbuff, count);
            return;
        }
        if (txbuf == nullptr) { // receive only!
            spi_read_blocking(_spi, 0xFF, rxbuff, count);
            return;
        }
        // transmit and receive!
        spi_write_read_blocking(_spi, txbuff, rxbuff, count);
        return;
    }

    // If its LSB this isn't nearly as fun, we'll just let transfer(x) do it :(
    for (size_t i = 0; i < count; i++) {
        *rxbuff = transfer(*txbuff);
        *rxbuff = (_spis.getBitOrder() == MSBFIRST) ? *rxbuff : _helper.reverseByte(*rxbuff);
        txbuff++;
        rxbuff++;
    }
    DEBUGSPI("SPI::transfer completed\n");
}

void SPIClassRP2040::beginTransaction(SPISettings settings) {
    DEBUGSPI("SPI::beginTransaction(clk=%lu, bo=%s)\n", settings.getClockFreq(), (settings.getBitOrder() == MSBFIRST) ? "MSB" : "LSB");
    if (_initted && settings == _spis) {
        DEBUGSPI("SPI: Reusing existing initted SPI\n");
    } else {
        /* Only de-init if the clock changes frequency */
        if (settings.getClockFreq() != _spis.getClockFreq()) {
            if (_initted) {
                DEBUGSPI("SPI: deinitting currently active SPI\n");
                spi_deinit(_spi);
            }
            DEBUGSPI("SPI: initting SPI\n");
            spi_init(_spi, settings.getClockFreq());
            DEBUGSPI("SPI: actual baudrate=%u\n", spi_get_baudrate(_spi));
        }
        _spis = settings;
        spi_set_format(_spi, 8, _helper.cpol(_spis), _helper.cpha(_spis), SPI_MSB_FIRST);
        _initted = true;
    }
    _helper.maskInterrupts();
}

void SPIClassRP2040::endTransaction(void) {
    DEBUGSPI("SPI::endTransaction()\n");
    _helper.unmaskInterrupts();
}

bool SPIClassRP2040::transferAsync(const void *send, void *recv, size_t bytes) {
    DEBUGSPI("SPI::transferAsync(%p, %p, %d)\n", send, recv, bytes);
    const uint8_t *txbuff = reinterpret_cast<const uint8_t *>(send);
    uint8_t *rxbuff = reinterpret_cast<uint8_t *>(recv);
    _dummy = 0xffffffff;

    if (!_initted || (!send && !recv)) {
        return false;
    }

    _channelDMA = dma_claim_unused_channel(false);
    if (_channelDMA == -1) {
        return false;
    }
    _channelSendDMA = dma_claim_unused_channel(false);
    if (_channelSendDMA == -1) {
        dma_channel_unclaim(_channelDMA);
        return false;
    }

    if (send && (_spis.getBitOrder() != MSBFIRST)) {
        _dmaBuffer = (uint8_t *)malloc(bytes);
        if (!_dmaBuffer) {
            dma_channel_unclaim(_channelDMA);
            dma_channel_unclaim(_channelSendDMA);
            return false;
        }
        for (size_t i = 0; i < bytes; i++) {
            _dmaBuffer[i] = _helper.reverseByte(txbuff[i]);
        }
    }
    _dmaBytes = bytes;
    _rxFinalBuffer = rxbuff;

    hw_write_masked(&spi_get_hw(_spi)->cr0, (8 - 1) << SPI_SSPCR0_DSS_LSB, SPI_SSPCR0_DSS_BITS); // Fast set to 8-bits

    dma_channel_config c = dma_channel_get_default_config(_channelSendDMA);
    channel_config_set_transfer_data_size(&c, DMA_SIZE_8); // 8b transfers into SPI FIFO
    channel_config_set_read_increment(&c, send ? true : false); // Reading incrementing addresses
    channel_config_set_write_increment(&c, false); // Writing to the same FIFO address
    channel_config_set_dreq(&c, spi_get_dreq(_spi, true)); // Wait for the TX FIFO specified
    channel_config_set_chain_to(&c, _channelSendDMA); // No chaining
    channel_config_set_irq_quiet(&c, true); // No need for IRQ
    dma_channel_configure(_channelSendDMA, &c, &spi_get_hw(_spi)->dr, !send ? (uint8_t *)&_dummy : (_spis.getBitOrder() != MSBFIRST ? _dmaBuffer : txbuff), bytes, false);

    c = dma_channel_get_default_config(_channelDMA);
    channel_config_set_transfer_data_size(&c, DMA_SIZE_8); // 8b transfers into SPI FIFO
    channel_config_set_read_increment(&c, false); // Reading same FIFO address
    channel_config_set_write_increment(&c, recv ? true : false); // Writing to the buffer
    channel_config_set_dreq(&c, spi_get_dreq(_spi, false)); // Wait for the RX FIFO specified
    channel_config_set_chain_to(&c, _channelDMA); // No chaining
    channel_config_set_irq_quiet(&c, true); // No need for IRQ
    dma_channel_configure(_channelDMA, &c, !recv ? (uint8_t *)&_dummy : rxbuff, &spi_get_hw(_spi)->dr, bytes, false);

    spi_get_hw(_spi)->dmacr = 1 | (1 << 1); // TDMAE | RDMAE

    dma_channel_start(_channelDMA);
    dma_channel_start(_channelSendDMA);
    return true;
}

bool SPIClassRP2040::finishedAsync() {
    if (!_initted) {
        return true;
    }
    if (dma_channel_is_busy(_channelDMA) || (spi_get_hw(_spi)->sr & SPI_SSPSR_BSY_BITS)) {
        return false;
    }
    dma_channel_cleanup(_channelDMA);
    dma_channel_unclaim(_channelDMA);
    dma_channel_cleanup(_channelSendDMA);
    dma_channel_unclaim(_channelSendDMA);
    spi_get_hw(_spi)->dmacr = 0;
    if (_spis.getBitOrder() != MSBFIRST) {
        for (int i = 0; i < _dmaBytes; i++) {
            _rxFinalBuffer[i] = _helper.reverseByte(_rxFinalBuffer[i]);
        }
        free(_dmaBuffer);
        _dmaBuffer = nullptr;
    }
    return true;
}

void SPIClassRP2040::abortAsync() {
    if (!_initted) {
        return;
    }
    dma_channel_cleanup(_channelDMA);
    dma_channel_unclaim(_channelDMA);
    dma_channel_cleanup(_channelSendDMA);
    dma_channel_unclaim(_channelSendDMA);
    spi_get_hw(_spi)->dmacr = 0;
    free(_dmaBuffer);
    _dmaBuffer = nullptr;
}


bool SPIClassRP2040::setRX(pin_size_t pin) {
#if defined(PICO_RP2350) && !PICO_RP2350A // RP2350B
    constexpr uint64_t valid[2] = { __bitset({0, 4, 16, 20, 32, 36}) /* SPI0 */,
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

bool SPIClassRP2040::setCS(pin_size_t pin) {
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

bool SPIClassRP2040::setSCK(pin_size_t pin) {
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

bool SPIClassRP2040::setTX(pin_size_t pin) {
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
    endTransaction();
}

void SPIClassRP2040::end() {
    DEBUGSPI("SPI::end()\n");
    if (_initted) {
        DEBUGSPI("SPI: deinitting currently active SPI\n");
        _initted = false;
        spi_deinit(_spi);
    }
    gpio_set_function(_RX, GPIO_FUNC_SIO);
    if (_hwCS) {
        gpio_set_function(_CS, GPIO_FUNC_SIO);
    }
    gpio_set_function(_SCK, GPIO_FUNC_SIO);
    gpio_set_function(_TX, GPIO_FUNC_SIO);
    _spis = SPISettings(0, LSBFIRST, SPI_MODE0);
}

void SPIClassRP2040::setBitOrder(BitOrder order) {
    _spis = SPISettings(_spis.getClockFreq(), order, _spis.getDataMode());
    beginTransaction(_spis);
    endTransaction();
}

void SPIClassRP2040::setDataMode(uint8_t uc_mode) {
    _spis = SPISettings(_spis.getClockFreq(), _spis.getBitOrder(), uc_mode);
    beginTransaction(_spis);
    endTransaction();
}

void SPIClassRP2040::setClockDivider(uint8_t uc_div) {
    (void) uc_div; // no-op
}

#ifndef __SPI0_DEVICE
#define __SPI0_DEVICE spi0
#endif
#ifndef __SPI1_DEVICE
#define __SPI1_DEVICE spi1
#endif

#ifdef PIN_SPI0_MISO
SPIClassRP2040 SPI(__SPI0_DEVICE, PIN_SPI0_MISO, PIN_SPI0_SS, PIN_SPI0_SCK, PIN_SPI0_MOSI);
#endif

#ifdef PIN_SPI1_MISO
SPIClassRP2040 SPI1(__SPI1_DEVICE, PIN_SPI1_MISO, PIN_SPI1_SS, PIN_SPI1_SCK, PIN_SPI1_MOSI);
#endif
