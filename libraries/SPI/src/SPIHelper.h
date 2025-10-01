/*
    SPI internal helper utils library for the Raspberry Pi Pico RP2040

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

#include <Arduino.h>
#include <map>
#include <hardware/spi.h>
#include <hardware/gpio.h>
#include <hardware/structs/iobank0.h>
#include <hardware/irq.h>

/**
    @brief Helper routined shared by SPI and SoftwareSPI
*/
class SPIHelper {
public:
    SPIHelper() { /* noop */ }
    ~SPIHelper() { /* noop */ }

    /**
        @brief Returns the SDK CPOL setting for a given SPISettings configuration

        @param _spis SPISettings to parse
        @returns SDK-defined CPOL value
    */
    inline spi_cpol_t cpol(const SPISettings &_spis) {
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

    /**
        @brief Returns the SDK CPHA setting for a given SPISettings configuration

        @param _spis SPISettings to parse
        @returns SDK-defined CPHA value
    */
    inline spi_cpha_t cpha(const SPISettings &_spis) {
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

    /**
        @brief Reverses bits in a byte for MSB->LSB swapping

        @param b Input byte
        @returns Bit-reversed byte
    */
    inline uint8_t reverseByte(uint8_t b) {
        b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
        b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
        b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
        return b;
    }

    /**
        @brief MSB->LSB bit reversal for 16b quantities

        @param w 16-b input value
        @returns 16-bit reversed value
    */
    inline uint16_t reverse16Bit(uint16_t w) {
        return (reverseByte(w & 0xff) << 8) | (reverseByte(w >> 8));
    }

#if defined(PICO_RP2350) && !PICO_RP2350A // RP2350B
    static constexpr int GPIOIRQREGS = 6;
#else
    static constexpr int GPIOIRQREGS = 4;
#endif

    /**
        @brief Disables any GPIO interrupts registered before an SPI transaction begins
    */
    void maskInterrupts() {
        if (_usingIRQs.empty()) {
            return;
        }
        noInterrupts(); // Avoid possible race conditions if IRQ comes in while main app is in middle of this
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

    /**
        @brief Restores GPIO interrupts masks after an SPI transaction completes
    */
    void unmaskInterrupts() {
        if (_usingIRQs.empty()) {
            return;
        }
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


    /**
        @brief Adds an interrupt to be masked during SPI transactions

        @param interruptNumber GPIO number to mask off
    */
    void usingInterrupt(int interruptNumber) {
        _usingIRQs.insert({interruptNumber, 0});
    }

    /**
        @brief Removes an interrupt from the to-be-masked list for SPI transactions

        @param interruptNumber GPIO number to remove
    */
    void notUsingInterrupt(int interruptNumber) {
        _usingIRQs.erase(interruptNumber);
    }

private:
    std::map<int, int> _usingIRQs;
};
