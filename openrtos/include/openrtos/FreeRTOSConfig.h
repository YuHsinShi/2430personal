/*
 * FreeRTOS Kernel V10.0.1
 * Copyright (C) 2017 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://www.FreeRTOS.org
 * http://aws.amazon.com/freertos
 *
 * 1 tab == 4 spaces!
 */

#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

/* Dimensions a buffer that can be used by the FreeRTOS+CLI command
interpreter.  Set this value to 1 to save RAM if FreeRTOS+CLI does not supply
the output butter.  See the FreeRTOS+CLI documentation for more information:
http://www.FreeRTOS.org/FreeRTOS-Plus/FreeRTOS_Plus_CLI/ */
#define configCOMMAND_INT_MAX_OUTPUT_SIZE           1024

/*-----------------------------------------------------------
 * Application specific definitions.
 *
 * These definitions should be adjusted for your particular hardware and
 * application requirements.
 *
 * THESE PARAMETERS ARE DESCRIBED WITHIN THE 'CONFIGURATION' SECTION OF THE
 * FreeRTOS API DOCUMENTATION AVAILABLE ON THE FreeRTOS.org WEB SITE.
 *
 * See http://www.freertos.org/a00110.html.
 *----------------------------------------------------------*/

#define configUSE_PREEMPTION            1
#define configUSE_IDLE_HOOK             1
#define configUSE_TICK_HOOK             0

#ifndef __ASSEMBLER__
    #ifdef __SM32__
        unsigned int ithGetCpuClock(void);
        #define configCPU_CLOCK_HZ          ( ithGetCpuClock() )
    #else
        unsigned int ithGetBusClock(void);
        #define configCPU_CLOCK_HZ          ( ithGetBusClock() )
    #endif // __SM32__
#endif // __ASSEMBLER__

#define configENABLE_BACKWARD_COMPATIBILITY 1
#define configTICK_RATE_HZ              ( ( TickType_t ) 1000 )
#define configMAX_PRIORITIES            ( 6UL )
#define configMINIMAL_STACK_SIZE        ( ( unsigned short ) 20000 )
#define configTOTAL_HEAP_SIZE           ( ( size_t ) ( CFG_OPENRTOS_HEAP_SIZE ) )
#define configMAX_TASK_NAME_LEN         ( 16 )
#define configUSE_16_BIT_TICKS          0
#define configIDLE_SHOULD_YIELD         1
#define configUSE_MUTEXES               1
#define configQUEUE_REGISTRY_SIZE       10
#define configUSE_RECURSIVE_MUTEXES     1
#define configUSE_MALLOC_FAILED_HOOK    0
#define configUSE_APPLICATION_TASK_TAG  1
#define configUSE_COUNTING_SEMAPHORES   1
#define configUSE_NEWLIB_REENTRANT      1

#ifndef NDEBUG
    #define configCHECK_FOR_STACK_OVERFLOW  2
#endif

#ifdef CFG_OPENRTOS_USE_TRACE_FACILITY
    #define configUSE_TRACE_FACILITY        1
#endif

#ifdef CFG_OPENRTOS_GENERATE_RUN_TIME_STATS
    #define configGENERATE_RUN_TIME_STATS   1
#endif

/* This demo makes use of one or more example stats formatting functions.  These
format the raw data provided by the uxTaskGetSystemState() function in to human
readable ASCII form.  See the notes in the implementation of vTaskList() within
FreeRTOS/Source/tasks.c for limitations. */
#define configUSE_STATS_FORMATTING_FUNCTIONS    1

/* Co-routine definitions. */
#define configUSE_CO_ROUTINES       0
#define configMAX_CO_ROUTINE_PRIORITIES ( 2 )

/* Software timer definitions. */
#define configUSE_TIMERS                1
#define configTIMER_TASK_PRIORITY       ( 5 )
#define configTIMER_QUEUE_LENGTH        ( 50 )
#define configTIMER_TASK_STACK_DEPTH    ( 10000 )

/* Set the following definitions to 1 to include the API function, or zero
to exclude the API function. */

#define INCLUDE_vTaskPrioritySet        1
#define INCLUDE_uxTaskPriorityGet       1
#define INCLUDE_vTaskDelete             1
#define INCLUDE_vTaskCleanUpResources   1
#define INCLUDE_vTaskSuspend            1
#define INCLUDE_vTaskDelayUntil         1
#define INCLUDE_vTaskDelay              1
#define INCLUDE_xTaskGetCurrentTaskHandle   1
#define INCLUDE_xTaskGetIdleTaskHandle  1
#define INCLUDE_pcTaskGetTaskName       1
#define INCLUDE_xTimerPendFunctionCall  1

/*-----------------------------------------------------------
 * Macros required to setup the timer for the run time stats.
 *-----------------------------------------------------------*/
#ifndef __ASSEMBLER__
    void vConfigureTimerForRunTimeStats( void );
    unsigned long ulGetRunTimeCounterValue( void );
    #define portCONFIGURE_TIMER_FOR_RUN_TIME_STATS() vConfigureTimerForRunTimeStats()
    #define portGET_RUN_TIME_COUNTER_VALUE() ulGetRunTimeCounterValue()
#endif

#ifndef __NDS32__
    /* Use the optimised task selection rather than the generic C code version. */
    #define configUSE_PORT_OPTIMISED_TASK_SELECTION 1

    #ifdef CFG_POWER_TICKLESS_IDLE
        #define configUSE_TICKLESS_IDLE 1
    #endif
#endif

//#define configCHECK_CRITICAL_TIME     3

#if defined(CFG_DBG_TRACE_ANALYZER) && defined(CFG_DBG_VCD)

#ifndef __ASSEMBLER__
    void portTASK_SWITCHED_IN( void );
    void portTASK_SWITCHED_OUT( void );
    void portTASK_DELAY( void );
    void portTASK_CREATE( void* xTask );
    void portTASK_DELETE( void* xTask );

#endif // !__ASSEMBLER__

#define traceTASK_SWITCHED_IN()     portTASK_SWITCHED_IN()
#define traceTASK_SWITCHED_OUT()    portTASK_SWITCHED_OUT()
#define traceTASK_DELAY()           portTASK_DELAY()
#define traceTASK_CREATE(xTask)     portTASK_CREATE(xTask)
#define traceTASK_DELETE(xTask)     portTASK_DELETE(xTask)

#elif configCHECK_CRITICAL_TIME == 3

#ifndef __ASSEMBLER__
void vTaskSwitchedOut( void );
void vTaskSwitchedIn( void );
#endif

#define traceTASK_SWITCHED_OUT()    vTaskSwitchedOut()
#define traceTASK_SWITCHED_IN()     vTaskSwitchedIn()

#endif // CFG_DBG_TRACE_ANALYZER && CFG_DBG_VCD

#if (CFG_CHIP_FAMILY == 9070 || CFG_CHIP_FAMILY == 9910)
    #define portTIMER       ITH_TIMER6
    #define portTIMER_INTR  ITH_INTR_TIMER6
#elif (CFG_CHIP_FAMILY == 9850)
    // Default uses ITH_TIMER8 on IT9850. Change to TIMER6 due to there's bug on TIMER7 and TIMER8 on IT9859A0.
    #define portTIMER       ITH_TIMER6
    #define portTIMER_INTR  ITH_INTR_TIMER6
#else
    #define portTIMER       ITH_TIMER8
    #define portTIMER_INTR  ITH_INTR_TIMER8
#endif // (CFG_CHIP_FAMILY == 9070 || CFG_CHIP_FAMILY == 9910)

#if defined(CFG_DBG_TRACE_ANALYZER) && defined(CFG_DBG_TRACE)
    #include "trcRecorder.h"
#endif

/* Normal assert() semantics without relying on the provision of an assert.h
header file. */
void vAssertCalled( const char * pcFunc, unsigned long ulLine );
//#define configASSERT( x ) if( ( x ) == 0 ) vAssertCalled( __FUNCTION__, __LINE__ );

#endif /* FREERTOS_CONFIG_H */
