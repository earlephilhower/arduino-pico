/*
    _freertos.h - Internal core definitions for FreeRTOS

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

#pragma once
#include "pico/mutex.h"

// Cannot include refs to FreeRTOS's actual semaphore calls because they
// are implemented as macros, so we have a wrapper in our variant hook
// to handle it.

extern bool __isFreeRTOS;

// FreeRTOS has been set up
extern volatile bool __freeRTOSinitted;

extern "C" {
#ifndef INC_FREERTOS_H
    struct QueueDefinition; /* Using old naming convention so as not to break kernel aware debuggers. */
    typedef struct QueueDefinition   * QueueHandle_t;
    typedef QueueHandle_t SemaphoreHandle_t;
#endif

    extern SemaphoreHandle_t __freertos_mutex_create() __attribute__((weak));
    extern SemaphoreHandle_t _freertos_recursive_mutex_create() __attribute__((weak));

    extern void __freertos_mutex_take(SemaphoreHandle_t mtx) __attribute__((weak));
    extern int __freertos_mutex_try_take(SemaphoreHandle_t mtx) __attribute__((weak));
    extern void __freertos_mutex_give(SemaphoreHandle_t mtx) __attribute__((weak));

    extern void __freertos_recursive_mutex_take(SemaphoreHandle_t mtx) __attribute__((weak));
    extern int __freertos_recursive_mutex_try_take(SemaphoreHandle_t mtx) __attribute__((weak));
    extern void __freertos_recursive_mutex_give(SemaphoreHandle_t mtx) __attribute__((weak));

#ifndef INC_FREERTOS_H
    extern void vTaskSuspendAll() __attribute__((weak));
    extern int32_t xTaskResumeAll() __attribute__((weak));

    typedef struct tskTaskControlBlock * TaskHandle_t;
    extern void vTaskPreemptionDisable(TaskHandle_t p) __attribute__((weak));
    extern void vTaskPreemptionEnable(TaskHandle_t p) __attribute__((weak));
#endif

    extern SemaphoreHandle_t __get_freertos_mutex_for_ptr(mutex_t *m);
}

// Halt the FreeRTOS PendSV task switching magic
extern "C" int __holdUpPendSV;
