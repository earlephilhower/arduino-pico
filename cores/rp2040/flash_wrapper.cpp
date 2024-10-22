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
static volatile uint32_t __wastedsum = 0;
static void __no_inline_not_in_flash_func(flushcache)() {
    //for (volatile uint8_t* cache = (volatile uint8_t*)0x18000001; cache < (volatile uint8_t*)(0x18000001 + 2048 * 8); cache += 8) {
    //    *cache = 0;
    //}
    uint32_t sum = 0; // Ignored, just to ensure not optimized out
    for (volatile uint32_t *flash = (volatile uint32_t *)0x11000000; flash < (volatile uint32_t *)(0x11000000 + 48 * 1024 * 4); flash++) {
        sum += *flash;
    }
    __wastedsum += sum;
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
