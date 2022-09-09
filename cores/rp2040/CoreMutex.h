/*
    CoreMutex for the Raspberry Pi Pico RP2040

    Implements a deadlock-safe multicore mutex for sharing things like the
    USB or UARTs.

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

#pragma once

#include "pico/mutex.h"
#include "_freertos.h"

class CoreMutex {
public:
    CoreMutex(mutex_t *mutex, bool debugEnable = true) {
        _mutex = mutex;
        _acquired = false;
        if (__isFreeRTOS) {
            auto m = __get_freertos_mutex_for_ptr(mutex);
            __freertos_mutex_take(m);
        } else {
            uint32_t owner;
            if (!mutex_try_enter(_mutex, &owner)) {
                if (owner == get_core_num()) { // Deadlock!
                    if (debugEnable) {
                        DEBUGCORE("CoreMutex - Deadlock detected!\n");
                    }
                    return;
                }
                mutex_enter_blocking(_mutex);
            }
        }
        _acquired = true;
    }

    ~CoreMutex() {
        if (_acquired) {
            if (__isFreeRTOS) {
                auto m = __get_freertos_mutex_for_ptr(_mutex);
                __freertos_mutex_give(m);
            } else {
                mutex_exit(_mutex);
            }
        }
    }

    operator bool() {
        return _acquired;
    }

private:
    mutex_t *_mutex;
    bool _acquired;
};
