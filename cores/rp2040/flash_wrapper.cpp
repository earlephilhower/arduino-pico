/*
    Flash wrappers to protect PSRAM access on the RP2350

    Copyright (c) 2024 Earle F. Philhower, III <earlephilhower@yahoo.com>

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
#include <hardware/flash.h>

#ifdef PICO_RP2350
#include <hardware/structs/qmi.h>
#endif

#if defined(PICO_RP2350) && defined(RP2350_PSRAM_CS)
static void __no_inline_not_in_flash_func(flushcache)() {
    // From https://forums.raspberrypi.com/viewtopic.php?t=378249#p2263677
    // Perform clean-by-set/way on all lines
    for (uint32_t i = 0; i < 2048; ++i) {
        // Use the upper 16k of the maintenance space (0x1bffc000 through 0x1bffffff):
        *(volatile uint8_t*)(XIP_SRAM_BASE + (XIP_MAINTENANCE_BASE - XIP_BASE) + i * 8u + 0x1u) = 0;
    }
}
#elif defined(PICO_RP2350)
static void __no_inline_not_in_flash_func(flushcache)() {
    // Null
}
#endif


extern "C" {
    extern void __real_flash_range_erase(uint32_t flash_offs, size_t count);
    void __wrap_flash_range_erase(uint32_t flash_offs, size_t count) {
#ifdef PICO_RP2350
        auto s = qmi_hw->m[1];
        flushcache();
#endif
        __real_flash_range_erase(flash_offs, count);
#ifdef PICO_RP2350
        qmi_hw->m[1] = s;
        __compiler_memory_barrier();
#endif
    }

    extern void __real_flash_range_program(uint32_t flash_offs, const uint8_t *data, size_t count);
    void __wrap_flash_range_program(uint32_t flash_offs, const uint8_t *data, size_t count) {
#ifdef PICO_RP2350
        auto s = qmi_hw->m[1];
        flushcache();
#endif
        __real_flash_range_program(flash_offs, data, count);
#ifdef PICO_RP2350
        qmi_hw->m[1] = s;
        __compiler_memory_barrier();
#endif
    }

    extern void __real_flash_get_unique_id(uint8_t *id_out);
    void __wrap_flash_get_unique_id(uint8_t *id_out) {
#ifdef PICO_RP2350
        auto s = qmi_hw->m[1];
        flushcache();
#endif
        __real_flash_get_unique_id(id_out);
#ifdef PICO_RP2350
        qmi_hw->m[1] = s;
        __compiler_memory_barrier();
#endif
    }

    extern void __real_flash_do_cmd(const uint8_t *txbuf, uint8_t *rxbuf, size_t count);
    void __wrap_flash_do_cmd(const uint8_t *txbuf, uint8_t *rxbuf, size_t count) {
#ifdef PICO_RP2350
        auto s = qmi_hw->m[1];
        flushcache();
#endif
        __real_flash_do_cmd(txbuf, rxbuf, count);
#ifdef PICO_RP2350
        qmi_hw->m[1] = s;
        __compiler_memory_barrier();
#endif
    }
};
