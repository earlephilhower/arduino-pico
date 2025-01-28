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

#pragma once

#include <Arduino.h>
#include <SPI.h> // For SPIHelper
#include <api/HardwareSPI.h>
#include <hardware/spi.h>

/**
    @brief Implements a PIO-based SPI interface without pin restrictions
*/
class SoftwareSPI : public arduino::HardwareSPI {
public:
    /**
        @brief Create a PIO-based SPI instance

        @param [in] sck SCK GPIO
        @param [in] miso MISO GPIO
        @param [in] mosi MOSI GPIO
        @param [in] cs Optional CS pin for HW CS, must be SCK+1
    */
    SoftwareSPI(pin_size_t sck, pin_size_t miso, pin_size_t mosi, pin_size_t cs = -1);

    /**
        @brief Send an 8-bit byte of data and return read-back 8-bit value

        @param [in] data Data to send
        @returns Read back byte from SPI interface
    */
    byte transfer(uint8_t data) override;

    /**
        @brief Send a 16-bit quantity over SPI and return read-back 16-bit value under a single CS assertion

        @param [in] data Data to send
        @returns Read back 16-bit quantity
    */
    uint16_t transfer16(uint16_t data) override;

    /**
        @brief Sends buffer in 8 bit chunks under a single CS. Overwrites buffer with read data

        @param [in, out] buf Buffer to read and write back into
        @param [in] count Number of bytes to transmit/read
    */
    void transfer(void *buf, size_t count) override;

    /**
        @brief Sends one buffer and receives into another under a single CS. Can set rx or txbuf to nullptr

        @param [in] txbuf Buffer to transmit or nullptr to send 0s
        @param [out] rxbuf Buffer to read back into or nullptr to ignore returned data
        @param [in] count Numbner of bytes to transmit/receive
    */
    void transfer(const void *txbuf, void *rxbuf, size_t count) override;

    /**
        @brief Begin an SPI transaction, sets SPI speed and masks necessary interrupts

        @param [in] SPISettings SPI configuration parameters, including the clock speed
    */
    void beginTransaction(SPISettings settings) override;

    /**
        @brief Ends an SPI transaction, unmasks and masked GPIO interrupts
    */
    void endTransaction(void) override;

    /**
        @brief Sets the MISO(RX) pin. Call before begin()

        @param [in] pin The GPIO number to assign to
        @returns True on success
    */
    bool setMISO(pin_size_t pin);

    /**
        @brief Sets the MISO(RX) pin. Call before begin()

        @param [in] pin The GPIO number to assign to
        @returns True on success
    */
    inline bool setRX(pin_size_t pin) {
        return setMISO(pin);
    }

    /**
        @brief Sets the CS pin. Call before begin()

        @param [in] pin The GPIO number to assign to
        @returns True on success
    */
    bool setCS(pin_size_t pin);

    /**
        @brief Sets the SCK pin. Call before begin()

        @param [in] pin The GPIO number to assign to
        @returns True on success
    */
    bool setSCK(pin_size_t pin);

    /**
        @brief Sets the MOSI(TX) pin. Call before begin()

        @param [in] pin The GPIO number to assign to
        @returns True on success
    */
    bool setMOSI(pin_size_t pin);

    /**
        @brief Sets the MOSI(TX) pin. Call before begin()

        @param [in] pin The GPIO number to assign to
        @returns True on success
    */
    inline bool setTX(pin_size_t pin) {
        return setMOSI(pin);
    }

    /**
        @brief Call once to init/deinit SPI class, select pins, etc.
    */
    virtual void begin() override {
        begin(false);
    }

    /**
        @brief Call once to init/deinit SPI class, select pins, etc.

        @param [in] hwCS Pass in true to enable HW-controlled CS.  Otherwise application needs to assert/deassert CS.
    */
    void begin(bool hwCS);

    /**
        @brief Call to deinit and disable the SPI interface.
    */
    void end() override;

    /**
        @brief Deprecated, do not use

        @param [in] order Deprecated
    */
    void setBitOrder(BitOrder order) __attribute__((deprecated));

    /**
        @brief Deprecated, do not use

        @param [in] uc_mode Deprecated
    */
    void setDataMode(uint8_t uc_mode) __attribute__((deprecated));

    /**
        @brief Deprecated, do not use

        @param [in] uc_div Deprecated
    */
    void setClockDivider(uint8_t uc_div) __attribute__((deprecated));

    /**
        @brief Ensure specific GPIO interrupt is disabled during and SPI transaction to protect against re-entrancy.  Multiple GPIOs supported by multiple calls.

        @param [in] interruptNumber GPIO pin to mask
    */
    virtual void usingInterrupt(int interruptNumber) override {
        _helper.usingInterrupt(interruptNumber);
    }

    /**
        @brief Remove a GPIO from the masked-during-transaction list.

        @param [in] interruptNumber GPIO pin to unmask
    */
    virtual void notUsingInterrupt(int interruptNumber) override {
        _helper.notUsingInterrupt(interruptNumber);
    }

    /**
        @brief Deprecated, do not use
    */
    virtual void attachInterrupt() override __attribute__((deprecated)) { /* noop */ }

    /**
        @brief Deprecated, do not use
    */
    virtual void detachInterrupt() override __attribute__((deprecated)) { /* noop */ }

private:
    void _adjustPIO(int bits);

    PIOProgram *_spi;
    PIO _pio;
    int _sm;
    int _off;

    SPISettings _spis;
    pin_size_t _sck, _miso, _mosi, _cs;
    bool _hwCS;
    bool _running; // SPI port active
    bool _initted; // Transaction begun
    int _bits;
    SPIHelper _helper;
};
