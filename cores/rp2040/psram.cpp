// Originally taken from the MicroPython project: micropython/ports/rp2/rp2_psram.c
// Modified to work with Arduino-Pico core and TLSF memory manager

/*
    This file is part of the MicroPython project, http://micropython.org/

    The MIT License (MIT)

    Copyright (c) 2025 Phil Howard
                      Mike Bell
                      Kirk D. Benell

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
    THE SOFTWARE.
*/

// Originally from https://github.com/sparkfun/sparkfun-pico

#include <Arduino.h>

#ifdef RP2350_PSRAM_CS

#include <hardware/address_mapped.h>
#include <hardware/clocks.h>
#include <hardware/gpio.h>
#include <hardware/regs/addressmap.h>
#include <hardware/spi.h>
#include <hardware/structs/qmi.h>
#include <hardware/structs/xip_ctrl.h>
#include <pico/runtime_init.h>

// Include TLSF in this compilation unit
#include "../../lib/tlsf/tlsf.c"
static tlsf_t _mem_heap = nullptr;
static pool_t _mem_psram_pool = nullptr;

// PSRAM heap minus PSRAM global/static variables from the linker
extern "C" {
    extern uint8_t __psram_start__;
    extern uint8_t __psram_heap_start__;
}

static bool _bInitalized = false;
size_t __psram_size = 0;
size_t __psram_heap_size = 0;

#define PICO_RUNTIME_INIT_PSRAM "11001" // Towards the end, after alarms

#ifndef RP2350_PSRAM_MAX_SCK_HZ
#define RP2350_PSRAM_MAX_SCK_HZ (109'000'000)
#endif

#ifndef RP2350_PSRAM_ID
#define RP2350_PSRAM_ID (0x5D)
#endif



#include <hardware/structs/ioqspi.h>
#include <hardware/structs/qmi.h>
#include <hardware/structs/xip_ctrl.h>
#include <hardware/clocks.h>
#include <hardware/sync.h>

size_t __no_inline_not_in_flash_func(psram_detect)(void) {
    int psram_size = 0;

    // Try and read the PSRAM ID via direct_csr.
    qmi_hw->direct_csr = 30 << QMI_DIRECT_CSR_CLKDIV_LSB | QMI_DIRECT_CSR_EN_BITS;

    // Need to poll for the cooldown on the last XIP transfer to expire
    // (via direct-mode BUSY flag) before it is safe to perform the first
    // direct-mode operation
    while ((qmi_hw->direct_csr & QMI_DIRECT_CSR_BUSY_BITS) != 0) {
    }

    // Exit out of QMI in case we've inited already
    qmi_hw->direct_csr |= QMI_DIRECT_CSR_ASSERT_CS1N_BITS;

    // Transmit as quad.
    qmi_hw->direct_tx = QMI_DIRECT_TX_OE_BITS | QMI_DIRECT_TX_IWIDTH_VALUE_Q << QMI_DIRECT_TX_IWIDTH_LSB | 0xf5;

    while ((qmi_hw->direct_csr & QMI_DIRECT_CSR_BUSY_BITS) != 0) {
    }

    (void)qmi_hw->direct_rx;

    qmi_hw->direct_csr &= ~(QMI_DIRECT_CSR_ASSERT_CS1N_BITS);

    // Read the id
    qmi_hw->direct_csr |= QMI_DIRECT_CSR_ASSERT_CS1N_BITS;
    uint8_t kgd = 0;
    uint8_t eid = 0;

    for (size_t i = 0; i < 7; i++) {
        if (i == 0) {
            qmi_hw->direct_tx = 0x9f;
        } else {
            qmi_hw->direct_tx = 0xff;
        }

        while ((qmi_hw->direct_csr & QMI_DIRECT_CSR_TXEMPTY_BITS) == 0) {
        }

        while ((qmi_hw->direct_csr & QMI_DIRECT_CSR_BUSY_BITS) != 0) {
        }

        if (i == 5) {
            kgd = qmi_hw->direct_rx;
        } else if (i == 6) {
            eid = qmi_hw->direct_rx;
        } else {
            (void)qmi_hw->direct_rx;
        }
    }

    // Disable direct csr.
    qmi_hw->direct_csr &= ~(QMI_DIRECT_CSR_ASSERT_CS1N_BITS | QMI_DIRECT_CSR_EN_BITS);

    if (kgd == 0x5D) {
        psram_size = 1024 * 1024; // 1 MiB
        uint8_t size_id = eid >> 5;
        if (eid == 0x26 || size_id == 2) {
            psram_size *= 8; // 8 MiB
        } else if (size_id == 0) {
            psram_size *= 2; // 2 MiB
        } else if (size_id == 1) {
            psram_size *= 4; // 4 MiB
        }
    }

    return psram_size;
}

size_t __no_inline_not_in_flash_func(psram_init)(uint cs_pin) {
    gpio_set_function(cs_pin, GPIO_FUNC_XIP_CS1);

    uint32_t intr_stash = save_and_disable_interrupts();

    size_t psram_size = psram_detect();

    if (!psram_size) {
        restore_interrupts(intr_stash);
        return 0;
    }

    // Enable direct mode, PSRAM CS, clkdiv of 10.
    qmi_hw->direct_csr = 10 << QMI_DIRECT_CSR_CLKDIV_LSB | \
                         QMI_DIRECT_CSR_EN_BITS | \
                         QMI_DIRECT_CSR_AUTO_CS1N_BITS;
    while (qmi_hw->direct_csr & QMI_DIRECT_CSR_BUSY_BITS) {
    }

    // Enable QPI mode on the PSRAM
    const uint CMD_QPI_EN = 0x35;
    qmi_hw->direct_tx = QMI_DIRECT_TX_NOPUSH_BITS | CMD_QPI_EN;

    while (qmi_hw->direct_csr & QMI_DIRECT_CSR_BUSY_BITS) {
    }

    // Set PSRAM timing for APS6404
    //
    // Using an rxdelay equal to the divisor isn't enough when running the APS6404 close to 133MHz.
    // So: don't allow running at divisor 1 above 100MHz (because delay of 2 would be too late),
    // and add an extra 1 to the rxdelay if the divided clock is > 100MHz (i.e. sys clock > 200MHz).
    const int max_psram_freq = RP2350_PSRAM_MAX_SCK_HZ;
    const int clock_hz = clock_get_hz(clk_sys);
    int divisor = (clock_hz + max_psram_freq - 1) / max_psram_freq;
    if (divisor == 1 && clock_hz > 100000000) {
        divisor = 2;
    }
    int rxdelay = divisor;
    if (clock_hz / divisor > 100000000) {
        rxdelay += 1;
    }

    // - Max select must be <= 8us.  The value is given in multiples of 64 system clocks.
    // - Min deselect must be >= 18ns.  The value is given in system clock cycles - ceil(divisor / 2).
    const int clock_period_fs = 1000000000000000ll / clock_hz;
    const int max_select = (125 * 1000000) / clock_period_fs;  // 125 = 8000ns / 64
    const int min_deselect = (18 * 1000000 + (clock_period_fs - 1)) / clock_period_fs - (divisor + 1) / 2;

    qmi_hw->m[1].timing = 1 << QMI_M1_TIMING_COOLDOWN_LSB |
                          QMI_M1_TIMING_PAGEBREAK_VALUE_1024 << QMI_M1_TIMING_PAGEBREAK_LSB |
                          max_select << QMI_M1_TIMING_MAX_SELECT_LSB |
                          min_deselect << QMI_M1_TIMING_MIN_DESELECT_LSB |
                          rxdelay << QMI_M1_TIMING_RXDELAY_LSB |
                          divisor << QMI_M1_TIMING_CLKDIV_LSB;

    // Set PSRAM commands and formats
    qmi_hw->m[1].rfmt =
        QMI_M0_RFMT_PREFIX_WIDTH_VALUE_Q << QMI_M0_RFMT_PREFIX_WIDTH_LSB | \
        QMI_M0_RFMT_ADDR_WIDTH_VALUE_Q << QMI_M0_RFMT_ADDR_WIDTH_LSB | \
        QMI_M0_RFMT_SUFFIX_WIDTH_VALUE_Q << QMI_M0_RFMT_SUFFIX_WIDTH_LSB | \
        QMI_M0_RFMT_DUMMY_WIDTH_VALUE_Q << QMI_M0_RFMT_DUMMY_WIDTH_LSB | \
        QMI_M0_RFMT_DATA_WIDTH_VALUE_Q << QMI_M0_RFMT_DATA_WIDTH_LSB | \
        QMI_M0_RFMT_PREFIX_LEN_VALUE_8 << QMI_M0_RFMT_PREFIX_LEN_LSB | \
        6 << QMI_M0_RFMT_DUMMY_LEN_LSB;

    qmi_hw->m[1].rcmd = 0xEB;

    qmi_hw->m[1].wfmt =
        QMI_M0_WFMT_PREFIX_WIDTH_VALUE_Q << QMI_M0_WFMT_PREFIX_WIDTH_LSB | \
        QMI_M0_WFMT_ADDR_WIDTH_VALUE_Q << QMI_M0_WFMT_ADDR_WIDTH_LSB | \
        QMI_M0_WFMT_SUFFIX_WIDTH_VALUE_Q << QMI_M0_WFMT_SUFFIX_WIDTH_LSB | \
        QMI_M0_WFMT_DUMMY_WIDTH_VALUE_Q << QMI_M0_WFMT_DUMMY_WIDTH_LSB | \
        QMI_M0_WFMT_DATA_WIDTH_VALUE_Q << QMI_M0_WFMT_DATA_WIDTH_LSB | \
        QMI_M0_WFMT_PREFIX_LEN_VALUE_8 << QMI_M0_WFMT_PREFIX_LEN_LSB;

    qmi_hw->m[1].wcmd = 0x38;

    // Disable direct mode
    qmi_hw->direct_csr = 0;

    // Enable writes to PSRAM
    hw_set_bits(&xip_ctrl_hw->ctrl, XIP_CTRL_WRITABLE_M1_BITS);

    restore_interrupts(intr_stash);

    return psram_size;
}


//-----------------------------------------------------------------------------
/// @brief The setup_psram function - note that this is not in flash
///
///
static void __no_inline_not_in_flash_func(runtime_init_setup_psram)(/*uint32_t psram_cs_pin*/) {
    __psram_size = psram_init(RP2350_PSRAM_CS);
    uint32_t used_psram_size = &__psram_heap_start__ - &__psram_start__;
    __psram_heap_size = __psram_size - used_psram_size;
}
PICO_RUNTIME_INIT_FUNC_RUNTIME(runtime_init_setup_psram, PICO_RUNTIME_INIT_PSRAM);

// update timing -- used if the system clock/timing was changed.
void psram_reinit_timing(uint32_t hz) {
    if (!hz) {
        hz = (uint32_t)clock_get_hz(clk_sys);
    }
    psram_init(RP2350_PSRAM_CS);
}

static bool __psram_heap_init() {
    if (_bInitalized) {
        return true;
    }

    if (!__psram_heap_size) {
        return false;
    }
    _mem_heap = NULL;
    _mem_psram_pool = NULL;
    _mem_heap = tlsf_create_with_pool((void *)&__psram_heap_start__, __psram_heap_size, 16 * 1024 * 1024);
    if (!_mem_heap) {
        return false;
    }
    _mem_psram_pool = tlsf_get_pool(_mem_heap);
    if (!_mem_psram_pool) {
        return false;
    }
    _bInitalized = true;
    return true;
}

void *__psram_malloc(size_t size) {
    if (!__psram_heap_init() || !_mem_heap) {
        return NULL;
    }
    return tlsf_malloc(_mem_heap, size);
}

void __psram_free(void *ptr) {
    if (!__psram_heap_init() || !_mem_heap) {
        return;
    }
    tlsf_free(_mem_heap, ptr);
}

void *__psram_realloc(void *ptr, size_t size) {
    if (!__psram_heap_init() || !_mem_heap) {
        return NULL;
    }
    return tlsf_realloc(_mem_heap, ptr, size);
}

void *__psram_calloc(size_t num, size_t size) {
    if (!__psram_heap_init() || !_mem_heap) {
        return NULL;
    }
    void *ptr = tlsf_malloc(_mem_heap, num * size);
    if (ptr) {
        bzero(ptr, num * size);
    }
    return ptr;
}

static bool max_free_walker(void *ptr, size_t size, int used, void *user) {
    size_t *max_size = (size_t *)user;
    if (!used && *max_size < size) {
        *max_size = size;
    }
    return true;
}

size_t __psram_largest_free_block() {
    if (!__psram_heap_init() || !_mem_heap) {
        return 0;
    }
    size_t max_free = 0;
    if (_mem_psram_pool) {
        tlsf_walk_pool(_mem_psram_pool, max_free_walker, &max_free);
    }
    return max_free;
}

static bool memory_size_walker(void *ptr, size_t size, int used, void *user) {
    *((size_t *)user) += size;
    return true;
}

size_t __psram_total_space() {
    if (!__psram_heap_init() || !_mem_heap) {
        return 0;
    }
    size_t total_size = 0;
    if (_mem_psram_pool) {
        tlsf_walk_pool(_mem_psram_pool, memory_size_walker, &total_size);
    }
    return total_size;
}

static bool memory_used_walker(void *ptr, size_t size, int used, void *user) {
    if (used) {
        *((size_t *)user) += size;
    }
    return true;
}

size_t __psram_total_used() {
    if (!__psram_heap_init() || !_mem_heap) {
        return 0;
    }
    size_t total_size = 0;
    if (_mem_psram_pool) {
        tlsf_walk_pool(_mem_psram_pool, memory_used_walker, &total_size);
    }
    return total_size;
}

#endif // RP2350_PSRAM_CS
