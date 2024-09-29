/*
    RP2040 utility class

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

#include <hardware/clocks.h>
#include <hardware/irq.h>
#include <hardware/pio.h>
#include <pico/unique_id.h>
#include <hardware/exception.h>
#include <hardware/watchdog.h>
#include <hardware/structs/rosc.h>
#include <hardware/structs/systick.h>
#include <pico/multicore.h>
#include <hardware/dma.h>
#include <pico/rand.h>
#include <pico/util/queue.h>
#include <pico/bootrom.h>
#include "CoreMutex.h"
#include "PIOProgram.h"
#include "ccount.pio.h"
#include <malloc.h>

#include "_freertos.h"

extern "C" volatile bool __otherCoreIdled;

class _MFIFO {
public:
    _MFIFO() { /* noop */ };
    ~_MFIFO() { /* noop */ };

    void begin(int cores) {
        constexpr int FIFOCNT = 8;
        if (cores == 1) {
            _multicore = false;
            return;
        }
        mutex_init(&_idleMutex);
        queue_init(&_queue[0], sizeof(uint32_t), FIFOCNT);
        queue_init(&_queue[1], sizeof(uint32_t), FIFOCNT);
        _multicore = true;
    }

    void registerCore() {
        if (!__isFreeRTOS) {
            multicore_fifo_clear_irq();
#ifdef PICO_RP2350
            irq_set_exclusive_handler(SIO_IRQ_FIFO, _irq);
            irq_set_enabled(SIO_IRQ_FIFO, true);
#else
            irq_set_exclusive_handler(SIO_IRQ_PROC0 + get_core_num(), _irq);
            irq_set_enabled(SIO_IRQ_PROC0 + get_core_num(), true);
#endif
        }
        // FreeRTOS port.c will handle the IRQ hooking
    }

    void push(uint32_t val) {
        while (!push_nb(val)) { /* noop */ }
    }

    bool push_nb(uint32_t val) {
        // Push to the other FIFO
        return queue_try_add(&_queue[get_core_num() ^ 1], &val);
    }

    uint32_t pop() {
        uint32_t ret;
        while (!pop_nb(&ret)) { /* noop */ }
        return ret;
    }

    bool pop_nb(uint32_t *val) {
        // Pop from my own FIFO
        return queue_try_remove(&_queue[get_core_num()], val);
    }

    int available() {
        return queue_get_level(&_queue[get_core_num()]);
    }

    void idleOtherCore() {
        if (!_multicore) {
            return;
        }
        if (__isFreeRTOS) {
            __freertos_idle_other_core();
        } else {
            mutex_enter_blocking(&_idleMutex);
            __otherCoreIdled = false;
            multicore_fifo_push_blocking(_GOTOSLEEP);
            while (!__otherCoreIdled) { /* noop */ }
        }
    }

    void resumeOtherCore() {
        if (!_multicore) {
            return;
        }
        mutex_exit(&_idleMutex);
        __otherCoreIdled = false;
        if (__isFreeRTOS) {
            __freertos_resume_other_core();
        }

        // Other core will exit busy-loop and return to operation
        // once __otherCoreIdled == false.
    }

    void clear() {
        uint32_t val;

        while (queue_try_remove(&_queue[0], &val)) {
            tight_loop_contents();
        }

        while (queue_try_remove(&_queue[1], &val)) {
            tight_loop_contents();
        }
    }

private:
    static void __no_inline_not_in_flash_func(_irq)() {
        if (!__isFreeRTOS) {
            multicore_fifo_clear_irq();
            noInterrupts(); // We need total control, can't run anything
            while (multicore_fifo_rvalid()) {
                if (_GOTOSLEEP == multicore_fifo_pop_blocking()) {
                    __otherCoreIdled = true;
                    while (__otherCoreIdled) { /* noop */ }
                    break;
                }
            }
            interrupts();
        }
    }

    bool _multicore = false;
    mutex_t _idleMutex;
    queue_t _queue[2];
    static constexpr uint32_t _GOTOSLEEP = 0xC0DED02E;
};


class RP2040;
extern RP2040 rp2040;
extern "C" void main1();
extern "C" char __StackLimit;
extern "C" char __bss_end__;
extern "C" void setup1() __attribute__((weak));
extern "C" void loop1() __attribute__((weak));
extern "C" bool core1_separate_stack;
extern "C" uint32_t* core1_separate_stack_address;

class RP2040 {
public:
    RP2040()  { /* noop */ }
    ~RP2040() { /* noop */ }

    void begin() {
        _epoch = 0;
#if !defined(__riscv)
        if (!__isFreeRTOS) {
            // Enable SYSTICK exception
            exception_set_exclusive_handler(SYSTICK_EXCEPTION, _SystickHandler);
            systick_hw->csr = 0x7;
            systick_hw->rvr = 0x00FFFFFF;
        } else {
#endif
            int off = 0;
            _ccountPgm = new PIOProgram(&ccount_program);
            _ccountPgm->prepare(&_pio, &_sm, &off);
            ccount_program_init(_pio, _sm, off);
            pio_sm_set_enabled(_pio, _sm, true);
#if !defined(__riscv)
        }
#endif
    }

    // Convert from microseconds to PIO clock cycles
    static int usToPIOCycles(int us) {
        // Parenthesis needed to guarantee order of operations to avoid 32bit overflow
        return (us * (clock_get_hz(clk_sys) / 1'000'000));
    }

    // Get current clock frequency
    static int f_cpu() {
        return clock_get_hz(clk_sys);
    }

    // Get current CPU core number
    static int cpuid() {
        return sio_hw->cpuid;
    }

    // Get CPU cycle count.  Needs to do magic to extens 24b HW to something longer
    volatile uint64_t _epoch = 0;
    inline uint32_t getCycleCount() {
#if !defined(__riscv)
        if (!__isFreeRTOS) {
            uint32_t epoch;
            uint32_t ctr;
            do {
                epoch = (uint32_t)_epoch;
                ctr = systick_hw->cvr;
            } while (epoch != (uint32_t)_epoch);
            return epoch + (1 << 24) - ctr; /* CTR counts down from 1<<24-1 */
        } else {
#endif
            return ccount_read(_pio, _sm);
#if !defined(__riscv)
        }
#endif
    }

    inline uint64_t getCycleCount64() {
#if !defined(__riscv)
        if (!__isFreeRTOS) {
            uint64_t epoch;
            uint64_t ctr;
            do {
                epoch = _epoch;
                ctr = systick_hw->cvr;
            } while (epoch != _epoch);
            return epoch + (1LL << 24) - ctr;
        } else {
#endif
            return ccount_read(_pio, _sm);
#if !defined(__riscv)
        }
#endif
    }

    inline int getFreeHeap() {
        return getTotalHeap() - getUsedHeap();
    }

    inline int getUsedHeap() {
        struct mallinfo m = mallinfo();
        return m.uordblks;
    }

    inline int getTotalHeap() {
        return &__StackLimit  - &__bss_end__;
    }

    inline int getFreePSRAMHeap() {
        return getTotalPSRAMHeap() - getUsedPSRAMHeap();
    }

    inline int getUsedPSRAMHeap() {
#if defined(RP2350_PSRAM_CS)
        extern size_t __psram_total_used();
        return __psram_total_used();
#else
        return 0;
#endif
    }

    inline int getTotalPSRAMHeap() {
#if defined(RP2350_PSRAM_CS)
        extern size_t __psram_total_space();
        return __psram_total_space();
#else
        return 0;
#endif
    }

    inline uint32_t getStackPointer() {
        uint32_t *sp;
        asm volatile("mov %0, sp" : "=r"(sp));
        return (uint32_t)sp;
    }

    inline int getFreeStack() {
        const unsigned int sp = getStackPointer();
        uint32_t ref = 0x20040000;
        if (setup1 || loop1) {
            if (core1_separate_stack) {
                ref = cpuid() ? (unsigned int)core1_separate_stack_address : 0x20040000;
            } else {
                ref = cpuid() ? 0x20040000 : 0x20041000;
            }
        }
        return sp - ref;
    }

    inline size_t getPSRAMSize() {
#if defined(RP2350_PSRAM_CS)
        extern size_t __psram_size;
        return __psram_size;
#else
        return 0;
#endif
    }

    void idleOtherCore() {
        fifo.idleOtherCore();
    }

    void resumeOtherCore() {
        fifo.resumeOtherCore();
    }

    void restartCore1() {
        multicore_reset_core1();
        fifo.clear();
        multicore_launch_core1(main1);
    }

    void reboot() {
        watchdog_reboot(0, 0, 10);
        while (1) {
            continue;
        }
    }

    inline void restart() {
        reboot();
    }

    inline void rebootToBootloader() {
        reset_usb_boot(0, 0);
        while (1) {
            continue;
        }
    }

    static void enableDoubleResetBootloader();

    void wdt_begin(uint32_t delay_ms) {
        watchdog_enable(delay_ms, 1);
    }

    void wdt_reset() {
        watchdog_update();
    }

    const char *getChipID() {
        static char id[2 * PICO_UNIQUE_BOARD_ID_SIZE_BYTES + 1] = { 0 };
        if (!id[0]) {
            pico_get_unique_board_id_string(id, sizeof(id));
        }
        return id;
    }

#pragma GCC push_options
#pragma GCC optimize ("Os")
    void *memcpyDMA(void *dest, const void *src, size_t n) {
        // Allocate a DMA channel on 1st call, reuse it every call after
        if (memcpyDMAChannel < 1) {
            memcpyDMAChannel = dma_claim_unused_channel(true);
            dma_channel_config c = dma_channel_get_default_config(memcpyDMAChannel);
            channel_config_set_transfer_data_size(&c, DMA_SIZE_32);
            channel_config_set_read_increment(&c, true);
            channel_config_set_write_increment(&c, true);
            channel_config_set_irq_quiet(&c, true);
            dma_channel_set_config(memcpyDMAChannel, &c, false);
        }
        // If there's any misalignment or too small, use regular memcpy which can handle it
        if ((n < 64) || (((uint32_t)dest) | ((uint32_t)src) | n) & 3) {
            return memcpy(dest, src, n);
        }

        int words = n / 4;
        dma_channel_set_read_addr(memcpyDMAChannel, src, false);
        dma_channel_set_write_addr(memcpyDMAChannel, dest, false);
        dma_channel_set_trans_count(memcpyDMAChannel, words, false);
        dma_channel_start(memcpyDMAChannel);
        while (dma_channel_is_busy(memcpyDMAChannel)) {
            /* busy wait dma */
        }
        return dest;
    }
#pragma GCC pop_options

    // Multicore comms FIFO
    _MFIFO fifo;


    uint32_t hwrand32() {
        return get_rand_32();
    }

    bool isPicoW() {
#if !defined(ARDUINO_RASPBERRY_PI_PICO_W)
        return false;
#else
        extern bool __isPicoW;
        return __isPicoW;
#endif
    }


private:
    static void _SystickHandler() {
        rp2040._epoch += 1LL << 24;
    }
    PIO _pio;
    int _sm;
    PIOProgram *_ccountPgm;
    int memcpyDMAChannel = -1;
};
