/*
    LFS wrappers to protect against timer-based re-entrancy
    Copyright (c) 2026 Earle F. Philhower, III <earlephilhower@yahoo.com>

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
#include "wrap_lfs.h"
#ifdef __FREERTOS
#include "wrap_lfs_freertos.h"
#endif

recursive_mutex_t __lfsMutex;

#ifndef __FREERTOS
void __initLFSMutex() {
    recursive_mutex_init(&__lfsMutex);
}
#endif

extern "C" {
#include "./wrap_lfs_functions.inc"
}; // extern "C"
