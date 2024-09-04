#ifndef configNUMBER_OF_CORES
#define configNUMBER_OF_CORES             2
#endif
#ifndef configUSE_CORE_AFFINITY
#define configUSE_CORE_AFFINITY           1
#endif
#ifndef configRUN_MULTIPLE_PRIORITIES
#define configRUN_MULTIPLE_PRIORITIES     1
#endif

#ifndef configUSE_PREEMPTION
#define configUSE_PREEMPTION              1
#endif
#ifndef configUSE_IDLE_HOOK
#define configUSE_IDLE_HOOK               1
#endif
#ifndef configUSE_PASSIVE_IDLE_HOOK
#define configUSE_PASSIVE_IDLE_HOOK       1
#endif
#ifndef configUSE_TICK_HOOK
#define configUSE_TICK_HOOK               1
#endif
#ifndef configCPU_CLOCK_HZ
#define configCPU_CLOCK_HZ                ( ( unsigned long ) F_CPU  )
#endif
#ifndef configTICK_RATE_HZ
#define configTICK_RATE_HZ                ( ( TickType_t ) 1000 )
#endif
#ifndef configMAX_PRIORITIES
#define configMAX_PRIORITIES              ( 8 )
#endif
#ifndef configMINIMAL_STACK_SIZE
#define configMINIMAL_STACK_SIZE          ( ( unsigned short ) 256 )
#endif
#ifndef configTOTAL_HEAP_SIZE
#define configTOTAL_HEAP_SIZE             ( ( size_t ) ( 164 * 1024 ) )
#endif
#ifndef configMAX_TASK_NAME_LEN
#define configMAX_TASK_NAME_LEN           ( 10 )
#endif
#ifndef configUSE_TRACE_FACILITY
#define configUSE_TRACE_FACILITY          1
#endif
#ifndef configUSE_16_BIT_TICKS
#define configUSE_16_BIT_TICKS            0
#endif
#ifndef configIDLE_SHOULD_YIELD
#define configIDLE_SHOULD_YIELD           1
#endif
#ifndef configUSE_MUTEXES
#define configUSE_MUTEXES                 1
#endif
#ifndef configQUEUE_REGISTRY_SIZE
#define configQUEUE_REGISTRY_SIZE         8
#endif
#ifndef configUSE_RECURSIVE_MUTEXES
#define configUSE_RECURSIVE_MUTEXES       1
#endif
#ifndef configUSE_MALLOC_FAILED_HOOK
#define configUSE_MALLOC_FAILED_HOOK      1
#endif
#ifndef configUSE_APPLICATION_TASK_TAG
#define configUSE_APPLICATION_TASK_TAG    0
#endif
#ifndef configUSE_COUNTING_SEMAPHORES
#define configUSE_COUNTING_SEMAPHORES     1
#endif
#ifndef configUSE_QUEUE_SETS
#define configUSE_QUEUE_SETS              1
#endif
#ifndef configSUPPORT_DYNAMIC_ALLOCATION
#define configSUPPORT_DYNAMIC_ALLOCATION  1
#endif
#ifndef configSUPPORT_STATIC_ALLOCATION
#define configSUPPORT_STATIC_ALLOCATION   1
#endif
#ifndef configSTACK_DEPTH_TYPE
#define configSTACK_DEPTH_TYPE            uint32_t
#endif
#ifndef configUSE_TASK_PREEMPTION_DISABLE
#define configUSE_TASK_PREEMPTION_DISABLE 1
#endif

#ifndef configUSE_NEWLIB_REENTRANT
#define configUSE_NEWLIB_REENTRANT        1
#endif
#ifndef configNEWLIB_REENTRANT_IS_DYNAMIC
#define configNEWLIB_REENTRANT_IS_DYNAMIC 1
#endif

/* Run time stats related definitions. */
#ifndef configGENERATE_RUN_TIME_STATS
#define configGENERATE_RUN_TIME_STATS	1
#endif
#ifdef configGENERATE_RUN_TIME_STATS
extern void vMainConfigureTimerForRunTimeStats(void);
extern unsigned long ulMainGetRunTimeCounterValue(void);
#define portCONFIGURE_TIMER_FOR_RUN_TIME_STATS() //vMainConfigureTimerForRunTimeStats()
#define portGET_RUN_TIME_COUNTER_VALUE() ulMainGetRunTimeCounterValue()
#endif

/* Co-routine definitions. */
#ifndef configUSE_CO_ROUTINES
#define configUSE_CO_ROUTINES           0
#endif
#ifndef configMAX_CO_ROUTINE_PRIORITIES
#define configMAX_CO_ROUTINE_PRIORITIES ( 2 )
#endif

/* Software timer definitions. */
#ifndef configUSE_TIMERS
#define configUSE_TIMERS             1
#endif
#ifndef configTIMER_TASK_PRIORITY
#define configTIMER_TASK_PRIORITY    ( 2 )
#endif
#ifndef configTIMER_QUEUE_LENGTH
#define configTIMER_QUEUE_LENGTH     5
#endif
#ifndef configTIMER_TASK_STACK_DEPTH
#define configTIMER_TASK_STACK_DEPTH ( 1024 )
#endif

/*  Set the following definitions to 1 to include the API function, or zero
    to exclude the API function. */
#ifndef INCLUDE_eTaskGetState
#define INCLUDE_eTaskGetState                   1
#endif
#ifndef INCLUDE_uxTaskGetStackHighWaterMark
#define INCLUDE_uxTaskGetStackHighWaterMark 	1
#endif
#ifndef INCLUDE_uxTaskPriorityGet
#define INCLUDE_uxTaskPriorityGet               1
#endif
#ifndef INCLUDE_vTaskCleanUpResources
#define INCLUDE_vTaskCleanUpResources           1
#endif
#ifndef INCLUDE_vTaskDelay
#define INCLUDE_vTaskDelay                      1
#endif
#ifndef INCLUDE_vTaskDelayUntil
#define INCLUDE_vTaskDelayUntil                 1
#endif
#ifndef INCLUDE_vTaskDelete
#define INCLUDE_vTaskDelete                     1
#endif
#ifndef INCLUDE_vTaskPrioritySet
#define INCLUDE_vTaskPrioritySet                1
#endif
#ifndef INCLUDE_vTaskSuspend
#define INCLUDE_vTaskSuspend                    1
#endif
#ifndef INCLUDE_xQueueGetMutexHolder
#define INCLUDE_xQueueGetMutexHolder            1
#endif
#ifndef INCLUDE_xTaskAbortDelay
#define INCLUDE_xTaskAbortDelay                 1
#endif
#ifndef INCLUDE_xTaskGetCurrentTaskHandle
#define INCLUDE_xTaskGetCurrentTaskHandle       1
#endif
#ifndef INCLUDE_xTaskGetHandle
#define INCLUDE_xTaskGetHandle                  1
#endif
#ifndef INCLUDE_xTaskGetIdleTaskHandle
#define INCLUDE_xTaskGetIdleTaskHandle          1
#endif
#ifndef INCLUDE_xTaskGetSchedulerState
#define INCLUDE_xTaskGetSchedulerState          1
#endif
#ifndef INCLUDE_xTaskResumeFromISR
#define INCLUDE_xTaskResumeFromISR              1
#endif
#ifndef INCLUDE_xTimerPendFunctionCall
#define INCLUDE_xTimerPendFunctionCall          1
#endif

#ifndef configUSE_STATS_FORMATTING_FUNCTIONS
#define configUSE_STATS_FORMATTING_FUNCTIONS	1
#endif

#ifndef configUSE_MUTEXES
#define configUSE_MUTEXES              1
#endif
#ifndef configUSE_MALLOC_FAILED_HOOK
#define configUSE_MALLOC_FAILED_HOOK   1
#endif
#ifndef configCHECK_FOR_STACK_OVERFLOW
#define configCHECK_FOR_STACK_OVERFLOW 2
#endif

/* Cortex-M specific definitions. */
#ifndef configPRIO_BITS
#undef __NVIC_PRIO_BITS
#ifdef __NVIC_PRIO_BITS
/* __BVIC_PRIO_BITS will be specified when CMSIS is being used. */
#define configPRIO_BITS __NVIC_PRIO_BITS
#else
#define configPRIO_BITS 3 /* 8 priority levels */
#endif
#endif

#define configENABLE_MPU                        0
#define configENABLE_TRUSTZONE                  0
#define configRUN_FREERTOS_SECURE_ONLY          1
#define configENABLE_FPU                        1
/*  The lowest interrupt priority that can be used in a call to a "set priority"
    function. */
#ifndef configLIBRARY_LOWEST_INTERRUPT_PRIORITY
#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY 0x7
#endif

/*  The highest interrupt priority that can be used by any interrupt service
    routine that makes calls to interrupt safe FreeRTOS API functions.  DO NOT CALL
    INTERRUPT SAFE FREERTOS API FUNCTIONS FROM ANY INTERRUPT THAT HAS A HIGHER
    PRIORITY THAN THIS! (higher priorities are lower numeric values. */
#ifndef configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY 5
#endif

/*  Interrupt priorities used by the kernel port layer itself.  These are generic
    to all Cortex-M ports, and do not rely on any particular library functions. */
#ifndef configKERNEL_INTERRUPT_PRIORITY
#define configKERNEL_INTERRUPT_PRIORITY ( configLIBRARY_LOWEST_INTERRUPT_PRIORITY << (8 - configPRIO_BITS) )
#endif

/* !!!! configMAX_SYSCALL_INTERRUPT_PRIORITY must not be set to zero !!!!
    See http://www.FreeRTOS.org/RTOS-Cortex-M3-M4.html. */
#ifndef configMAX_SYSCALL_INTERRUPT_PRIORITY
#ifdef PICO_RP2350
#define configMAX_SYSCALL_INTERRUPT_PRIORITY    16
#else
#define configMAX_SYSCALL_INTERRUPT_PRIORITY ( configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << (8 - configPRIO_BITS) )
#endif
#endif

#ifndef configASSERT
#ifdef __cplusplus
extern "C" {
#endif
void rtosFatalError(void);
#ifdef __cplusplus
};
#endif

#define configASSERT( x ) \
	if( ( x ) == 0 ) { portDISABLE_INTERRUPTS(); rtosFatalError(); }
#endif

#ifndef configUSE_DYNAMIC_EXCEPTION_HANDLERS
#define configUSE_DYNAMIC_EXCEPTION_HANDLERS 0
#endif
#ifndef configSUPPORT_PICO_SYNC_INTEROP
#define configSUPPORT_PICO_SYNC_INTEROP      1
#endif
#ifndef configSUPPORT_PICO_TIME_INTEROP
#define configSUPPORT_PICO_TIME_INTEROP      1
#endif

#ifndef LIB_PICO_MULTICORE
#define LIB_PICO_MULTICORE 1
#endif

#include "rp2040_config.h"
