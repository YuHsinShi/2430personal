/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * PAL Initialize functions.
 *
 * @author Jim Tan
 * @version 1.0
 */
#include <sys/ioctl.h>
#include <errno.h>
#include <locale.h>
#include <stdlib.h>
#include <time.h>
#include "openrtos/FreeRTOS.h"
#include "fat/api/api_fat.h"
#include "itp_cfg.h"
#include "i2s/i2s.h"
#include "ite/ite_risc.h"
#include "alt_cpu/alt_cpu_device.h"
#include "alt_cpu/swUart/swUart.h"

extern uint32_t __lcd_base_a;
extern uint32_t __lcd_base_b;
extern uint32_t __lcd_base_c;

extern void __libc_init_array(void);  /* calls CTORS of static objects */
extern void itpErrorMemDbgAbort(void) __naked;

#ifdef CFG_MEMDBG_ENABLE
    #if (CFG_CHIP_FAMILY == 970 || CFG_CHIP_FAMILY == 9860)

        #define AXI0_ID_CNT 16
static uint16_t   axi0_id_map[AXI0_ID_CNT] = {
    0x00, 0x10, 0x20, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x40, 0x50, 0x60, 0x70, 0x80
};

static const char *dbgAxi0String[] = {
    "ARM instruction read",
    "ARM data read/write",
    "ARM p port read/write",
    "DMA ch0 read/write",
    "DMA ch1 read/write",
    "DMA ch2 read/write",
    "DMA ch3 read/write",
    "DMA ch4 read/write",
    "DMA ch5 read/write",
    "DMA ch6 read/write",
    "DMA ch7 read/write",
    "AHB Bridge 0 read/write",
    "AHB Bridge 1 read/write",
    "AHB Bridge 2 read/write",
    "GMAC read/write id",
    "ARM's DMA read/write"
};
static const char *dbgAxi1String[] = {
    "H264 read",
    "H264 write"
};
static const char *dbgAxi2String[] = {
    "LCD read",
    "VP read",
    "SADC write",
    "CMDQ0 read/write",
    "RISC read/write",
    "I2S write",
    "USB read/write",
    "TSI read/write",
    "DCPS read/write",
    "UIENC read/write",
    "BIST2 read/write"
};
static const char *dbgAxi3String[] = {
    "CAP read/write",
    "VP write",
    "JPEG read/write",
    "2D read/write",
    "CMDQ1 read/write",
    "I2S read",
    "DPU read/write",
    "TSM read/write",
    "BIST0 read/write",
    "BIST1 read/write",
    "IQ read/write"
};
    #else
        #error "dbgFlag no defined"
    #endif

static void MemDbgHandler(void *arg)
{
    uint32_t id0, id1, id2, id3, i;

    if (ithMemDbgGetFlag(ITH_MEMDBG0, &id0, &id1, &id2, &id3) >= 0)
    {
        ithPrintf("\n");
        ithPrintf("0x%08X = 0x%08X (intr status)\n", ITH_MEMDBG0 + ITH_MEMDBG_EN_REG,     ithReadRegA(ITH_MEMDBG0 + ITH_MEMDBG_EN_REG));
        ithPrintf("0x%08X = 0x%08X (id status\n",    ITH_MEMDBG0 + ITH_MEMDBG_STATUS_REG, ithReadRegA(ITH_MEMDBG0 + ITH_MEMDBG_STATUS_REG));
        ithPrintf("range: 0x%08X - 0x%08X \n",       ithReadRegA(ITH_MEMDBG0 + ITH_MEMDBG_ADDR_LO_REG),
                  ithReadRegA(ITH_MEMDBG0 + ITH_MEMDBG_ADDR_HI_REG));

        if (id0 != ITH_MEMDBG_PASSID_AXI0_NONE)
        {
            for (i = 0; i < AXI0_ID_CNT; i++)
            {
                if (id0 == axi0_id_map[i])
                    ithPrintf("[ERROR] Memory access detect on region 0 by AXI0:%s.\n", dbgAxi0String[i]);
            }
        }
        if (id1 != ITH_MEMDBG_PASSID_AXI1_NONE)
            ithPrintf("[ERROR] Memory access detect on region 0 by AXI1:%s.\n", dbgAxi1String[id1]);
        if (id2 != ITH_MEMDBG_PASSID_AXI2_NONE)
            ithPrintf("[ERROR] Memory access detect on region 0 by AXI2:%s.\n", dbgAxi2String[id2]);
        if (id3 != ITH_MEMDBG_PASSID_AXI3_NONE)
            ithPrintf("[ERROR] Memory access detect on region 0 by AXI3:%s.\n", dbgAxi3String[id3]);
    }

    if (ithMemDbgGetFlag(ITH_MEMDBG1, &id0, &id1, &id2, &id3) >= 0)
    {
        ithPrintf("\n");
        ithPrintf("0x%08X = 0x%08X (intr status)\n", ITH_MEMDBG1 + ITH_MEMDBG_EN_REG,     ithReadRegA(ITH_MEMDBG1 + ITH_MEMDBG_EN_REG));
        ithPrintf("0x%08X = 0x%08X (id status\n",    ITH_MEMDBG1 + ITH_MEMDBG_STATUS_REG, ithReadRegA(ITH_MEMDBG1 + ITH_MEMDBG_STATUS_REG));
        ithPrintf("range: 0x%08X - 0x%08X \n",       ithReadRegA(ITH_MEMDBG1 + ITH_MEMDBG_ADDR_LO_REG),
                  ithReadRegA(ITH_MEMDBG1 + ITH_MEMDBG_ADDR_HI_REG));

        if (id0 != ITH_MEMDBG_PASSID_AXI0_NONE)
        {
            for (i = 0; i < AXI0_ID_CNT; i++)
            {
                if (id0 == axi0_id_map[i])
                    ithPrintf("[ERROR] Memory access detect on region 1 by AXI0:%s.\n", dbgAxi0String[i]);
            }
        }
        if (id1 != ITH_MEMDBG_PASSID_AXI1_NONE)
            ithPrintf("[ERROR] Memory access detect on region 1 by AXI1:%s.\n", dbgAxi1String[id1]);
        if (id2 != ITH_MEMDBG_PASSID_AXI2_NONE)
            ithPrintf("[ERROR] Memory access detect on region 1 by AXI2:%s.\n", dbgAxi2String[id2]);
        if (id3 != ITH_MEMDBG_PASSID_AXI3_NONE)
            ithPrintf("[ERROR] Memory access detect on region 1 by AXI3:%s.\n", dbgAxi3String[id3]);
    }
    ithMemDbgClearIntr();
    itpErrorMemDbgAbort();
}
#endif // CFG_MEMDBG_ENABLE

void itpInit(void)
{
    struct itimerspec value;

    int               wiegand_bit_count;
    int               wiegand_gpio[2];
    int               swuart_gpio;
    int               swuart_baudrate;

    // init lcd
#ifdef CFG_LCD_ENABLE
    itpRegisterDevice(ITP_DEVICE_SCREEN, &itpDeviceScreen);

    #ifndef CFG_LCD_MULTIPLE

        #if !defined(CFG_BL_SHOW_LOGO) && !defined(CFG_CHIP_FT)
            #ifdef CFG_GPIO_LCD_PWR_EN
                #if (CFG_GPIO_LCD_PWR_EN > 0)
    ithGpioSetMode(CFG_GPIO_LCD_PWR_EN, ITH_GPIO_MODE0);  //LCD_PWR_EN
    ithGpioSetOut(CFG_GPIO_LCD_PWR_EN);
                    #ifdef CFG_GPIO_LCD_PWR_EN_ACTIVE_LOW
    ithGpioClear(CFG_GPIO_LCD_PWR_EN);
                    #else
    ithGpioSet(CFG_GPIO_LCD_PWR_EN);
                    #endif
                #endif
            #endif //CFG_GPIO_LCD_PWR_EN
    ioctl(ITP_DEVICE_SCREEN, ITP_IOCTL_RESET, NULL);
    ithLcdSetBaseAddrA((uint32_t) &__lcd_base_a);
    ithLcdSetBaseAddrB((uint32_t) &__lcd_base_b);

            #if defined(CFG_VIDEO_ENABLE) || defined(CFG_LCD_TRIPLE_BUFFER)
    ithLcdSetBaseAddrC((uint32_t) &__lcd_base_c);
            #endif
        #endif // !CFG_BL_SHOW_LOGO

    #endif     // !CFG_LCD_MULTIPLE

#endif         // CFG_LCD_ENABLE

#ifdef CFG_BACKLIGHT_ENABLE
    itpRegisterDevice(ITP_DEVICE_BACKLIGHT, &itpDeviceBacklight);
    ioctl(ITP_DEVICE_BACKLIGHT, ITP_IOCTL_INIT, NULL);
#endif // CFG_BACKLIGHT_ENABLE

#if defined(CFG_DBG_LCDCONSOLE) && !defined(CFG_LCD_MULTIPLE)
    // init lcd console device
    itpRegisterDevice(ITP_DEVICE_STD,        &itpDeviceLcdConsole);
    itpRegisterDevice(ITP_DEVICE_LCDCONSOLE, &itpDeviceLcdConsole);

    ioctl(ITP_DEVICE_LCDCONSOLE, ITP_IOCTL_INIT,       NULL);
    ioctl(ITP_DEVICE_LCDCONSOLE, ITP_IOCTL_CLEAR,      NULL);
    ioctl(ITP_DEVICE_SCREEN,     ITP_IOCTL_POST_RESET, NULL);
    ioctl(ITP_DEVICE_BACKLIGHT,  ITP_IOCTL_RESET,      NULL);

#elif defined(CFG_DBG_OSDCONSOLE) && !defined(CFG_LCD_MULTIPLE)
    // init osd console device
    itpRegisterDevice(ITP_DEVICE_STD,        &itpDeviceOsdConsole);
    itpRegisterDevice(ITP_DEVICE_OSDCONSOLE, &itpDeviceOsdConsole);

    ioctl(ITP_DEVICE_OSDCONSOLE, ITP_IOCTL_INIT,       NULL);
    ioctl(ITP_DEVICE_SCREEN,     ITP_IOCTL_POST_RESET, NULL);
    ioctl(ITP_DEVICE_BACKLIGHT,  ITP_IOCTL_RESET,      NULL);

#endif // defined(CFG_DBG_LCDCONSOLE)

#ifdef CFG_MEMDBG_ENABLE
    // init memory controller interrupt, cannot enable on ice debug mode!!!
    ithIntrRegisterHandlerIrq(ITH_INTR_MC, MemDbgHandler, NULL);
    ithIntrEnableIrq(ITH_INTR_MC);
    //ithIntrEnableFiq(ITH_INTR_MC);
#endif // CFG_MEMDBG_ENABLE

#if defined (CFG_RISC_ENABLE)
    iteRiscInit();
#endif

    // init armlite cpu
#ifdef CFG_ARMLITE_ENABLE
    itpRegisterDevice(ITP_DEVICE_ARMLITE, &itpDeviceArmLite);
#endif

    // init alt cpu
#ifdef CFG_ALT_CPU_ENABLE
    itpRegisterDevice(ITP_DEVICE_ALT_CPU, &itpDeviceAltCpu);
#endif

    //init software Uart
#if defined (CFG_SW_UART)
    #if defined (CFG_DBG_SWUART_CODEC)
    {
        //SWUART DEBUG
        int               altCpuEngineType = ALT_CPU_SW_UART;
        SW_UART_INIT_DATA tUartInitData    = {0};

        itpRegisterDevice(ITP_DEVICE_STD, &itpDeviceAltCpu);

        ioctl(ITP_DEVICE_ALT_CPU, ITP_IOCTL_ALT_CPU_SWITCH_ENG, &altCpuEngineType);
        ioctl(ITP_DEVICE_ALT_CPU, ITP_IOCTL_INIT,               NULL);
        ioctl(ITP_DEVICE_ALT_CPU, ITP_IOCTL_INIT_DBG_UART,      NULL);

        tUartInitData.cpuClock   = ithGetRiscCpuClock();
        tUartInitData.baudrate   = CFG_SWUARTDBGPRINTF_BAUDRATE;
        tUartInitData.uartRxGpio = -1;
        tUartInitData.uartTxGpio = CFG_SWUARTDBGPRINTF_GPIO;
        tUartInitData.parity     = NONE;
        ioctl(ITP_DEVICE_ALT_CPU, ITP_IOCTL_INIT_UART_PARAM, &tUartInitData);
    }

    #else
    {
        //SW UART
        int               altCpuEngineType = ALT_CPU_SW_UART;
        SW_UART_INIT_DATA tUartInitData    = {0};

        ioctl(ITP_DEVICE_ALT_CPU, ITP_IOCTL_ALT_CPU_SWITCH_ENG, &altCpuEngineType);
        ioctl(ITP_DEVICE_ALT_CPU, ITP_IOCTL_INIT,               NULL);

        tUartInitData.cpuClock   = ithGetRiscCpuClock();
        tUartInitData.baudrate   = CFG_SW_UART_BAUDRATE;
        tUartInitData.uartRxGpio = CFG_GPIO_SW_UART_RX;
        tUartInitData.uartTxGpio = CFG_GPIO_SW_UART_TX;
        tUartInitData.parity     = NONE;
        ioctl(ITP_DEVICE_ALT_CPU, ITP_IOCTL_INIT_UART_PARAM, &tUartInitData);
    }
    #endif
#endif

#ifdef CFG_DBG_STATS
    itpStatsInit();
#endif

#ifdef CFG_WATCHDOG_ENABLE
    itpRegisterDevice(ITP_DEVICE_WATCHDOG, &itpDeviceWatchDog);
    ioctl(ITP_DEVICE_WATCHDOG, ITP_IOCTL_INIT,   NULL);
    ioctl(ITP_DEVICE_WATCHDOG, ITP_IOCTL_ENABLE, NULL);
#endif // CFG_WATCHDOG_ENABLE

    // init i2c0 device
#ifdef CFG_I2C0_ENABLE
    IIC_MODE iic_port0_mode = MASTER_MODE;
    itpRegisterDevice(ITP_DEVICE_I2C0, &itpDeviceI2c0);
    ioctl(ITP_DEVICE_I2C0, ITP_IOCTL_INIT, (void *)iic_port0_mode);
#endif

    // init i2c1 device
#ifdef CFG_I2C1_ENABLE
    IIC_MODE iic_port1_mode = MASTER_MODE;
    itpRegisterDevice(ITP_DEVICE_I2C1, &itpDeviceI2c1);
    ioctl(ITP_DEVICE_I2C1, ITP_IOCTL_INIT, (void *)iic_port1_mode);
#endif

    // init i2c2 device
#if defined(CFG_SW_I2C_ENABLE) || defined(CFG_I2C2_ENABLE)
    IIC_MODE iic_port2_mode = MASTER_MODE;
    itpRegisterDevice(ITP_DEVICE_I2C2, &itpDeviceI2c2);
    ioctl(ITP_DEVICE_I2C2, ITP_IOCTL_INIT, (void *)iic_port2_mode);
#endif

    // init i2c3 device
#ifdef CFG_I2C3_ENABLE
    IIC_MODE iic_port3_mode = MASTER_MODE;
    itpRegisterDevice(ITP_DEVICE_I2C3, &itpDeviceI2c3);
    ioctl(ITP_DEVICE_I2C3, ITP_IOCTL_INIT, (void *)iic_port3_mode);
#endif

#ifdef CFG_RTC_ENABLE
    itpRegisterDevice(ITP_DEVICE_RTC, &itpDeviceRtc);
    ioctl(ITP_DEVICE_RTC, ITP_IOCTL_INIT, NULL);
#endif // CFG_RTC_ENABLE

    // init RGBToMIPI device
#ifdef CFG_RGBTOMIPI_ENABLE
    itpRegisterDevice(ITP_DEVICE_RGBTOMIPI, &itpDeviceRGBtoMIPI);
    ioctl(ITP_DEVICE_RGBTOMIPI, ITP_IOCTL_INIT, NULL);
#endif

    // init keypad device
#ifdef CFG_KEYPAD_ENABLE
    itpRegisterDevice(ITP_DEVICE_KEYPAD, &itpDeviceKeypad);
    ioctl(ITP_DEVICE_KEYPAD, ITP_IOCTL_INIT, NULL);
#endif

    // init power device
#if defined(CFG_BATTERY_ENABLE) || defined(CFG_POWER_ON) || defined(CFG_POWER_STANDBY) || defined(CFG_POWER_SLEEP) || defined(CFG_POWER_HIBERNATION) || defined(CFG_POWER_SUSPEND) || defined(CFG_POWER_OFF)
    itpRegisterDevice(ITP_DEVICE_POWER, &itpDevicePower);
    ioctl(ITP_DEVICE_POWER, ITP_IOCTL_INIT, NULL);
#endif

    // init tuner device
#if defined(CFG_TUNER_ENABLE)
    itpRegisterDevice(ITP_DEVICE_TUNER, &itpDeviceTuner);
    ioctl(ITP_DEVICE_TUNER, ITP_IOCTL_INIT, NULL);
#endif

    // init STN LCD device
#if defined(CFG_STNLCD_ENABLE)
    itpRegisterDevice(ITP_DEVICE_STNLCD, &itpDeviceStnLcd);
    ioctl(ITP_DEVICE_STNLCD, ITP_IOCTL_INIT, NULL);
#endif

#if defined(CFG_VIDEO_ENABLE)
    // init Stc [2012/03/07]
    itpRegisterDevice(ITP_DEVICE_STC, &itpDeviceStc);
    ioctl(ITP_DEVICE_STC, ITP_IOCTL_INIT, NULL);
#endif

    // init uart0 device
#if defined(CFG_UART0_ENABLE) && !defined(CFG_DBG_UART0)
    itpRegisterDevice(ITP_DEVICE_UART0, &itpDeviceUart0);
	ioctl(ITP_DEVICE_UART0, ITP_IOCTL_INIT, NULL);
#endif

    // init uart1 device
#if defined(CFG_UART1_ENABLE) && !defined(CFG_DBG_UART1)
    itpRegisterDevice(ITP_DEVICE_UART1, &itpDeviceUart1);
	ioctl(ITP_DEVICE_UART1, ITP_IOCTL_INIT, NULL);
#endif

    //init uart2 device
#if defined(CFG_UART2_ENABLE) && !defined(CFG_DBG_UART2)
    itpRegisterDevice(ITP_DEVICE_UART2, &itpDeviceUart2);
	ioctl(ITP_DEVICE_UART2, ITP_IOCTL_INIT, NULL);
#endif

    //init uart3 device
#if defined(CFG_UART3_ENABLE) && !defined(CFG_DBG_UART3)
    itpRegisterDevice(ITP_DEVICE_UART3, &itpDeviceUart3);
	ioctl(ITP_DEVICE_UART3, ITP_IOCTL_INIT, NULL);
#endif

    //init uart4 device
#if defined(CFG_UART4_ENABLE) && !defined(CFG_DBG_UART4)
    itpRegisterDevice(ITP_DEVICE_UART4, &itpDeviceUart4);
	ioctl(ITP_DEVICE_UART4, ITP_IOCTL_INIT, NULL);
#endif

    //init uart5 device
#if defined(CFG_UART5_ENABLE) && !defined(CFG_DBG_UART5)
    itpRegisterDevice(ITP_DEVICE_UART5, &itpDeviceUart5);
	ioctl(ITP_DEVICE_UART5, ITP_IOCTL_INIT, NULL);
#endif

    // init RS485_0 device
#if defined(CFG_RS485_0_ENABLE)
    itpRegisterDevice(ITP_DEVICE_RS485_0, &itpDeviceRS485_0);
	ioctl(ITP_DEVICE_RS485_0, ITP_IOCTL_INIT, NULL);
#endif

    // init RS485_1 device
#if defined(CFG_RS485_1_ENABLE)
    itpRegisterDevice(ITP_DEVICE_RS485_1, &itpDeviceRS485_1);
	ioctl(ITP_DEVICE_RS485_1, ITP_IOCTL_INIT, NULL);
#endif

    // init RS485_2 device
#if defined(CFG_RS485_2_ENABLE)
    itpRegisterDevice(ITP_DEVICE_RS485_2, &itpDeviceRS485_2);
	ioctl(ITP_DEVICE_RS485_2, ITP_IOCTL_INIT, NULL);
#endif

    // init RS485_3 device
#if defined(CFG_RS485_3_ENABLE)
    itpRegisterDevice(ITP_DEVICE_RS485_3, &itpDeviceRS485_3);
	ioctl(ITP_DEVICE_RS485_3, ITP_IOCTL_INIT, NULL);
#endif

    // init RS485_4 device
#if defined(CFG_RS485_4_ENABLE)
    itpRegisterDevice(ITP_DEVICE_RS485_4, &itpDeviceRS485_4);
	ioctl(ITP_DEVICE_RS485_4, ITP_IOCTL_INIT, NULL);
#endif

    // init RS485_5 device
#if defined(CFG_RS485_5_ENABLE)
    itpRegisterDevice(ITP_DEVICE_RS485_5, &itpDeviceRS485_5);
	ioctl(ITP_DEVICE_RS485_5, ITP_IOCTL_INIT, NULL);
#endif

/*
 #if defined(CFG_WIEGAND0_ENABLE)
    itpRegisterDevice(ITP_DEVICE_WIEGAND1, &itpDeviceWiegand1);
    ioctl(ITP_DEVICE_WIEGAND1, ITP_IOCTL_INIT, NULL);
    wiegand_bit_count = WIEGAND0_BIT_COUNT;
    ioctl(ITP_DEVICE_WIEGAND1, ITP_IOCTL_SET_BIT_COUNT, &wiegand_bit_count);
    wiegand_gpio[0] = CFG_WIEGAND0_GPIO0;
    wiegand_gpio[1] = CFG_WIEGAND0_GPIO1;
    ioctl(ITP_DEVICE_WIEGAND1, ITP_IOCTL_SET_GPIO_PIN, wiegand_gpio); // must set before enable
    ioctl(ITP_DEVICE_WIEGAND1, ITP_IOCTL_ENABLE, NULL);
    while(1);
 #endif
 */

    // init ir0 device
#ifdef CFG_IR0_ENABLE
    itpRegisterDevice(ITP_DEVICE_IR0, &itpDeviceIr0);
    ioctl(ITP_DEVICE_IR0, ITP_IOCTL_INIT, NULL);
#endif

    // init ir1 device
#ifdef CFG_IR1_ENABLE
    itpRegisterDevice(ITP_DEVICE_IR1, &itpDeviceIr1);
    ioctl(ITP_DEVICE_IR1, ITP_IOCTL_INIT, NULL);
#endif

    // init ir2 device
#ifdef CFG_IR2_ENABLE
    itpRegisterDevice(ITP_DEVICE_IR2, &itpDeviceIr2);
    ioctl(ITP_DEVICE_IR2, ITP_IOCTL_INIT, NULL);
#endif

    // init ir3 device
#ifdef CFG_IR3_ENABLE
    itpRegisterDevice(ITP_DEVICE_IR2, &itpDeviceIr3);
    ioctl(ITP_DEVICE_IR2, ITP_IOCTL_INIT, NULL);
#endif

    // init spi device
#ifdef CFG_SPI_ENABLE
    itpRegisterDevice(ITP_DEVICE_SPI, &itpDeviceSpi0);
    ioctl(ITP_DEVICE_SPI, ITP_IOCTL_INIT, NULL);
#endif

#ifdef CFG_SPI1_ENABLE
    itpRegisterDevice(ITP_DEVICE_SPI1, &itpDeviceSpi1);
    ioctl(ITP_DEVICE_SPI1, ITP_IOCTL_INIT, NULL);
#endif

    // init fat
#ifdef CFG_FS_FAT
    itpRegisterDevice(ITP_DEVICE_FAT, &itpFSDeviceFat.dev);
    ioctl(ITP_DEVICE_FAT, ITP_IOCTL_INIT,   NULL);
    ioctl(ITP_DEVICE_FAT, ITP_IOCTL_ENABLE, NULL);
#endif

    // init card device
#if defined(CFG_SD0_ENABLE) || defined(CFG_SD1_ENABLE) || defined(CFG_MSC_ENABLE) || defined(CFG_RAMDISK_ENABLE) || defined(CFG_USBH_CD_MST)
    itpRegisterDevice(ITP_DEVICE_CARD, &itpDeviceCard);
    ioctl(ITP_DEVICE_CARD, ITP_IOCTL_INIT, NULL);
#endif

    // init nand device
#ifdef CFG_NAND_ENABLE 
    itpRegisterDevice(ITP_DEVICE_NAND, &itpDeviceNand);
    ioctl(ITP_DEVICE_NAND, ITP_IOCTL_INIT, NULL);
#endif

    // init nor device
#ifdef CFG_NOR_ENABLE
    itpRegisterDevice(ITP_DEVICE_NOR, &itpDeviceNor);
    ioctl(ITP_DEVICE_NOR, ITP_IOCTL_INIT, NULL);
#endif

    // init sdio device
#ifdef CFG_SDIO_ENABLE
    itpRegisterDevice(ITP_DEVICE_SDIO, &itpDeviceSdio);
    ioctl(ITP_DEVICE_SDIO, ITP_IOCTL_INIT, NULL);
#endif

    // init sd0 device
#ifdef CFG_SD0_STATIC
    itpRegisterDevice(ITP_DEVICE_SD0, &itpDeviceSd0);
    ioctl(ITP_DEVICE_SD0, ITP_IOCTL_INIT, NULL);
#endif

    // init sd1 device
#ifdef CFG_SD1_STATIC
    itpRegisterDevice(ITP_DEVICE_SD1, &itpDeviceSd1);
    ioctl(ITP_DEVICE_SD1, ITP_IOCTL_INIT, NULL);
#endif

#if !defined(CFG_IPTV_RX)
    // init usb
#if defined(CFG_USB0_ENABLE) || defined(CFG_USB1_ENABLE) || defined(CFG_USBHCC)
    itpRegisterDevice(ITP_DEVICE_USB, &itpDeviceUsb);
    ioctl(ITP_DEVICE_USB, ITP_IOCTL_INIT, NULL);
#endif

#if defined(CFG_USB_OPTION)
    itpRegisterDevice(ITP_DEVICE_USBOPTION, &itpDeviceUsbOption);
#endif

#ifdef CFG_USB_HID
    if (ioctl(ITP_DEVICE_USB, ITP_IOCTL_IS_AVAIL, NULL))
    {
        itpRegisterDevice(ITP_DEVICE_USBHID, &itpDeviceUsbHid);
        ioctl(ITP_DEVICE_USBHID, ITP_IOCTL_INIT, NULL);
    }
#else

    // init usb mouse device
    #ifdef CFG_USB_MOUSE
    if (ioctl(ITP_DEVICE_USB, ITP_IOCTL_IS_AVAIL, NULL))
    {
        itpRegisterDevice(ITP_DEVICE_USBMOUSE, &itpDeviceUsbMouse);
        ioctl(ITP_DEVICE_USBMOUSE, ITP_IOCTL_INIT, NULL);
    }
    #endif

    // init usb keyboard device
    #ifdef CFG_USB_KBD
    if (ioctl(ITP_DEVICE_USB, ITP_IOCTL_IS_AVAIL, NULL))
    {
        itpRegisterDevice(ITP_DEVICE_USBKBD, &itpDeviceUsbKbd);
        ioctl(ITP_DEVICE_USBKBD, ITP_IOCTL_INIT, NULL);
    }
    #endif
#endif

#endif
    // init ethernet device
#if defined(CFG_NET_ETHERNET) || defined(CFG_USB_ECM) || defined(CFG_USBD_NCM)
    itpRegisterDevice(ITP_DEVICE_ETHERNET, &itpDeviceEthernet);
    ioctl(ITP_DEVICE_ETHERNET, ITP_IOCTL_INIT, NULL);
#endif

    // enable gpio interrupt
    ithIntrEnableIrq(ITH_INTR_GPIO);

    // init wifi device
#if defined (CFG_NET_WIFI)
    #if defined(CFG_USB0_ENABLE) || defined(CFG_USB1_ENABLE)
    if (ioctl(ITP_DEVICE_USB, ITP_IOCTL_IS_AVAIL, NULL))
    #endif
    {
        itpRegisterDevice(ITP_DEVICE_WIFI, &itpDeviceWifi);
        printf("====>itpInit itpRegisterDevice(ITP_DEVICE_WIFI)\n");
    #if !defined(CFG_NET_WIFI_SDIO_NGPL) && !defined (CFG_NET_WIFI_SDIO_SSV)
        ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_INIT, NULL);
    #endif
    }

    #if defined(CFG_NET_WIFI_SDIO_NGPL)
    {
        itpRegisterDevice(ITP_DEVICE_WIFI_NGPL, &itpDeviceWifiNgpl);
        printf("====>itpInit itpRegisterDevice(ITP_DEVICE_WIFI_NGPL)\n");
        ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_INIT, NULL);
    }
    #elif defined(CFG_NET_WIFI_SDIO_SSV)
    {
        itpRegisterDevice(ITP_DEVICE_WIFI_SSV, &itpDeviceWifiSsv);
        printf("====>itpInit itpRegisterDevice(ITP_DEVICE_WIFI_SSV)\n");
        ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_INIT, NULL);
    }
    #endif
#endif

    // init socket device
#if defined(CFG_NET_ENABLE) && !defined(CFG_NET_WIFI_REDEFINE) && !defined(CFG_USB_ECM) && !defined(CFG_USBD_NCM)
    itpRegisterDevice(ITP_DEVICE_SOCKET, &itpDeviceSocket);
    ioctl(ITP_DEVICE_SOCKET, ITP_IOCTL_INIT, NULL);
#endif

    // init usb device mode as mass storage device
#if defined(CFG_USBD_FILE_STORAGE) || defined(CFG_USBD_MASS_STORAGE)
    itpRegisterDevice(ITP_DEVICE_USBDFSG, &itpDeviceUsbdFsg);
    ioctl(ITP_DEVICE_USBDFSG, ITP_IOCTL_INIT, NULL);
#endif

    // init usb device mode as rs232 device
#if defined(CFG_USBD_ACM)
    itpRegisterDevice(ITP_DEVICE_USBDACM, &itpDeviceUsbdAcm);
    ioctl(ITP_DEVICE_USBDACM, ITP_IOCTL_INIT, NULL);
#endif

#if defined(CFG_USBD_NCM)
    itpRegisterDevice(ITP_DEVICE_USBDNCM, &itpDeviceUsbdNcm);
    ioctl(ITP_DEVICE_USBDNCM, ITP_IOCTL_INIT, NULL);
#endif

#ifdef CFG_USBD_HID
    itpRegisterDevice(ITP_DEVICE_USBDHID, &itpDeviceUsbdHid);
    ioctl(ITP_DEVICE_USBDHID, ITP_IOCTL_INIT, NULL);
#endif

    // init usb device
#if defined(CFG_USB_DEVICE)
    itpRegisterDevice(ITP_DEVICE_USBD, &itpDeviceUsbd);
    ioctl(ITP_DEVICE_USBD, ITP_IOCTL_INIT, NULL);
#endif

    // init g-sensor device
#ifdef CFG_GSENSOR_ENABLE
    itpRegisterDevice(ITP_DEVICE_GSENSOR, &itpDeviceGSensor);
    ioctl(ITP_DEVICE_GSENSOR, ITP_IOCTL_INIT, NULL);
#endif

    // init headset device
#ifdef CFG_HEADSET_ENABLE
    itpRegisterDevice(ITP_DEVICE_HEADSET, &itpDeviceHeadset);
    ioctl(ITP_DEVICE_HEADSET, ITP_IOCTL_INIT, NULL);
#endif

    // init amplifier device
#ifdef CFG_AMPLIFIER_ENABLE
    itpRegisterDevice(ITP_DEVICE_AMPLIFIER, &itpDeviceAmplifier);
    ioctl(ITP_DEVICE_AMPLIFIER, ITP_IOCTL_INIT,   NULL);
    ioctl(ITP_DEVICE_AMPLIFIER, ITP_IOCTL_ENABLE, NULL);
#endif

    // init led device
#ifdef CFG_LED_ENABLE
    itpRegisterDevice(ITP_DEVICE_LED, &itpDeviceLed);
    ioctl(ITP_DEVICE_LED, ITP_IOCTL_INIT, NULL);
#endif

    // init switch device
#ifdef CFG_SWITCH_ENABLE
    itpRegisterDevice(ITP_DEVICE_SWITCH, &itpDeviceSwitch);
    ioctl(ITP_DEVICE_SWITCH, ITP_IOCTL_INIT, NULL);
#endif

    // init decompress
#ifdef CFG_DCPS_ENABLE
    itpRegisterDevice(ITP_DEVICE_DECOMPRESS, &itpDeviceDecompress);
#endif

    // init DPU
#ifdef CFG_DPU_ENABLE
    itpRegisterDevice(ITP_DEVICE_DPU, &itpDeviceDpu);
    ioctl(ITP_DEVICE_DPU, ITP_IOCTL_INIT, NULL);
#endif

    // init codec
#ifdef CFG_AUDIO_ENABLE
    itpRegisterDevice(ITP_DEVICE_CODEC, &itpDeviceCodec);
    //itp_codec_depop();
#endif

    // init drive table
#if defined(CFG_FS_FAT)
    itpRegisterDevice(ITP_DEVICE_DRIVE, &itpDeviceDrive);
    ioctl(ITP_DEVICE_DRIVE, ITP_IOCTL_INIT,      NULL);
    ioctl(ITP_DEVICE_DRIVE, ITP_IOCTL_MOUNT,     NULL);
    ioctl(ITP_DEVICE_DRIVE, ITP_IOCTL_ENABLE,    NULL);
    #ifdef CFG_TASK_DRIVE_PROBE
    ioctl(ITP_DEVICE_DRIVE, ITP_IOCTL_INIT_TASK, NULL);
    #endif
    itpRegisterDevice(ITP_DEVICE_LOGDISK, &itpDeviceLogDisk);
#endif

#ifdef CFG_DBG_NETCONSOLE
    // init network console device
    itpRegisterDevice(ITP_DEVICE_NETCONSOLE, &itpDeviceNetConsole);
    ioctl(ITP_DEVICE_NETCONSOLE, ITP_IOCTL_INIT, NULL);
    itpRegisterDevice(ITP_DEVICE_STD, &itpDeviceNetConsole);
#endif

#ifdef CFG_SPREAD_SPECTRUM_PLL1_ENABLE
    ithSetSpreadSpectrum(ITH_PLL1, CFG_SPREAD_SPECTRUM_PLL1_MODE, CFG_SPREAD_SPECTRUM_PLL1_WIDTH, CFG_SPREAD_SPECTRUM_PLL1_FREQ);
    ithEnableSpreadSpectrum(ITH_PLL1);
#endif

#ifdef CFG_SPREAD_SPECTRUM_PLL2_ENABLE
    ithSetSpreadSpectrum(ITH_PLL2, CFG_SPREAD_SPECTRUM_PLL2_MODE, CFG_SPREAD_SPECTRUM_PLL2_WIDTH, CFG_SPREAD_SPECTRUM_PLL2_FREQ);
    ithEnableSpreadSpectrum(ITH_PLL2);
#endif

#ifdef CFG_SPREAD_SPECTRUM_PLL3_ENABLE
    ithSetSpreadSpectrum(ITH_PLL3, CFG_SPREAD_SPECTRUM_PLL3_MODE, CFG_SPREAD_SPECTRUM_PLL3_WIDTH, CFG_SPREAD_SPECTRUM_PLL3_FREQ);
    ithEnableSpreadSpectrum(ITH_PLL3);
#endif

#if defined(CFG_DBG_TRACE_ANALYZER) && defined(CFG_DBG_VCD)
    itpTaskVcdInit();
#endif

    // set locale
    setlocale(LC_CTYPE, "C-UTF-8");

    // Invoke the C++ static constructors
    __libc_init_array();

    // print debug information
    ithClockStats();
    //ithGpioStats();
    //ithIntrStats();
}

void itpExit(void)
{
#ifdef CFG_FS_FAT
    ioctl(ITP_DEVICE_FAT, ITP_IOCTL_DISABLE, NULL);
    ioctl(ITP_DEVICE_FAT, ITP_IOCTL_EXIT,    NULL);
#endif
}
