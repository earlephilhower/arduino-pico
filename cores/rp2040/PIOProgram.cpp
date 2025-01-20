/*
    RP2040 PIO utility class

    Copyright (c) 2023 Earle F. Philhower, III <earlephilhower@yahoo.com>

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
#include "PIOProgram.h"
#include <map>
#include <hardware/claim.h>

#if defined(PICO_RP2350)
#define PIOS pio0, pio1, pio2
#define PIOCNT 3
#elif defined(PICO_RP2040)
#define PIOS pio0, pio1
#define PIOCNT 2
#endif

static std::map<const pio_program_t *, int> __pioMap[PIOCNT];
static bool __pioAllocated[PIOCNT];
auto_init_mutex(_pioMutex);

PIOProgram::PIOProgram(const pio_program_t *pgm) {
    _pgm = pgm;
    _pio = nullptr;
    _sm = -1;
}

// We leave the INSN loaded in INSN RAM
PIOProgram::~PIOProgram() {
    if (_pio) {
        pio_sm_unclaim(_pio, _sm);
    }
}

// Possibly load into a PIO and allocate a SM
bool PIOProgram::prepare(PIO *pio, int *sm, int *offset, int start, int cnt) {
    CoreMutex m(&_pioMutex);
    PIO pi[PIOCNT] = { PIOS };

#if 0
    uint usm;
    uint uoff;
    auto ret = pio_claim_free_sm_and_add_program_for_gpio_range(_pgm, pio, &usm, &uoff, start, cnt, true);
    *sm = usm;
    *offset = uoff;
    DEBUGV("clain %d\n", ret);
    return ret;
#endif

    uint gpioBaseNeeded = ((start + cnt) >= 32) ? 16 : 0;
    DEBUGV("PIOProgram %p: Searching for base=%d, pins %d-%d\n", _pgm, gpioBaseNeeded, start, start + cnt - 1);

    // If it's already loaded into PIO IRAM, try and allocate in that specific PIO
    for (int o = 0; o < PIOCNT; o++) {
        auto p = __pioMap[o].find(_pgm);
        if ((p != __pioMap[o].end()) && (pio_get_gpio_base(pio_get_instance(o)) == gpioBaseNeeded)) {
            int idx = pio_claim_unused_sm(pi[o], false);
            if (idx >= 0) {
                DEBUGV("PIOProgram %p: Reusing IMEM ON PIO %p(base=%d) for pins %d-%d\n", _pgm, pi[o], pio_get_gpio_base(pio_get_instance(o)), start, start + cnt - 1);
                _pio = pi[o];
                _sm = idx;
                *pio = pi[o];
                *sm = idx;
                *offset = p->second;
                return true;
            }
        }
    }

    // Not in any PIO IRAM, so try and add
    for (int o = 0; o < PIOCNT; o++) {
        if (__pioAllocated[o] && (pio_get_gpio_base(pio_get_instance(o)) == gpioBaseNeeded)) {
            DEBUGV("PIOProgram: Checking PIO %p\n", pi[o]);
            if (pio_can_add_program(pi[o], _pgm)) {
                int idx = pio_claim_unused_sm(pi[o], false);
                if (idx >= 0) {
                    DEBUGV("PIOProgram %p: Adding IMEM ON PIO %p(base=%d) for pins %d-%d\n", _pgm, pi[o], pio_get_gpio_base(pio_get_instance(o)), start, start + cnt - 1);
                    int off = pio_add_program(pi[o], _pgm);
                    __pioMap[o].insert({_pgm, off});
                    _pio = pi[o];
                    _sm = idx;
                    *pio = pi[o];
                    *sm = idx;
                    *offset = off;
                    return true;
                } else {
                    DEBUGV("PIOProgram: can't claim unused SM\n");
                }
            } else {
                DEBUGV("PIOProgram: can't add program\n");
            }
        } else {
            DEBUGV("PIOProgram: Skipping PIO %p, wrong allocated/needhi\n", pi[o]);
        }
    }

    // No existing PIOs can meet, is there an unallocated one we can allocate?
    PIO p;
    uint idx;
    uint off;
    auto rc = pio_claim_free_sm_and_add_program_for_gpio_range(_pgm, &p, &idx, &off, start, cnt, true);
    if (rc) {
        int o = 0;
        while (p != pi[o]) {
            o++;
        }
        assert(!__pioAllocated[o]);
        __pioAllocated[o]  = true;
        DEBUGV("PIOProgram %p: Allocating new PIO %p(base=%d) for pins %d-%d\n", _pgm, pi[o], pio_get_gpio_base(pio_get_instance(o)), start, start + cnt - 1);
        __pioMap[o].insert({_pgm, off});
        _pio = pi[o];
        _sm = idx;
        *pio = pi[o];
        *sm = idx;
        *offset = off;
        return true;
    }

    // Nope, no room either for SMs or INSNs
    return false;
}
