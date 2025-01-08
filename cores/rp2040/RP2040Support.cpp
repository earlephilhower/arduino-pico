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

#include <Arduino.h>
#include <pico/runtime.h>

#ifdef PICO_RP2040

#include <hardware/structs/psm.h>

extern "C" void boot_double_tap_check();

// The following check will never actually execute, but it will cause the boot reset
// checker to be linked in as part of the constructors.

void RP2040::enableDoubleResetBootloader() {
    if (psm_hw->done == 0) {
        boot_double_tap_check();
    }
}

#endif

#ifdef __PROFILE
Stream *__profileFile;
int __writeProfileCB(const void *data, int len) {
    return __profileFile->write((const char *)data, len);
}

#ifdef __PROFILE
extern "C" void runtime_init_setup_profiling();
#define PICO_RUNTIME_INIT_PROFILING "11011" // Towards the end, after PSRAM
PICO_RUNTIME_INIT_FUNC_RUNTIME(runtime_init_setup_profiling, PICO_RUNTIME_INIT_PROFILING);
#endif

#endif
