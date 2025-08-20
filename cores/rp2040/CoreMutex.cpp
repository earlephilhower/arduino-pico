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

#include "Arduino.h"
#include "CoreMutex.h"

CoreMutex::CoreMutex(mutex_t *mutex, uint8_t option) {
    _mutex = mutex;
    _acquired = false;
    _option = option;
#ifdef __FREERTOS
    _pxHigherPriorityTaskWoken = 0; // pdFALSE
    auto m = __get_freertos_mutex_for_ptr(mutex);

    if (portCHECK_IF_IN_ISR()) {
        if (!xSemaphoreTakeFromISR(m, &_pxHigherPriorityTaskWoken)) {
            return;
        }
        // At this point we have the mutex in ISR
    } else {
        // Grab the mutex normally, possibly waking other tasks to get it
        xSemaphoreTake(m, portMAX_DELAY);
    }
#else
    uint32_t owner;
    if (!mutex_try_enter(_mutex, &owner)) {
        if (owner == get_core_num()) { // Deadlock!
            if (_option & DebugEnable) {
                DEBUGCORE("CoreMutex - Deadlock detected!\n");
            }
            return;
        }
        mutex_enter_blocking(_mutex);
    }
#endif
    _acquired = true;
}

CoreMutex::~CoreMutex() {
    if (_acquired) {
#ifdef __FREERTOS
        auto m = __get_freertos_mutex_for_ptr(_mutex);
        if (portCHECK_IF_IN_ISR()) {
            xSemaphoreGiveFromISR(m, &_pxHigherPriorityTaskWoken);
            portYIELD_FROM_ISR(_pxHigherPriorityTaskWoken);
        } else {
            xSemaphoreGive(m);
        }
#else
        mutex_exit(_mutex);
#endif
    }
}
