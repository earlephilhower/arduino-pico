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

#include <Arduino.h>
#include <hardware/dma.h>
#include <hardware/gpio.h>
#include <hardware/i2c.h>
#include <hardware/irq.h>
#include <hardware/regs/intctrl.h>
#include "Wire.h"

#ifdef USE_TINYUSB
// For Serial when selecting TinyUSB.  Can't include in the core because Arduino IDE
// will not link in libraries called from the core.  Instead, add the header to all
// the standard libraries in the hope it will still catch some user cases where they
// use these libraries.
// See https://github.com/earlephilhower/arduino-pico/issues/167#issuecomment-848622174
#include <Adafruit_TinyUSB.h>
#endif

TwoWire::TwoWire(i2c_inst_t *i2c, pin_size_t sda, pin_size_t scl) {
    _sda = sda;
    _scl = scl;
    _i2c = i2c;
    _clkHz = TWI_CLOCK;
    _running = false;
    _txBegun = false;
    _buffLen = 0;

    // allocate buffer memory early, so we don't fragment the heap later
    _buffSize = WIRE_BUFFER_SIZE;
    _buff = (uint8_t *)malloc(_buffSize);
}

bool TwoWire::setSDA(pin_size_t pin) {
#if defined(PICO_RP2350) && !PICO_RP2350A // RP2350B
    constexpr uint64_t valid[2] = { __bitset({0, 4, 8, 12, 16, 20, 24, 28, 32, 36, 40, 44}) /* I2C0 */,
                                    __bitset({2, 6, 10, 14, 18, 22, 26, 30, 34, 38, 42, 46})  /* I2C1 */
                                  };
#else
    constexpr uint64_t valid[2] = { __bitset({0, 4, 8, 12, 16, 20, 24, 28}) /* I2C0 */,
                                    __bitset({2, 6, 10, 14, 18, 22, 26})  /* I2C1 */
                                  };
#endif
    if ((!_running) && ((1LL << pin) & valid[i2c_hw_index(_i2c)])) {
        _sda = pin;
        return true;
    }

    if (_sda == pin) {
        return true;
    }

    if (_running) {
        panic("FATAL: Attempting to set Wire%s.SDA while running", i2c_hw_index(_i2c) ? "1" : "");
    } else {
        panic("FATAL: Attempting to set Wire%s.SDA to illegal pin %d", i2c_hw_index(_i2c) ? "1" : "", pin);
    }
    return false;
}

bool TwoWire::setSCL(pin_size_t pin) {
#if defined(PICO_RP2350) && !PICO_RP2350A // RP2350B
    constexpr uint64_t valid[2] = { __bitset({1, 5, 9, 13, 17, 21, 25, 29, 33, 37, 41, 45}) /* I2C0 */,
                                    __bitset({3, 7, 11, 15, 19, 23, 27, 31, 35, 39, 43, 47})  /* I2C1 */
                                  };
#else
    constexpr uint64_t valid[2] = { __bitset({1, 5, 9, 13, 17, 21, 25, 29}) /* I2C0 */,
                                    __bitset({3, 7, 11, 15, 19, 23, 27})  /* I2C1 */
                                  };
#endif
    if ((!_running) && ((1LL << pin) & valid[i2c_hw_index(_i2c)])) {
        _scl = pin;
        return true;
    }

    if (_scl == pin) {
        return true;
    }

    if (_running) {
        panic("FATAL: Attempting to set Wire%s.SCL while running", i2c_hw_index(_i2c) ? "1" : "");
    } else {
        panic("FATAL: Attempting to set Wire%s.SCL to illegal pin %d", i2c_hw_index(_i2c) ? "1" : "", pin);
    }
    return false;
}

void TwoWire::setClock(uint32_t hz) {
    _clkHz = hz;
    if (_running) {
        i2c_set_baudrate(_i2c, hz);
    }
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

static void _handler0() {
#if defined(__WIRE0_DEVICE)
    if (__WIRE0_DEVICE == i2c0) {
        Wire.onIRQ();
    } else {
        Wire1.onIRQ();
    }
#else
    Wire.onIRQ();
#endif
}

static void _handler1() {
#if defined(__WIRE1_DEVICE)
    if (__WIRE1_DEVICE == i2c0) {
        Wire.onIRQ();
    } else {
        Wire1.onIRQ();
    }
#else
    Wire1.onIRQ();
#endif
}

// Slave mode
void TwoWire::begin(uint8_t addr) {
    if (_running) {
        // ERROR
        return;
    }

    // allocate buffer if necessary
    if (!_buff) {
        _buff = (uint8_t *)malloc(_buffSize);
        if (!_buff) {
            // ERROR
            return;
        }
    }

    _slave = true;
    i2c_init(_i2c, _clkHz);
    i2c_set_slave_mode(_i2c, true, addr);

    // Our callback IRQ
    _i2c->hw->intr_mask = (1 << 12) | (1 << 10) | (1 << 9) | (1 << 6) | (1 << 5) | (1 << 2);

    int irqNo = I2C0_IRQ + i2c_hw_index(_i2c);
    irq_set_exclusive_handler(irqNo, i2c_hw_index(_i2c) == 0 ? _handler0 : _handler1);
    irq_set_enabled(irqNo, true);

    gpio_set_function(_sda, GPIO_FUNC_I2C);
    gpio_pull_up(_sda);
    gpio_set_function(_scl, GPIO_FUNC_I2C);
    gpio_pull_up(_scl);

    _running = true;
}

// See: https://github.com/earlephilhower/arduino-pico/issues/979#issuecomment-1328237128
#pragma GCC push_options
#pragma GCC optimize ("O0")
void TwoWire::onIRQ() {
    // Make a local copy of the IRQ status up front.  If it changes while we're
    // running the IRQ callback will fire again after returning.  Avoids potential
    // race conditions
    uint32_t irqstat = _i2c->hw->intr_stat;
    if (irqstat == 0) {
        return;
    }

    // First, pull off any data available
    if (irqstat & (1 << 2)) {
        // RX_FULL
        if (_buffLen < int(_buffSize)) {
            _buff[_buffLen++] = _i2c->hw->data_cmd & 0xff;
        } else {
            _i2c->hw->data_cmd;
        }
    }
    // RD_REQ
    if (irqstat & (1 << 5)) {
        if (_onRequestCallback) {
            _onRequestCallback();
        }
        _i2c->hw->clr_rd_req;
    }
    // TX_ABRT
    if (irqstat & (1 << 6)) {
        _i2c->hw->clr_tx_abrt;
    }
    // START_DET
    if (irqstat & (1 << 10)) {
        _slaveStartDet = true;
        _i2c->hw->clr_start_det;
    }
    // RESTART_DET
    if (irqstat & (1 << 12)) {
        if (_onReceiveCallback && _buffLen) {
            _onReceiveCallback(_buffLen);
        }
        _buffLen = 0;
        _buffOff = 0;
        _slaveStartDet = false;
        _i2c->hw->clr_restart_det;
    }
    // STOP_DET
    if (irqstat & (1 << 9)) {
        if (_onReceiveCallback && _buffLen) {
            _onReceiveCallback(_buffLen);
        }
        _buffLen = 0;
        _buffOff = 0;
        _slaveStartDet = false;
        _i2c->hw->clr_stop_det;
    }
}
#pragma GCC pop_options

void TwoWire::end() {
    endAsync();

    if (!_running) {
        // ERROR
        return;
    }

    if (_slave) {
        int irqNo = I2C0_IRQ + i2c_hw_index(_i2c);
        irq_remove_handler(irqNo, i2c_hw_index(_i2c) == 0 ? _handler0 : _handler1);
        irq_set_enabled(irqNo, false);
    }

    i2c_deinit(_i2c);

    pinMode(_sda, INPUT);
    pinMode(_scl, INPUT);
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
    _buffOff = 0;
    _txBegun = true;
}

size_t TwoWire::requestFrom(uint8_t address, size_t quantity, bool stopBit) {
    if (!_running || _txBegun || !quantity || (quantity > _buffSize)) {
        return 0;
    }

    _buffLen = i2c_read_blocking_until(_i2c, address, _buff, quantity, !stopBit, make_timeout_time_ms(_timeout));
    if ((_buffLen == PICO_ERROR_GENERIC) || (_buffLen == PICO_ERROR_TIMEOUT)) {
        if (_buffLen == PICO_ERROR_TIMEOUT) {
            _handleTimeout(_reset_with_timeout);
        }
        _buffLen = 0;
    }
    _buffOff = 0;
    return _buffLen;
}

size_t TwoWire::requestFrom(uint8_t address, size_t quantity) {
    return requestFrom(address, quantity, true);
}

static bool _clockStretch(pin_size_t pin) {
    auto end = time_us_64() + 500;
    while ((time_us_64() < end) && (!digitalRead(pin))) { /* noop */ }
    return digitalRead(pin);
}

bool _probe(int addr, pin_size_t sda, pin_size_t scl, int freq) {
    int delay = (1000000 / freq) / 2;
    bool ack = false;

    pinMode(sda, INPUT_PULLUP);
    pinMode(scl, INPUT_PULLUP);
    gpio_set_function(scl, GPIO_FUNC_SIO);
    gpio_set_function(sda, GPIO_FUNC_SIO);

    digitalWrite(sda, HIGH);
    sleep_us(delay);
    digitalWrite(scl, HIGH);
    if (!_clockStretch(scl)) {
        goto stop;
    }
    digitalWrite(sda, LOW);
    sleep_us(delay);
    digitalWrite(scl, LOW);
    sleep_us(delay);
    for (int i = 0; i < 8; i++) {
        addr <<= 1;
        digitalWrite(sda, (addr & (1 << 7)) ? HIGH : LOW);
        sleep_us(delay);
        digitalWrite(scl, HIGH);
        sleep_us(delay);
        if (!_clockStretch(scl)) {
            goto stop;
        }
        digitalWrite(scl, LOW);
        sleep_us(5); // Ensure we don't change too close to clock edge
    }

    digitalWrite(sda, HIGH);
    sleep_us(delay);
    digitalWrite(scl, HIGH);
    if (!_clockStretch(scl)) {
        goto stop;
    }

    ack = digitalRead(sda) == LOW;
    sleep_us(delay);
    digitalWrite(scl, LOW);

stop:
    sleep_us(delay);
    digitalWrite(sda, LOW);
    sleep_us(delay);
    digitalWrite(scl, HIGH);
    sleep_us(delay);
    digitalWrite(sda, HIGH);
    sleep_us(delay);
    gpio_set_function(scl, GPIO_FUNC_I2C);
    gpio_set_function(sda, GPIO_FUNC_I2C);

    return ack;
}

void TwoWire::_handleTimeout(bool reset) {
    _timeoutFlag = true;

    if (reset) {
        if (_slave) {
            uint8_t prev_addr = _addr;
            int prev_clkHz = _clkHz;
            end();
            setClock(prev_clkHz);
            begin(prev_addr);
        } else {
            int prev_clkHz = _clkHz;
            end();

            // Attempt bus recovery if SDA is held LOW by another device
            // See RP2040 datasheet "Bus clear feature" (not implemented in HW)
            int delay = 5; //5us LOW/HIGH -> 10us period -> 100kHz freq
            pinMode(_sda, INPUT_PULLUP);
            pinMode(_scl, INPUT_PULLUP);
            gpio_set_function(_scl, GPIO_FUNC_SIO);
            gpio_set_function(_sda, GPIO_FUNC_SIO);

            if (digitalRead(_sda) == LOW) {
                int sclPulseCount = 0;
                while (sclPulseCount < 9 && digitalRead(_sda) == LOW) {
                    sclPulseCount++;
                    digitalWrite(_scl, LOW);
                    sleep_us(delay);
                    digitalWrite(_scl, HIGH);
                    sleep_us(delay);
                }

                if (digitalRead(_sda) == HIGH) {
                    // Bus recovered : send a STOP
                    digitalWrite(_sda, LOW);
                    sleep_us(delay);
                    digitalWrite(_sda, HIGH);
                }
            }

            setClock(prev_clkHz);
            begin();
        }
    }
}

// Errors:
//  0 : Success
//  1 : Data too long
//  2 : NACK on transmit of address
//  3 : NACK on transmit of data
//  4 : Other error
//  5 : Timeout
uint8_t TwoWire::endTransmission(bool stopBit) {
    if (!_running || !_txBegun) {
        return 4;
    }
    _txBegun = false;
    if (!_buffLen) {
        // Special-case 0-len writes which are used for I2C probing
        return _probe(_addr, _sda, _scl, _clkHz) ? 0 : 2;
    } else {
        auto len = _buffLen;
        auto ret = i2c_write_blocking_until(_i2c, _addr, _buff, _buffLen, !stopBit, make_timeout_time_ms(_timeout));
        if (ret == PICO_ERROR_TIMEOUT) {
            _handleTimeout(_reset_with_timeout);
            return 5;
        }
        _buffLen = 0;
        return (ret == len) ? 0 : 4;
    }
}

uint8_t TwoWire::endTransmission() {
    return endTransmission(true);
}

size_t TwoWire::write(uint8_t ucData) {
    if (!_running) {
        return 0;
    }

    if (_slave) {
        // Wait for a spot in the TX FIFO and return in case of timeout
        auto end = make_timeout_time_ms(_timeout);
        while ((i2c_get_write_available(_i2c) == 0) && !time_reached(end)) { /* noop wait */ }

        if (i2c_get_write_available(_i2c) > 0) {
            _i2c->hw->data_cmd = ucData;
            return 1;
        } else {
            _handleTimeout(_reset_with_timeout);
            return 0;
        }
    } else {
        if (!_txBegun || (_buffLen == int(_buffSize))) {
            return 0;
        }
        _buff[_buffLen++] = ucData;
        return 1 ;
    }
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

bool TwoWire::busIdle() {
    // Check hardware status
    uint32_t status = _i2c->hw->status;
    bool tfe = (status & I2C_IC_STATUS_TFE_BITS);
    bool mast = (status & I2C_IC_STATUS_MST_ACTIVITY_BITS);
    return (tfe && !mast);
}

// DMA/asynchronous transfers.  Do not combime with synchronous runs or bad stuff will happen
// All buffers must be valid for entire DMA and not touched until `finishedAsync()` returns true.
bool TwoWire::writeReadAsync(uint8_t address, const void *wbuffer, size_t wbytes, const void *rbuffer, size_t rbytes, bool sendStop) {
    if (!_running || _txBegun || (wbytes == 0 && rbytes == 0)) {
        return false;
    }

    if (!busIdle()) {
        return false;
    }

    if (!_dmaRunning) {
        beginAsync();
        if (!_dmaRunning) {
            return false;
        }
    }

    // Abort any ongoing transaction
    abortAsync();

    // Create or enlarge dma command buffer, we need one entry for every i2c byte we want to write/read
    const size_t bufLen = (wbytes + rbytes) * sizeof(uint16_t);
    if (_dmaSendBufferLen < bufLen) {
        if (_dmaSendBuffer) {
            free(_dmaSendBuffer);
            _dmaSendBuffer = nullptr;
            _dmaSendBufferLen = 0;
        }
        _dmaSendBuffer = (uint16_t *)malloc(bufLen);
        if (!_dmaSendBuffer) {
            return false;
        }
        _dmaSendBufferLen = bufLen;
    }

    // Fill the dma command buffer
    for (size_t i = 0; i < wbytes; i++) {
        _dmaSendBuffer[i] = ((uint8_t*) wbuffer)[i];
    }
    for (size_t i = 0; i < rbytes; i++) {
        _dmaSendBuffer[wbytes + i] = I2C_IC_DATA_CMD_CMD_BITS; // -> 1 for read
    }
    if (_i2c->restart_on_next) {
        _dmaSendBuffer[0] |= I2C_IC_DATA_CMD_RESTART_BITS;
    }
    if (wbytes > 0 && rbytes > 0) {
        _dmaSendBuffer[wbytes + 0] |= I2C_IC_DATA_CMD_RESTART_BITS;
    }
    if (sendStop) {
        _dmaSendBuffer[wbytes + rbytes - 1] |= I2C_IC_DATA_CMD_STOP_BITS;
    }

    // Cleanup and Setup dma send channel
    dma_channel_cleanup(_dmaChannelSend);
    dma_channel_config c = dma_channel_get_default_config(_dmaChannelSend);
    channel_config_set_transfer_data_size(&c, DMA_SIZE_16); // 16b transfers into I2C FIFO
    channel_config_set_read_increment(&c, true); // Reading incrementing addresses
    channel_config_set_write_increment(&c, false); // Writing to the same FIFO address
    channel_config_set_dreq(&c, i2c_get_dreq(_i2c, true)); // Wait for the TX FIFO specified
    channel_config_set_chain_to(&c, _dmaChannelSend); // No chaining
    channel_config_set_irq_quiet(&c, false); // Enable interrupt (can be disabled later with dma_channel_set_irq0_enabled() as needed)
    dma_channel_configure(_dmaChannelSend, &c, &_i2c->hw->data_cmd, _dmaSendBuffer, wbytes + rbytes, false);

    // Cleanup and setup dma receive channel
    dma_channel_cleanup(_dmaChannelReceive);
    c = dma_channel_get_default_config(_dmaChannelReceive);
    channel_config_set_transfer_data_size(&c, DMA_SIZE_8); // 8b transfers from I2C FIFO
    channel_config_set_read_increment(&c, false); // Reading same FIFO address
    channel_config_set_write_increment(&c, true); // Writing to the buffer
    channel_config_set_dreq(&c, i2c_get_dreq(_i2c, false)); // Wait for the RX FIFO specified
    channel_config_set_chain_to(&c, _dmaChannelReceive); // No chaining
    channel_config_set_irq_quiet(&c, false); // Enable interrupt (can be disabled later with dma_channel_set_irq0_enabled() as needed)
    dma_channel_configure(_dmaChannelReceive, &c, (void*) rbuffer, &_i2c->hw->data_cmd, rbytes, false);

    // Enable dma completed interrupt
    dma_channel_set_irq0_enabled(_dmaChannelSend, (rbytes == 0)); //write only, enable irq on Send channel
    dma_channel_set_irq0_enabled(_dmaChannelReceive, (rbytes > 0)); //when reading, enable irq on Receive channel

    // Setup i2c hardware
    _i2c->hw->enable = 0;
    _i2c->hw->tar = address;
    _i2c->hw->dma_cr = 1 << 1 | (rbytes > 0 ? 1 : 0) ; // TDMAE + RDMAE when rbytes>0
    _i2c->hw->enable = 1;
    _i2c->restart_on_next = !sendStop;

    // Start dma channel(s)
    _txBegun = true;
    _dmaFinished = false;
    if (rbytes > 0) {
        dma_channel_start(_dmaChannelReceive);
    }
    dma_channel_start(_dmaChannelSend);

    return true;
}
bool TwoWire::writeAsync(uint8_t address, const void *buffer, size_t bytes, bool sendStop) {
    return writeReadAsync(address, buffer, bytes, nullptr, 0, sendStop);
}

bool TwoWire::readAsync(uint8_t address, void *buffer, size_t bytes, bool sendStop) {
    return writeReadAsync(address, nullptr, 0, buffer, bytes, sendStop);
}

bool TwoWire::finishedAsync() {
    return _dmaFinished;
}

void TwoWire::abortAsync() {
    if (!_dmaRunning) {
        return;
    }
    if (!_dmaFinished) {
        dma_channel_abort(_dmaChannelSend);
        dma_channel_abort(_dmaChannelReceive);
        _i2c->hw->dma_cr = 0;
    }
    _txBegun = false;
    _dmaFinished = true;
}

void TwoWire::onFinishedAsync(void(*function)(void)) {
    _dmaOnFinished = function;
}

// Dma irq mask and wire instance for low level dma completed handlers
static uint32_t _dma_i2c0_irq_mask = 0;
static uint32_t _dma_i2c1_irq_mask = 0;
static TwoWire * _dma_i2c0_wire_instance = nullptr;
static TwoWire * _dma_i2c1_wire_instance = nullptr;

// Low level dma completed handlers, calls TwoWire::_dma_irq_handler() to do the work
void _dma_i2c0_irq_handler() {
    uint32_t status = dma_hw->ints0;
    if (status & _dma_i2c0_irq_mask && _dma_i2c0_wire_instance) {
        _dma_i2c0_wire_instance->_dma_irq_handler();
    }
    dma_hw->ints0 = (status & _dma_i2c0_irq_mask); //clear interrupt status
}

void _dma_i2c1_irq_handler() {
    uint32_t status = dma_hw->ints0;
    if (status & _dma_i2c1_irq_mask && _dma_i2c1_wire_instance) {
        _dma_i2c1_wire_instance->_dma_irq_handler();
    }
    dma_hw->ints0 = (status & _dma_i2c1_irq_mask); //clear interrupt status
}

void TwoWire::_dma_irq_handler() {
    _i2c->hw->dma_cr = 0;
    _txBegun = false;
    _dmaFinished = true;
    // Disable the DMA IRQs
    dma_channel_set_irq0_enabled(_dmaChannelSend, false);
    dma_channel_set_irq0_enabled(_dmaChannelReceive, false);
    // Call the user handler
    if (_dmaOnFinished) {
        _dmaOnFinished();
    }
}

void TwoWire::beginAsync() {
    if (_dmaRunning) {
        return;
    }

    // Claim dma channels
    _dmaChannelReceive = dma_claim_unused_channel(false);
    if (_dmaChannelReceive == -1) {
        return;
    }
    _dmaChannelSend = dma_claim_unused_channel(false);
    if (_dmaChannelSend == -1) {
        dma_channel_unclaim(_dmaChannelReceive);
        return;
    }

    // Setup dma irq
    if (i2c_hw_index(_i2c) == 0) {
        _dma_i2c0_irq_mask = (1u << _dmaChannelReceive) | (1u << _dmaChannelSend);
        _dma_i2c0_wire_instance = this;
        irq_add_shared_handler(DMA_IRQ_0, _dma_i2c0_irq_handler, PICO_SHARED_IRQ_HANDLER_DEFAULT_ORDER_PRIORITY);
    } else {
        _dma_i2c1_irq_mask = (1u << _dmaChannelReceive) | (1u << _dmaChannelSend);
        _dma_i2c1_wire_instance = this;
        irq_add_shared_handler(DMA_IRQ_0, _dma_i2c1_irq_handler, PICO_SHARED_IRQ_HANDLER_DEFAULT_ORDER_PRIORITY);
    }
    irq_set_enabled(DMA_IRQ_0, true);

    _dmaRunning = true;
}

void TwoWire::endAsync() {
    if (!_dmaRunning) {
        return;
    }

    if (i2c_hw_index(_i2c) == 0) {
        _dma_i2c0_irq_mask = 0;
        _dma_i2c0_wire_instance = nullptr;
        irq_remove_handler(DMA_IRQ_0, _dma_i2c0_irq_handler);
    } else {
        _dma_i2c1_irq_mask = 0;
        _dma_i2c1_wire_instance = nullptr;
        irq_remove_handler(DMA_IRQ_0, _dma_i2c1_irq_handler);
    }
    if (_dmaChannelReceive >= 0) {
        dma_channel_cleanup(_dmaChannelReceive);
        dma_channel_unclaim(_dmaChannelReceive);
        _dmaChannelReceive = -1;
    }
    if (_dmaChannelSend >= 0) {
        dma_channel_cleanup(_dmaChannelSend);
        dma_channel_unclaim(_dmaChannelSend);
        _dmaChannelSend = -1;
    }
    _i2c->hw->dma_cr = 0;
    free(_dmaSendBuffer);
    _dmaSendBuffer = nullptr;
    _dmaSendBufferLen = 0;
    _txBegun = false;
    _dmaFinished = true;
    _dmaOnFinished = nullptr;
    _dmaRunning = false;
}

void TwoWire::onReceive(void(*function)(int)) {
    _onReceiveCallback = function;
}

void TwoWire::onRequest(void(*function)(void)) {
    _onRequestCallback = function;
}

void TwoWire::setTimeout(uint32_t timeout, bool reset_with_timeout) {
    _timeoutFlag = false;
    Stream::setTimeout(timeout);
    _reset_with_timeout = reset_with_timeout;
}

bool TwoWire::getTimeoutFlag() {
    return _timeoutFlag;
}

void TwoWire::clearTimeoutFlag() {
    _timeoutFlag = false;
}

size_t TwoWire::setBufferSize(size_t bSize) {
    if (_running) {
        // ERROR - transmission already running. Report back current buffer size
        return _buffSize;
    }
    // only free the buffer, if it already exists and the new size differs from the current one
    if (_buff && (bSize != _buffSize)) {
        free(_buff);
        _buff = nullptr;
    }
    _buffSize = max(WIRE_BUFFER_SIZE_MIN, int(bSize)); // enforce minimum buffer size
    return _buffSize;
}

#ifndef __WIRE0_DEVICE
#define __WIRE0_DEVICE i2c0
#endif
#ifndef __WIRE1_DEVICE
#define __WIRE1_DEVICE i2c1
#endif

#ifdef PIN_WIRE0_SDA
TwoWire Wire(__WIRE0_DEVICE, PIN_WIRE0_SDA, PIN_WIRE0_SCL);
#endif

#ifdef PIN_WIRE1_SDA
TwoWire Wire1(__WIRE1_DEVICE, PIN_WIRE1_SDA, PIN_WIRE1_SCL);
#endif
