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

#pragma once

#include <hardware/clocks.h>
#include <hardware/irq.h>
#include <hardware/pio.h>
#include <pico/unique_id.h>
#ifdef PICO_RP2350
#include <hardware/regs/powman.h>
#else
#include <hardware/regs/vreg_and_chip_reset.h>
#endif
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

extern "C" {
#ifdef __PROFILE
    typedef int (*profileWriteCB)(const void *data, int len);
    extern void _writeProfile(profileWriteCB writeCB);
#endif
}

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

/**
    @brief RP2040/RP2350 helper function for HW-specific features
*/
class RP2040 {
public:
    RP2040()  { /* noop */ }
    ~RP2040() { /* noop */ }

    void begin(int cpuid) {
        _epoch[cpuid] = 0;
#if !defined(__riscv) && !defined(__PROFILE)
        if (!__isFreeRTOS) {
            // Enable SYSTICK exception
            exception_set_exclusive_handler(SYSTICK_EXCEPTION, _SystickHandler);
            systick_hw->csr = 0x7;
            systick_hw->rvr = 0x00FFFFFF;
        } else {
#endif
            // Only start 1 instance of the PIO SM
            if (cpuid == 0) {
                int off = 0;
                _ccountPgm = new PIOProgram(&ccount_program);
                _ccountPgm->prepare(&_pio, &_sm, &off);
                ccount_program_init(_pio, _sm, off);
                pio_sm_set_enabled(_pio, _sm, true);
            }
#if !defined(__riscv) && !defined(__PROFILE)
        }
#endif
    }

    /**
        @brief Convert from microseconds to PIO clock cycles

        @returns the PIO cycles for a given microsecond delay
    */
    static int usToPIOCycles(int us) {
        // Parenthesis needed to guarantee order of operations to avoid 32bit overflow
        return (us * (clock_get_hz(clk_sys) / 1'000'000));
    }

    /**
        @brief Gets the active CPU speed (may differ from F_CPU

        @returns CPU frequency in Hz
    */
    static int f_cpu() {
        return clock_get_hz(clk_sys);
    }

    /**
        @brief Get the core ID that is currently executing this code

        @returns 0 for Core 0, 1 for Core 1
    */
    static int cpuid() {
        return sio_hw->cpuid;
    }

    /**
        @brief CPU cycle counter epoch (24-bit cycle).  For internal use
    */
    volatile uint64_t _epoch[2] = {};
    /**
        @brief Get the count of CPU clock cycles since power on.

        @details
        The 32-bit count will overflow every 4 billion cycles, so consider using ``getCycleCount64`` for
        longer measurements

        @returns CPU clock cycles since power up
    */
    inline uint32_t getCycleCount() {
#if !defined(__riscv) && !defined(__PROFILE)
        // Get CPU cycle count.  Needs to do magic to extend 24b HW to something longer
        if (!__isFreeRTOS) {
            uint32_t epoch;
            uint32_t ctr;
            do {
                epoch = (uint32_t)_epoch[sio_hw->cpuid];
                ctr = systick_hw->cvr;
            } while (epoch != (uint32_t)_epoch[sio_hw->cpuid]);
            return epoch + (1 << 24) - ctr; /* CTR counts down from 1<<24-1 */
        } else {
#endif
            return ccount_read(_pio, _sm);
#if !defined(__riscv) && !defined(__PROFILE)
        }
#endif
    }
    /**
        @brief Get the count of CPU clock cycles since power on as a 64-bit quantrity

        @returns CPU clock cycles since power up
    */
    inline uint64_t getCycleCount64() {
#if !defined(__riscv) && !defined(__PROFILE)
        if (!__isFreeRTOS) {
            uint64_t epoch;
            uint64_t ctr;
            do {
                epoch = _epoch[sio_hw->cpuid];
                ctr = systick_hw->cvr;
            } while (epoch != _epoch[sio_hw->cpuid]);
            return epoch + (1LL << 24) - ctr;
        } else {
#endif
            return ccount_read(_pio, _sm);
#if !defined(__riscv) && !defined(__PROFILE)
        }
#endif
    }

    /**
        @brief Gets total unused heap (dynamic memory)

        @details
        Note that the allocations of the size of the total free heap may fail due to fragmentation.
        For example, ``getFreeHeap`` can report 100KB available, but an allocation of 90KB may fail
        because there may not be a contiguous 90KB space available

        @returns Free heap in bytes
    */
    inline int getFreeHeap() {
        return getTotalHeap() - getUsedHeap();
    }

    /**
        @brief Gets total used heap (dynamic memory)

        @returns Used heap in bytes
    */
    inline int getUsedHeap() {
        struct mallinfo m = mallinfo();
        return m.uordblks;
    }

    /**
        @brief Gets total heap (dynamic memory) compiled into the program

        @returns Total heap size in bytes
    */
    inline int getTotalHeap() {
        return &__StackLimit  - &__bss_end__;
    }

    /**
        @brief On the RP2350, returns the amount of heap (dynamic memory) available in PSRAM

        @returns Total free heap in PSRAM, or 0 if no PSRAM present
    */
    inline int getFreePSRAMHeap() {
        return getTotalPSRAMHeap() - getUsedPSRAMHeap();
    }

    /**
        @brief On the RP2350, returns the total amount of PSRAM heap (dynamic memory) used

        @returns Bytes used in PSRAM, or 0 if no PSRAM present
    */
    inline int getUsedPSRAMHeap() {
#if defined(RP2350_PSRAM_CS)
        extern size_t __psram_total_used();
        return __psram_total_used();
#else
        return 0;
#endif
    }

    /**
        @brief On the RP2350, gets total heap (dynamic memory) compiled into the program

        @returns Total PSRAM heap size in bytes, or 0 if no PSRAM present
    */
    inline int getTotalPSRAMHeap() {
#if defined(RP2350_PSRAM_CS)
        extern size_t __psram_total_space();
        return __psram_total_space();
#else
        return 0;
#endif
    }

    /**
        @brief Gets the current stack pointer in a ARM/RISC-V safe manner

        @returns Current SP
    */
    inline uint32_t getStackPointer() {
        uint32_t *sp;
#if defined(__riscv)
        asm volatile("mv %0, sp" : "=r"(sp));
#else
        asm volatile("mov %0, sp" : "=r"(sp));
#endif
        return (uint32_t)sp;
    }

    /**
        @brief Calculates approximately how much stack space is still available for the running core.  Handles multiprocessing and separate stacks.

        @details
        Not valid in FreeRTOS.  Use the FreeRTOS internal functions to access this information.

        @returns Approximation of the amount of stack available for use on the specific core
    */
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

    /**
        @brief On the RP2350, gets the size of attached PSRAM

        @returns PSRAM size in bytes, or 0 if no PSRAM present
    */
    inline size_t getPSRAMSize() {
#if defined(RP2350_PSRAM_CS)
        extern size_t __psram_size;
        return __psram_size;
#else
        return 0;
#endif
    }

    /**
        @brief Freezes the other core in a flash-write-safe state.  Not generally needed by applications

        @details
        When the external flash chip is erasing or writing, the Pico cannot fetch instructions from it.
        In this case both the core doing the writing and the other core (if active) need to run from a
        routine that's contained in RAM.  This call forces the other core into a tight, RAM-based loop
        safe for this operation.  When flash erase/write is completed, ``resumeOtherCore`` to return
        it to operation.

        Be sure to disable any interrupts or task switches before calling to avoid deadlocks.

        If the second core is not started, this is a no-op.
    */
    void idleOtherCore() {
        fifo.idleOtherCore();
    }

    /**
        @brief Resumes normal operation of the other core
    */
    void resumeOtherCore() {
        fifo.resumeOtherCore();
    }

    /**
        @brief Hard resets the 2nd core (CORE1).

        @details
        Because core1 will restart with the heap and global variables not in the same state as
        power-on, this call may not work as desired and a full CPU reset may be necessary in
        certain cases.
    */
    void restartCore1() {
        multicore_reset_core1();
        fifo.clear();
        multicore_launch_core1(main1);
    }

    /**
        @brief Warm-reboots the chip in normal mode
    */
    void reboot() {
        watchdog_reboot(0, 0, 10);
        while (1) {
            continue;
        }
    }

    /**
        @brief Warm-reboots the chip in normal mode
    */
    inline void restart() {
        reboot();
    }

    /**
        @brief Warm-reboots the chip into the USB bootloader mode
    */
    inline void rebootToBootloader() {
        reset_usb_boot(0, 0);
        while (1) {
            continue;
        }
    }

#ifdef PICO_RP2040
    static void enableDoubleResetBootloader();
#endif

    /**
        @brief Starts the hardware watchdog timer.  The CPU will reset if the watchdog is not fed every delay_ms

        @param [in] delay_ms Milliseconds without a wdt_reset before rebooting
    */
    void wdt_begin(uint32_t delay_ms) {
        watchdog_enable(delay_ms, 1);
    }

    /**
        @brief Feeds the watchdog timer, resetting it for another delay_ms countdown
    */
    void wdt_reset() {
        watchdog_update();
    }

    /**
        @brief Best-effort reasons for chip reset
    */
    enum resetReason_t {UNKNOWN_RESET, PWRON_RESET, RUN_PIN_RESET, SOFT_RESET, WDT_RESET, DEBUG_RESET, GLITCH_RESET, BROWNOUT_RESET};

    /**
        @brief Attempts to determine the reason for the last chip reset.  May not always be able to determine accurately

        @returns Reason for reset
    */
    resetReason_t getResetReason(void) {
        io_rw_32 *WD_reason_reg = (io_rw_32 *)(WATCHDOG_BASE + WATCHDOG_REASON_OFFSET);

        if (watchdog_caused_reboot() && watchdog_enable_caused_reboot()) { // watchdog timer
            return WDT_RESET;
        }

        if (*WD_reason_reg & WATCHDOG_REASON_TIMER_BITS) { // soft reset() or reboot()
            return SOFT_RESET;
        }

#ifdef PICO_RP2350
        // **** RP2350 is untested ****
        io_rw_32 *rrp = (io_rw_32 *)(POWMAN_BASE + POWMAN_CHIP_RESET_OFFSET);

        if (*rrp & POWMAN_CHIP_RESET_HAD_POR_BITS) { // POR: power-on reset (brownout is separately detected on RP2350)
            return PWRON_RESET;
        }

        if (*rrp & POWMAN_CHIP_RESET_HAD_RUN_LOW_BITS) { // RUN pin
            return RUN_PIN_RESET;
        }

        if ((*rrp & POWMAN_CHIP_RESET_HAD_DP_RESET_REQ_BITS) || (*rrp & POWMAN_CHIP_RESET_HAD_RESCUE_BITS) || (*rrp & POWMAN_CHIP_RESET_HAD_HZD_SYS_RESET_REQ_BITS)) { // DEBUG port
            return DEBUG_RESET;
        }

        if (*rrp & POWMAN_CHIP_RESET_HAD_GLITCH_DETECT_BITS) { // power supply glitch
            return GLITCH_RESET;
        }

        if (*rrp & POWMAN_CHIP_RESET_HAD_BOR_BITS) { // power supply brownout reset
            return BROWNOUT_RESET;
        }

#else
        io_rw_32 *rrp = (io_rw_32 *)(VREG_AND_CHIP_RESET_BASE + VREG_AND_CHIP_RESET_CHIP_RESET_OFFSET);

        if (*rrp & VREG_AND_CHIP_RESET_CHIP_RESET_HAD_POR_BITS) { // POR: power-on reset or brown-out detection
            return PWRON_RESET;
        }

        if (*rrp & VREG_AND_CHIP_RESET_CHIP_RESET_HAD_RUN_BITS) { // RUN pin
            return RUN_PIN_RESET;
        }

        if (*rrp & VREG_AND_CHIP_RESET_CHIP_RESET_HAD_PSM_RESTART_BITS) { // DEBUG port
            return DEBUG_RESET; // **** untested **** debug reset may just cause a rebootToBootloader()
        }
#endif
        return UNKNOWN_RESET;
    }

    /**
        @brief Get unique ID string for the running board
        @returns String with the unique board ID as determined by the SDK
    */
    const char *getChipID() {
        static char id[2 * PICO_UNIQUE_BOARD_ID_SIZE_BYTES + 1] = { 0 };
        if (!id[0]) {
            pico_get_unique_board_id_string(id, sizeof(id));
        }
        return id;
    }

#pragma GCC push_options
#pragma GCC optimize ("Os")
    /**
        @brief Perform a memcpy using a DMA engine for speed

        @details
        Uses the DMA to copy to and from RAM.  Only works on 4-byte aligned, 4-byte multiple length
        sources and destination (i.e. word-aligned, word-length).  Falls back to normal memcpy otherwise.

        @param [out] dest Memcpy destination, 4-byte aligned
        @param [in] src Memcpy source, 4-byte aligned
        @param [in] n Count in bytes to transfer (should be a multiple of 4 bytes)
    */
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

    /**
        @brief Multicore communications FIFO
    */
    _MFIFO fifo;


    /**
        @brief Return a 32-bit from the hardware random number generator

        @returns Random value using appropriate hardware (RP2350 has true RNG, RP2040 has a less true RNG method)
    */
    uint32_t hwrand32() {
        return get_rand_32();
    }

    /**
        @brief Determines if code is running on a Pico or a PicoW

        @details
        Code compiled for the RP2040 PicoW can run on the RP2040 Pico.  This call lets an application
        identify if the current device is really a Pico or PicoW and handle appropriately.  For
        the RP2350, this runtime detection is not available and the call returns whether it was
        compiled for the CYW43 WiFi driver

        @returns True if running on a PicoW board with CYW43 WiFi chip.
    */
    bool isPicoW() {
#if !defined(PICO_CYW43_SUPPORTED)
        return false;
#else
        extern bool __isPicoW;
        return __isPicoW;
#endif
    }

#ifdef __PROFILE
    void writeProfiling(Stream *f) {
        extern Stream *__profileFile;
        extern int __writeProfileCB(const void *data, int len);
        __profileFile = f;
        _writeProfile(__writeProfileCB);
    }

    size_t getProfileMemoryUsage() {
        extern int __profileMemSize;
        return (size_t) __profileMemSize;
    }
#endif



private:
    static void __no_inline_not_in_flash_func(_SystickHandler)() {
        rp2040._epoch[sio_hw->cpuid] += 1LL << 24;
    }
    PIO _pio;
    int _sm;
    PIOProgram *_ccountPgm;
    int memcpyDMAChannel = -1;
};
