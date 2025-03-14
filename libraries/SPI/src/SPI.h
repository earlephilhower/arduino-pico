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

#pragma once

#include <Arduino.h>
#include <api/HardwareSPI.h>
#include <hardware/spi.h>
#include "SPIHelper.h"

/**
    @brief Implements a hardware-based SPI interface using the Pico's SPI blocks
*/
class SPIClassRP2040 : public arduino::HardwareSPI {
public:
    /**
            @brief Create a PIO-based SPI instance, pins can be changed before begin() call

            @param [in] spi SPI hardware instance (spi0/spi1)
            @param [in] rx MISO GPIO
            @param [in] cs CS GPIO
            @param [in] sck SCK GPIO
            @param [in] tx MOSI GPIO
    */
    SPIClassRP2040(spi_inst_t *spi, pin_size_t rx, pin_size_t cs, pin_size_t sck, pin_size_t tx);

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

    // DMA/asynchronous transfers.  Do not combime with synchronous runs or bad stuff will happen
    // All buffers must be valid for entire DMA and not touched until `finished()` returns true.
    /**
        @brief Perform a transfer() using DMA in the background. Returns immediately, need to check for completion

        @details
        Do not combine asynchronous and synchronous transfers.  All buffers must be valid until
        the transfer reports that it is completed (``finished`` returns true).

        @param [in] send Buffer to transmit, must remain valid through entire operation
        @param [out] recv Buffer to receive, must remain valid through entire operation
        @param [in] bytes Number of bytes to transfer under single CS
    */
    bool transferAsync(const void *send, void *recv, size_t bytes);
    /**
        @brief Call to check if the async operations is completed and the buffer can be reused/read

        @returns True if the asynchronous SPI operation has completed and ``recv`` buffer is valid
    */
    bool finishedAsync();

    /**
        @brief Aborts an ongoing asynchronous SPI operation, if one is still operating

        @details
        Not normally needed, but in the case where a large, long SPI operation needs to be aborted
        this call allows an application to safely stop the SPI and dispose of the ``recv`` and
        ``send`` buffers
    */
    void abortAsync();


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
    bool setRX(pin_size_t pin);

    /**
            @brief Sets the MISO(RX) pin. Call before begin()

            @param [in] pin The GPIO number to assign to
            @returns True on success
    */
    inline bool setMISO(pin_size_t pin) {
        return setRX(pin);
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
    bool setTX(pin_size_t pin);

    /**
          @brief Sets the MOSI(TX) pin. Call before begin()

          @param [in] pin The GPIO number to assign to
          @returns True on success
    */
    inline bool setMOSI(pin_size_t pin) {
        return setTX(pin);
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

            @param [in] order Deprecated
    */
    void setDataMode(uint8_t uc_mode) __attribute__((deprecated));

    /**
            @brief Deprecated, do not use

            @param [in] order Deprecated
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
    virtual void attachInterrupt() override  __attribute__((deprecated)) { /* noop */ }

    /**
           @brief Deprecated, do not use
    */
    virtual void detachInterrupt() override  __attribute__((deprecated)) { /* noop */ }

private:
    void adjustBuffer(const void *s, void *d, size_t cnt, bool by16);

    spi_inst_t *_spi;
    SPISettings _spis;
    pin_size_t _RX, _TX, _SCK, _CS;
    bool _hwCS;
    bool _running; // SPI port active
    bool _initted; // Transaction begun

    // DMA
    int _channelDMA;
    int _channelSendDMA;
    uint8_t *_dmaBuffer = nullptr;
    int _dmaBytes;
    uint8_t *_rxFinalBuffer;
    uint32_t _dummy;
    SPIHelper _helper;
};

extern SPIClassRP2040 SPI;
extern SPIClassRP2040 SPI1;
