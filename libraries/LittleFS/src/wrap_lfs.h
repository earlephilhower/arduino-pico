/*
    FreeRTOS LFS wrappers, implement a single LWIP work task
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

#pragma once

#include <Arduino.h>
#include "./lfs_local_config.h"
#include "../lib/littlefs/lfs.h"

#if !defined(__FREERTOS)
extern recursive_mutex_t __lfsMutex;
#else
#include <FreeRTOS.h>
extern SemaphoreHandle_t __lfsMutex;
#endif

class LFSMutex {
public:
    LFSMutex() {
#if !defined(__FREERTOS)
        // In bare metal we do LWIP callbacks in core0 IRQ context and they
        // might access the FS (i.e. WebServer sending a log file).  Avoid
        // deadlock/corruption by disabling IRQs per call.
        if (rp2040.cpuid() == 0) {
            noInterrupts();
        }
        recursive_mutex_enter_blocking(&__lfsMutex);
#else
        xSemaphoreTake(__lfsMutex, portMAX_DELAY);
#endif
    }

    ~LFSMutex() {
#if !defined(__FREERTOS)
        recursive_mutex_exit(&__lfsMutex);
        if (rp2040.cpuid() == 0) {
            interrupts();
        }
#else
        xSemaphoreGive(__lfsMutex);
#endif
    }
};

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
#include "./wrap_lfs_enums.inc"
} __lfs_op;

#include "./wrap_lfs_externs.inc"
#include "./wrap_lfs_structs.inc"

#ifdef __cplusplus
};
#endif
