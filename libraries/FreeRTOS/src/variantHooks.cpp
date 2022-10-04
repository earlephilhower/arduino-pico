/*
    Copyright (C) 2021 Phillip Stevens  All Rights Reserved.
    Modifications by Earle F. Philhower, III, for Arduino-Pico

    Permission is hereby granted, free of charge, to any person obtaining a copy of
    this software and associated documentation files (the "Software"), to deal in
    the Software without restriction, including without limitation the rights to
    use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
    the Software, and to permit persons to whom the Software is furnished to do so,
    subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
    FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
    COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
    IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
    CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.


    This file is NOT part of the FreeRTOS distribution.

*/
#include <stdlib.h>

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "semphr.h"

/* Arduino Core includes */
#include <Arduino.h>
#include <RP2040USB.h>
#include "tusb.h"

/* Raspberry PI Pico includes */
#include <pico.h>
#include <pico/time.h>

#include "_freertos.h"

/*-----------------------------------------------------------*/

extern void __initFreeRTOSMutexes();
void initFreeRTOS(void) {
    __initFreeRTOSMutexes();
}

extern void setup() __attribute__((weak));
extern void loop() __attribute__((weak));
extern void setup1() __attribute__((weak));
extern void loop1() __attribute__((weak));
// Idle functions (USB, events, ...) from the core
extern void __loop();
volatile bool __usbInitted = false;

static void __core0(void *params) {
    (void) params;
#ifndef NO_USB
    while (!__usbInitted) {
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
#ifndef NO_USB
    while (!__usbInitted) {
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

extern mutex_t __usb_mutex;
static TaskHandle_t __usbTask;
static void __usb(void *param);
extern volatile bool __freeRTOSinitted;
void startFreeRTOS(void) {

    TaskHandle_t c0;
    xTaskCreate(__core0, "CORE0", 1024, 0, configMAX_PRIORITIES / 2, &c0);
    vTaskCoreAffinitySet(c0, 1 << 0);

    if (setup1 || loop1) {
        TaskHandle_t c1;
        xTaskCreate(__core1, "CORE1", 1024, 0, configMAX_PRIORITIES / 2, &c1);
        vTaskCoreAffinitySet(c1, 1 << 1);
    }

    // Initialise and run the freeRTOS scheduler. Execution should never return here.
    __freeRTOSinitted = true;
    vTaskStartScheduler();

    while (true) {
        /* noop */
    }
}


/*-----------------------------------------------------------*/

void prvDisableInterrupts() {
    portDISABLE_INTERRUPTS();
}

void prvEnableInterrupts() {
    portENABLE_INTERRUPTS();
}

/*-----------------------------------------------------------*/
#if ( configUSE_IDLE_HOOK == 1 )
/*
    Call the user defined loop() function from within the idle task.
    This allows the application designer to add background functionality
    without the overhead of a separate task.

    NOTE: vApplicationIdleHook() MUST NOT, UNDER ANY CIRCUMSTANCES, CALL A FUNCTION THAT MIGHT BLOCK.

*/

extern "C"
void vApplicationIdleHook(void) __attribute__((weak));


void vApplicationIdleHook(void) {
    __wfe(); // Low power idle if nothing to do...
}

#endif /* configUSE_IDLE_HOOK == 1 */
/*-----------------------------------------------------------*/

#if ( configUSE_MINIMAL_IDLE_HOOK == 1 )
/*
    Call the user defined minimalIdle() function from within the idle task.
    This allows the application designer to add background functionality
    without the overhead of a separate task.

    NOTE: vApplicationMinimalIdleHook() MUST NOT, UNDER ANY CIRCUMSTANCES, CALL A FUNCTION THAT MIGHT BLOCK.

*/
void minimalIdle(void) __attribute__((weak));
void minimalIdle() {} //Empty minimalIdle function

extern "C"
void vApplicationMinimalIdleHook(void) __attribute__((weak));

void vApplicationMinimalIdleHook(void) {
    minimalIdle();
}

#endif /* configUSE_MINIMAL_IDLE_HOOK == 1 */
/*-----------------------------------------------------------*/

#if ( configUSE_TICK_HOOK == 1 )
/*
    Call the user defined minimalIdle() function from within the idle task.
    This allows the application designer to add background functionality
    without the overhead of a separate task.

    NOTE: vApplicationMinimalIdleHook() MUST NOT, UNDER ANY CIRCUMSTANCES, CALL A FUNCTION THAT MIGHT BLOCK.

*/
void tick(void) __attribute__((weak));
void tick() {} //Empty minimalIdle function

extern "C"
void vApplicationTickHook(void) __attribute__((weak));

void vApplicationTickHook(void) {
    tick();
}

#endif /* configUSE_TICK_HOOK == 1 */
/*-----------------------------------------------------------*/

#if ( configUSE_MALLOC_FAILED_HOOK == 1 || configCHECK_FOR_STACK_OVERFLOW >= 1 || configDEFAULT_ASSERT == 1 )

/**
    Private function to enable board led to use it in application hooks
*/
void prvSetMainLedOn(void) {
    gpio_init(LED_BUILTIN);
    gpio_set_dir(LED_BUILTIN, true);
    gpio_put(LED_BUILTIN, true);
}

/**
    Private function to blink board led to use it in application hooks
*/
void prvBlinkMainLed(void) {
    gpio_put(LED_BUILTIN, !gpio_get(LED_BUILTIN));
}

#endif

/*  ---------------------------------------------------------------------------*\
    Usage:
	called on fatal error (interrupts disabled already)
    \*---------------------------------------------------------------------------*/
extern "C"
void rtosFatalError(void) {
    prvSetMainLedOn(); // Main LED on.

    for (;;) {
        // Main LED slow flash
        sleep_ms(100);
        prvBlinkMainLed();
        sleep_ms(2000);
        prvBlinkMainLed();
    }
}

#if ( configUSE_MALLOC_FAILED_HOOK == 1 )
/*  ---------------------------------------------------------------------------*\
    Usage:
    called by task system when a malloc failure is noticed
    Description:
    Malloc failure handler -- Shut down all interrupts, send serious complaint
    to command port. FAST Blink on main LED.
    Arguments:
    pxTask - pointer to task handle
    pcTaskName - pointer to task name
    Results:
    <none>
    Notes:
    This routine will never return.
    This routine is referenced in the task.c file of FreeRTOS as an extern.
    \*---------------------------------------------------------------------------*/
extern "C"
void vApplicationMallocFailedHook(void) __attribute__((weak));

void vApplicationMallocFailedHook(void) {
    prvSetMainLedOn(); // Main LED on.

    for (;;) {
        sleep_ms(50);
        prvBlinkMainLed(); // Main LED fast blink.
    }
}

#endif /* configUSE_MALLOC_FAILED_HOOK == 1 */
/*-----------------------------------------------------------*/


#if ( configCHECK_FOR_STACK_OVERFLOW >= 1 )

extern "C"
void vApplicationStackOverflowHook(TaskHandle_t xTask,
                                   char * pcTaskName) __attribute__((weak));

void vApplicationStackOverflowHook(TaskHandle_t xTask __attribute__((unused)),
                                   char * pcTaskName __attribute__((unused))) {
    prvSetMainLedOn(); // Main LED on.

    for (;;) {
        sleep_ms(2000);
        prvBlinkMainLed();  // Main LED slow blink.
    }
}

#endif /* configCHECK_FOR_STACK_OVERFLOW >= 1 */
/*-----------------------------------------------------------*/

#if ( configSUPPORT_STATIC_ALLOCATION >= 1 )

extern "C"
void vApplicationGetIdleTaskMemory(StaticTask_t ** ppxIdleTaskTCBBuffer,
                                   StackType_t ** ppxIdleTaskStackBuffer,
                                   configSTACK_DEPTH_TYPE * pulIdleTaskStackSize) __attribute__((weak));

void vApplicationGetIdleTaskMemory(StaticTask_t ** ppxIdleTaskTCBBuffer,
                                   StackType_t ** ppxIdleTaskStackBuffer,
                                   configSTACK_DEPTH_TYPE * pulIdleTaskStackSize) {
    static StaticTask_t xIdleTaskTCB;
    static StackType_t uxIdleTaskStack[ configMINIMAL_STACK_SIZE ];

    *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;
    *ppxIdleTaskStackBuffer = uxIdleTaskStack;
    *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}

#if ( configUSE_TIMERS >= 1 )

extern "C"
void vApplicationGetTimerTaskMemory(StaticTask_t ** ppxTimerTaskTCBBuffer,
                                    StackType_t ** ppxTimerTaskStackBuffer,
                                    configSTACK_DEPTH_TYPE * pulTimerTaskStackSize) __attribute__((weak));

void vApplicationGetTimerTaskMemory(StaticTask_t ** ppxTimerTaskTCBBuffer,
                                    StackType_t ** ppxTimerTaskStackBuffer,
                                    configSTACK_DEPTH_TYPE * pulTimerTaskStackSize) {
    static StaticTask_t xTimerTaskTCB;
    static StackType_t uxTimerTaskStack[ configTIMER_TASK_STACK_DEPTH ];

    *ppxTimerTaskTCBBuffer = &xTimerTaskTCB;
    *ppxTimerTaskStackBuffer = uxTimerTaskStack;
    *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
}

#endif /* configUSE_TIMERS >= 1 */

#endif /* configSUPPORT_STATIC_ALLOCATION >= 1 */

/**
    configASSERT default implementation
*/
#if configDEFAULT_ASSERT == 1

extern "C"
void vApplicationAssertHook() {

    taskDISABLE_INTERRUPTS(); // Disable task interrupts

    prvSetMainLedOn(); // Main LED on.
    for (;;) {
        sleep_ms(100);
        prvBlinkMainLed(); // Led off.

        sleep_ms(2000);
        prvBlinkMainLed(); // Led on.

        sleep_ms(100);
        prvBlinkMainLed(); // Led off

        sleep_ms(100);
        prvBlinkMainLed(); // Led on.
    }
}
#endif


static void __usb(void *param) {
    (void) param;

    tusb_init();

    Serial.begin(115200);

    __usbInitted = true;

    while (true) {
        if (mutex_try_enter(&__usb_mutex, NULL)) {
            tud_task();
            mutex_exit(&__usb_mutex);
        }
        vTaskDelay(1 / portTICK_PERIOD_MS);
    }
}

extern void __SetupDescHIDReport();
extern void __SetupUSBDescriptor();

void __USBStart() {
    mutex_init(&__usb_mutex);

    __SetupDescHIDReport();
    __SetupUSBDescriptor();

    // Make highest prio and locked to core 0
    xTaskCreate(__usb, "USB", 256, 0, configMAX_PRIORITIES - 1, &__usbTask);
    vTaskCoreAffinitySet(__usbTask, 1 << 0);
}


// Interfaces for the main core to use FreeRTOS mutexes
extern "C" {

    SemaphoreHandle_t __freertos_mutex_create() {
        return xSemaphoreCreateMutex();
    }

    SemaphoreHandle_t _freertos_recursive_mutex_create() {
        return xSemaphoreCreateRecursiveMutex();
    }

    void __freertos_mutex_take(SemaphoreHandle_t mtx) {
        xSemaphoreTake(mtx, portMAX_DELAY);
    }

    int __freertos_mutex_try_take(SemaphoreHandle_t mtx) {
        return xSemaphoreTake(mtx, 0);
    }

    void __freertos_mutex_give(SemaphoreHandle_t mtx) {
        xSemaphoreGive(mtx);
    }

    void __freertos_recursive_mutex_take(SemaphoreHandle_t mtx) {
        xSemaphoreTakeRecursive(mtx, portMAX_DELAY);
    }

    int __freertos_recursive_mutex_try_take(SemaphoreHandle_t mtx) {
        return xSemaphoreTakeRecursive(mtx, 0);
    }

    void __freertos_recursive_mutex_give(SemaphoreHandle_t mtx) {
        xSemaphoreGiveRecursive(mtx);
    }

}

