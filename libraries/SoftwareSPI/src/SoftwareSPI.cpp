/*
    PIO-based SPI Master library for the Raspberry Pi Pico RP2040

    Copyright (c) 2025 Earle F. Philhower, III <earlephilhower@yahoo.com>

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

#include "SoftwareSPI.h"
#include <hardware/gpio.h>
#include <hardware/structs/iobank0.h>
#include <hardware/irq.h>
#include "spi.pio.h"

#ifdef USE_TINYUSB
// For Serial when selecting TinyUSB.  Can't include in the core because Arduino IDE
// will not link in libraries called from the core.  Instead, add the header to all
// the standard libraries in the hope it will still catch some user cases where they
// use these libraries.
// See https://github.com/earlephilhower/arduino-pico/issues/167#issuecomment-848622174
#include <Adafruit_TinyUSB.h>
#endif

SoftwareSPI::SoftwareSPI(pin_size_t sck, pin_size_t miso, pin_size_t mosi, pin_size_t cs) {
    _running = false;
    _initted = false;
    _spis = SPISettings(1, LSBFIRST, SPI_MODE0); // Ensure spi_init called by setting current freq to 0
    _sck = sck;
    _miso = miso;
    _mosi = mosi;
    _cs = cs;
}

inline spi_cpol_t SoftwareSPI::cpol() {
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

inline spi_cpha_t SoftwareSPI::cpha() {
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

inline uint8_t SoftwareSPI::reverseByte(uint8_t b) {
    b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
    b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
    b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
    return b;
}

inline uint16_t SoftwareSPI::reverse16Bit(uint16_t w) {
    return (reverseByte(w & 0xff) << 8) | (reverseByte(w >> 8));
}

// The HW can't do LSB first, only MSB first, so need to bitreverse
void SoftwareSPI::adjustBuffer(const void *s, void *d, size_t cnt, bool by16) {
    if (_spis.getBitOrder() == MSBFIRST) {
        memcpy(d, s, cnt * (by16 ? 2 : 1));
    } else if (!by16) {
        const uint8_t *src = (const uint8_t *)s;
        uint8_t *dst = (uint8_t *)d;
        for (size_t i = 0; i < cnt; i++) {
            *(dst++) = reverseByte(*(src++));
        }
    } else { /* by16 */
        const uint16_t *src = (const uint16_t *)s;
        uint16_t *dst = (uint16_t *)d;
        for (size_t i = 0; i < cnt; i++) {
            *(dst++) = reverse16Bit(*(src++));
        }
    }
}

void SoftwareSPI::_adjustPIO(int bits) {
    if (_bits == bits) {
        return; // Nothing to do!
    }
    // Manually set the shiftctl and possibly Y for 8 bits
    pio_sm_set_enabled(_pio, _sm, false);
    uint32_t v = _pio->sm[_sm].shiftctrl ;
    v &= ~0x3e000000 | ~0x01f00000;
    if (bits == 8) {
        v |= 0x108 << 20; // Hardcode push/pull threshold 0'b0100001000, there is no simple accessor I can find
    } else {
        v |= 0x210 << 20; // 0b'1000010000
    }
    _pio->sm[_sm].shiftctrl = v;
    if (_hwCS) {
        pio_sm_exec(_pio, _sm, pio_encode_set(pio_x, bits - 2));
        pio_sm_exec(_pio, _sm, pio_encode_set(pio_y, bits - 2));
    }
    pio_sm_set_enabled(_pio, _sm, true);
    _bits = bits;
}

byte SoftwareSPI::transfer(uint8_t data) {
    uint8_t ret;
    if (!_initted) {
        return 0;
    }
    data = (_spis.getBitOrder() == MSBFIRST) ? data : reverseByte(data);
    DEBUGSPI("SPI::transfer(%02x), cpol=%d, cpha=%d\n", data, cpol(), cpha());
    _adjustPIO(8);
    io_rw_8 *txfifo = (io_rw_8 *) &_pio->txf[_sm];
    io_rw_8 *rxfifo = (io_rw_8 *) &_pio->rxf[_sm];
    while (pio_sm_is_tx_fifo_full(_pio, _sm)) { /* noop wait */ }
    *txfifo = data;
    while (pio_sm_is_rx_fifo_empty(_pio, _sm)) { /* noop wait for in data */ }
    ret = *rxfifo;
    ret = (_spis.getBitOrder() == MSBFIRST) ? ret : reverseByte(ret);
    DEBUGSPI("SPI: read back %02x\n", ret);
    return ret;
}

uint16_t SoftwareSPI::transfer16(uint16_t data) {
    uint16_t ret;
    if (!_initted) {
        return 0;
    }
    data = (_spis.getBitOrder() == MSBFIRST) ? data : reverse16Bit(data);
    DEBUGSPI("SPI::transfer16(%04x), cpol=%d, cpha=%d\n", data, cpol(), cpha());
    _adjustPIO(16);
    io_rw_16 *txfifo = (io_rw_16 *) &_pio->txf[_sm];
    io_rw_16 *rxfifo = (io_rw_16 *) &_pio->rxf[_sm];
    while (pio_sm_is_tx_fifo_full(_pio, _sm)) { /* noop wait */ }
    *txfifo = data;
    while (pio_sm_is_rx_fifo_empty(_pio, _sm)) { /* noop wait for in data */ }
    ret = *rxfifo;
    ret = (_spis.getBitOrder() == MSBFIRST) ? ret : reverse16Bit(ret);
    DEBUGSPI("SPI: read back %04x\n", ret);
    return ret;
}

void SoftwareSPI::transfer(void *buf, size_t count) {
    transfer(buf, buf, count);
}

void SoftwareSPI::transfer(const void *csrc, void *cdest, size_t count) {
    if (!_initted) {
        return;
    }
    DEBUGSPI("SPI::transfer(%p, %p %d)\n", csrc, cdest, count);
    const uint8_t *src = reinterpret_cast<const uint8_t *>(csrc);
    uint8_t *dest = reinterpret_cast<uint8_t *>(cdest);
    _adjustPIO(8);
    io_rw_8 *txfifo = (io_rw_8 *) &_pio->txf[_sm];
    io_rw_8 *rxfifo = (io_rw_8 *) &_pio->rxf[_sm];
    int txleft = count;
    int rxleft = count;

    if (_spis.getBitOrder() == !MSBFIRST) {
        // We're going to hack like heck here and reverse the txbuf into the receive buff (because txbuff is const
        // Then by construction SPI will send before it received, we can use the rx buff to trans and recv
        for (size_t i = 0; i < count; i++) {
            dest[i] = reverseByte(src[i]);
        }
        src = dest; // We'll transmit the flipped data...
    }

    while (txleft || rxleft) {
        while (txleft && !pio_sm_is_tx_fifo_full(_pio, _sm)) {
            *txfifo = *src++;
            txleft--;
        }
        while (rxleft && !pio_sm_is_rx_fifo_empty(_pio, _sm)) {
            *dest++ = *rxfifo;
            rxleft--;
        }
    }

    if (_spis.getBitOrder() == !MSBFIRST) {
        // Now we have data in recv but also need to flip it before returning to the app
        for (size_t i = 0; i < count; i++) {
            dest[i] = reverseByte(dest[i]);
        }
    }
    DEBUGSPI("SPI::transfer completed\n");
}

#ifdef PICO_RP2350B
#define GPIOIRQREGS 6
#else
#define GPIOIRQREGS 4
#endif

void SoftwareSPI::beginTransaction(SPISettings settings) {
    noInterrupts(); // Avoid possible race conditions if IRQ comes in while main app is in middle of this
    DEBUGSPI("SPI::beginTransaction(clk=%lu, bo=%s)\n", settings.getClockFreq(), (settings.getBitOrder() == MSBFIRST) ? "MSB" : "LSB");
    if (_initted && settings == _spis) {
        DEBUGSPI("SPI: Reusing existing initted SPI\n");
    } else {
        /* Only de-init if the clock changes frequency */
        if (settings.getClockFreq() != _spis.getClockFreq()) {
            DEBUGSPI("SPI: initting SPI\n");
            float divider = (float)rp2040.f_cpu() / (float)settings.getClockFreq();
            divider /= _hwCS ? 4.0f : 4.0f;
            pio_sm_set_clkdiv(_pio, _sm, divider);
            DEBUGSPI("SPI: divider=%f\n", divider);
        }
        _spis = settings;
        // Note we can only change frequency, not CPOL/CPHA (which would be physically not too useful anyway)
        _initted = true;
    }
    // Disable any IRQs that are being used for SPI
    io_bank0_irq_ctrl_hw_t *irq_ctrl_base = get_core_num() ? &iobank0_hw->proc1_irq_ctrl : &iobank0_hw->proc0_irq_ctrl;
    DEBUGSPI("SPI: IRQ masks before = %08x %08x %08x %08x %08x %08x\n", (unsigned)irq_ctrl_base->inte[0],
             (unsigned)irq_ctrl_base->inte[1], (unsigned)irq_ctrl_base->inte[2], (unsigned)irq_ctrl_base->inte[3],
             (GPIOIRQREGS > 4) ? (unsigned)irq_ctrl_base->inte[4] : 0, (GPIOIRQREGS > 5) ? (unsigned)irq_ctrl_base->inte[5] : 0);
    for (auto entry : _usingIRQs) {
        int gpio = entry.first;

        // There is no gpio_get_irq, so manually twiddle the register
        io_rw_32 *en_reg = &irq_ctrl_base->inte[gpio / 8];
        uint32_t val = ((*en_reg) >> (4 * (gpio % 8))) & 0xf;
        _usingIRQs.insert_or_assign(gpio, val);
        DEBUGSPI("SPI: GPIO %d = %lu\n", gpio, val);
        (*en_reg) ^= val << (4 * (gpio % 8));
    }
    DEBUGSPI("SPI: IRQ masks after = %08x %08x %08x %08x %08x %08x\n", (unsigned)irq_ctrl_base->inte[0],
             (unsigned)irq_ctrl_base->inte[1], (unsigned)irq_ctrl_base->inte[2], (unsigned)irq_ctrl_base->inte[3],
             (GPIOIRQREGS > 4) ? (unsigned)irq_ctrl_base->inte[4] : 0, (GPIOIRQREGS > 5) ? (unsigned)irq_ctrl_base->inte[5] : 0);
    interrupts();
}

void SoftwareSPI::endTransaction(void) {
    noInterrupts(); // Avoid race condition so the GPIO IRQs won't come back until all state is restored
    DEBUGSPI("SPI::endTransaction()\n");
    // Re-enable IRQs
    for (auto entry : _usingIRQs) {
        int gpio = entry.first;
        int mode = entry.second;
        gpio_set_irq_enabled(gpio, mode, true);
    }
    io_bank0_irq_ctrl_hw_t *irq_ctrl_base = get_core_num() ? &iobank0_hw->proc1_irq_ctrl : &iobank0_hw->proc0_irq_ctrl;
    (void) irq_ctrl_base;
    DEBUGSPI("SPI: IRQ masks = %08x %08x %08x %08x %08x %08x\n", (unsigned)irq_ctrl_base->inte[0], (unsigned)irq_ctrl_base->inte[1],
             (unsigned)irq_ctrl_base->inte[2], (unsigned)irq_ctrl_base->inte[3], (GPIOIRQREGS > 4) ? (unsigned)irq_ctrl_base->inte[4] : 0,
             (GPIOIRQREGS > 5) ? (unsigned)irq_ctrl_base->inte[5] : 0);
    interrupts();
}

bool SoftwareSPI::setCS(pin_size_t pin) {
    if (pin < 1) {
        // CS is SCK+1, so has to be at least GPIO1
        return false;
    }
    if (!_running || (_cs == pin)) {
        _cs = pin;
        _sck = _cs - 1;
        return true;
    }
    return false;
}

bool SoftwareSPI::setSCK(pin_size_t pin) {
    if (!_running || (_sck == pin)) {
        _sck = pin;
        _cs = pin + 1;
        return true;
    }
    return false;
}

bool SoftwareSPI::setMISO(pin_size_t pin) {
    if (!_running || (_miso == pin)) {
        _miso = pin;
        return true;
    }
    return false;
}

bool SoftwareSPI::setMOSI(pin_size_t pin) {
    if (!_running || (_mosi == pin)) {
        _mosi = pin;
        return true;
    }
    return false;
}

void SoftwareSPI::begin(bool hwCS) {
    DEBUGSPI("SPI::begin(%d), rx=%d, cs=%d, sck=%d, tx=%d\n", hwCS, _miso, _cs, _sck, _mosi);
    float divider = (float)rp2040.f_cpu() / (float)_spis.getClockFreq();
    DEBUGSPI("SPI: divider=%f\n", divider);
    if (!hwCS) {
        _spi = new PIOProgram(cpha() == SPI_CPHA_0 ? &spi_cpha0_program : &spi_cpha1_program);
        if (!_spi->prepare(&_pio, &_sm, &_off, _sck, 1)) {
            _running = false;
            delete _spi;
            _spi = nullptr;
            return;
        }
        pio_spi_init(_pio, _sm, _off, 8, divider / 4.0f, cpha(), cpol(), _sck, _mosi, _miso);
    } else {
        _spi = new PIOProgram(cpha() == SPI_CPHA_0 ? &spi_cpha0_cs_program : &spi_cpha1_cs_program);
        if (!_spi->prepare(&_pio, &_sm, &_off, _sck, 2)) {
            _running = false;
            delete _spi;
            _spi = nullptr;
            return;
        }
        pio_spi_cs_init(_pio, _sm, _off, 8, divider / 4.0f, cpha(), cpol(), _sck, _mosi, _miso);
    }
    _hwCS = hwCS;
    _bits = 8;
    // Give a default config in case user doesn't use beginTransaction
    beginTransaction(_spis);
    endTransaction();
}

void SoftwareSPI::end() {
    DEBUGSPI("SPI::end()\n");
    if (_initted) {
        DEBUGSPI("SPI: deinitting currently active SPI\n");
        _initted = false;
    }
    _spis = SPISettings(0, LSBFIRST, SPI_MODE0);
}

void SoftwareSPI::setBitOrder(BitOrder order) {
    _spis = SPISettings(_spis.getClockFreq(), order, _spis.getDataMode());
    beginTransaction(_spis);
    endTransaction();
}

void SoftwareSPI::setDataMode(uint8_t uc_mode) {
    _spis = SPISettings(_spis.getClockFreq(), _spis.getBitOrder(), uc_mode);
    beginTransaction(_spis);
    endTransaction();
}

void SoftwareSPI::setClockDivider(uint8_t uc_div) {
    (void) uc_div; // no-op
}
