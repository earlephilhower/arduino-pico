/*
    Newlib retargetable lock class using RP2040 SDK

    Overrides weak functions in Newlib for locking to safely support
    multi-core operation.  Does not need any --wrap for memory allocators.

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


#include <pico/mutex.h>
#include <sys/lock.h>

#include "_freertos.h"

// HACK ALERT
// Pico-SDK defines mutex which can be at global scope, but when the auto_init_
// macros are used they are defined as static.  Newlib needs global access to
// these mutextes, so instead of hacking pico-sdk just make "static" a no-op.

#define static
auto_init_recursive_mutex(__lock___sinit_recursive_mutex);
auto_init_recursive_mutex(__lock___sfp_recursive_mutex);
auto_init_recursive_mutex(__lock___atexit_recursive_mutex);
auto_init_mutex(__lock___at_quick_exit_mutex);
auto_init_recursive_mutex(__lock___malloc_recursive_mutex);
auto_init_recursive_mutex(__lock___env_recursive_mutex);
auto_init_mutex(__lock___tz_mutex);
auto_init_mutex(__lock___dd_hash_mutex);
auto_init_mutex(__lock___arc4random_mutex);
#undef static

// FreeRTOS hack - Allow Newlib to use FreeRTOS mutexes which preserve TASKID which
// is needed to support multithread

SemaphoreHandle_t __lock___sinit_recursive_mutex_freertos;
SemaphoreHandle_t __lock___sfp_recursive_mutex_freertos;
SemaphoreHandle_t __lock___atexit_recursive_mutex_freertos;
SemaphoreHandle_t __lock___at_quick_exit_mutex_freertos;
SemaphoreHandle_t __lock___malloc_recursive_mutex_freertos;
SemaphoreHandle_t __lock___env_recursive_mutex_freertos;
SemaphoreHandle_t __lock___tz_mutex_freertos;
SemaphoreHandle_t __lock___dd_hash_mutex_freertos;
SemaphoreHandle_t __lock___arc4random_mutex_freertos;

void __initFreeRTOSMutexes() {
    __lock___sinit_recursive_mutex_freertos = _freertos_recursive_mutex_create();
    __lock___sfp_recursive_mutex_freertos = _freertos_recursive_mutex_create();
    __lock___atexit_recursive_mutex_freertos = _freertos_recursive_mutex_create();
    __lock___at_quick_exit_mutex_freertos = __freertos_mutex_create();
    __lock___malloc_recursive_mutex_freertos = _freertos_recursive_mutex_create();
    __lock___env_recursive_mutex_freertos = _freertos_recursive_mutex_create();
    __lock___tz_mutex_freertos = __freertos_mutex_create();
    __lock___dd_hash_mutex_freertos = __freertos_mutex_create();
    __lock___arc4random_mutex_freertos = __freertos_mutex_create();
}

static SemaphoreHandle_t __getFreeRTOSMutex(_LOCK_T lock) {
    mutex_t *l = (mutex_t *)lock;
    if (l == &__lock___at_quick_exit_mutex) {
        return __lock___at_quick_exit_mutex_freertos;
    } else if (l == &__lock___tz_mutex) {
        return __lock___tz_mutex_freertos;
    } else if (l == &__lock___dd_hash_mutex) {
        return __lock___dd_hash_mutex_freertos;
    } else if (l == &__lock___arc4random_mutex) {
        return __lock___arc4random_mutex_freertos;
    }
    return nullptr;
}

static SemaphoreHandle_t __getFreeRTOSRecursiveMutex(_LOCK_T lock) {
    recursive_mutex_t *l = (recursive_mutex_t *)lock;
    if (l  == &__lock___sinit_recursive_mutex) {
        return __lock___sinit_recursive_mutex_freertos;
    } else if (l == &__lock___sfp_recursive_mutex) {
        return __lock___sfp_recursive_mutex_freertos;
    } else if (l == &__lock___atexit_recursive_mutex) {
        return __lock___atexit_recursive_mutex_freertos;
    } else if (l == &__lock___malloc_recursive_mutex) {
        return __lock___malloc_recursive_mutex_freertos;
    } else if (l == &__lock___env_recursive_mutex) {
        return __lock___env_recursive_mutex_freertos;
    }
    return nullptr;
}

void __retarget_lock_init(_LOCK_T *lock) {
    if (__freeRTOSinitted) {
        /* Already done in initFreeRTOSMutexes() */
    } else {
        mutex_init((mutex_t*) lock);
    }
}

void __retarget_lock_init_recursive(_LOCK_T *lock) {
    if (__freeRTOSinitted) {
        /* Already done in initFreeRTOSMutexes() */
    } else {
        recursive_mutex_init((recursive_mutex_t*) lock);
    }
}

void __retarget_lock_close(_LOCK_T lock) {
    (void) lock;
}

void __retarget_lock_close_recursive(_LOCK_T lock) {
    (void) lock;
}

void __retarget_lock_acquire(_LOCK_T lock) {
    if (__freeRTOSinitted) {
        auto mtx = __getFreeRTOSMutex(lock);
        __freertos_mutex_take(mtx);
    } else {
        mutex_enter_blocking((mutex_t*)lock);
    }
}

void __retarget_lock_acquire_recursive(_LOCK_T lock) {
    if (__freeRTOSinitted) {
        auto mtx = __getFreeRTOSRecursiveMutex(lock);
        __freertos_recursive_mutex_take(mtx);
    } else {
        recursive_mutex_enter_blocking((recursive_mutex_t*)lock);
    }
}

int __retarget_lock_try_acquire(_LOCK_T lock) {
    int ret;
    if (__freeRTOSinitted) {
        auto mtx = __getFreeRTOSMutex(lock);
        ret = __freertos_mutex_try_take(mtx);
    } else {
        ret = mutex_try_enter((mutex_t *)lock, nullptr);
    }
    return ret;
}

int __retarget_lock_try_acquire_recursive(_LOCK_T lock) {
    int ret;
    if (__freeRTOSinitted) {
        auto mtx = __getFreeRTOSRecursiveMutex(lock);
        ret = __freertos_recursive_mutex_try_take(mtx);
    } else {
        ret = recursive_mutex_try_enter((recursive_mutex_t*)lock, nullptr);
    }
    return ret;
}

void __retarget_lock_release(_LOCK_T lock) {
    if (__freeRTOSinitted) {
        auto mtx = __getFreeRTOSMutex(lock);
        __freertos_mutex_give(mtx);
    } else {
        mutex_exit((mutex_t*)lock);
    }
}

void __retarget_lock_release_recursive(_LOCK_T lock) {
    if (__freeRTOSinitted) {
        auto mtx = __getFreeRTOSRecursiveMutex(lock);
        __freertos_recursive_mutex_give(mtx);
    } else {
        recursive_mutex_exit((recursive_mutex_t*)lock);
    }
}
