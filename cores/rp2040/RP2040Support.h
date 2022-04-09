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
#include <hardware/exception.h>
#include <hardware/structs/systick.h>
#include <pico/multicore.h>
#include <pico/util/queue.h>
#include "CoreMutex.h"

#ifndef USE_FREERTOS

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
        multicore_fifo_clear_irq();
        irq_set_exclusive_handler(SIO_IRQ_PROC0 + get_core_num(), _irq);
        irq_set_enabled(SIO_IRQ_PROC0 + get_core_num(), true);
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
        mutex_enter_blocking(&_idleMutex);
        _otherIdled = false;
        multicore_fifo_push_blocking(_GOTOSLEEP);
        while (!_otherIdled) { /* noop */ }
    }

    void resumeOtherCore() {
        if (!_multicore) {
            return;
        }
        mutex_exit(&_idleMutex);
        _otherIdled = false;
        // Other core will exit busy-loop and return to operation
        // once otherIdled == false.
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
        multicore_fifo_clear_irq();
        noInterrupts(); // We need total control, can't run anything
        while (multicore_fifo_rvalid()) {
            if (_GOTOSLEEP == multicore_fifo_pop_blocking()) {
                _otherIdled = true;
                while (_otherIdled) { /* noop */ }
                break;
            }
        }
        interrupts();
    }
    bool _multicore = false;

    mutex_t _idleMutex;
    static volatile bool _otherIdled;
    queue_t _queue[2];

    static constexpr int _GOTOSLEEP = 0x66666666;
};

class RP2040;
extern RP2040 rp2040;
extern "C" void main1();

#endif

class RP2040 {
public:
#ifndef USE_FREERTOS
    RP2040() {
        _epoch = 0;
		// Enable SYSTICK exception
		exception_set_exclusive_handler(SYSTICK_EXCEPTION, _SystickHandler);
		systick_hw->csr = 0x7;
		systick_hw->rvr = 0x00FFFFFF;
    }

    ~RP2040() { /* noop */ }
#endif

    // Convert from microseconds to PIO clock cycles
    static int usToPIOCycles(int us) {
        // Parenthesis needed to guarantee order of operations to avoid 32bit overflow
        return (us * (clock_get_hz(clk_sys) / 1000000));
    }

    // Get current clock frequency
    static int f_cpu() {
        return clock_get_hz(clk_sys);
    }

#ifndef USE_FREERTOS
    // Get CPU cycle count.  Needs to do magic to extens 24b HW to something longer
    volatile uint64_t _epoch = 0;
    inline uint32_t getCycleCount() {
        uint32_t epoch;
        uint32_t ctr;
        do {
            epoch = (uint32_t)_epoch;
            ctr = systick_hw->cvr;
        } while (epoch != (uint32_t)_epoch);
        return epoch + (1 << 24) - ctr; /* CTR counts down from 1<<24-1 */
    }

    inline uint64_t getCycleCount64() {
        uint64_t epoch;
        uint64_t ctr;
        do {
            epoch = _epoch;
            ctr = systick_hw->cvr;
        } while (epoch != _epoch);
        return epoch + (1LL << 24) - ctr;
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

    // Multicore comms FIFO
    _MFIFO fifo;

private:
    static void _SystickHandler() {
        rp2040._epoch += 1LL << 24;
    }
#endif
};

// Wrapper class for PIO programs, abstracting common operations out
// TODO - Add unload/destructor
class PIOProgram {
public:
    PIOProgram(const pio_program_t *pgm) {
        _pgm = pgm;
    }

    // Possibly load into a PIO and allocate a SM
    bool prepare(PIO *pio, int *sm, int *offset) {
        extern mutex_t _pioMutex;
        CoreMutex m(&_pioMutex);
        // Is there an open slot to run in, first?
        if (!_findFreeSM(pio, sm)) {
            return false;
        }
        // Is it loaded on that PIO?
        if (_offset[pio_get_index(*pio)] < 0) {
            // Nope, need to load it
            if (!pio_can_add_program(*pio, _pgm)) {
                return false;
            }
            _offset[pio_get_index(*pio)] = pio_add_program(*pio, _pgm);
        }
        // Here it's guaranteed loaded, return values
        // PIO and SM already set
        *offset = _offset[pio_get_index(*pio)];
        return true;
    }

private:
    // Find an unused PIO state machine to grab, returns false when none available
    static bool _findFreeSM(PIO *pio, int *sm) {
        int idx = pio_claim_unused_sm(pio0, false);
        if (idx >= 0) {
            *pio = pio0;
            *sm = idx;
            return true;
        }
        idx = pio_claim_unused_sm(pio1, false);
        if (idx >= 0) {
            *pio = pio1;
            *sm = idx;
            return true;
        }
        return false;
    }


private:
    int _offset[2] = { -1, -1 };
    const pio_program_t *_pgm;
};
