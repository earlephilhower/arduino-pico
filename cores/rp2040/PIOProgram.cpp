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

static std::map<const pio_program_t *, int> __pioMap[2];
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
bool PIOProgram::prepare(PIO *pio, int *sm, int *offset) {
    CoreMutex m(&_pioMutex);
    PIO pi[2] = { pio0, pio1 };

    // If it's already loaded into PIO IRAM, try and allocate in that specific PIO
    for (int o = 0; o < 2; o++) {
        auto p = __pioMap[o].find(_pgm);
        if (p != __pioMap[o].end()) {
            int idx = pio_claim_unused_sm(pi[o], false);
            if (idx >= 0) {
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
    for (int o = 0; o < 2; o++) {
        if (pio_can_add_program(pi[o], _pgm)) {
            int idx = pio_claim_unused_sm(pi[o], false);
            if (idx >= 0) {
                int off = pio_add_program(pi[o], _pgm);
                __pioMap[o].insert({_pgm, off});
                _pio = pi[o];
                _sm = idx;
                *pio = pi[o];
                *sm = idx;
                *offset = off;
                return true;
            }
        }
    }

    // Nope, no room either for SMs or INSNs
    return false;
}
