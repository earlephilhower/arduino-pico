#ifdef __FREERTOS
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

#include <Arduino.h>
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "semphr.h"
#include <stdlib.h>

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

extern "C" void vApplicationIdleHook(void) __attribute__((weak));
void vApplicationIdleHook(void) {
    __wfe(); // Low power idle if nothing to do...
}

#endif /* configUSE_IDLE_HOOK == 1 */
/*-----------------------------------------------------------*/

/*
    Call the user defined minimalIdle() function from within the idle task.
    This allows the application designer to add background functionality
    without the overhead of a separate task.

    NOTE: vApplicationMinimalIdleHook() MUST NOT, UNDER ANY CIRCUMSTANCES, CALL A FUNCTION THAT MIGHT BLOCK.

*/
void passiveIdle(void) __attribute__((weak));
void passiveIdle() {} //Empty minimalIdle function

extern "C" void vApplicationPassiveIdleHook(void) {
    passiveIdle();
}

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

extern "C" void vApplicationTickHook(void) __attribute__((weak));
void vApplicationTickHook(void) {
    tick();
}

#endif /* configUSE_TICK_HOOK == 1 */
/*-----------------------------------------------------------*/


/*  ---------------------------------------------------------------------------*\
    Usage:
	called on fatal error (interrupts disabled already)
    \*---------------------------------------------------------------------------*/
extern "C" void rtosFatalError(void) {
    panic("Fatal error");
}

#if ( configUSE_MALLOC_FAILED_HOOK == 1 )
/*  ---------------------------------------------------------------------------*\
    Usage:
    called by task system when a malloc failure is noticed
    \*---------------------------------------------------------------------------*/
extern "C"
void vApplicationMallocFailedHook(void) __attribute__((weak));
void vApplicationMallocFailedHook(void) {
    panic("Malloc failed");
}

#endif /* configUSE_MALLOC_FAILED_HOOK == 1 */
/*-----------------------------------------------------------*/


#if ( configCHECK_FOR_STACK_OVERFLOW >= 1 )

extern "C" void vApplicationStackOverflowHook(TaskHandle_t xTask, char * pcTaskName) __attribute__((weak));
void vApplicationStackOverflowHook(TaskHandle_t xTask __attribute__((unused)), char * pcTaskName __attribute__((unused))) {
    panic("Stack overflow");
}

#endif /* configCHECK_FOR_STACK_OVERFLOW >= 1 */
/*-----------------------------------------------------------*/

extern "C" void vApplicationGetPassiveIdleTaskMemory(StaticTask_t ** ppxIdleTaskTCBBuffer, StackType_t ** ppxIdleTaskStackBuffer, configSTACK_DEPTH_TYPE * puxIdleTaskStackSize, BaseType_t xPassiveIdleTaskIndex) {
    static StaticTask_t xIdleTaskTCBs[ configNUMBER_OF_CORES ];
    static StackType_t uxIdleTaskStacks[ configNUMBER_OF_CORES ][ configMINIMAL_STACK_SIZE ];

    *ppxIdleTaskTCBBuffer = &(xIdleTaskTCBs[ xPassiveIdleTaskIndex ]);
    *ppxIdleTaskStackBuffer = &(uxIdleTaskStacks[ xPassiveIdleTaskIndex ][ 0 ]);
    *puxIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}


#if ( configSUPPORT_STATIC_ALLOCATION >= 1 )

extern "C"
void vApplicationGetIdleTaskMemory(StaticTask_t ** ppxIdleTaskTCBBuffer, StackType_t ** ppxIdleTaskStackBuffer, configSTACK_DEPTH_TYPE * pulIdleTaskStackSize) __attribute__((weak));
void vApplicationGetIdleTaskMemory(StaticTask_t ** ppxIdleTaskTCBBuffer, StackType_t ** ppxIdleTaskStackBuffer, configSTACK_DEPTH_TYPE * pulIdleTaskStackSize) {
    static StaticTask_t xIdleTaskTCB;
    static StackType_t uxIdleTaskStack[ configMINIMAL_STACK_SIZE ];

    *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;
    *ppxIdleTaskStackBuffer = uxIdleTaskStack;
    *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}

#if ( configUSE_TIMERS >= 1 )

extern "C" void vApplicationGetTimerTaskMemory(StaticTask_t ** ppxTimerTaskTCBBuffer, StackType_t ** ppxTimerTaskStackBuffer, configSTACK_DEPTH_TYPE * pulTimerTaskStackSize) __attribute__((weak));
void vApplicationGetTimerTaskMemory(StaticTask_t ** ppxTimerTaskTCBBuffer, StackType_t ** ppxTimerTaskStackBuffer, configSTACK_DEPTH_TYPE * pulTimerTaskStackSize) {
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
    panic("vApplicationAssertHook");
}
#endif

#endif
