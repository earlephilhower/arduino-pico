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

#pragma once

#include <hardware/pio.h>

// Wrapper class for PIO programs, abstracting common operations out
class PIOProgram {
public:
    PIOProgram(const pio_program_t *pgm);
    ~PIOProgram();
    // Possibly load into a PIO and allocate a SM
    bool prepare(PIO *pio, int *sm, int *offset, int gpio_start = 0, int gpio_cnt = 1);

private:
    const pio_program_t *_pgm;
    PIO _pio;
    int _sm;
};
