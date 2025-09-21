/*
    Main app loop and infrastructure for FreeRTOS mode

    Copyright (c) 2025 Earle F. Philhower, III <earlephilhower@yahoo.com>

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

#include <stdlib.h>

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "semphr.h"

/* Arduino Core includes */
#include <Arduino.h>

#include <lwip_wrap.h>
#include <_freertos.h>
#include "freertos-lwip.h"
#include <RP2040USB.h>

/*-----------------------------------------------------------*/

extern void setup() __attribute__((weak));
extern void loop() __attribute__((weak));
extern void setup1() __attribute__((weak));
extern void loop1() __attribute__((weak));
extern void initVariant();
extern void __loop();
static void __core1(void *params);
extern volatile bool __freeRTOSinitted;

static TaskHandle_t __idleCoreTask[2];

void initFreeRTOS(void) {
    __initFreeRTOSMutexes();
}

static void __core0(void *params) {
    (void) params;
    initVariant();

    if (setup1 || loop1) {
        TaskHandle_t c1;
        xTaskCreate(__core1, "CORE1", 1024, 0, configMAX_PRIORITIES / 2, &c1);
        vTaskCoreAffinitySet(c1, 1 << 1);
    }

#if !defined(NO_USB) && !defined(USE_TINYUSB)
    while (!USB.initted) {
        delay(1);
    }
#endif
    if (setup) {
        setup();
    }
    if (loop) {
        while (1) {
            loop();
            __loop();
        }
    } else {
        while (1) {
            __loop();
        }
    }
}

static void __core1(void *params) {
    (void) params;
#if !defined(NO_USB) && !defined(USE_TINYUSB)
    while (!USB.initted) {
        delay(1);
    }
#endif
    if (setup1) {
        setup1();
    }
    if (loop1) {
        while (1) {
            loop1();
        }
    } else {
        while (1) {
            vTaskDelay(1000);
        }
    }
}

extern "C" void delay(unsigned long ms) {
    vTaskDelay(ms / portTICK_PERIOD_MS);
}

extern "C" void yield() {
    taskYIELD();
}

static void __no_inline_not_in_flash_func(IdleThisCore)(void *param) {
    (void) param;
    while (true) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        vTaskPreemptionDisable(nullptr);
        portDISABLE_INTERRUPTS();
        __otherCoreIdled = true;
        while (__otherCoreIdled) {
            /* noop */
        }
        portENABLE_INTERRUPTS();
        vTaskPreemptionEnable(nullptr);
    }
}

extern "C" void __no_inline_not_in_flash_func(__freertos_idle_other_core)() {
    vTaskPreemptionDisable(nullptr);
    xTaskNotifyGive(__idleCoreTask[ 1 ^ sio_hw->cpuid ]);
    while (!__otherCoreIdled) {
        /* noop */
    }
    portDISABLE_INTERRUPTS();
    vTaskSuspendAll();
}

extern "C" void __no_inline_not_in_flash_func(__freertos_resume_other_core)() {
    __otherCoreIdled = false;
    portENABLE_INTERRUPTS();
    xTaskResumeAll();
    vTaskPreemptionEnable(nullptr);
}

void startFreeRTOS(void) {
    TaskHandle_t c0;
    xTaskCreate(__core0, "CORE0", 1024, 0, configMAX_PRIORITIES / 2, &c0);
    vTaskCoreAffinitySet(c0, 1 << 0);

    // Create the idle-other-core tasks (for when flash is being written)
    xTaskCreate(IdleThisCore, "IdleCore0", 128, 0, configMAX_PRIORITIES - 1, __idleCoreTask + 0);
    vTaskCoreAffinitySet(__idleCoreTask[0], 1 << 0);
    xTaskCreate(IdleThisCore, "IdleCore1", 128, 0, configMAX_PRIORITIES - 1, __idleCoreTask + 1);
    vTaskCoreAffinitySet(__idleCoreTask[1], 1 << 1);

    // Initialise and run the freeRTOS scheduler. Execution should never return here.
    __freeRTOSinitted = true;
    vTaskStartScheduler();

    while (true) {
        /* noop */
    }
}

#endif
