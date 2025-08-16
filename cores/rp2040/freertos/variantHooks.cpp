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

#include <_freertos.h>

#include <lwip_wrap.h>

static TaskHandle_t __lwipTask;
static QueueHandle_t __lwipQueue;
typedef struct {
    __lwip_op op;
    void *req;
    TaskHandle_t wakeup;
} LWIPWork;

#define TASK_NOTIFY_LWIP_WAKEUP (configTASK_NOTIFICATION_ARRAY_ENTRIES - 1)
// Interfaces for the main core to use FreeRTOS mutexes
extern "C" {
    extern volatile bool __otherCoreIdled;
    static UBaseType_t __savedIrqs[configNUMBER_OF_CORES];

    SemaphoreHandle_t __freertos_mutex_create() {
        return xSemaphoreCreateMutex();
    }

    SemaphoreHandle_t _freertos_recursive_mutex_create() {
        return xSemaphoreCreateRecursiveMutex();
    }

    void __freertos_mutex_take(SemaphoreHandle_t mtx) {
        xSemaphoreTake(mtx, portMAX_DELAY);
    }

    int __freertos_mutex_take_from_isr(SemaphoreHandle_t mtx, BaseType_t* pxHigherPriorityTaskWoken) {
        return xSemaphoreTakeFromISR(mtx, pxHigherPriorityTaskWoken);
    }

    int __freertos_mutex_try_take(SemaphoreHandle_t mtx) {
        return xSemaphoreTake(mtx, 0);
    }

    void __freertos_mutex_give(SemaphoreHandle_t mtx) {
        xSemaphoreGive(mtx);
    }

    void __freertos_mutex_give_from_isr(SemaphoreHandle_t mtx, BaseType_t* pxHigherPriorityTaskWoken) {
        BaseType_t hiPrio = pxHigherPriorityTaskWoken ? *pxHigherPriorityTaskWoken : pdFALSE;
        xSemaphoreGiveFromISR(mtx, &hiPrio);
        portYIELD_FROM_ISR(hiPrio);
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

    bool __freertos_check_if_in_isr() {
        return portCHECK_IF_IN_ISR();
    }

    void __freertos_task_exit_critical() {
        if (portGET_CRITICAL_NESTING_COUNT() == 1U && portCHECK_IF_IN_ISR()) {
            taskEXIT_CRITICAL_FROM_ISR(__savedIrqs[portGET_CORE_ID()]);
        } else {
            taskEXIT_CRITICAL();
        }
    }

    void __freertos_task_enter_critical() {
        if (portGET_CRITICAL_NESTING_COUNT() == 0U && portCHECK_IF_IN_ISR()) {
            __savedIrqs[portGET_CORE_ID()] = taskENTER_CRITICAL_FROM_ISR();
        } else {
            taskENTER_CRITICAL();
        }
    }
}


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

extern void initVariant();
static void __core1(void *params);
static void __core0(void *params) {
    (void) params;
    initVariant();

    if (setup1 || loop1) {
        TaskHandle_t c1;
        xTaskCreate(__core1, "CORE1", 1024, 0, configMAX_PRIORITIES / 2, &c1);
        vTaskCoreAffinitySet(c1, 1 << 1);
    }

#if !defined(NO_USB) && !defined(USE_TINYUSB)
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
#if !defined(NO_USB) && !defined(USE_TINYUSB)
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

static TaskHandle_t __idleCoreTask[2];
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


extern mutex_t __usb_mutex;
static TaskHandle_t __usbTask;
static void __usb(void *param);
extern volatile bool __freeRTOSinitted;
static void lwipThread(void *params);
void startFreeRTOS(void) {

    TaskHandle_t c0;
    xTaskCreate(__core0, "CORE0", 1024, 0, configMAX_PRIORITIES / 2, &c0);
    vTaskCoreAffinitySet(c0, 1 << 0);

    // Create the idle-other-core tasks (for when flash is being written)
    xTaskCreate(IdleThisCore, "IdleCore0", 128, 0, configMAX_PRIORITIES - 1, __idleCoreTask + 0);
    vTaskCoreAffinitySet(__idleCoreTask[0], 1 << 0);
    xTaskCreate(IdleThisCore, "IdleCore1", 128, 0, configMAX_PRIORITIES - 1, __idleCoreTask + 1);
    vTaskCoreAffinitySet(__idleCoreTask[1], 1 << 1);

    // LWIP runs on core 0 only
    __lwipQueue = xQueueCreate(16, sizeof(LWIPWork));
    //__hwMutex = xSemaphoreCreateMutex();
    xTaskCreate(lwipThread, "LWIP", 1024, 0, configMAX_PRIORITIES - 1, &__lwipTask);
    vTaskCoreAffinitySet(__lwipTask, 1 << 0);

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

//#if ( configUSE_MINIMAL_IDLE_HOOK == 1 )
/*
    Call the user defined minimalIdle() function from within the idle task.
    This allows the application designer to add background functionality
    without the overhead of a separate task.

    NOTE: vApplicationMinimalIdleHook() MUST NOT, UNDER ANY CIRCUMSTANCES, CALL A FUNCTION THAT MIGHT BLOCK.

*/
void passiveIdle(void) __attribute__((weak));
void passiveIdle() {} //Empty minimalIdle function

extern "C"
//void vApplicationPassiveIdleHook(void) __attribute__((weak));

void vApplicationPassiveIdleHook(void) {
    passiveIdle();
}

//#endif /* configUSE_MINIMAL_IDLE_HOOK == 1 */
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
#ifdef LED_BUILTIN
    gpio_init(LED_BUILTIN);
    gpio_set_dir(LED_BUILTIN, true);
    gpio_put(LED_BUILTIN, true);
#endif
}

/**
    Private function to blink board led to use it in application hooks
*/
void prvBlinkMainLed(void) {
#ifdef LED_BUILTIN
    gpio_put(LED_BUILTIN, !gpio_get(LED_BUILTIN));
#endif
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



extern "C" void vApplicationGetPassiveIdleTaskMemory(StaticTask_t ** ppxIdleTaskTCBBuffer,
        StackType_t ** ppxIdleTaskStackBuffer,
        configSTACK_DEPTH_TYPE * puxIdleTaskStackSize,
        BaseType_t xPassiveIdleTaskIndex) {
    static StaticTask_t xIdleTaskTCBs[ configNUMBER_OF_CORES ];
    static StackType_t uxIdleTaskStacks[ configNUMBER_OF_CORES ][ configMINIMAL_STACK_SIZE ];

    *ppxIdleTaskTCBBuffer = &(xIdleTaskTCBs[ xPassiveIdleTaskIndex ]);
    *ppxIdleTaskStackBuffer = &(uxIdleTaskStacks[ xPassiveIdleTaskIndex ][ 0 ]);
    *puxIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}



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

BaseType_t ss;

static void __usb(void *param) {
    (void) param;

    tusb_init();

    Serial.begin(115200);

    __usbInitted = true;

    while (true) {
        ss = xTaskGetSchedulerState();
        if (ss != taskSCHEDULER_SUSPENDED) {
            auto m = __get_freertos_mutex_for_ptr(&__usb_mutex);
            if (xSemaphoreTake(m, 0)) {
                tud_task();
                xSemaphoreGive(m);
            }
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

    // Make high prio and locked to core 0
    xTaskCreate(__usb, "USB", 256, 0, configMAX_PRIORITIES - 2, &__usbTask);
    vTaskCoreAffinitySet(__usbTask, 1 << 0);
}





extern "C" void __lwip(__lwip_op op, void *req) {
    LWIPWork w;
    TaskStatus_t t;
    vTaskGetInfo(nullptr, &t, pdFALSE, eInvalid); // TODO - can we speed this up???
    w.op = op;
    w.req = req;
    w.wakeup = t.xHandle;
    if (!xQueueSend(__lwipQueue, &w, portMAX_DELAY)) {
        panic("LWIP task send failed");
    }
    ulTaskNotifyTakeIndexed(TASK_NOTIFY_LWIP_WAKEUP, pdTRUE, portMAX_DELAY);
}

extern "C" bool __isLWIPThread() {
    TaskStatus_t t;
    vTaskGetInfo(nullptr, &t, pdFALSE, eInvalid); // TODO - can we speed this up???
    return t.xHandle == __lwipTask;
}

static void lwipThread(void *params) {
    (void) params;
    LWIPWork w;
    assert(__isLWIPThread());

    while (true) {
        if (xQueueReceive(__lwipQueue, &w, portMAX_DELAY)) {
            switch (w.op) {
                case __lwip_init:
                {
                    __real_lwip_init();
                    break;
                }
                case __pbuf_header:
                {
                    __pbuf_header_req *r = (__pbuf_header_req *)w.req;
                    *(r->ret) = __real_pbuf_header(r->p, r->header_size);
                    break;
                }
                case __pbuf_free:
                {
                    __pbuf_free_req *r = (__pbuf_free_req *)w.req;
                    *(r->ret) = __real_pbuf_free(r->p);
                    break;
                }
                case __pbuf_alloc:
                {
                    __pbuf_alloc_req *r = (__pbuf_alloc_req *)w.req;
                    *(r->ret) = __real_pbuf_alloc(r->l, r->length, r->type);
                    break;
                }
                case __pbuf_take:
                {
                    __pbuf_take_req *r = (__pbuf_take_req *)w.req;
                    *(r->ret) = __real_pbuf_take(r->buf, r->dataptr, r->len);
                    break;
                }
                case __pbuf_copy_partial:
                {
                    __pbuf_copy_partial_req *r = (__pbuf_copy_partial_req *)w.req;
                    *(r->ret) = __real_pbuf_copy_partial(r->p, r->dataptr, r->len, r->offset);
                    break;
                }
                case __pbuf_ref:
                {
                    __pbuf_ref_req *r = (__pbuf_ref_req *)w.req;
                    __real_pbuf_ref(r->p);
                    break;
                }
                case __pbuf_get_at:
                {
                    __pbuf_get_at_req *r = (__pbuf_get_at_req *)w.req;
                    *(r->ret) = __real_pbuf_get_at(r->p, r->offset);
                    break;
                }
                case __pbuf_get_contiguous:
                {
                    __pbuf_get_contiguous_req *r = (__pbuf_get_contiguous_req *)w.req;
                    *(r->ret) = __real_pbuf_get_contiguous(r->p, r->buffer, r->bufsize, r->len, r->offset);
                    break;
                }
                case __pbuf_cat:
                {
                    __pbuf_cat_req *r = (__pbuf_cat_req *)w.req;
                    __real_pbuf_cat(r->head, r->tail);
                    break;
                }
                case __tcp_arg:
                {
                    __tcp_arg_req *r = (__tcp_arg_req *)w.req;
                    __real_tcp_arg(r->pcb, r->arg);
                    break;
                }
                case __tcp_new:
                {
                    __tcp_new_req *r = (__tcp_new_req *)w.req;
                    *(r->ret) = __real_tcp_new();
                    break;
                }
                case __tcp_new_ip_type:
                {
                    __tcp_new_ip_type_req *r = (__tcp_new_ip_type_req *)w.req;
                    *(r->ret) = __real_tcp_new_ip_type(r->type);
                    break;
                }
                case __tcp_bind:
                {
                    __tcp_bind_req *r = (__tcp_bind_req *)w.req;
                    *(r->ret) = __real_tcp_bind(r->pcb, r->ipaddr, r->port);
                    break;
                }
                case __tcp_bind_netif:
                {
                    __tcp_bind_netif_req *r = (__tcp_bind_netif_req *)w.req;
                    *(r->ret) = __real_tcp_bind_netif(r->pcb, r->netif);
                    break;
                }
                case __tcp_listen_with_backlog:
                {
                    __tcp_listen_with_backlog_req *r = (__tcp_listen_with_backlog_req *)w.req;
                    *(r->ret) = __real_tcp_listen_with_backlog(r->pcb, r->backlog);
                    break;
                }
#if 0
                case __tcp_listen_with_backlog_and_err:
                {
                    __tcp_listen_with_backlog_and_err_req *r = (__tcp_listen_with_backlog_and_err_req *)w.req;
                    *(r->ret) = __real_tcp_listen_with_backlog_and_err(r->pcb, r->backlog, r->err);
                    break;
                }
#endif
                case __tcp_accept:
                {
                    __tcp_accept_req *r = (__tcp_accept_req *)w.req;
                    __real_tcp_accept(r->pcb, r->accept);
                    break;
                }
                case __tcp_connect:
                {
                    __tcp_connect_req *r = (__tcp_connect_req *)w.req;
                    *(r->ret) = __real_tcp_connect(r->pcb, r->ipaddr, r->port, r->connected);
                    break;
                }
                case __tcp_write:
                {
                    __tcp_write_req *r = (__tcp_write_req *)w.req;
                    *(r->ret) = __real_tcp_write(r->pcb, r->dataptr, r->len, r->apiflags);
                    break;
                }
                case __tcp_sent:
                {
                    __tcp_sent_req *r = (__tcp_sent_req *)w.req;
                    __real_tcp_sent(r->pcb, r->sent);
                    break;
                }
                case __tcp_recv:
                {
                    __tcp_recv_req *r = (__tcp_recv_req *)w.req;
                    __real_tcp_recv(r->pcb, r->recv);
                    break;
                }
                case __tcp_recved:
                {
                    __tcp_recved_req *r = (__tcp_recved_req *)w.req;
                    __real_tcp_recved(r->pcb, r->len);
                    break;
                }
                case __tcp_poll:
                {
                    __tcp_poll_req *r = (__tcp_poll_req *)w.req;
                    __real_tcp_poll(r->pcb, r->poll, r->interval);
                    break;
                }
                case __tcp_close:
                {
                    __tcp_close_req *r = (__tcp_close_req *)w.req;
                    *(r->ret) = __real_tcp_close(r->pcb);
                    break;
                }
                case __tcp_abort:
                {
                    __tcp_abort_req *r = (__tcp_abort_req *)w.req;
                    __real_tcp_abort(r->pcb);
                    break;
                }
                case __tcp_err:
                {
                    __tcp_err_req *r = (__tcp_err_req *)w.req;
                    __real_tcp_err(r->pcb, r->err);
                    break;
                }
                case __tcp_output:
                {
                    __tcp_output_req *r = (__tcp_output_req *)w.req;
                    *(r->ret) = __real_tcp_output(r->pcb);
                    break;
                }
                case __tcp_setprio:
                {
                    __tcp_setprio_req *r = (__tcp_setprio_req *)w.req;
                    __real_tcp_setprio(r->pcb, r->prio);
                    break;
                }
                case __tcp_shutdown:
                {
                    __tcp_shutdown_req *r = (__tcp_shutdown_req *)w.req;
                    *(r->ret) = __real_tcp_shutdown(r->pcb, r->shut_rx, r->shut_tx);
                    break;
                }
                case __tcp_backlog_delayed:
                {
                    __tcp_backlog_delayed_req *r = (__tcp_backlog_delayed_req *)w.req;
                    __real_tcp_backlog_delayed(r->pcb);
                    break;
                }
                case __tcp_backlog_accepted:
                {
                    __tcp_backlog_accepted_req *r = (__tcp_backlog_accepted_req *)w.req;
                    __real_tcp_backlog_accepted(r->pcb);
                    break;
                }
                case __udp_new:
                {
                    __udp_new_req *r = (__udp_new_req *)w.req;
                    *(r->ret) = __real_udp_new();
                    break;
                }
                case __udp_new_ip_type:
                {
                    __udp_new_ip_type_req *r = (__udp_new_ip_type_req *)w.req;
                    *(r->ret) = __real_udp_new_ip_type(r->type);
                    break;
                }
                case  __udp_remove:
                {
                    __udp_remove_req *r = (__udp_remove_req *)w.req;
                    __real_udp_remove(r->pcb);
                    break;
                }
                case __udp_bind:
                {
                    __udp_bind_req *r = (__udp_bind_req *)w.req;
                    *(r->ret) = __real_udp_bind(r->pcb, r->ipaddr, r->port);
                    break;
                }
                case __udp_connect:
                {
                    __udp_connect_req *r = (__udp_connect_req *)w.req;
                    *(r->ret) = __real_udp_connect(r->pcb, r->ipaddr, r->port);
                    break;
                }
                case __udp_disconnect:
                {
                    __udp_disconnect_req *r = (__udp_disconnect_req *)w.req;
                    *(r->ret) = __real_udp_disconnect(r->pcb);
                    break;
                }
                case __udp_send:
                {
                    __udp_send_req *r = (__udp_send_req *)w.req;
                    *(r->ret) = __real_udp_send(r->pcb, r->p);
                    break;
                }
                case __udp_recv:
                {
                    __udp_recv_req *r = (__udp_recv_req *)w.req;
                    __real_udp_recv(r->pcb, r->recv, r->recv_arg);
                    break;
                }
                case __udp_sendto:
                {
                    __udp_sendto_req *r = (__udp_sendto_req *)w.req;
                    *(r->ret) = __real_udp_sendto(r->pcb, r->p, r->dst_ip, r->dst_port);
                    break;
                }
                case __udp_sendto_if:
                {
                    __udp_sendto_if_req *r = (__udp_sendto_if_req *)w.req;
                    *(r->ret) = __real_udp_sendto_if(r->pcb, r->p, r->dst_ip, r->dst_port, r->netif);
                    break;
                }
                case __udp_sendto_if_src:
                {
                    __udp_sendto_if_src_req *r = (__udp_sendto_if_src_req *)w.req;
                    *(r->ret) = __real_udp_sendto_if_src(r->pcb, r->p, r->dst_ip, r->dst_port, r->netif, r->src_ip);
                    break;
                }
                case __sys_check_timeouts:
                {
                    __real_sys_check_timeouts();
                    break;
                }
                case __dns_gethostbyname:
                {
                    __dns_gethostbyname_req *r = (__dns_gethostbyname_req *)w.req;
                    *(r->ret) = __real_dns_gethostbyname(r->hostname, r->addr, r->found, r->callback_arg);
                    break;
                }
                case __dns_gethostbyname_addrtype:
                {
                    __dns_gethostbyname_addrtype_req *r = (__dns_gethostbyname_addrtype_req *)w.req;
                    *(r->ret) = __real_dns_gethostbyname_addrtype(r->hostname, r->addr, r->found, r->callback_arg, r->dns_addrtype);
                    break;
                }
                case __raw_new:
                {
                    __raw_new_req *r = (__raw_new_req *)w.req;
                    *(r->ret) = __real_raw_new(r->proto);
                    break;
                }
                case __raw_new_ip_type:
                {
                    __raw_new_ip_type_req *r = (__raw_new_ip_type_req *)w.req;
                    *(r->ret) = __real_raw_new_ip_type(r->type, r->proto);
                    break;
                }
                case __raw_connect:
                {
                    __raw_connect_req *r = (__raw_connect_req *)w.req;
                    *(r->ret) = __real_raw_connect(r->pcb, r->ipaddr);
                    break;
                }
                case __raw_recv:
                {
                    __raw_recv_req *r = (__raw_recv_req *)w.req;
                    __real_raw_recv(r->pcb, r->recv, r->recv_arg);
                    break;
                }
                case __raw_bind:
                {
                    __raw_bind_req *r = (__raw_bind_req *)w.req;
                    *(r->ret) = __real_raw_bind(r->pcb, r->ipaddr);
                    break;
                }
                case __raw_sendto:
                {
                    __raw_sendto_req *r = (__raw_sendto_req *)w.req;
                    *(r->ret) = __real_raw_sendto(r->pcb, r->p, r->ipaddr);
                    break;
                }
                case __raw_send:
                {
                    __raw_send_req *r = (__raw_send_req *)w.req;
                    *(r->ret) = __real_raw_send(r->pcb, r->p);
                    break;
                }
                case __raw_remove:
                {
                    __raw_remove_req *r = (__raw_remove_req *)w.req;
                    __real_raw_remove(r->pcb);
                    break;
                }
                case __netif_add:
                {
                    __netif_add_req *r = (__netif_add_req *)w.req;
                    *(r->ret) = __real_netif_add(r->netif, r->ipaddr, r->netmask, r->gw, r->state, r->init, r->input);
                    break;
                }
                case __netif_remove:
                {
                    __netif_remove_req *r = (__netif_remove_req *)w.req;
                    __real_netif_remove(r->netif);
                    break;
                }
                case __ethernet_input:
                {
                    __ethernet_input_req *r = (__ethernet_input_req *)w.req;
                    printf("__real_ethernet_input\n");
                    *(r->ret) = __real_ethernet_input(r->p, r->netif);
                    break;
                }
                case __callback:
                {
                    __callback_req *r = (__callback_req *)w.req;
                    r->cb(r->cbData);
                    break;
                }
                default:
                {
                    // Any new unimplemented calls = ERROR!!!
                    panic("Unimplemented LWIP thread action");
                    break;
                }
            }
            // Work done, return value set, just tickle the calling task
            xTaskNotifyGiveIndexed(w.wakeup, TASK_NOTIFY_LWIP_WAKEUP);
        }
    }
}

#endif
