/*
 * RP2040 utility class
 *
 * Copyright (c) 2021 Earle F. Philhower, III <earlephilhower@yahoo.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <hardware/pio.h>
#include <hardware/clocks.h>

class RP2040 {
public:
    // Convert from microseconds to PIO clock cycles
    static int usToPIOCycles(int us) {
        // Parenthesis needed to guarantee order of operations to avoid 32bit overflow
        return (us * ( clock_get_hz(clk_sys) / 1000000 ));
    }

    // Get current clock frequency
    static int f_cpu() {
        return clock_get_hz(clk_sys);
    }
};

// Wrapper class for PIO programs, abstracting common operations out
// TODO - Make dualcore safe
// TODO - Add unload/destructor
class PIOProgram {
public:
    PIOProgram(const pio_program_t *pgm) { _pgm = pgm; }

    // Possibly load into a PIO and allocate a SM
    bool prepare(PIO *pio, int *sm, int *offset) {
        // Is there an open slot to run in, first?
        if (!_findFreeSM(pio, sm)) return false;
	// Is it loaded on that PIO?
        if (_offset[pio_get_index(*pio)] < 0) {
            // Nope, need to load it
            if (!pio_can_add_program(*pio, _pgm)) return false;
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

