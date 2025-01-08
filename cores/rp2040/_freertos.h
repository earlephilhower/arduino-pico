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
#include <pico/mutex.h>

// Cannot include refs to FreeRTOS's actual semaphore calls because they
// are implemented as macros, so we have a wrapper in our variant hook
// to handle it.

extern bool __isFreeRTOS;

// FreeRTOS has been set up
extern volatile bool __freeRTOSinitted;

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus
struct QueueDefinition; /* Using old naming convention so as not to break kernel aware debuggers. */
typedef struct QueueDefinition   * QueueHandle_t;
typedef QueueHandle_t SemaphoreHandle_t;
typedef int32_t BaseType_t;

extern bool __freertos_check_if_in_isr() __attribute__((weak));

extern SemaphoreHandle_t __freertos_mutex_create() __attribute__((weak));
extern SemaphoreHandle_t _freertos_recursive_mutex_create() __attribute__((weak));

extern void __freertos_mutex_take(SemaphoreHandle_t mtx) __attribute__((weak));

extern int __freertos_mutex_take_from_isr(SemaphoreHandle_t mtx, BaseType_t* pxHigherPriorityTaskWoken) __attribute__((weak));
extern int __freertos_mutex_try_take(SemaphoreHandle_t mtx) __attribute__((weak));
extern void __freertos_mutex_give(SemaphoreHandle_t mtx) __attribute__((weak));
extern void __freertos_mutex_give_from_isr(SemaphoreHandle_t mtx, BaseType_t* pxHigherPriorityTaskWoken) __attribute__((weak));

extern void __freertos_recursive_mutex_take(SemaphoreHandle_t mtx) __attribute__((weak));
extern int __freertos_recursive_mutex_try_take(SemaphoreHandle_t mtx) __attribute__((weak));
extern void __freertos_recursive_mutex_give(SemaphoreHandle_t mtx) __attribute__((weak));

extern void __freertos_idle_other_core() __attribute__((weak));
extern void __freertos_resume_other_core() __attribute__((weak));

extern void __freertos_task_exit_critical() __attribute__((weak));
extern void __freertos_task_enter_critical() __attribute__((weak));
#ifdef __cplusplus
}
extern SemaphoreHandle_t __get_freertos_mutex_for_ptr(mutex_t *m, bool recursive = false);
#endif // __cplusplus
