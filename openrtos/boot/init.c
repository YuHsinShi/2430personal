/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * Initialize functions.
 *
 * @author Jim Tan
 * @version 1.0
 */
#include <stdio.h>
#include <sys/ioctl.h>
#include "openrtos/FreeRTOS.h"
#include "openrtos/semphr.h"
#include "ite/itp.h"
#include "alt_cpu/alt_cpu_device.h"
#include "alt_cpu/swUart/swUart.h"

// FIXME: remove inline the tlb.c here. (Kuoping)
// I don't know why it cannot compile the tlb.c standalone. It must
// include inlined here, else the linker do not link the tlb.o.

extern uint32_t __mem_end;

#define VMEM_START  (64) // keep 0 for NULL

int int_init(void);
int int_enable(unsigned long vect);
void BSP_InitIntCtrl(void);

void __init BootInit(void)
{
    static ITHVmem vmem;

#if (CFG_CHIP_FAMILY == 970 || CFG_CHIP_FAMILY == 9860)
    uint32_t targetClkRegVal = ithReadRegA(0xD800027C);
    if (targetClkRegVal)
    {
        //adjust Arm CPU clock to target clock speed.
        ithWriteRegA(0xD800000C, targetClkRegVal);
    }
    //nclk adjust back to 324 Mhz
    //ithWriteRegA(0xD8000018, 0x002a9803);
#endif

#if (CFG_CHIP_FAMILY == 970)
    // workaround SD/NOR co-bus issue
#if !defined(CFG_SD0_ENABLE) && !defined(CFG_SD1_ENABLE)
    ithWriteRegMaskA(0xD800006C, 0x00000005, 0x000001FF);
#endif
    // workaround AX1 JPEG bug
    if (ithGetRevisionId() == 0)
        ithWriteRegMaskA(0xD8000038, 0x0000080A, 0x0000080F);
#endif

    uint32_t bootTime = ithTimerGetTime(portTIMER);

#if defined(CFG_DBG_TRACE_ANALYZER) && defined(CFG_DBG_TRACE)
    vTraceEnable(TRC_START);
#endif

#ifdef CFG_OPENRTOS_MEMPOOL_ENABLE
    // init openrtos memory pool
    vPortInitialiseBlocks();
#endif

    // init video memory management for write-back memory
#if defined(CFG_CPU_WB) || CFG_WT_SIZE > 0
    vmem.startAddr      = VMEM_START;
    vmem.totalSize      = CFG_WT_SIZE;
    vmem.mutex          = xSemaphoreCreateMutex();
    vmem.usedMcbCount = vmem.freeSize = 0;

    ithVmemInit(&vmem);
#endif // defined(CFG_CPU_WB) || CFG_WT_SIZE > 0

#ifdef __SM32__
    int_init();
    int_enable(3);

#elif defined(__NDS32__)
    BSP_InitIntCtrl();

#endif // __SM32__

    // init hal module
    ithInit();

#if defined(CFG_DBG_PRINTBUF)
    // init print buffer device
    itpRegisterDevice(ITP_DEVICE_STD, &itpDevicePrintBuf);
    itpRegisterDevice(ITP_DEVICE_PRINTBUF, &itpDevicePrintBuf);
    ioctl(ITP_DEVICE_PRINTBUF, ITP_IOCTL_INIT, NULL);

//#elif defined(CFG_DBG_SWUART)
    // init sw uart device
//    itpRegisterDevice(ITP_DEVICE_STD, &itpDeviceSwUart);
//    itpRegisterDevice(ITP_DEVICE_SWUART, &itpDeviceSwUart);
//    ioctl(ITP_DEVICE_SWUART, ITP_IOCTL_INIT, (void*)CFG_SWUART_BAUDRATE);

#elif defined(CFG_DBG_UART0)
    // init uart device
    itpRegisterDevice(ITP_DEVICE_STD, &itpDeviceUart0);
    itpRegisterDevice(ITP_DEVICE_UART0, &itpDeviceUart0);
    ioctl(ITP_DEVICE_UART0, ITP_IOCTL_INIT, NULL);
#elif defined(CFG_DBG_UART1)
    itpRegisterDevice(ITP_DEVICE_STD, &itpDeviceUart1);
    itpRegisterDevice(ITP_DEVICE_UART1, &itpDeviceUart1);
    ioctl(ITP_DEVICE_UART1, ITP_IOCTL_INIT, NULL);
#elif defined(CFG_DBG_UART2)
	// init uart device
	itpRegisterDevice(ITP_DEVICE_STD, &itpDeviceUart2);
	itpRegisterDevice(ITP_DEVICE_UART2, &itpDeviceUart2);
	ioctl(ITP_DEVICE_UART2, ITP_IOCTL_INIT, NULL);
#elif defined(CFG_DBG_UART3)
	itpRegisterDevice(ITP_DEVICE_STD, &itpDeviceUart3);
	itpRegisterDevice(ITP_DEVICE_UART3, &itpDeviceUart3);
	ioctl(ITP_DEVICE_UART3, ITP_IOCTL_INIT, NULL);
#elif defined(CFG_DBG_UART4)
	// init uart device
	itpRegisterDevice(ITP_DEVICE_STD, &itpDeviceUart4);
	itpRegisterDevice(ITP_DEVICE_UART4, &itpDeviceUart4);
	ioctl(ITP_DEVICE_UART4, ITP_IOCTL_INIT, NULL);
#elif defined(CFG_DBG_UART5)
	itpRegisterDevice(ITP_DEVICE_STD, &itpDeviceUart5);
	itpRegisterDevice(ITP_DEVICE_UART5, &itpDeviceUart5);
	ioctl(ITP_DEVICE_UART5, ITP_IOCTL_INIT, NULL);
#elif defined(CFG_DBG_SWUART_CODEC)
#if defined (CFG_SW_UART) && !defined(CFG_RS485_4_ENABLE)
	//init Risc
	iteRiscInit();

	//init alt cpu
	itpRegisterDevice(ITP_DEVICE_ALT_CPU, &itpDeviceAltCpu);

	{
		//NOTICE: alt_cpu only for chip 9850
		int altCpuEngineType = ALT_CPU_SW_UART;
		SW_UART_INIT_DATA tUartInitData = {0};

		itpRegisterDevice(ITP_DEVICE_STD, &itpDeviceAltCpu);

		ioctl(ITP_DEVICE_ALT_CPU, ITP_IOCTL_ALT_CPU_SWITCH_ENG, &altCpuEngineType);
		ioctl(ITP_DEVICE_ALT_CPU, ITP_IOCTL_INIT, NULL);
		ioctl(ITP_DEVICE_ALT_CPU, ITP_IOCTL_INIT_DBG_UART, NULL);

		tUartInitData.cpuClock = ithGetRiscCpuClock();
		tUartInitData.baudrate = CFG_SWUARTDBGPRINTF_BAUDRATE;
		tUartInitData.uartRxGpio = -1;
		tUartInitData.uartTxGpio = CFG_SWUARTDBGPRINTF_GPIO;
		tUartInitData.parity = NONE;
		ioctl(ITP_DEVICE_ALT_CPU, ITP_IOCTL_INIT_UART_PARAM, &tUartInitData);
	}
#else
    int swuart_gpio;
    int swuart_baudrate;
   //int swuart_parity;
    iteRiscInit();
    iteRiscOpenEngine(1, 1);
    {
        int i;
        for (i=0; i<1000000; i++)
            asm("");
    }

    itpRegisterDevice(ITP_DEVICE_STD, &itpDeviceSwUartCodecDbg);
    itpRegisterDevice(ITP_DEVICE_SWUARTDBG, &itpDeviceSwUartCodecDbg);

    swuart_gpio = CFG_SWUARTDBGPRINTF_GPIO;
    ioctl(ITP_DEVICE_SWUARTDBG, ITP_IOCTL_SET_GPIO_PIN, &swuart_gpio);
    swuart_baudrate = CFG_SWUARTDBGPRINTF_BAUDRATE;
    ioctl(ITP_DEVICE_SWUARTDBG, ITP_IOCTL_SET_BAUDRATE, &swuart_baudrate);
    //swuart_parity = ITP_SWUART_NONE;
    //ioctl(ITP_DEVICE_SWUARTDBG, ITP_IOCTL_SET_PARITY, &swuart_parity);
    ioctl(ITP_DEVICE_SWUARTDBG, ITP_IOCTL_INIT, NULL);
#endif
#endif // defined(CFG_DBG_PRINTBUF)

    ithPrintf(CFG_SYSTEM_NAME "/" CFG_PROJECT_NAME " ver " CFG_VERSION_MAJOR_STR "." CFG_VERSION_MINOR_STR "." CFG_VERSION_PATCH_STR "." CFG_VERSION_CUSTOM_STR "." CFG_VERSION_TWEAK_STR "\n");

#if (CFG_CHIP_FAMILY != 970 && CFG_CHIP_FAMILY != 9860)  //970 and 9860 using this code will hang it.
    ithPrintRegA(0xd090002c, 28);
#endif

    // booting time
    ithPrintf("booting time: %ums\r\n", bootTime / 1000);

    // Host ID using AW1/AW2 for IT9860
    if((ithReadRegA(ITH_HOST_BASE + 0x274) & 0x1) == 0x0) {
        ithPrintf("WARNNING: AW1 is ES sample, recommend to use MP revision.\n");
    }
}
