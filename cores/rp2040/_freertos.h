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

#ifdef __FREERTOS
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <FreeRTOS.h>
#include "semphr.h"
#include "task.h"

#include <pico/mutex.h>

// FreeRTOS has been set up
extern volatile bool __freeRTOSinitted;

extern void __freertos_idle_other_core() __attribute__((weak));
extern void __freertos_resume_other_core() __attribute__((weak));

extern SemaphoreHandle_t __get_freertos_mutex_for_ptr(mutex_t *m, bool recursive = false);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __FREERTOS
