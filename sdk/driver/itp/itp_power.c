/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * PAL Power functions.
 *
 * @author Jim Tan
 * @version 1.0
 */
#include <sys/ioctl.h>
#include <assert.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#include "ite/ite_usbex.h"

#ifdef __OPENRTOS__
    #include "openrtos/FreeRTOS.h"
    #include "openrtos/task.h"
#endif // __OPENRTOS__

#include "itp_cfg.h"

#define TP_GPIO_PIN CFG_GPIO_TOUCH_INT

static ITPPowerStatus   powerLastStatus;
static ITPPowerState    powerState;
static unsigned int     powerCpuRatio, powerBusRatio;

#ifdef CFG_POWER_WAKEUP_KEYPAD

static const unsigned int powerKpGpioTable[] = { CFG_GPIO_KEYPAD };
    #define KEYPAD_GPIO_COUNT (ITH_COUNT_OF(powerKpGpioTable))

#endif // CFG_POWER_WAKEUP_KEYPAD

#if defined(CFG_POWER_WAKEUP_UART)
static const uint8_t sendMsg[8] = {0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8};
#define SEND_MSG_LEN    8
#define RCV_MSG_LEN     8
#define UART_NUM        ITH_UART0
#define UART_BAUDRATE   CFG_UART0_BAUDRATE
#define TIMER_NUM       ITH_TIMER7
#define TIMEOUT         1000*10   //10s
#endif

#if defined(CFG_WATCHDOG_ENABLE)
#define TIMEOUT_WD         1000*1            //1s
#endif

#if defined(CFG_POWER_WAKEUP_GPIO)
#define WAKEUP_GPIO     CFG_POWER_WAKEUP_GPIO_PIN
#define WAKEUP_GPIO_HIGH       //Wakeup when GPIO is high
#endif
static int PowerRead(int file, char *ptr, int len, void *info)
{
#ifdef CFG_BATTERY_ENABLE
    ITPBatteryState batState    = itpBatteryGetState();
    int             percent     = itpBatteryGetPercent();
    if (len >= sizeof(ITPPowerStatus) && (powerState != powerLastStatus.powerState || batState != powerLastStatus.batteryState || percent != powerLastStatus.batteryPercent))
    {
        ITPPowerStatus *status = (ITPPowerStatus *)ptr;

        status->powerState      = powerLastStatus.powerState = powerState;
        status->batteryState    = powerLastStatus.batteryState = batState;
        status->batteryPercent  = powerLastStatus.batteryPercent = percent;
        return sizeof(ITPPowerStatus);
    }
#else
    if (len >= sizeof(ITPPowerStatus) && powerState != powerLastStatus.powerState)
    {
        ITPPowerStatus *status = (ITPPowerStatus *)ptr;

        status->powerState = powerLastStatus.powerState = powerState;
        return sizeof(ITPPowerStatus);
    }
#endif // CFG_BATTERY_ENABLE
    return 0;
}

static void PowerOff(void)
{
#ifdef CFG_POWER_OFF
    #ifdef CFG_RTC_ENABLE
    ithRtcCtrlEnable(ITH_RTC_SW_POWEROFF);
    #endif
    ithGpioClear(CFG_GPIO_POWER_ON);
#endif // CFG_POWER_OFF
}

#ifdef CFG_POWER_OFF

static void PowerOffIntrHandler(unsigned int pin, void *arg)
{
    if (ithGpioGet(CFG_GPIO_POWER_OFF_DETECT) == 0)
        powerState = ITP_POWER_OFF;
}

#endif // CFG_POWER_OFF

static void PowerInit(void)
{
    powerLastStatus.powerState      = powerState = ITP_POWER_NORMAL;
    powerLastStatus.batteryState    = ITP_BATTERY_UNKNOWN;
    powerLastStatus.batteryPercent  = 100;

#ifdef CFG_POWER_ON
    ithGpioSetOut(CFG_GPIO_POWER_ON);
    ithGpioSet(CFG_GPIO_POWER_ON);
    ithGpioEnable(CFG_GPIO_POWER_ON);
#endif

#ifdef CFG_BATTERY_ENABLE
    itpBatteryInit();
#endif

#ifdef CFG_POWER_OFF
    ithGpioRegisterIntrHandler(CFG_GPIO_POWER_OFF_DETECT, PowerOffIntrHandler, NULL);
    ithGpioCtrlEnable(CFG_GPIO_POWER_OFF_DETECT, ITH_GPIO_INTR_BOTHEDGE);
    ithGpioEnableIntr(CFG_GPIO_POWER_OFF_DETECT);
    ithGpioSetIn(CFG_GPIO_POWER_OFF_DETECT);
    ithGpioEnable(CFG_GPIO_POWER_OFF_DETECT);
#endif
}

static void PowerResumeForDoubleClick(void)
{
    ithEnterCritical();
	
#ifdef CFG_RISC_ENABLE
	// wakeup Risc
	iteRiscWakeup();
#endif

#ifdef CFG_NET_ETHERNET
    extern int iteMacResume(void);
    ioctl(ITP_DEVICE_ETHERNET, ITP_IOCTL_OFF, (void *)ITP_PHY_WOL);    /* disable PHY wol */
    iteMacResume();
#endif

#ifdef CFG_WATCHDOG_ENABLE
    //reset watch dog counter
    ithWatchDogSetTimeout(1000*CFG_WATCHDOG_TIMEOUT);
    ithWriteRegA(ITH_WD_BASE + ITH_WD_RESTART_REG, ITH_WD_AUTORELOAD);

    ithWatchDogEnable();
#endif

// determined by driver or AP
/*#if defined(CFG_VIDEO_ENABLE) || defined(CFG_JPEG_ENABLE)
    ithIspEnableClock();
#endif*/

#ifdef CFG_CMDQ_ENABLE
    ithCmdQEnableClock();
#endif

// determined by driver or AP
/*#ifdef CFG_JPEG_ENABLE
    ithJpegEnableClock();
#endif

#ifdef CFG_VIDEO_ENABLE
    ithVideoEnableClock();
#endif*/

#ifdef CFG_DCPS_ENABLE
    ithDcpsResume();
#endif

#ifdef CFG_DPU_ENABLE
    ithDpuResume();
#endif

#ifdef CFG_XD_ENABLE
    ithXdEnableClock();
#endif

#ifdef CFG_NAND_ENABLE
    ithNandEnableClock();
#endif

#ifdef CFG_LCD_ENABLE
    ithLcdEnable();
#endif

#ifdef CFG_TVOUT_ENABLE
    ithTveEnableClock();
#endif

#ifdef CFG_SARADC_ENABLE
    ithSARADCEnableClock();
#endif

#ifdef CFG_AMPLIFIER_ENABLE
    ioctl(ITP_DEVICE_AMPLIFIER, ITP_IOCTL_ENABLE, NULL);
#endif

    ithExitCritical();

#ifdef CFG_AUDIO_ENABLE
    itp_codec_wake_up();
#endif

#ifdef CFG_BACKLIGHT_ENABLE
    usleep(50000);     // to avoid white flicker
    ioctl(ITP_DEVICE_BACKLIGHT, ITP_IOCTL_ON, NULL);
#endif

#if defined(CFG_SDIO0_STATIC) || defined(CFG_SDIO1_STATIC)
    ioctl(ITP_DEVICE_SDIO, ITP_IOCTL_INIT, NULL);
#if defined(CFG_NET_AMEBA_SDIO) // just for test
	ioctl(ITP_DEVICE_ETHERNET, ITP_IOCTL_RESUME, NULL);
#endif

#if defined(CFG_NET_WIFI_SDIO_NGPL)
    ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_INIT, NULL);
#endif

#endif


#ifdef CFG_USB1_ENABLE
    ithUsbResume(ITH_USB1);
    ioctl(ITP_DEVICE_USB, ITP_IOCTL_RESUME, (void *)USB1);
#endif

#if defined(CFG_USB0_ENABLE) || defined(CFG_USBHCC)
    ithUsbResume(ITH_USB0);
    ioctl(ITP_DEVICE_USB, ITP_IOCTL_RESUME, (void *)USB0);
#endif

#ifdef CFG_DBG_NETCONSOLE
    ioctl(ITP_DEVICE_NETCONSOLE, ITP_IOCTL_ENABLE, NULL);
#endif

    powerState = ITP_POWER_NORMAL;
    printf("PowerSleep end\n");
}

static void PowerClockResume(void)
{
#ifdef CFG_POWER_STANDBY
    if (powerState != ITP_POWER_STANDBY)
        return;

    #ifdef CFG_AUDIO_ENABLE
    itp_codec_wake_up();
    #endif

    ithEnterCritical();

    ithSetCpuClockRatio(powerCpuRatio);
    ithSetBusClockRatio(powerBusRatio);

    //re-init portTIMER  
    ithTimerSetMatch(portTIMER, configCPU_CLOCK_HZ / configTICK_RATE_HZ);
	ithTimerSetCounter(portTIMER, 0);

    #ifdef CFG_BACKLIGHT_ENABLE
    ioctl(ITP_DEVICE_BACKLIGHT, ITP_IOCTL_STANDBY, NULL);
    #endif

    #ifdef CFG_TVOUT_ENABLE
    ithTveEnableClock();
    #endif

    ithExitCritical();

    #ifdef CFG_BACKLIGHT_ENABLE
    usleep(50000); // to avoid white flicker
    ioctl(ITP_DEVICE_BACKLIGHT, ITP_IOCTL_ON, NULL);
    #endif

    #ifdef CFG_AMPLIFIER_ENABLE
    ioctl(ITP_DEVICE_AMPLIFIER, ITP_IOCTL_ENABLE, NULL);
    #endif

    #if defined(CFG_UART0_ENABLE) || defined(CFG_DBG_UART0)
	ioctl(ITP_DEVICE_UART0, ITP_IOCTL_RESET, NULL);
    #endif

    #if defined(CFG_UART1_ENABLE) || defined(CFG_DBG_UART1)
	ioctl(ITP_DEVICE_UART1, ITP_IOCTL_RESET, NULL);
    #endif

    #if defined(CFG_UART2_ENABLE) || defined(CFG_DBG_UART2)
	ioctl(ITP_DEVICE_UART2, ITP_IOCTL_RESET, NULL);
    #endif

    #if defined(CFG_UART3_ENABLE) || defined(CFG_DBG_UART3)
	ioctl(ITP_DEVICE_UART3, ITP_IOCTL_RESET, NULL);
    #endif

	#if defined(CFG_UART4_ENABLE) || defined(CFG_DBG_UART4)
	ioctl(ITP_DEVICE_UART4, ITP_IOCTL_RESET, NULL);
	#endif

	#if defined(CFG_UART5_ENABLE) || defined(CFG_DBG_UART5)
	ioctl(ITP_DEVICE_UART5, ITP_IOCTL_RESET, NULL);
	#endif

    #if defined(CFG_WIEGAND0_ENABLE)
    ioctl(ITP_DEVICE_WIEGAND0, ITP_IOCTL_RESUME, NULL);
    #endif

    #if defined(CFG_WIEGAND1_ENABLE)
    ioctl(ITP_DEVICE_WIEGAND1, ITP_IOCTL_RESUME, NULL);
    #endif

    #if defined(CFG_KEYPAD_ENABLE) && defined(CFG_SARADC_ENABLE)
    itpKeypadResume();
    #endif

#endif // CFG_POWER_STANDBY

    powerState = ITP_POWER_NORMAL;
}

static void PowerResume(void)
{
#ifdef CFG_POWER_STANDBY
    if (powerState != ITP_POWER_STANDBY)
        return;

    #ifdef CFG_AUDIO_ENABLE
    itp_codec_wake_up();
    #endif

    ithEnterCritical();

    #ifdef CFG_DCPS_ENABLE
    ithDcpsEnableClock();
    #endif

    ithSetCpuClockRatio(powerCpuRatio);
    ithSetBusClockRatio(powerBusRatio);

    //re-init portTIMER  
    ithTimerSetMatch(portTIMER, configCPU_CLOCK_HZ / configTICK_RATE_HZ);
	ithTimerSetCounter(portTIMER, 0);

    #ifdef CFG_LCD_ENABLE
    ithLcdEnable();
    #endif

    #ifdef CFG_TVOUT_ENABLE
    ithTveEnableClock();
    #endif

    ithExitCritical();

    #ifdef CFG_BACKLIGHT_ENABLE
    usleep(50000); // to avoid white flicker
    ioctl(ITP_DEVICE_BACKLIGHT, ITP_IOCTL_ON, NULL);
    #endif

    #ifdef CFG_AMPLIFIER_ENABLE
    ioctl(ITP_DEVICE_AMPLIFIER, ITP_IOCTL_ENABLE, NULL);
    #endif

    #if defined(CFG_UART0_ENABLE) || defined(CFG_DBG_UART0)
	ioctl(ITP_DEVICE_UART0, ITP_IOCTL_RESET, NULL);
    #endif

    #if defined(CFG_UART1_ENABLE) || defined(CFG_DBG_UART1)
	ioctl(ITP_DEVICE_UART1, ITP_IOCTL_RESET, NULL);
    #endif

    #if defined(CFG_UART2_ENABLE) || defined(CFG_DBG_UART2)
	ioctl(ITP_DEVICE_UART2, ITP_IOCTL_RESET, NULL);
    #endif

    #if defined(CFG_UART3_ENABLE) || defined(CFG_DBG_UART3)
	ioctl(ITP_DEVICE_UART3, ITP_IOCTL_RESET, NULL);
    #endif

	#if defined(CFG_UART4_ENABLE) || defined(CFG_DBG_UART4)
	ioctl(ITP_DEVICE_UART4, ITP_IOCTL_RESET, NULL);
	#endif

	#if defined(CFG_UART5_ENABLE) || defined(CFG_DBG_UART5)
	ioctl(ITP_DEVICE_UART5, ITP_IOCTL_RESET, NULL);
	#endif

    #if defined(CFG_WIEGAND0_ENABLE)
    ioctl(ITP_DEVICE_WIEGAND0, ITP_IOCTL_RESUME, NULL);
    #endif

    #if defined(CFG_WIEGAND1_ENABLE)
    ioctl(ITP_DEVICE_WIEGAND1, ITP_IOCTL_RESUME, NULL);
    #endif

    #if defined(CFG_KEYPAD_ENABLE) && defined(CFG_SARADC_ENABLE)
    itpKeypadResume();
    #endif

#endif // CFG_POWER_STANDBY

    powerState = ITP_POWER_NORMAL;
}

static void PowerClockStandby(void)
{
#ifdef CFG_POWER_STANDBY
    if (powerState != ITP_POWER_NORMAL)
        return;

    powerState = ITP_POWER_STANDBY;

    #ifdef CFG_AUDIO_ENABLE
    itp_codec_standby();
    #endif

    #ifdef CFG_AMPLIFIER_ENABLE
    ioctl(ITP_DEVICE_AMPLIFIER, ITP_IOCTL_DISABLE, NULL);
    #endif

    #ifdef CFG_CMDQ_ENABLE
    ithCmdQWaitEmpty(   ITH_CMDQ0_OFFSET);
    //ithCmdQWaitEmpty(   ITH_CMDQ1_OFFSET);
    #endif

    #ifdef CFG_BACKLIGHT_ENABLE
    ioctl(ITP_DEVICE_BACKLIGHT, ITP_IOCTL_OFF, NULL);
    #endif

    ithEnterCritical();

    #ifdef CFG_TVOUT_ENABLE
    ithTveDisableClock();
    #endif

    #ifdef CFG_WATCHDOG_ENABLE
    ithWatchDogRestart();
    #endif

    powerCpuRatio   = ithGetCpuClockRatio();
    powerBusRatio   = ithGetBusClockRatio();
    LOG_DBG "orgional cpu/bus clock ratio=%d/%d, change to %d/%d\n",
    powerCpuRatio, powerBusRatio, CFG_POWER_STANDBY_CPU_FREQ_RATIO, CFG_POWER_STANDBY_BUS_FREQ_RATIO LOG_END

    ithSetCpuClockRatioMultiple(powerCpuRatio, CFG_POWER_STANDBY_CPU_FREQ_RATIO);
	ithSetBusClockRatioMultiple(powerBusRatio, CFG_POWER_STANDBY_BUS_FREQ_RATIO);

    //re-init portTIMER    
    ithTimerSetMatch(portTIMER, configCPU_CLOCK_HZ / configTICK_RATE_HZ);
	ithTimerSetCounter(portTIMER, 0);

    #ifdef CFG_BACKLIGHT_ENABLE
    ioctl(ITP_DEVICE_BACKLIGHT, ITP_IOCTL_STANDBY, NULL);
    #endif

    ithExitCritical();

    #ifdef CFG_WATCHDOG_ENABLE
    ithWatchDogRestart();
    #endif

    #if defined(CFG_UART0_ENABLE) || defined(CFG_DBG_UART0)
	ioctl(ITP_DEVICE_UART0, ITP_IOCTL_RESET, NULL);
    #endif

    #if defined(CFG_UART1_ENABLE) || defined(CFG_DBG_UART1)
	ioctl(ITP_DEVICE_UART1, ITP_IOCTL_RESET, NULL);
    #endif

    #if defined(CFG_UART2_ENABLE) || defined(CFG_DBG_UART2)
	ioctl(ITP_DEVICE_UART2, ITP_IOCTL_RESET, NULL);
    #endif

    #if defined(CFG_UART3_ENABLE) || defined(CFG_DBG_UART3)
	ioctl(ITP_DEVICE_UART3, ITP_IOCTL_RESET, NULL);
    #endif

	#if defined(CFG_UART4_ENABLE) || defined(CFG_DBG_UART4)
	ioctl(ITP_DEVICE_UART4, ITP_IOCTL_RESET, NULL);
	#endif

	#if defined(CFG_UART5_ENABLE) || defined(CFG_DBG_UART5)
	ioctl(ITP_DEVICE_UART5, ITP_IOCTL_RESET, NULL);
	#endif

    #if defined(CFG_WIEGAND0_ENABLE)
    ioctl(ITP_DEVICE_WIEGAND0, ITP_IOCTL_SUSPEND, NULL);
    #endif

    #if defined(CFG_WIEGAND1_ENABLE)
    ioctl(ITP_DEVICE_WIEGAND1, ITP_IOCTL_SUSPEND, NULL);
    #endif

    #if defined(CFG_KEYPAD_ENABLE) && defined(CFG_SARADC_ENABLE)
    itpKeypadStandby();
    #endif

#endif // CFG_POWER_STANDBY
}

static void PowerStandby(void)
{
#ifdef CFG_POWER_STANDBY
    if (powerState != ITP_POWER_NORMAL)
        return;

    powerState = ITP_POWER_STANDBY;

    #ifdef CFG_AUDIO_ENABLE
    itp_codec_standby();
    #endif

    #ifdef CFG_AMPLIFIER_ENABLE
    ioctl(ITP_DEVICE_AMPLIFIER, ITP_IOCTL_DISABLE, NULL);
    #endif

    #ifdef CFG_CMDQ_ENABLE
    ithCmdQWaitEmpty(   ITH_CMDQ0_OFFSET);
    //ithCmdQWaitEmpty(   ITH_CMDQ1_OFFSET);
    #endif

    #ifdef CFG_BACKLIGHT_ENABLE
    ioctl(ITP_DEVICE_BACKLIGHT, ITP_IOCTL_OFF, NULL);
    #endif

    ithEnterCritical();

    #ifdef CFG_TVOUT_ENABLE
    ithTveDisableClock();
    #endif

    #ifdef CFG_LCD_ENABLE
    ithLcdDisable();
    #endif

    #ifdef CFG_WATCHDOG_ENABLE
    ithWatchDogRestart();
    #endif

    powerCpuRatio   = ithGetCpuClockRatio();
    powerBusRatio   = ithGetBusClockRatio();
    LOG_DBG "orgional cpu/bus clock ratio=%d/%d, change to %d/%d\n",
    powerCpuRatio, powerBusRatio, CFG_POWER_STANDBY_CPU_FREQ_RATIO, CFG_POWER_STANDBY_BUS_FREQ_RATIO LOG_END

    ithSetCpuClockRatioMultiple(powerCpuRatio, CFG_POWER_STANDBY_CPU_FREQ_RATIO);
	ithSetBusClockRatioMultiple(powerBusRatio, CFG_POWER_STANDBY_BUS_FREQ_RATIO);

    #ifdef CFG_DCPS_ENABLE
    ithDcpsDisableClock();
    #endif

    //re-init portTIMER    
    ithTimerSetMatch(portTIMER, configCPU_CLOCK_HZ / configTICK_RATE_HZ);
	ithTimerSetCounter(portTIMER, 0);

    ithExitCritical();

    #ifdef CFG_WATCHDOG_ENABLE
    ithWatchDogRestart();
    #endif

    #if defined(CFG_UART0_ENABLE) || defined(CFG_DBG_UART0)
	ioctl(ITP_DEVICE_UART0, ITP_IOCTL_RESET, NULL);
    #endif

    #if defined(CFG_UART1_ENABLE) || defined(CFG_DBG_UART1)
	ioctl(ITP_DEVICE_UART1, ITP_IOCTL_RESET, NULL);
    #endif

    #if defined(CFG_UART2_ENABLE) || defined(CFG_DBG_UART2)
	ioctl(ITP_DEVICE_UART2, ITP_IOCTL_RESET, NULL);
    #endif

    #if defined(CFG_UART3_ENABLE) || defined(CFG_DBG_UART3)
	ioctl(ITP_DEVICE_UART3, ITP_IOCTL_RESET, NULL);
    #endif

	#if defined(CFG_UART4_ENABLE) || defined(CFG_DBG_UART4)
	ioctl(ITP_DEVICE_UART4, ITP_IOCTL_RESET, NULL);
	#endif

	#if defined(CFG_UART5_ENABLE) || defined(CFG_DBG_UART5)
	ioctl(ITP_DEVICE_UART5, ITP_IOCTL_RESET, NULL);
	#endif

    #if defined(CFG_WIEGAND0_ENABLE)
    ioctl(ITP_DEVICE_WIEGAND0, ITP_IOCTL_SUSPEND, NULL);
    #endif

    #if defined(CFG_WIEGAND1_ENABLE)
    ioctl(ITP_DEVICE_WIEGAND1, ITP_IOCTL_SUSPEND, NULL);
    #endif

    #if defined(CFG_KEYPAD_ENABLE) && defined(CFG_SARADC_ENABLE)
    itpKeypadStandby();
    #endif

#endif // CFG_POWER_STANDBY
}

#ifdef CFG_POWER_SLEEP
    #ifdef _WIN32

unsigned int __power_sleep_func_start, __power_sleep_func_end;

    #else // _WIN32

extern unsigned int __power_sleep_func_start, __power_sleep_func_end;

static void __attribute__ ((section(".power_sleep_func"), no_instrument_function, optimize("O0")))
SleepFunc(register uint32_t *inParam, register uint32_t *outParam, register uint32_t *var)
{
#if defined(CFG_POWER_WAKEUP_UART)
        // UART_INPUT length: SEND_MSG_LEN + 3
        #define UART_INPUT      ((inParam) + 0)
#elif defined(CFG_WATCHDOG_ENABLE)
        // WD_INPUT length: 1
        #define WD_INPUT        ((inParam) + 0)
#else
        #define KEYPAD_TABLE    ((inParam) + 0)
#endif
        #define NCLK_SRC        (*((var) + 0))
        #define WCLK_SRC        (*((var) + 1))
        #define FCLK_SRC        (*((var) + 2))
        #define RET             (*((var) + 3))
        #define PIN             (*((var) + 4))
        #undef VAL
        #define VAL             (*((var) + 5))
        #define J               (*((var) + 6))
        #define K               (*((var) + 7))
        #define NCLK_RATIO      (*((var) + 8))
        #define WCLK_RATIO      (*((var) + 9))
        #define FCLK_RATIO      (*((var) + 10))
        #define GPIO_MASK       (*((var) + 11))
        #define TIME_VAL        (*((var) + 12))        


    // enble auto self refresh mode
    ITH_SET_REG_BIT_A(ITH_MEM_PD_CTL_REG, ITH_AUTO_SELF_REFRESH_BIT);

	// enter self refresh mode
	ITH_SET_REG_BIT_A(ITH_MEM_STATE_CTL_REG, ITH_EN_SELF_REFRESH_BIT);	
    do {
        VAL = ITH_READ_REG_A(ITH_MEM_STATE_CTL_REG);
    }while(!(VAL & (0x1 << ITH_CHK_SELF_REFRESH_BIT)));


    // set clock sources to 12M
    NCLK_SRC    = (ITH_READ_REG_A(ITH_NCLK) & ITH_NCLK_SRC_SEL_MASK) >> ITH_NCLK_SRC_SEL_BIT;
    WCLK_SRC    = (ITH_READ_REG_A(ITH_WCLK) & ITH_WCLK_SRC_SEL_MASK) >> ITH_WCLK_SRC_SEL_BIT;
    FCLK_SRC    = (ITH_READ_REG_A(ITH_FCLK) & ITH_AXICLK_SRC_SEL_MASK) >> ITH_AXICLK_SRC_SEL_BIT;

    ITH_WRITE_REG_MASK_A(ITH_NCLK,
                         (0x1 << ITH_NCLK_UPD_BIT) | (ITH_CLK_CKSYS << ITH_NCLK_SRC_SEL_BIT),
                         ITH_NCLK_UPD_MASK | ITH_NCLK_SRC_SEL_MASK);

    ITH_WRITE_REG_MASK_A(ITH_WCLK,
                         (0x1 << ITH_WCLK_UPD_BIT) | (ITH_CLK_CKSYS << ITH_WCLK_SRC_SEL_BIT),
                         ITH_WCLK_UPD_MASK | ITH_WCLK_SRC_SEL_MASK);
	
    ITH_WRITE_REG_MASK_A(ITH_FCLK,
                         (0x1 << ITH_AXICLK_UPD_BIT) | (ITH_CLK_CKSYS << ITH_AXICLK_SRC_SEL_BIT),
                         ITH_AXICLK_UPD_MASK | ITH_AXICLK_SRC_SEL_MASK);


    //power down pll except pll1 output1
    ITH_WRITE_REG_MASK_A(ITH_PLL2_1_SET2_REG,
                         (0x0 << ITH_PLL_SET2_EN_BIT) | (0x1 << ITH_PLL_SET2_UPD_BIT),
                         ITH_PLL_SET2_EN_MSK | ITH_PLL_SET2_UPD_MSK);
	
	ITH_WRITE_REG_MASK_A(ITH_PLL2_2_SET2_REG,
                         (0x0 << ITH_PLL_SET2_EN_BIT) | (0x1 << ITH_PLL_SET2_UPD_BIT),
                         ITH_PLL_SET2_EN_MSK | ITH_PLL_SET2_UPD_MSK);

	ITH_WRITE_REG_MASK_A(ITH_PLL3_1_SET2_REG,
                         (0x0 << ITH_PLL_SET2_EN_BIT) | (0x1 << ITH_PLL_SET2_UPD_BIT),
                         ITH_PLL_SET2_EN_MSK | ITH_PLL_SET2_UPD_MSK);

	
	// disable MCLK, before this point all MEM access should be stop.
	ITH_CLEAR_REG_BIT_A(ITH_MCLK2, ITH_EN_DIV_MCLK2_BIT);

	// disable MCLK PHY. NOTICE: MUST after entering self-refresh mode
	ITH_CLEAR_REG_BIT_A(ITH_MCLK, ITH_EN_DIV_MCLK_BIT);

    ITH_WRITE_REG_A(0xD8100340, 0);   
    //for (VAL = 0; VAL < 300; VAL++)
    //    __asm__ (""); // avoid to be optimized

	
	// power down pll1 output1
	ITH_WRITE_REG_MASK_A(ITH_PLL1_1_SET2_REG,
                         (0x0 << ITH_PLL_SET2_EN_BIT) | (0x1 << ITH_PLL_SET2_UPD_BIT),
                         ITH_PLL_SET2_EN_MSK | ITH_PLL_SET2_UPD_MSK);


#if !defined(CFG_POWER_WAKEUP_UART) && !defined(CFG_WATCHDOG_ENABLE)
    // set cpu clock ratio to minimize
    NCLK_RATIO  = (ITH_READ_REG_A(ITH_NCLK) & ITH_NCLK_RATIO_MASK) >> ITH_NCLK_RATIO_BIT;
    WCLK_RATIO  = (ITH_READ_REG_A(ITH_WCLK) & ITH_WCLK_RATIO_MASK) >> ITH_WCLK_RATIO_BIT;
    FCLK_RATIO  = (ITH_READ_REG_A(ITH_FCLK) & ITH_AXICLK_RATIO_MASK) >> ITH_AXICLK_RATIO_BIT;

    ITH_WRITE_REG_MASK_A(ITH_NCLK,
                         (0x1 << ITH_NCLK_UPD_BIT) | (1023 << ITH_NCLK_RATIO_BIT),
                         ITH_NCLK_UPD_MASK | ITH_NCLK_RATIO_MASK);

    ITH_WRITE_REG_MASK_A(ITH_WCLK,
                         (0x1 << ITH_WCLK_UPD_BIT) | (1023 << ITH_WCLK_RATIO_BIT),
                         ITH_WCLK_UPD_MASK | ITH_WCLK_RATIO_MASK);

    ITH_WRITE_REG_MASK_A(ITH_FCLK,
                         (0x1 << ITH_AXICLK_UPD_BIT) | (1023 << ITH_AXICLK_RATIO_BIT),
                         ITH_AXICLK_UPD_MASK | ITH_AXICLK_RATIO_MASK);
#endif

        #if defined(CFG_POWER_WAKEUP_DELAY)

    for (VAL = 0; VAL < CFG_POWER_WAKEUP_DELAYLOOP; VAL++)
        __asm__ (""); // avoid to be optimized

        #elif defined(CFG_POWER_WAKEUP_TOUCH_ONE_CLICK) || defined(CFG_POWER_WAKEUP_TOUCH_DOUBLE_CLICK)

#if defined(CFG_WATCHDOG_ENABLE)
    //Reset Watch Dog counter
    ITH_WRITE_REG_A(ITH_WD_BASE + ITH_WD_LOAD_REG, WD_INPUT[0]);

    //Watch Dog restart
    ITH_WRITE_REG_A(ITH_WD_BASE + ITH_WD_RESTART_REG, ITH_WD_AUTORELOAD);

    //Watch Dog enable
    ITH_SET_REG_BIT_A(ITH_WD_BASE + ITH_WD_CR_REG, ITH_WD_EN);
#endif

    if (TP_GPIO_PIN <= 31)
        GPIO_MASK = (1 << TP_GPIO_PIN);
    else if (TP_GPIO_PIN >= 32 && TP_GPIO_PIN <= 63)
        GPIO_MASK = (1 << (TP_GPIO_PIN - 32));
    else if (TP_GPIO_PIN >= 64 && TP_GPIO_PIN <= 95)
        GPIO_MASK = (1 << (TP_GPIO_PIN - 64));
    else if (TP_GPIO_PIN >= 96 && TP_GPIO_PIN <= 127)
        GPIO_MASK = (1 << (TP_GPIO_PIN - 96));

    for (;;)
    {
        ITH_GPIO_GET(RET, TP_GPIO_PIN);
            #if defined(CFG_GPIO_TOUCH_INT_ACTIVE_HIGH)
        if (RET & GPIO_MASK)
            #else
        if (!(RET & GPIO_MASK))
            #endif
            break;
#if defined(CFG_WATCHDOG_ENABLE)
        //Watch Dog restart
        ITH_WRITE_REG_A(ITH_WD_BASE + ITH_WD_RESTART_REG, ITH_WD_AUTORELOAD);
#endif
    }
#if defined(CFG_WATCHDOG_ENABLE)
    //Disable Watch Dog
    ITH_CLEAR_REG_BIT_A(ITH_WD_BASE + ITH_WD_CR_REG, ITH_WD_EN);
#endif

        #elif defined(CFG_POWER_WAKEUP_KEYPAD)

    // detect keypad input to exit suspend mode
    {
        register i;
        for (i = 0; i < KEYPAD_GPIO_COUNT; i++)
            ITH_GPIO_SET_IN(KEYPAD_TABLE[i]);
    }

    for (;;)
    {
        // probe first
        RET = -1;
        {
            register uint32_t i;
            for (i = 0; i < KEYPAD_GPIO_COUNT; i++)
            {
                PIN = KEYPAD_TABLE[i];
                VAL = ITH_GPIO_GET(PIN);

                if (PIN >= 32)
                    PIN -= 32;

                if ((VAL & (0x1 << PIN)) == 0)
                {
                    RET = i;
                    break;
                }
            }
        }
        if (RET == CFG_POWER_WAKEUP_KEYNUM)
            break;
        else if (RET != -1)
            continue;

        // probe next
        {
            register uint32_t i;
            for (i = 0; i < KEYPAD_GPIO_COUNT; i++)
            {
                PIN = KEYPAD_TABLE[i];

                ITH_GPIO_SET_OUT(PIN);
                ITH_GPIO_CLEAR(PIN);

                K = 0;
                for (J = 0; J < KEYPAD_GPIO_COUNT; J++)
                {
                    if (J == i)
                        continue;

                    PIN = KEYPAD_TABLE[J];
                    VAL = ITH_GPIO_GET(PIN);

                    if (PIN >= 32)
                        PIN -= 32;

                    if ((VAL & (0x1 << PIN)) == 0)
                    {
                        RET = KEYPAD_GPIO_COUNT + (KEYPAD_GPIO_COUNT - 1) * i + K;
                        break;
                    }
                    K++;
                }

                // reset to input PIN
                ITH_GPIO_SET_IN(KEYPAD_TABLE[i]);

                if (RET != -1)
                    break;
            }
        }

        if (RET == CFG_POWER_WAKEUP_KEYNUM)
            break;
    }
        #elif defined(CFG_POWER_WAKEUP_GPIO)

#if defined(CFG_WATCHDOG_ENABLE)
    //Reset Watch Dog counter
    ITH_WRITE_REG_A(ITH_WD_BASE + ITH_WD_LOAD_REG, WD_INPUT[0]);

    //Watch Dog restart
    ITH_WRITE_REG_A(ITH_WD_BASE + ITH_WD_RESTART_REG, ITH_WD_AUTORELOAD);

    //Watch Dog enable
    ITH_SET_REG_BIT_A(ITH_WD_BASE + ITH_WD_CR_REG, ITH_WD_EN);
#endif

    if (WAKEUP_GPIO <= 31)
        GPIO_MASK = (1 << WAKEUP_GPIO);
    else if (WAKEUP_GPIO >= 32 && WAKEUP_GPIO <= 63)
        GPIO_MASK = (1 << (WAKEUP_GPIO - 32));
    else if (WAKEUP_GPIO >= 64 && WAKEUP_GPIO <= 95)
        GPIO_MASK = (1 << (WAKEUP_GPIO - 64));
    else if (WAKEUP_GPIO >= 96 && WAKEUP_GPIO <= 127)
        GPIO_MASK = (1 << (WAKEUP_GPIO - 96));

    for (;;)
    {
        ITH_GPIO_GET(RET, WAKEUP_GPIO);
            #if defined(WAKEUP_GPIO_HIGH)
        if (RET & GPIO_MASK)
            #else
        if (!(RET & GPIO_MASK))
            #endif
            break;
        
#if defined(CFG_WATCHDOG_ENABLE)
        //Watch Dog restart
        ITH_WRITE_REG_A(ITH_WD_BASE + ITH_WD_RESTART_REG, ITH_WD_AUTORELOAD);
#endif
    }
#if defined(CFG_WATCHDOG_ENABLE)
    //Disable Watch Dog
    ITH_CLEAR_REG_BIT_A(ITH_WD_BASE + ITH_WD_CR_REG, ITH_WD_EN);
#endif
        
        #elif defined(CFG_POWER_WAKEUP_IR)
    do
    {
        VAL = ITH_READ_REG_A(ITH_IR_BASE + ITH_IR_RX_STATUS_REG);
    } while ((VAL & (0x1 << ITH_IR_DATAREADY_BIT)) == 0);

        #elif defined(CFG_POWER_WAKEUP_UART)

    //Uart Baudrate reset
    ITH_WRITE_REG_A(UART_NUM + ITH_UART_LCR_REG, ITH_UART_LCR_DLAB);
    ITH_WRITE_REG_A(UART_NUM + ITH_UART_DLM_REG, (UART_INPUT[SEND_MSG_LEN] & 0xF00) >> 8);
    ITH_WRITE_REG_A(UART_NUM + ITH_UART_DLL_REG, UART_INPUT[SEND_MSG_LEN] & 0xFF);
    ITH_WRITE_REG_A(UART_NUM + ITH_UART_DLH_REG, UART_INPUT[SEND_MSG_LEN+1] & 0xF);
    ITH_WRITE_REG_A(UART_NUM + ITH_UART_LCR_REG, 0x3);

    //timer enable
    ITH_SET_REG_BIT_A(ITH_TIMER_BASE + ITH_TIMER1_CR_REG + TIMER_NUM * 0x4, ITH_TIMER_EN_BIT);

    for(;;) {
        TIME_VAL = ITH_READ_REG_A(ITH_TIMER_BASE + TIMER_NUM * 0x10 + ITH_TIMER1_CNT_REG);

        if(TIME_VAL >= UART_INPUT[SEND_MSG_LEN+2]) { //1 s = (12000/3)*1000*1 ; 1 ms = (12000/3)*1
            //Reset Timer Counter
            ITH_WRITE_REG_A(ITH_TIMER_BASE + TIMER_NUM * 0x10 + ITH_TIMER1_CNT_REG, 0x0);

            //Uart Tx
            {
                for(J=0; J<SEND_MSG_LEN; J++) {
                    while (ITH_UART_IS_TX_FULL(UART_NUM));
                    ITH_WRITE_REG_A(UART_NUM + ITH_UART_THR_REG, UART_INPUT[J]);
                }
            }
        }

        //Uart Rx
        if(K < RCV_MSG_LEN) {
            while(K < RCV_MSG_LEN) {
                if(ITH_UART_IS_RX_READY(UART_NUM)) {
                    VAL = ITH_READ_REG_A(UART_NUM + ITH_UART_RBR_REG);
                    if(K == 0)
                        RET = VAL;
                    K++;
                }
                else {
                    break;
                }
            }

            if(K >= RCV_MSG_LEN) {
                if(RET == 0x1 ) {
                    break; //wakeup, when Uart receive 0x1 at index 0 of Msg.
                }
                K = 0;
            }
        }
    }

        #elif defined(CFG_POWER_WAKEUP_WOL)

    do
    {
        VAL = ithGpioGet(CFG_GPIO_ETHERNET_WAKE_ON_LAN);
    } while (VAL);

        #endif      // defined(CFG_POWER_WAKEUP_DELAY)


#if !defined(CFG_POWER_WAKEUP_UART) && !defined(CFG_WATCHDOG_ENABLE)
    // restore cpu clock ratio
    ITH_WRITE_REG_MASK_A(ITH_FCLK,
                         (0x1 << ITH_AXICLK_UPD_BIT) | (FCLK_RATIO << ITH_AXICLK_RATIO_BIT),
                         ITH_AXICLK_UPD_MASK | ITH_AXICLK_RATIO_MASK);
	
    ITH_WRITE_REG_MASK_A(ITH_WCLK,
                         (0x1 << ITH_WCLK_UPD_BIT) | (WCLK_RATIO << ITH_WCLK_RATIO_BIT),
                         ITH_WCLK_UPD_MASK | ITH_WCLK_RATIO_MASK);

    ITH_WRITE_REG_MASK_A(ITH_NCLK,
                         (0x1 << ITH_NCLK_UPD_BIT) | (NCLK_RATIO << ITH_NCLK_RATIO_BIT),
                         ITH_NCLK_UPD_MASK | ITH_NCLK_RATIO_MASK);
#endif


    ITH_WRITE_REG_MASK_A(ITH_PLL1_1_SET2_REG,
                         (0x1 << ITH_PLL_SET2_EN_BIT) | (0x1 << ITH_PLL_SET2_UPD_BIT),
                         ITH_PLL_SET2_EN_MSK | ITH_PLL_SET2_UPD_MSK);

   
    // enable MEM PHY
    ITH_SET_REG_BIT_A(ITH_MCLK, ITH_EN_DIV_MCLK_BIT);

    //ITH_WRITE_REG_A(0xD8100350, 0x00c50440);
    //ITH_WRITE_REG_A(0xD8100354, 0x00000003);
    //ITH_WRITE_REG_A(0xD8100358, 0x00008008);    
    for (VAL = 0; VAL < 1000; VAL++)
        __asm__ (""); // avoid to be optimized    
    ITH_WRITE_REG_A(0xD8100340, 0x00000001);
    for (VAL = 0; VAL < 1000; VAL++)
        __asm__ (""); // avoid to be optimized
    ITH_WRITE_REG_A(0xD8100340, 0x00000003);
    for (VAL = 0; VAL < 1000; VAL++)
        __asm__ (""); // avoid to be optimized
    ITH_WRITE_REG_A(0xD8100340, 0x00000007);
    for (VAL = 0; VAL < 1000; VAL++)
        __asm__ (""); // avoid to be optimized
    ITH_WRITE_REG_A(0xD8100340, 0x0000000f);

    // enable MCLK
    ITH_SET_REG_BIT_A(ITH_MCLK2, ITH_EN_DIV_MCLK2_BIT);


    // power up plls
    ITH_WRITE_REG_MASK_A(ITH_PLL3_1_SET2_REG,
                         (0x1 << ITH_PLL_SET2_EN_BIT) | (0x1 << ITH_PLL_SET2_UPD_BIT),
                         ITH_PLL_SET2_EN_MSK | ITH_PLL_SET2_UPD_MSK);

    ITH_WRITE_REG_MASK_A(ITH_PLL2_2_SET2_REG,
                         (0x1 << ITH_PLL_SET2_EN_BIT) | (0x1 << ITH_PLL_SET2_UPD_BIT),
                         ITH_PLL_SET2_EN_MSK | ITH_PLL_SET2_UPD_MSK);

    ITH_WRITE_REG_MASK_A(ITH_PLL2_1_SET2_REG,
                         (0x1 << ITH_PLL_SET2_EN_BIT) | (0x1 << ITH_PLL_SET2_UPD_BIT),
                         ITH_PLL_SET2_EN_MSK | ITH_PLL_SET2_UPD_MSK);


    // restore clock sources
    ITH_WRITE_REG_MASK_A(ITH_FCLK,
                         (0x1 << ITH_AXICLK_UPD_BIT) | (FCLK_SRC << ITH_AXICLK_SRC_SEL_BIT),
                         ITH_AXICLK_UPD_MASK | ITH_AXICLK_SRC_SEL_MASK);

    ITH_WRITE_REG_MASK_A(ITH_WCLK,
                         (0x1 << ITH_WCLK_UPD_BIT) | (WCLK_SRC << ITH_WCLK_SRC_SEL_BIT),
                         ITH_WCLK_UPD_MASK | ITH_WCLK_SRC_SEL_MASK);

    ITH_WRITE_REG_MASK_A(ITH_NCLK,
                         (0x1 << ITH_NCLK_UPD_BIT) | (NCLK_SRC << ITH_NCLK_SRC_SEL_BIT),
                         ITH_NCLK_UPD_MASK | ITH_NCLK_SRC_SEL_MASK);


    // disable auto self refresh mode
    ITH_CLEAR_REG_BIT_A(ITH_MEM_PD_CTL_REG, ITH_AUTO_SELF_REFRESH_BIT);

    // exit self refresh mode
    VAL = ITH_READ_REG_A(ITH_MEM_STATE_CTL_REG);
    if(VAL & (0x1 << ITH_CHK_SELF_REFRESH_BIT)) {
        ITH_SET_REG_BIT_A(ITH_MEM_STATE_CTL_REG, ITH_EXIT_SELF_REFRESH_BIT);
    }
}

    #endif  // _WIN32
#endif      // CFG_POWER_SLEEP

static void PowerSleepForDoubleClick(void)
{
    printf("PowerSleepContinue\n");

#ifdef CFG_POWER_SLEEP
    const unsigned int  sleepFuncSize = (unsigned int)&__power_sleep_func_end - (unsigned int)&__power_sleep_func_start;
    uint32_t            *inParams, *outParams, *var, *ptr;
    int                 i;
	int ratio = (ithReadRegA(ITH_APB_CLK1_REG) & ITH_WCLK_RATIO_MASK) >> ITH_WCLK_RATIO_BIT;
#if defined(CFG_WATCHDOG_ENABLE)    
    unsigned int WD_counter = 0;
#endif

    powerState = ITP_POWER_SLEEP;
    ithEnterCritical();

    ithClockSleep();

    // copy SleepFunc() to audio's SRAM
    #ifndef _WIN32
    memcpy((void *)ITH_SRAM_AUDIO_BASE, SleepFunc, sleepFuncSize);

    // setting parameters
    ptr         = (uint32_t *)(ITH_SRAM_AUDIO_BASE + sleepFuncSize);

    // input parameters
    inParams    = ptr;

        #ifdef CFG_POWER_WAKEUP_KEYPAD
    // keypad's gpio
    for (i = 0; i < KEYPAD_GPIO_COUNT; i++)
    {
        *ptr++ = powerKpGpioTable[i];
    }
        #endif // CFG_POWER_WAKEUP_KEYPAD

#if defined(CFG_WATCHDOG_ENABLE)
    //12M reset
    WD_counter = (12000/(ratio+1)) * TIMEOUT_WD;

    *ptr++ = WD_counter;
#endif

    // output parameters
    outParams   = ptr;

    // local variables
    var         = ptr;

    ithFlushDCache();
    ithInvalidateICache();

    // call SleepFunc() in SRAM
    ((void (*)(uint32_t *, uint32_t *, uint32_t *))ITH_SRAM_AUDIO_BASE)(inParams, outParams, var);
    #endif // !_WIN32

    // wakeup procedure
    ithClockWakeup();

    ithExitCritical();
#endif // CFG_POWER_SLEEP
}

static void PowerSleep(void)
{
#ifdef CFG_POWER_SLEEP
    const unsigned int  sleepFuncSize = (unsigned int)&__power_sleep_func_end - (unsigned int)&__power_sleep_func_start;
    uint32_t            *inParams, *outParams, *var, *ptr;
    int                 i;
    int ratio = (ithReadRegA(ITH_APB_CLK1_REG) & ITH_WCLK_RATIO_MASK) >> ITH_WCLK_RATIO_BIT;
#if defined(CFG_WATCHDOG_ENABLE)
    unsigned int WD_counter = 0;
#endif
#if defined(CFG_POWER_WAKEUP_UART)
    unsigned int totalDiv, intDiv, fDiv;
    unsigned int timeout;
#endif

    // must small than SRAM size and 32-bit align
    LOG_DBG "power sleep function size: %d\n", sleepFuncSize LOG_END
    assert(sleepFuncSize < ITH_SRAM_AUDIO_SIZE);
    assert(ITH_IS_ALIGNED(sleepFuncSize, 4));

    powerState = ITP_POWER_SLEEP;
    printf("PowerSleep\n");

    #ifdef CFG_DBG_NETCONSOLE
    ioctl(ITP_DEVICE_NETCONSOLE, ITP_IOCTL_DISABLE, NULL);
    #endif

    #ifdef CFG_AUDIO_ENABLE
    itp_codec_standby();
    #endif

    #if defined(CFG_USB_OPTION)
	ioctl(ITP_DEVICE_USBOPTION, ITP_IOCTL_DISABLE, NULL);
	#endif
	
    #if defined(CFG_USB_ECM)
	ioctl(ITP_DEVICE_ETHERNET, ITP_IOCTL_SLEEP, NULL);
	#endif

    #if defined(CFG_USB0_ENABLE) || defined(CFG_USBHCC)
    //OUT critical section. USB need to handshake for sleep mode.
    ioctl(ITP_DEVICE_USB, ITP_IOCTL_SLEEP, (void *)USB0);
    ithUsbSuspend(ITH_USB0);
    #endif

    #ifdef CFG_USB1_ENABLE
    ioctl(ITP_DEVICE_USB, ITP_IOCTL_SLEEP, (void *)USB1);
    ithUsbSuspend(ITH_USB1);
    #endif

#if defined(CFG_SDIO0_STATIC) || defined(CFG_SDIO1_STATIC)
#if defined(CFG_NET_AMEBA_SDIO)  // just for test
	ioctl(ITP_DEVICE_ETHERNET, ITP_IOCTL_SLEEP, NULL);
#endif
    ioctl(ITP_DEVICE_SDIO, ITP_IOCTL_EXIT, NULL);
#endif
    ithEnterCritical();

    #ifdef CFG_AMPLIFIER_ENABLE
    ioctl(ITP_DEVICE_AMPLIFIER, ITP_IOCTL_DISABLE, NULL);
    #endif

    #ifdef CFG_BACKLIGHT_ENABLE
    ioctl(ITP_DEVICE_BACKLIGHT, ITP_IOCTL_OFF, NULL);
    #endif

    #ifdef CFG_SARADC_ENABLE
    ithSARADCDisableClock();
    #endif

    #ifdef CFG_TVOUT_ENABLE
    ithTveDisableClock();
    #endif

    #ifdef CFG_LCD_ENABLE
    ithLcdDisable();
    #endif

    #ifdef CFG_NAND_ENABLE
    ithNandDisableClock();
    #endif

    #ifdef CFG_XD_ENABLE
    ithXdDisableClock();
    #endif

    #ifdef CFG_DPU_ENABLE
    ithDpuSuspend();
    #endif

    #ifdef CFG_DCPS_ENABLE
    ithDcpsSuspend();
    #endif

    // determined by driver or AP
    /*#ifdef CFG_VIDEO_ENABLE
    ithVideoDisableClock();
    #endif

    #ifdef CFG_JPEG_ENABLE
    ithJpegDisableClock();
    #endif*/

    #ifdef CFG_CMDQ_ENABLE
    ithCmdQWaitEmpty(   ITH_CMDQ0_OFFSET);
//    ithCmdQWaitEmpty(   ITH_CMDQ1_OFFSET);

    ithCmdQDisableClock();
    #endif

    // determined by driver or AP
    /*#if defined(CFG_VIDEO_ENABLE) || defined(CFG_JPEG_ENABLE)
    ithIspDisableClock();
    #endif*/

    #ifdef CFG_WATCHDOG_ENABLE
    ithWatchDogCtrlDisable(ITH_WD_EN);
    #endif

    #ifdef CFG_NET_ETHERNET
    extern int iteMacSuspend(void);
    iteMacSuspend();
    ioctl(ITP_DEVICE_ETHERNET, ITP_IOCTL_ON, (void *)ITP_PHY_WOL); /* enable PHY wol */
    #endif

    #ifdef CFG_POWER_WAKEUP_WOL
    // enable wake on lan gpio
    ithGpioSetIn(CFG_GPIO_ETHERNET_WAKE_ON_LAN);
    ithGpioEnable(CFG_GPIO_ETHERNET_WAKE_ON_LAN);
    #endif

#ifdef CFG_RISC_ENABLE
	// stop Risc
	iteRiscTerminate();
#endif

    ithClockSleep();

    // copy SleepFunc() to audio's SRAM
    #ifndef _WIN32
    memcpy((void *)ITH_SRAM_AUDIO_BASE, SleepFunc, sleepFuncSize);

    // setting parameters
    ptr         = (uint32_t *)(ITH_SRAM_AUDIO_BASE + sleepFuncSize);

    // input parameters
    inParams    = ptr;

        #ifdef CFG_POWER_WAKEUP_KEYPAD
    // keypad's gpio
    for (i = 0; i < KEYPAD_GPIO_COUNT; i++)
    {
        *ptr++ = powerKpGpioTable[i];
    }
        #endif // CFG_POWER_WAKEUP_KEYPAD

        #if defined(CFG_POWER_WAKEUP_UART)
    //disable timer
    ithTimerCtrlDisable(TIMER_NUM, ITH_TIMER_EN);

    //Init Timer
    ithTimerReset(TIMER_NUM);
    ithTimerCtrlEnable(TIMER_NUM, ITH_TIMER_UPCOUNT);
    ithTimerSetCounter(TIMER_NUM, 0x0);
    ithTimerSetLoad(TIMER_NUM, 0x0);

    if(ratio == 0) {
        ratio = 1;
    }

    //Init Uart's parameter
    totalDiv = (12000000/ratio) / UART_BAUDRATE; //SRAM bus_clk=12M
    intDiv = totalDiv >> 4;
    fDiv = totalDiv & 0xF;

    timeout = (12000/ratio) * TIMEOUT;

    for (i = 0; i < SEND_MSG_LEN; i++)
    {
        *ptr++ = sendMsg[i];
    }

    //other parameters
    *ptr++ = intDiv;
    *ptr++ = fDiv;
    *ptr++ = timeout;
        #endif

#if defined(CFG_WATCHDOG_ENABLE)
    //12M reset
    if(ratio == 0) {
        ratio = 1;
    }
    WD_counter = (12000/ratio) * TIMEOUT_WD;

    *ptr++ = WD_counter;
#endif

    // output parameters
    outParams   = ptr;

    // local variables
    var         = ptr;

    ithFlushDCache();
    ithInvalidateICache();

    // call SleepFunc() in SRAM
    ((void (*)(uint32_t *, uint32_t *, uint32_t *))ITH_SRAM_AUDIO_BASE)(inParams, outParams, var);
    #endif // !_WIN32

    #if defined(CFG_POWER_WAKEUP_UART)
    //disable timer
    ithTimerCtrlDisable(TIMER_NUM, ITH_TIMER_EN);

    //restore Uart Baudrate
    ithUartSetBaudRate(UART_NUM, UART_BAUDRATE);
    #endif

    // wakeup procedure
    ithClockWakeup();

    #if defined(CFG_POWER_WAKEUP_TOUCH_DOUBLE_CLICK) /* ifdoubleClick wakeup enable, leave form here. */
    ithExitCritical();
    powerState = ITP_POWER_STANDBY;
    return;
    #endif
	
#ifdef CFG_RISC_ENABLE
	// wakeup Risc
	iteRiscWakeup();
#endif

    #ifdef CFG_NET_ETHERNET
    extern int iteMacResume(void);
    ioctl(ITP_DEVICE_ETHERNET, ITP_IOCTL_OFF, (void *)ITP_PHY_WOL); /* disable PHY wol */
    iteMacResume();
    #endif

    #ifdef CFG_WATCHDOG_ENABLE
    //reset watch dog counter
    ithWatchDogSetTimeout(1000*CFG_WATCHDOG_TIMEOUT);
    ithWriteRegA(ITH_WD_BASE + ITH_WD_RESTART_REG, ITH_WD_AUTORELOAD);

    ithWatchDogEnable();
    #endif

    // determined by driver or AP
    /*#if defined(CFG_VIDEO_ENABLE) || defined(CFG_JPEG_ENABLE)
    ithIspEnableClock();
    #endif*/

    #ifdef CFG_CMDQ_ENABLE
    ithCmdQEnableClock();
    #endif

    // determined by driver or AP
    /*#ifdef CFG_JPEG_ENABLE
    ithJpegEnableClock();
    #endif

    #ifdef CFG_VIDEO_ENABLE
    ithVideoEnableClock();
    #endif*/
    
    #ifdef CFG_DCPS_ENABLE
    ithDcpsResume();
    #endif

    #ifdef CFG_DPU_ENABLE
    ithDpuResume();
    #endif

    #ifdef CFG_XD_ENABLE
    ithXdEnableClock();
    #endif

    #ifdef CFG_NAND_ENABLE
    ithNandEnableClock();
    #endif

    #ifdef CFG_LCD_ENABLE
    ithLcdEnable();

//    if (ithLcdGetPanelType() == ITH_LCD_MIPI)
//    {
//        while (ithReadRegA(0xd0c00038) != 0)
//        {
//            //printf("MIPI error status 0x38:0x%x !!\n", ithReadRegA(0xd0c00038));
//            ithLcdDisable();
//            ithLcdEnable();
//        }
//    }
    #endif

    #ifdef CFG_TVOUT_ENABLE
    ithTveEnableClock();
    #endif

    #ifdef CFG_SARADC_ENABLE
    ithSARADCEnableClock();
    #endif

    #ifdef CFG_AMPLIFIER_ENABLE
    ioctl(ITP_DEVICE_AMPLIFIER, ITP_IOCTL_ENABLE, NULL);
    #endif

    ithExitCritical();

    #ifdef CFG_AUDIO_ENABLE
    itp_codec_wake_up();
    #endif

    #ifdef CFG_BACKLIGHT_ENABLE
    usleep(50000); // to avoid white flicker
    ioctl(ITP_DEVICE_BACKLIGHT, ITP_IOCTL_ON, NULL);
    #endif

#if defined(CFG_SDIO0_STATIC) || defined(CFG_SDIO1_STATIC)
    ioctl(ITP_DEVICE_SDIO, ITP_IOCTL_INIT, NULL);
#if defined(CFG_NET_AMEBA_SDIO) // just for test
	ioctl(ITP_DEVICE_ETHERNET, ITP_IOCTL_RESUME, NULL);
#endif

#if defined(CFG_NET_WIFI_SDIO_NGPL)
    ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_INIT, NULL);
#endif

#endif

    #ifdef CFG_USB1_ENABLE
    ithUsbResume(ITH_USB1);
    ioctl(ITP_DEVICE_USB, ITP_IOCTL_RESUME, (void *)USB1);
    #endif

    #if defined(CFG_USB0_ENABLE) || defined(CFG_USBHCC)
    ithUsbResume(ITH_USB0);
    ioctl(ITP_DEVICE_USB, ITP_IOCTL_RESUME, (void *)USB0);
    #endif

    #ifdef CFG_DBG_NETCONSOLE
    ioctl(ITP_DEVICE_NETCONSOLE, ITP_IOCTL_ENABLE, NULL);
    #endif

    powerState = ITP_POWER_NORMAL;
    printf("PowerSleep end\n");

#endif // CFG_POWER_SLEEP
}

#ifdef CFG_POWER_HIBERNATION
    #ifndef _WIN32

extern uint32_t IRQAddr;
extern void ithIntrDoIrq(void);

void EmptyIrq(void) __attribute__((naked));
void EmptyIrq(void)
{
    portSAVE_CONTEXT();

    // TEST ONLY
    //ithPrintf("EmptyIrq()\n");
    //ithWriteRegA(ITH_INTR_BASE + ITH_INTR_IRQ1_CLR_REG, 0xFFFFFFFF);
    //ithWriteRegA(ITH_INTR_BASE + ITH_INTR_IRQ2_CLR_REG, 0xFFFFFFFF);
    //ithWriteRegA(ITH_INTR_BASE + ITH_INTR_FIQ1_CLR_REG, 0xFFFFFFFF);
    //ithWriteRegA(ITH_INTR_BASE + ITH_INTR_FIQ2_CLR_REG, 0xFFFFFFFF);

    // clear wake on lan gpio interrupt
    ithIntrClearIrq(ITH_INTR_GPIO);
    ithGpioClearIntr(CFG_GPIO_ETHERNET_WAKE_ON_LAN);

    portRESTORE_CONTEXT();
}

    #endif  // !_WIN32
#endif      // CFG_POWER_HIBERNATION

static void PowerHibernation(void)
{
#ifdef CFG_POWER_HIBERNATION
    powerState = ITP_POWER_HIBERNATION;

    #if defined(CFG_USB0_ENABLE) || defined(CFG_USBHCC)
    ioctl(ITP_DEVICE_USB, ITP_IOCTL_SLEEP, (void *)USB0);
    ithUsbSuspend(ITH_USB0);
    #endif

    #ifdef CFG_USB1_ENABLE
    ioctl(ITP_DEVICE_USB, ITP_IOCTL_SLEEP, (void *)USB1);
    ithUsbSuspend(ITH_USB1);
    #endif

    ithEnterCritical();

    #ifdef CFG_AMPLIFIER_ENABLE
    ioctl(ITP_DEVICE_AMPLIFIER, ITP_IOCTL_DISABLE, NULL);
    #endif

    #ifdef CFG_BACKLIGHT_ENABLE
    ioctl(ITP_DEVICE_BACKLIGHT, ITP_IOCTL_OFF, NULL);
    #endif

    #ifdef CFG_TVOUT_ENABLE
    ithTveDisableClock();
    #endif

    #ifdef CFG_LCD_ENABLE
    ithLcdDisable();
    #endif

    #ifdef CFG_AUDIO_ENABLE
    // TODO: IMPLEMENT
    #endif

    // determined by driver or AP
    /*#ifdef CFG_VIDEO_ENABLE
    ithVideoDisableClock();
    #endif

    #ifdef CFG_JPEG_ENABLE
    ithJpegDisableClock();
    #endif*/

    #ifdef CFG_CMDQ_ENABLE
    ithCmdQWaitEmpty(   ITH_CMDQ0_OFFSET);
    ithCmdQWaitEmpty(   ITH_CMDQ1_OFFSET);

    ithCmdQDisableClock();
    #endif

    // determined by driver or AP
    /*#if defined(CFG_VIDEO_ENABLE) || defined(CFG_JPEG_ENABLE)
    ithIspDisableClock();
    #endif*/

    #ifdef CFG_WATCHDOG_ENABLE
    ithWatchDogCtrlDisable(ITH_WD_EN);
    #endif

    //ithClockSleep();

    #ifndef _WIN32
    // backup interrupt settings
    ithIntrSuspend();

    // disable all interrupts
    ithWriteRegA(   ITH_INTR_BASE + ITH_INTR_IRQ1_EN_REG,   0);
    ithWriteRegA(   ITH_INTR_BASE + ITH_INTR_IRQ2_EN_REG,   0);
    ithWriteRegA(   ITH_INTR_BASE + ITH_INTR_FIQ1_EN_REG,   0);
    ithWriteRegA(   ITH_INTR_BASE + ITH_INTR_FIQ2_EN_REG,   0);

    IRQAddr = (uint32_t) EmptyIrq;
    ithFlushDCacheRange((void*)(&IRQAddr), 4);

    // enable wake on lan gpio interrupt
    ithGpioCtrlEnable(CFG_GPIO_ETHERNET_WAKE_ON_LAN, ITH_GPIO_INTR_LEVELTRIGGER);
    ithGpioEnableIntr(CFG_GPIO_ETHERNET_WAKE_ON_LAN);
    ithGpioSetIn(CFG_GPIO_ETHERNET_WAKE_ON_LAN);
    ithGpioEnable(CFG_GPIO_ETHERNET_WAKE_ON_LAN);
    ithIntrEnableIrq(ITH_INTR_GPIO);

    // POWER OFF CPU
    __asm__ __volatile__ (
        "mcr p15,0,r3,c7,c0,4\n"    /* execute idle mode */
        :
        :
        : "r3"  /* clobber list */
        );

    IRQAddr = (uint32_t) ithIntrDoIrq;

    // restore interrupt settings
    ithIntrResume();

    #endif // !_WIN32

    // wakeup procedure
    //ithClockWakeup();

    #ifdef CFG_WATCHDOG_ENABLE
    ithWatchDogEnable();
    #endif

    // determined by driver or AP
    /*#if defined(CFG_VIDEO_ENABLE) || defined(CFG_JPEG_ENABLE)
    ithIspEnableClock();
    #endif*/

    #ifdef CFG_CMDQ_ENABLE
    ithCmdQEnableClock();
    #endif

    // determined by driver or AP
    /*#ifdef CFG_JPEG_ENABLE
    ithJpegEnableClock();
    #endif

    #ifdef CFG_VIDEO_ENABLE
    ithVideoEnableClock();
    #endif*/

    #ifdef CFG_AUDIO_ENABLE
    // TODO: IMPLEMENT
    #endif

    #ifdef CFG_LCD_ENABLE
    ithLcdEnable();
    #endif

    #ifdef CFG_TVOUT_ENABLE
    ithTveEnableClock();
    #endif

    #ifdef CFG_BACKLIGHT_ENABLE
    ioctl(ITP_DEVICE_BACKLIGHT, ITP_IOCTL_ON, NULL);
    #endif

    #ifdef CFG_AMPLIFIER_ENABLE
    ioctl(ITP_DEVICE_AMPLIFIER, ITP_IOCTL_ENABLE, NULL);
    #endif

    ithExitCritical();

    #ifdef CFG_USB1_ENABLE
    ithUsbResume(ITH_USB1);
    ioctl(ITP_DEVICE_USB, ITP_IOCTL_RESUME, (void *)USB1);
    #endif

    #if defined(CFG_USB0_ENABLE) || defined(CFG_USBHCC)
    ithUsbResume(ITH_USB0);
    ioctl(ITP_DEVICE_USB, ITP_IOCTL_RESUME, (void *)USB0);
    #endif

    powerState = ITP_POWER_NORMAL;

#endif // CFG_POWER_HIBERNATION
}

#ifdef CFG_POWER_SUSPEND
    #ifdef _WIN32

unsigned int __power_suspend_func_start, __power_suspend_func_end;

    #else // _WIN32

extern uint32_t SWIAddr;
extern void itpSuspendSave(void);
extern void vPortYieldProcessor(void);

extern unsigned int __power_suspend_func_start, __power_suspend_func_end;

static void __attribute__ ((section(".power_suspend_func"), no_instrument_function))
SuspendFunc(void)
{
    // to protect MMU after self refresh
    register uint32_t val = ITH_READ_REG_A(ITH_RTC_BASE + ITH_RTC_CR_REG);

    // stop memory request from other modules
    ITH_SET_REG_BIT_H(ITH_MEM_CTRL6_REG, ITH_B0_RQ_SEV_STOP_BIT);
    do
    {
        val = ITH_READ_REG_H(ITH_MEM_CTRL6_REG);
    } while ((val & (0x1 << ITH_B0_RQ_EMPTY_BIT)) == 0);

    // enable self refresh mode
    ITH_SET_REG_BIT_H(ITH_MEM_CTRL5_REG, ITH_SELF_REFRESH_BIT);
    do
    {
        val = ITH_READ_REG_H(ITH_MEM_CTRL6_REG);
    } while (val & (0x1 << ITH_B0_REF_PERIOD_CHG_BIT));

    // power off
    ITH_SET_REG_BIT_A(ITH_RTC_BASE + ITH_RTC_CR_REG, ITH_RTC_SW_POWEROFF);

    // after power off, it should not be here
    while (1)
        ;
}

    #endif  // _WIN32
#endif      // CFG_POWER_SUSPEND

static void PowerSuspend(void)
{
#ifdef CFG_POWER_SUSPEND
    // ready to suspend
    const unsigned int suspendFuncSize = (unsigned int)&__power_suspend_func_end - (unsigned int)&__power_suspend_func_start;

    powerState = ITP_POWER_SUSPEND;

    ithEnterCritical();

    // TODO: IMPLEMENT
    #ifdef CFG_BACKLIGHT_ENABLE
    ioctl(ITP_DEVICE_BACKLIGHT, ITP_IOCTL_OFF, NULL);
    #endif

    #if defined(CFG_LCD_ENABLE) || defined(CFG_TVOUT_ENABLE)
    ioctl(ITP_DEVICE_SCREEN, ITP_IOCTL_SUSPEND, NULL);
    #endif

    #ifdef CFG_NAND_ENABLE
    ithNandSuspend();
    #endif

    #ifdef CFG_XD_ENABLE
    ithXdSuspend();
    #endif

    #ifdef CFG_DPU_ENABLE
    ithDpuSuspend();
    #endif

    #ifdef CFG_DCPS_ENABLE
    ithDcpsSuspend();
    #endif

    #ifdef CFG_WATCHDOG_ENABLE
    ithWatchDogRestart();
    ithWatchDogSuspend();
    #endif

    // determined by driver or AP
    /*#if defined(CFG_VIDEO_ENABLE) || defined(CFG_JPEG_ENABLE)
    ithIspDisableClock();
    #endif*/

    ithGpioSuspend();
    ithTimerSuspend();
    ithIntrSuspend();
    ithClockSuspend();
    ithHostSuspend();

    #ifndef _WIN32
    // copy SuspendFunc() to audio's SRAM
    memcpy((void *)ITH_SRAM_AUDIO_BASE, SuspendFunc, suspendFuncSize);

    SWIAddr = (uint32_t) itpSuspendSave;
    ithFlushDCacheRange((void*)(&SWIAddr), 4);
    ithInvalidateICache();
    asm volatile ( "SWI 0" );
    SWIAddr = (uint32_t) vPortYieldProcessor;
    #endif // !_WIN32

    if (ithRtcGetState() == 0x0)
    {
        // must small than SRAM size and 32-bit align
        LOG_DBG "power suspend function size: %d\n", suspendFuncSize LOG_END
        assert(suspendFuncSize < ITH_SRAM_AUDIO_SIZE);
        assert(ITH_IS_ALIGNED(suspendFuncSize, 4));

    #ifndef _WIN32
        ithRtcSetState(0x1);

        ithFlushDCache();
        ithFlushMemBuffer();

        // call SuspendFunc() in SRAM
        ((void (*)(void))ITH_SRAM_AUDIO_BASE)();

    #endif // !_WIN32
    }
    else
    {
        // resume from suspend
        ithRtcCtrlDisable(ITH_RTC_SW_POWEROFF);
        ithRtcSetState(0x0);
    }

    ithHostResume();
    ithClockResume();
    ithIntrResume();
    ithTimerResume();
    ithGpioResume();

    #ifdef CFG_DBG_UART0
	ioctl(ITP_DEVICE_UART0, ITP_IOCTL_RESET, NULL);
    #endif

    #ifdef CFG_DBG_UART1
	ioctl(ITP_DEVICE_UART1, ITP_IOCTL_RESET, NULL);
    #endif

	#ifdef CFG_DBG_UART2
	ioctl(ITP_DEVICE_UART2, ITP_IOCTL_RESET, NULL);
	#endif

	#ifdef CFG_DBG_UART3
	ioctl(ITP_DEVICE_UART3, ITP_IOCTL_RESET, NULL);
	#endif

	#ifdef CFG_DBG_UART4
	ioctl(ITP_DEVICE_UART4, ITP_IOCTL_RESET, NULL);
	#endif

	#ifdef CFG_DBG_UART5
	ioctl(ITP_DEVICE_UART5, ITP_IOCTL_RESET, NULL);
	#endif

    // determined by driver or AP
    /*#if defined(CFG_VIDEO_ENABLE) || defined(CFG_JPEG_ENABLE)
    ithIspEnableClock();
    #endif*/

    #ifdef CFG_WATCHDOG_ENABLE
    ithWatchDogResume();
    ithWatchDogRestart();
    #endif

    #ifdef CFG_DCPS_ENABLE
    ithDcpsResume();
    #endif

    #ifdef CFG_DPU_ENABLE
    ithDpuResume();
    #endif

    #ifdef CFG_XD_ENABLE
    ithXdResume();
    #endif

    #ifdef CFG_NAND_ENABLE
    ithNandResume();
    #endif

    #ifdef CFG_CMDQ_ENABLE
        #if (CFG_CHIP_FAMILY == 970)
    ithCmdQReset(   ITH_CMDQ0_OFFSET);
    ithCmdQReset(   ITH_CMDQ1_OFFSET);
        #else
    ithCmdQReset();
        #endif
    #endif

    #if defined(CFG_LCD_ENABLE) || defined(CFG_TVOUT_ENABLE)
    ioctl(ITP_DEVICE_SCREEN, ITP_IOCTL_RESUME, NULL);
    #endif

    #ifdef CFG_BACKLIGHT_ENABLE
    ioctl(ITP_DEVICE_BACKLIGHT, ITP_IOCTL_ON, NULL);
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

    // resume procedure
    ithExitCritical();

    powerState = ITP_POWER_NORMAL;

#endif // CFG_POWER_SUSPEND
}

static int PowerIoctl(int file, unsigned long request, void *ptr, void *info)
{    
    switch (request)
    {
    case ITP_IOCTL_INIT:
        PowerInit();
        break;

    case ITP_IOCTL_RESUME:
#if !defined(CFG_POWER_WAKEUP_TOUCH_DOUBLE_CLICK)
        PowerResume();
#else
        PowerResumeForDoubleClick();
#endif
        break;

    case ITP_IOCTL_STANDBY:
        PowerStandby();
        break;

    case ITP_IOCTL_RESUME_CLOCK:
        PowerClockResume();
        break;

    case ITP_IOCTL_STANDBY_CLOCK:
        PowerClockStandby();
        break;

    case ITP_IOCTL_SLEEP:
        PowerSleep();
        break;

    case ITP_IOCTL_SLEEP_CONTINUE:
        PowerSleepForDoubleClick();
        break;

    case ITP_IOCTL_HIBERNATION:
        PowerHibernation();
        break;

    case ITP_IOCTL_SUSPEND:
        PowerSuspend();
        break;

    case ITP_IOCTL_OFF:
        PowerOff();
        break;

    default:
        errno = (ITP_DEVICE_POWER << ITP_DEVICE_ERRNO_BIT) | 1;
        return -1;
    }
    return 0;
}

const ITPDevice itpDevicePower =
{
    ":power",
    itpOpenDefault,
    itpCloseDefault,
    PowerRead,
    itpWriteDefault,
    itpLseekDefault,
    PowerIoctl,
    NULL
};