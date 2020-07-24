/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * PAL RTC internal functions.
 *
 * @author Jim Tan
 * @version 1.0
 */
#include "../itp_cfg.h"

//#define TMUS_ENABLE
#define RTC_TIMER		(CFG_INTERNAL_RTC_TIMER - 1)
#define RTC_YEAR_2106_TIMESTAMP (0xFFFFFFFF)
#define RTC_YEAR_2100_TIMESTAMP (0xF485E680)

static void RtcSecIntrHandler(void* arg)
{
#ifndef TMUS_ENABLE
    ithTimerSetCounter(RTC_TIMER, 0);  // reset counter
#endif
    ithRtcClearIntr(ITH_RTC_SEC);
}

#ifdef TMUS_ENABLE
static long RtcGetUs(void)
{
	long usec = 0x0;

	usec = (long)ithReadRegA(ITH_TIMER_BASE + ITH_TIMER_TMUS_COUNTER_REG) & 0x0FFFFFFF;

	return usec;
}
#endif

void itpRtcInit(void)
{
    //printf("RTC itpRtcInit:long size = %d, %d\n",sizeof(long),sizeof(int));
    
    ithRtcInit(CFG_RTC_EXTCLK);
    if (ithRtcEnable())
    {
        LOG_INFO "First time boot\n" LOG_END
        ithRtcSetTime(CFG_RTC_DEFAULT_TIMESTAMP);
    }

#ifdef TMUS_ENABLE
	// init TmUs counter to calc usec of gettimeofday()
	ithWriteRegA(ITH_TIMER_BASE + ITH_TIMER_TMUS_EN_REG, (ithGetBusClock()/1000000)-1);
	ithWriteRegMaskA(ITH_TIMER_BASE + ITH_TIMER_TMUS_EN_REG, 1<<31, 1<<31);
#else
	// init timer4 to calc usec of gettimeofday()
    ithTimerReset(RTC_TIMER);
    ithTimerCtrlEnable(RTC_TIMER, ITH_TIMER_UPCOUNT);
    ithTimerSetCounter(RTC_TIMER, 0);
    ithTimerEnable(RTC_TIMER);
#endif

    // init rtc sec interrupt
    ithRtcCtrlEnable(ITH_RTC_INTR_SEC);
    ithIntrRegisterHandlerIrq(ITH_INTR_RTCSEC, RtcSecIntrHandler, NULL);
    ithIntrSetTriggerModeIrq(ITH_INTR_RTCSEC, ITH_INTR_EDGE);
    ithIntrEnableIrq(ITH_INTR_RTCSEC);
}

long itpRtcGetTime(long* usec)
{
    long sec1, sec2;
    do 
    {
        sec1 = ithRtcGetTime();
        if (usec) {
#ifdef TMUS_ENABLE
			*usec = RtcGetUs();
#else
            *usec = ithTimerGetTime(RTC_TIMER);
#endif
        }

        sec2 = ithRtcGetTime();
        
    } while (sec1 != sec2);

    LOG_DBG "[RTC] get time: sec=%x, usec=%x\n", sec1, *usec LOG_END

    if( ((unsigned long)sec1 < (unsigned long)CFG_RTC_DEFAULT_TIMESTAMP) || 
        ((unsigned long)sec1 > (unsigned long)RTC_YEAR_2100_TIMESTAMP) )
    {
        /* SW handle internal RTC low bettery power issue */
        /* report default timestemp when RTC < default or RTC > year 2100 */
        long stemp_def = (long)CFG_RTC_DEFAULT_TIMESTAMP;
        long stemp_2100 = (long)RTC_YEAR_2100_TIMESTAMP;
        
        LOG_ERR "[RTC] get time ERROR(ori=%x, default=%x, 2100=%x):[#%d]\n", sec1, stemp_def, stemp_2100, __LINE__ LOG_END
        sec1 = (long)CFG_RTC_DEFAULT_TIMESTAMP;
    }

    return sec1;
}

void itpRtcSetTime(long sec, long usec)
{
    LOG_DBG "[RTC] set time: sec=%x, usec=%x\n", sec, usec LOG_END
    
    if( ((unsigned long)sec < (unsigned long)CFG_RTC_DEFAULT_TIMESTAMP) || 
        ((unsigned long)sec > (unsigned long)RTC_YEAR_2100_TIMESTAMP) )
    {
        /* SW handle internal RTC low bettery power issue */
        /* set default timestemp when sec < default or sec > year 2100 */
        LOG_ERR "[RTC] set time ERROR(ori=%x, default=%x):[#%d]\n", sec, CFG_RTC_DEFAULT_TIMESTAMP, __LINE__ LOG_END
        ithRtcSetTime(CFG_RTC_DEFAULT_TIMESTAMP + (usec / 1000000));
    }
    else
    {
        ithRtcSetTime(sec + (usec / 1000000));
    }
}
