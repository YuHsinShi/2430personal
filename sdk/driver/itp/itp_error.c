/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * PAL error handling functions.
 *
 * @author Jim Tan
 * @version 1.0
 */
#include "itp_cfg.h"
#include <execinfo.h>
#include <malloc.h>
#include <sys/ioctl.h>

#ifdef __OPENRTOS__
    #include "openrtos/FreeRTOS.h"
    #include "openrtos/task.h"
#endif // __OPENRTOS__

//#define GDB_BACKTRACE_DEBUG
#define BACKTRACE_SIZE 100

#ifdef _WIN32

static char printbuf[CFG_DBG_PRINTBUF_SIZE + 4];
static char* __printbuf_addr = printbuf;
static int __printbuf_size = CFG_DBG_PRINTBUF_SIZE;
static int __printbuf_ptr;

#else

extern char* __printbuf_addr;
extern int __printbuf_size;
extern int __printbuf_ptr;

#endif // _WIN32

static void *btbuf[BACKTRACE_SIZE];
static int btcount;
static bool aborted = false;

#ifdef __arm__
static int regVal[16] = { 0 };
#define STORE_REG() {                                                               \
                        asm volatile ("push {r0}\n\t"                               \
                                      "ldr r0, =regVal\n\t"                         \
                                      "stmia r0, {r0-r15}\n\t"                      \
                                      "pop  {r0}\n\t"                               \
                                      "ldr r1, =regVal\n\t"                         \
                                      "str r0, [r1]\n\t"                            \
                                      "str sp, [r1, #52]");                         \
                    }

#else
#define STORE_REG()
#endif

static void PrintDumpRegInfo()
{
#ifdef __arm__
    int i = 0;

    ithPrintf("\r\n------------------Dump ARM register---------------\r\n");
    for (i = 0; i <= 12; i++)
    {
        ithPrintf("r%-12d: 0x%08X\r\n", i, regVal[i]);
    }
    ithPrintf("r13(sp)      : 0x%08X\r\n", regVal[i++]);
    ithPrintf("r14(lr)      : 0x%08X\r\n", regVal[i++]);
    ithPrintf("r15(pc)      : 0x%08X\r\n", regVal[i++]);

    ithPrintf("------------------ABT Mode register---------------\r\n");
    
    asm volatile ("msr cpsr_c, %0\n" : : "r"(ITH_CPU_ABT | 0xC0) : "cc");
    asm volatile ("mov %0, sp" : "=r" (regVal[0]));
    asm volatile ("mov %0, lr" : "=r" (regVal[1]));
    
    
    ithPrintf("sp(abt)      : 0x%08X\n", regVal[0]);
    ithPrintf("lr(abt)      : 0x%08X\n", regVal[1]);

    ithPrintf("------------------IRQ Mode register---------------\r\n");

    asm volatile ("msr cpsr_c, %0\n" : : "r"(ITH_CPU_IRQ | 0xC0) : "cc");
    asm volatile ("mov %0, sp" : "=r" (regVal[0]));
    asm volatile ("mov %0, lr" : "=r" (regVal[1]));
    
    ithPrintf("sp(irq)      : 0x%08X\r\n", regVal[0]);
    ithPrintf("lr(irq)      : 0x%08X\r\n", regVal[1]);

    ithPrintf("------------------SVC Mode register---------------\r\n");
    
    asm volatile ("msr cpsr_c, %0\n" : : "r"(ITH_CPU_SVC | 0xC0) : "cc");
    asm volatile ("mov %0, sp" : "=r" (regVal[0]));
    asm volatile ("mov %0, lr" : "=r" (regVal[1]));
    
    ithPrintf("sp(svc)      : 0x%08X\r\n", regVal[0]);
    ithPrintf("lr(svc)      : 0x%08X\r\n", regVal[1]);

    ithPrintf("----------------Undef Mode register---------------\r\n");
    
    asm volatile ("msr cpsr_c, %0\n" : : "r"(ITH_CPU_UND | 0xC0) : "cc");
    asm volatile ("mov %0, sp" : "=r" (regVal[0]));
    asm volatile ("mov %0, lr" : "=r" (regVal[1]));

    ithPrintf("sp(undef)    : 0x%08X\n", regVal[0]);
    ithPrintf("lr(undef)    : 0x%08X\n", regVal[1]);

    ithPrintf("----------------System Mode register---------------\r\n");

    asm volatile ("msr cpsr_c, %0\n" : : "r"(ITH_CPU_SYS | 0xC0) : "cc");
    asm volatile ("mov %0, sp" : "=r" (regVal[0]));
    asm volatile ("mov %0, lr" : "=r" (regVal[1]));

    asm volatile ("msr cpsr_c, %0\n" : : "r"(ITH_CPU_ABT | 0xC0) : "cc");

    ithPrintf("sp(sys)      : 0x%08X\r\n", regVal[0]);
    ithPrintf("lr(sys)      : 0x%08X\r\n\r\n", regVal[1]);
#endif
}

static void InitConsole(void)
{
#ifdef CFG_DBG_BLUESCREEN
    itpRegisterDevice(ITP_DEVICE_STD, &itpDeviceLcdConsole);
    itpRegisterDevice(ITP_DEVICE_LCDCONSOLE, &itpDeviceLcdConsole);
    ioctl(ITP_DEVICE_LCDCONSOLE, ITP_IOCTL_INIT, NULL);
    ioctl(ITP_DEVICE_LCDCONSOLE, ITP_IOCTL_SET_FGCOLOR, (void*)ITH_RGB565(255, 255, 255));
    ioctl(ITP_DEVICE_LCDCONSOLE, ITP_IOCTL_SET_BGCOLOR, (void*)ITH_RGB565(0, 0, 255));
#endif // CFG_DBG_BLUESCREEN
}

static void PrintStatus(void)
{
    // clock
    ithClockStats();
    // cmdq
#ifdef CFG_CMDQ_ENABLE
    ithCmdQStats();
#endif
}

static void Reboot(void)
{
#ifdef CFG_WATCHDOG_ENABLE
    ithPrintf("Reboot...\n");
    ithWatchDogEnable();
    ithWatchDogSetReload(0);
    ithWatchDogRestart();
#endif // CFG_WATCHDOG_ENABLE
}

void itpErrorUndef( void ) __naked;
void itpErrorUndef(void)
{
    int i;
#ifdef GDB_BACKTRACE_DEBUG
    while(1);
#endif
    
    STORE_REG();
#ifdef CFG_WATCHDOG_INTR
    ithWatchDogCtrlDisable(ITH_WD_INTR);
    ithWatchDogCtrlEnable(ITH_WD_RESET);
#endif

    if (aborted)
        while (1);

    aborted = true;
    btcount = backtrace(btbuf, BACKTRACE_SIZE, 0x4);

    InitConsole();

    ithPrintf("\r\nUndefined Error: %d\n", btcount);

    // backtrace
    for (i = 0; i < btcount; i++)
        ithPrintf("0x%X ", btbuf[i]);

    ithPrintf("\n");
    
    PrintDumpRegInfo();
    PrintStatus();

    Reboot();
    while (1);
}

void itpErrorPrefetchAbort( void ) __naked;
void itpErrorPrefetchAbort(void)
{
    int i;
#ifdef GDB_BACKTRACE_DEBUG
        while(1);
#endif
    STORE_REG();
    
#ifdef CFG_WATCHDOG_INTR
    ithWatchDogCtrlDisable(ITH_WD_INTR);
    ithWatchDogCtrlEnable(ITH_WD_RESET);
#endif

    if (aborted)
        while (1);

    aborted = true;
    btcount = backtrace(btbuf, BACKTRACE_SIZE, 0x4);

    InitConsole();

    ithPrintf("\r\nPrefetch Abort Error: %d\n", btcount);

    // backtrace
    for (i = 0; i < btcount; i++)
        ithPrintf("0x%X ", btbuf[i]);

    ithPrintf("\n");

    PrintDumpRegInfo();
    PrintStatus();

    Reboot();
    while (1);
}

void itpErrorDataAbort( void ) __naked;
void itpErrorDataAbort(void)
{
    int i;
#ifdef GDB_BACKTRACE_DEBUG
        while(1);
#endif
    STORE_REG();
#ifdef CFG_WATCHDOG_INTR
    ithWatchDogCtrlDisable(ITH_WD_INTR);
    ithWatchDogCtrlEnable(ITH_WD_RESET);
#endif

    if (aborted)
        while (1);

    aborted = true;
    btcount = backtrace(btbuf, BACKTRACE_SIZE, 0x8);

    InitConsole();

    ithPrintf("\r\nData Abort Error: %d\n", btcount);

    // backtrace
    for (i = 0; i < btcount; i++)
        ithPrintf("0x%X ", btbuf[i]);

    ithPrintf("\n");

    PrintDumpRegInfo();
    PrintStatus();

    Reboot();
    while (1);
}

void itpErrorDataAbortFIQ( void ) __naked;
void itpErrorDataAbortFIQ(void)
{
#ifdef GDB_BACKTRACE_DEBUG
        while(1);
#endif
#ifdef __arm__
    static int i;
    static volatile unsigned int fiqLr = 0;
    asm volatile ("mov %0, lr" : "=r" (fiqLr): : "lr", "ip");
	ithIntrDisableFiq(ITH_INTR_MC);
	//save register s of SYS mode instead of FIQ mode.
	asm volatile("msr cpsr_c, %0\n" : : "r"(ITH_CPU_SYS | 0xC0) : "cc");
    STORE_REG();

#ifdef CFG_WATCHDOG_INTR
    ithWatchDogCtrlDisable(ITH_WD_INTR);
    ithWatchDogCtrlEnable(ITH_WD_RESET);
#endif

    if (aborted)
        while (1);

    aborted = true;
    
    btcount = backtraceFIQ(btbuf, BACKTRACE_SIZE, fiqLr);
    InitConsole();

    ithPrintf("\r\nMemory Debug Error: %d\n", btcount);

    // backtrace
    for (i = 0; i < btcount; i++)
        ithPrintf("0x%X ", btbuf[i]);

    ithPrintf("\n");

    PrintDumpRegInfo();
    PrintStatus();
#endif
    Reboot();
    while(1);
}


void itpErrorStackOverflow( void ) __naked;
void itpErrorStackOverflow(void)
{
    int i;
#ifdef GDB_BACKTRACE_DEBUG
        while(1);
#endif

    STORE_REG();
    
#ifdef CFG_WATCHDOG_INTR
    ithWatchDogCtrlDisable(ITH_WD_INTR);
    ithWatchDogCtrlEnable(ITH_WD_RESET);
#endif

    if (aborted)
        while (1);

    aborted = true;
    btcount = backtrace(btbuf, BACKTRACE_SIZE, 0x4);

    InitConsole();

    ithPrintf("\r\nStack Overflow Error: %d\n", btcount);

    // backtrace
    for (i = 0; i < btcount; i++)
        ithPrintf("0x%X ", btbuf[i]);

    ithPrintf("\n");
    
    PrintDumpRegInfo();
    PrintStatus();

    Reboot();
    while (1);
}

#ifdef CFG_MEMDBG_ENABLE
void itpErrorMemDbgAbort( void ) __naked;
void itpErrorMemDbgAbort(void)
{
    int i;
#ifdef GDB_BACKTRACE_DEBUG
        while(1);
#endif

#ifdef CFG_WATCHDOG_INTR
    ithWatchDogCtrlDisable(ITH_WD_INTR);
    ithWatchDogCtrlEnable(ITH_WD_RESET);
#endif

    if (aborted)
        while (1);

    aborted = true;
#ifndef _WIN32
    btcount = itpBacktraceIrq(btbuf, BACKTRACE_SIZE);
#endif
    InitConsole();

    ithPrintf("\r\nMemory Debug Error: %d\n", btcount);

    // backtrace
    for (i = 0; i < btcount; i++)
        ithPrintf("0x%X ", btbuf[i]);

    ithPrintf("\n");

    PrintStatus();

    Reboot();
    while (1);
}
#endif // CFG_MEMDBG_ENABLE

void itpErrorDivideByZero( void ) __naked;
void itpErrorDivideByZero(void)
{
    int i;
#ifdef GDB_BACKTRACE_DEBUG
        while(1);
#endif

    STORE_REG();
    
#ifdef CFG_WATCHDOG_INTR
    ithWatchDogCtrlDisable(ITH_WD_INTR);
    ithWatchDogCtrlEnable(ITH_WD_RESET);
#endif

    if (aborted)
        while (1);

    aborted = true;
#ifdef __SM32__
    btcount = 0;    // FIXME: workaround align exception bug
#else
    btcount = backtrace(btbuf, BACKTRACE_SIZE, 0);
#endif

    InitConsole();

    ithPrintf("\r\nDivide by Zero Error: %d\n", btcount);

    // backtrace
    for (i = 0; i < btcount; i++)
        ithPrintf("0x%X ", btbuf[i]);

    ithPrintf("\n");

    PrintDumpRegInfo();
    PrintStatus();

    Reboot();
    while (1);
}
