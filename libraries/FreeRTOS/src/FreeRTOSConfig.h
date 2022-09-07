/* Cores */
#define configNUM_CORES                 2
#define configUSE_CORE_AFFINITY         1
#define configRUN_MULTIPLE_PRIORITIES   1

#define configUSE_PREEMPTION			1
#define configUSE_IDLE_HOOK				1
#define configUSE_MINIMAL_IDLE_HOOK     1
#define configUSE_TICK_HOOK				1
#define configCPU_CLOCK_HZ				( ( unsigned long ) F_CPU  )
#define configTICK_RATE_HZ				( ( TickType_t ) 1000 )
#define configMAX_PRIORITIES			( 8 )
#define configMINIMAL_STACK_SIZE		( ( unsigned short ) 256 )
#define configTOTAL_HEAP_SIZE			( ( size_t ) ( 164 * 1024 ) )
#define configMAX_TASK_NAME_LEN			( 10 )
#define configUSE_TRACE_FACILITY		1
#define configUSE_16_BIT_TICKS			0
#define configIDLE_SHOULD_YIELD			1
#define configUSE_MUTEXES				1
#define configQUEUE_REGISTRY_SIZE		8
#define configUSE_RECURSIVE_MUTEXES		1
#define configUSE_MALLOC_FAILED_HOOK	1
#define configUSE_APPLICATION_TASK_TAG	0
#define configUSE_COUNTING_SEMAPHORES	1
#define configUSE_QUEUE_SETS			1
#define configSUPPORT_DYNAMIC_ALLOCATION 1
#define configSUPPORT_STATIC_ALLOCATION 1
#define configSTACK_DEPTH_TYPE			uint32_t
#define configUSE_TASK_PREEMPTION_DISABLE 1

#define configUSE_NEWLIB_REENTRANT 1
#define configNEWLIB_REENTRANT_IS_DYNAMIC 0 /* Note that we have a different config option, portSET_IMPURE_PTR */
#include <reent.h>
extern void __register_impure_ptr(struct _reent *p);
#define portSET_IMPURE_PTR(x) __register_impure_ptr(x)

/* Run time stats related definitions. */
void vMainConfigureTimerForRunTimeStats(void);
unsigned long ulMainGetRunTimeCounterValue(void);
#define configGENERATE_RUN_TIME_STATS	1
#define portCONFIGURE_TIMER_FOR_RUN_TIME_STATS() //vMainConfigureTimerForRunTimeStats()
#define portGET_RUN_TIME_COUNTER_VALUE() ulMainGetRunTimeCounterValue()

/* Co-routine definitions. */
#define configUSE_CO_ROUTINES 			1
#define configMAX_CO_ROUTINE_PRIORITIES ( 2 )

/* Software timer definitions. */
#define configUSE_TIMERS				1
#define configTIMER_TASK_PRIORITY		( 2 )
#define configTIMER_QUEUE_LENGTH		5
#define configTIMER_TASK_STACK_DEPTH	( 1024 )

/*  Set the following definitions to 1 to include the API function, or zero
    to exclude the API function. */
#define INCLUDE_xTaskGetCurrentTaskHandle   1
#define INCLUDE_vTaskPrioritySet		1
#define INCLUDE_uxTaskPriorityGet		1
#define INCLUDE_vTaskDelete				1
#define INCLUDE_vTaskCleanUpResources	1
#define INCLUDE_vTaskSuspend			1
#define INCLUDE_vTaskDelayUntil			1
#define INCLUDE_vTaskDelay				1
#define INCLUDE_eTaskGetState			1

/*  This demo makes use of one or more example stats formatting functions.  These
    format the raw data provided by the uxTaskGetSystemState() function in to human
    readable ASCII form.  See the notes in the implementation of vTaskList() within
    FreeRTOS/Source/tasks.c for limitations. */
#define configUSE_STATS_FORMATTING_FUNCTIONS	1

#define configUSE_MUTEXES        				1
#define INCLUDE_uxTaskGetStackHighWaterMark 	1
#define INCLUDE_xTaskGetIdleTaskHandle 			1
#define configUSE_MALLOC_FAILED_HOOK			1
#define configCHECK_FOR_STACK_OVERFLOW 			2
/*  Normal assert() semantics without relying on the provision of an assert.h
    header file. */

/* Cortex-M specific definitions. */
#undef __NVIC_PRIO_BITS
#ifdef __NVIC_PRIO_BITS
/* __BVIC_PRIO_BITS will be specified when CMSIS is being used. */
#define configPRIO_BITS       		__NVIC_PRIO_BITS
#else
#define configPRIO_BITS       		3        /* 8 priority levels */
#endif

/*  The lowest interrupt priority that can be used in a call to a "set priority"
    function. */
#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY			0x7

/*  The highest interrupt priority that can be used by any interrupt service
    routine that makes calls to interrupt safe FreeRTOS API functions.  DO NOT CALL
    INTERRUPT SAFE FREERTOS API FUNCTIONS FROM ANY INTERRUPT THAT HAS A HIGHER
    PRIORITY THAN THIS! (higher priorities are lower numeric values. */
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY	5

/*  Interrupt priorities used by the kernel port layer itself.  These are generic
    to all Cortex-M ports, and do not rely on any particular library functions. */
#define configKERNEL_INTERRUPT_PRIORITY 		( configLIBRARY_LOWEST_INTERRUPT_PRIORITY << (8 - configPRIO_BITS) )
/* !!!! configMAX_SYSCALL_INTERRUPT_PRIORITY must not be set to zero !!!!
    See http://www.FreeRTOS.org/RTOS-Cortex-M3-M4.html. */
#define configMAX_SYSCALL_INTERRUPT_PRIORITY 	( configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << (8 - configPRIO_BITS) )

extern void rtosFatalError(void);
#define configASSERT( x ) \
	if( ( x ) == 0 ) { portDISABLE_INTERRUPTS(); rtosFatalError(); }

/*  Definitions that map the FreeRTOS port interrupt handlers to their CMSIS
    standard names - or at least those used in the unmodified vector table. */
//#define xPortPendSVHandler PendSV_Handler
//#define xPortSysTickHandler SysTick_Handler
//#define vPortSVCHandler    SVC_Handler

/*  The size of the global output buffer that is available for use when there
    are multiple command interpreters running at once (for example, one on a UART
    and one on TCP/IP).  This is done to prevent an output buffer being defined by
    each implementation - which would waste RAM.  In this case, there is only one
    command interpreter running. */
#define configCOMMAND_INT_MAX_OUTPUT_SIZE 2048


#define configUSE_DYNAMIC_EXCEPTION_HANDLERS 0
#define configSUPPORT_PICO_SYNC_INTEROP 1
#define configSUPPORT_PICO_TIME_INTEROP 1


#include "rp2040_config.h"
//#include "task.h"
