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

#include <hardware/flash.h>

#ifdef PICO_RP2350
#include <hardware/structs/qmi.h>
#endif

extern "C" {
    extern void __real_flash_range_erase(uint32_t flash_offs, size_t count);
    void __wrap_flash_range_erase(uint32_t flash_offs, size_t count) {
#ifdef PICO_RP2350
        auto s = qmi_hw->m[1];
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
#endif
        __real_flash_do_cmd(txbuf, rxbuf, count);
#ifdef PICO_RP2350
        qmi_hw->m[1] = s;
        __compiler_memory_barrier();
#endif
    }
};
