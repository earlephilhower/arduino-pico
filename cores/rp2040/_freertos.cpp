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

#include "_freertos.h"
#include <pico/mutex.h>
#include <stdlib.h>
#include "Arduino.h"

typedef struct {
    mutex_t *src;
    SemaphoreHandle_t dst;
} FMMap;

static FMMap *_map = nullptr;
SemaphoreHandle_t __get_freertos_mutex_for_ptr(mutex_t *m, bool recursive) {
    if (!_map) {
        _map = (FMMap *)calloc(sizeof(FMMap), 16);
    }
    // Pre-existing map
    for (int i = 0; i < 16; i++) {
        if (m == _map[i].src) {
            return _map[i].dst;
        }
    }

    for (int i = 0; i < 16; i++) {
        if (_map[i].src == nullptr) {
            // Make a new mutex
            SemaphoreHandle_t fm;
            if (recursive) {
                fm = _freertos_recursive_mutex_create();
            } else {
                fm = __freertos_mutex_create();
            }
            if (fm == nullptr) {
                return nullptr;
            }

            _map[i].src = m;
            _map[i].dst = fm;
            return fm;
        }
    }
    return nullptr; // Need to make space for more mutex maps!
}


// The HW Random code needs a precise sleep_until() in order to assure it
// grabs the ROSC bit when it wants.  Unfortunately, under FreeRTOS the
// sleep_until() becomes imprecise and the "did I get the bit when I wanted"
// check in the pico_rand code always fails and you get an infinite loop.

// This block wraps the 2 get_rand calls to set a flag to convert
// sleep_until() (which can do a task swap and cause bad timing) into a
// busy wait.

extern "C" {
    static bool __inRand = false;

    extern uint64_t __real_get_rand_64();
    uint64_t __wrap_get_rand_64() {
        if (__isFreeRTOS) {
            rp2040.idleOtherCore();
            __inRand = true;
            auto r = __real_get_rand_64();
            __inRand = false;
            rp2040.resumeOtherCore();
            return r;
        } else {
            return __real_get_rand_64();
        }
    }

    uint32_t __wrap_get_rand_32() {
        return (uint32_t) __wrap_get_rand_64();
    }

    extern void __real_sleep_until(absolute_time_t t);
    void __wrap_sleep_until(absolute_time_t t) {
        if (__inRand) {
            busy_wait_until(t);
        } else {
            __real_sleep_until(t);
        }
    }

}
