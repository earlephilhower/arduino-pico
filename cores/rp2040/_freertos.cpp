/*
    _freertos.cpp - Internal core definitions for FreeRTOS

    Copyright (c) 2022 Earle F. Philhower, III <earlephilhower@yahoo.com>

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

#ifdef __FREERTOS

#include "_freertos.h"
#include <pico/mutex.h>
#include <stdlib.h>
#include "Arduino.h"

typedef struct {
    mutex_t *src;
    SemaphoreHandle_t dst;
} FMMap;

static FMMap _map[16];
static StaticSemaphore_t _mapMutex[16];

static SemaphoreHandle_t __find_freertos_mutex_for_ptr(mutex_t *m) {
    for (int i = 0; i < 16; i++) {
        // Acquire pairs with the release in __get_freertos_mutex_for_ptr so dst is valid once src matches
        if (m == __atomic_load_n(&_map[i].src, __ATOMIC_ACQUIRE)) {
            return _map[i].dst;
        }
    }
    return nullptr;
}

SemaphoreHandle_t __get_freertos_mutex_for_ptr(mutex_t *m, bool recursive) {
    // Pre-existing map
    SemaphoreHandle_t fm = __find_freertos_mutex_for_ptr(m);
    if (fm) {
        return fm;
    }

    // Serialize lookup+create+publish across tasks, ISRs, and both cores
    UBaseType_t savedIrqs = 0;
    bool fromISR = portGET_CRITICAL_NESTING_COUNT() == 0U && portCHECK_IF_IN_ISR();
    if (fromISR) {
        savedIrqs = taskENTER_CRITICAL_FROM_ISR();
    } else {
        taskENTER_CRITICAL();
    }
    fm = __find_freertos_mutex_for_ptr(m);
    for (int i = 0; !fm && i < 16; i++) {
        if (_map[i].src == nullptr) {
            // Make a new mutex, static so no malloc (and its newlib lock) inside the critical section
            if (recursive) {
                fm = xSemaphoreCreateRecursiveMutexStatic(&_mapMutex[i]);
            } else {
                fm = xSemaphoreCreateMutexStatic(&_mapMutex[i]);
            }
            _map[i].dst = fm;
            __atomic_store_n(&_map[i].src, m, __ATOMIC_RELEASE);
        }
    }
    if (fromISR) {
        taskEXIT_CRITICAL_FROM_ISR(savedIrqs);
    } else {
        taskEXIT_CRITICAL();
    }
    return fm; // nullptr if we need to make space for more mutex maps!
}

#endif
