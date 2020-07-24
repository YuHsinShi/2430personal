#include <stdio.h>
#include "ite/itp.h"

#define REG_RTC_SEC                    (0x00)
#define REG_RTC_MIN                    (0x04)
#define REG_RTC_HOUR                   (0x08)
#define REG_RTC_DAY                    (0x0C)
#define REG_RTC_WEEK                   (0x10)
#define REG_RTC_ALARM_SEC              (0x14)
#define REG_RTC_ALARM_MIN              (0x18)
#define REG_RTC_ALARM_HOUR             (0x1C)
#define REG_RTC_ALARM_DAY              (0x20)
#define REG_RTC_ALARM_WEEK             (0x24)
#define REG_RTC_RECORD                 (0x28)
#define REG_RTC_CR                     (0x2C)
#define REG_RTC_WSEC                   (0x30)
#define REG_RTC_WMIN                   (0x34)
#define REG_RTC_WHOUR                  (0x38)
#define REG_RTC_WDAY                   (0x3C)
#define REG_RTC_WWEEK                  (0x40)
#define REG_RTC_INTR_STATE             (0x44)
#define REG_RTC_DIV                    (0x48)
#define REG_RTC_REVISION               (0x50)

#define Sec_Error 5 //Error Value: 5 Sec


static long gLastSec = 0;
static long gLastUs = 0;

static void _reset_rtc(void)
{
	uint32_t rtc_cr;

	//reset RTC
	rtc_cr = ithReadRegA(ITH_RTC_BASE + REG_RTC_CR);	
	rtc_cr |= (0x1 << 31);
	ithWriteRegA(ITH_RTC_BASE + REG_RTC_CR, rtc_cr);
	rtc_cr &= (0x0 << 31);
	ithWriteRegA(ITH_RTC_BASE + REG_RTC_CR, rtc_cr);

	//set divider to 32KHz
	ithWriteRegA(ITH_RTC_BASE + REG_RTC_DIV, 0x8000 | 0x3 << 30);
}

static void _set_rtc_time(long sec, long usec)
{
	uint32_t rtc_cr;
	uint32_t i, year, mon, mday, days = 0, start_year = 2015;
	struct  tm *t = localtime(&sec);
	uint8_t mon_days[12] = {31, 0, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
	
	year = t->tm_year + 1900;
	mon = t->tm_mon + 1;
	mday = t->tm_mday;

	while(start_year <= year)
	{
		if(start_year % 4)
			mon_days[1] = 28;
		else if(!(start_year % 100) && (start_year % 400))
			mon_days[1] = 28;
		else
			mon_days[1] = 29;

		for(i=0; i<12; i++)
		{
			if(start_year == year && i == (mon - 1))
			{
				days += mday;
				break;
			}
			else
				days += mon_days[i];
		}
		
		start_year++;
	}
	
	ithWriteRegMaskA(ITH_RTC_BASE + REG_RTC_WSEC, t->tm_sec, 0x3f);
	ithWriteRegMaskA(ITH_RTC_BASE + REG_RTC_WMIN, t->tm_min, 0x3f);
	ithWriteRegMaskA(ITH_RTC_BASE + REG_RTC_WHOUR, t->tm_hour, 0x1f);
	ithWriteRegMaskA(ITH_RTC_BASE + REG_RTC_WDAY, days, 0xffff);
	ithWriteRegMaskA(ITH_RTC_BASE + REG_RTC_WWEEK, t->tm_wday, 0x7);

	//set RTC enable
	rtc_cr = ithReadRegA(ITH_RTC_BASE + REG_RTC_CR);	
	rtc_cr |= 0x1;
	ithWriteRegA(ITH_RTC_BASE + REG_RTC_CR, rtc_cr);
	//set RTC write reload
	rtc_cr = ithReadRegA(ITH_RTC_BASE + REG_RTC_CR);	
	rtc_cr |= 0x100;
	ithWriteRegA(ITH_RTC_BASE + REG_RTC_CR, rtc_cr);
}

static long _get_rtc_time(void)
{
	uint32_t i, sec, min, hour, days, week, start_year = 1970;  //IT970 internal RTC's year start from 1970
	uint8_t mon_days[12] = {31, 0, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
	long rtcSec=0;
	struct  tm tInfo;

   	sec = ithReadRegA(ITH_RTC_BASE + REG_RTC_SEC) & 0x3f;
	min = ithReadRegA(ITH_RTC_BASE + REG_RTC_MIN) & 0x3f;
	hour = ithReadRegA(ITH_RTC_BASE + REG_RTC_HOUR) & 0x1f;
	days = ithReadRegA(ITH_RTC_BASE + REG_RTC_DAY) & 0xffff;
	week = ithReadRegA(ITH_RTC_BASE + REG_RTC_WEEK) & 0x7;

	while(days)
	{
		if(start_year % 4)
			mon_days[1] = 28;
		else if(!(start_year % 100) && (start_year % 400))
			mon_days[1] = 28;
		else
			mon_days[1] = 29;

		for(i=0; i<12; i++)
		{
			if(days <= mon_days[i])
				break;
			else
				days -= mon_days[i];
		}
		
		if(i == 12)
			start_year++;
		else
			break;
	}
	
	tInfo.tm_sec = sec;						//sec:0~59
    tInfo.tm_min = min;						//min:0~59
    tInfo.tm_hour = hour;					//hour:0~23
    tInfo.tm_wday = week;					//week:1~7 (1:sun, 2:mon, 3:tue,..., 7:sat)(but linux week define is 0~6)       
    tInfo.tm_mday = days;					//day:1~31 (without 0)
    tInfo.tm_mon = i;						//mon:1~12 (1:jan, 2:feb, 3:mar..., 12:Dec)(without 0, but linux month define is 0~11)
    tInfo.tm_year = start_year - 1900;		//year:2015~2099

	printf("get time = %d-%d-%d-%d.%d.%d\n", start_year, i+1, days+1, hour, min, sec); //IT970 internal RTC Reg_date start from 0
	rtcSec = mktime((struct tm*)&tInfo);

	return rtcSec;
}

static void _check_rtc_value(struct timeval *t)
{
	int rtcErr = 0;
	
    if(gLastSec > t->tv_sec)
    {
    	printf("	@@@ RTC_ERROR1: time has returned! \n");
    	rtcErr++;
    }
    	
    if(gLastSec == t->tv_sec)
	{
		if(gLastUs >= t->tv_usec)
		{
			printf("	@@@ RTC_ERROR2: time has returned!\n");
			rtcErr++;
		}
			
		if(gLastUs < t->tv_usec)
		{
			if( (t->tv_usec - gLastUs) > 100000 )
			{
				printf("	@@@ RTC_ERROR3: over 100 ms! \n");
				rtcErr++;
			}
		}
	}
	
	if(gLastSec < t->tv_sec)
	{
		if( (t->tv_sec - gLastSec) > 1 )
		{
			printf("	@@@ RTC_ERROR4: over 2s \n");
			rtcErr++;
		}
		
		if(gLastUs <= t->tv_usec)
		{
			printf("	@@@ RTC_ERROR5: lastUs should be larger than thisUs! \n");
			rtcErr++;
		}
		
		if(gLastUs > t->tv_usec)
		{
			if( (t->tv_usec + 1000000 - gLastUs) > 100000 )
			{
				printf("	@@@ RTC_ERROR6: over 100 ms! \n");
				rtcErr++;
			}
		}
	}
	
	if(rtcErr)	printf("	*** RTC error: s1=%d, us1=%06d, s2=%d, us2=%06d, errCnt=%01d ***\n",t->tv_sec,t->tv_usec,gLastSec,gLastUs,rtcErr);
		
    gLastSec = t->tv_sec;
    gLastUs = t->tv_usec;	
}

void* TestFunc_count(void* arg)
{
	struct timeval tv = {0};

#ifdef _WIN32
	struct timezone tz = {0};
	unsigned long CurrRtcTime = 0;
	unsigned long CurrWinTime = 0;
	
	//Win32 Case: Error Value Test between Windows SystemTime and internal RTC

	printf("Start RTC Test\n");

	CurrRtcTime = ithRtcGetTime();
	printf("Current internal RTC Time= %d sec\n", CurrRtcTime);

	gettimeofday(&tv,&tz);
	printf("Current Windows: %d (sec), %d (GSM)\n", tv.tv_sec, tz.tz_minuteswest);
#if 0
	// Windows Time(including time zone)
	CurrWinTime = tv.tv_sec + (abs(tz.tz_minuteswest) * 60);
#else
	CurrWinTime = tv.tv_sec;
#endif
	printf("Current Windows Time: %d sec\n", CurrWinTime);

	ithRtcSetTime(CurrWinTime);

	CurrRtcTime = ithRtcGetTime();
	printf("Current internal RTC Time= %d sec\n", CurrRtcTime);

	while (1) {
		gettimeofday(&tv, &tz);
		CurrRtcTime=ithRtcGetTime();
		CurrWinTime = tv.tv_sec;

		if(CurrRtcTime > CurrWinTime) {
			if (CurrRtcTime - CurrWinTime >= Sec_Error)
				break;
		}
		else if(CurrRtcTime < CurrWinTime){
			if (CurrWinTime - CurrRtcTime >= Sec_Error)
				break;
		}
	}

	printf("End RTC Test, RTC= %d Windows= %d\n", CurrRtcTime, CurrWinTime);
	
#else 
	itpRegisterDevice(ITP_DEVICE_RTC, &itpDeviceRtc);
	ioctl(ITP_DEVICE_RTC, ITP_IOCTL_INIT, NULL);

    //_get_rtc_time();  //IT970 can use this function to get rtc time
    //gettimeofday(&tv,NULL); //Get RTC time directly, return Sec
    
	ithRtcSetTime(1575868292);
	sleep(1);
	printf("wday: %d\n", ithReadRegA(ITH_RTC_BASE + ITH_RTC_WEEK_REG) & ITH_RTC_WEEK_MASK);
	sleep(1);
    while(1)
    {
    	gettimeofday(&tv,NULL);
    	//printf("tv_sec (%d), tv_usec (%06d)\n", (int)tv.tv_sec, (int)tv.tv_usec);
		printf("sec: %ld\n", ithRtcGetTime());
    	_check_rtc_value(&tv);
    	
    	usleep(33000);
    }
#endif	
	return NULL;
}

static void RtcSecIntrHandler(void* arg)
{
	ithPrintf("==============A sec==============\n");
}

void *TestFunc_MS(void *arg) // MS stands for micro second not M$
{
	// rtc microsec counter init
	ithWriteRegA(ITH_TIMER_BASE + ITH_TIMER_TMUS_EN_REG, (ithGetBusClock() / 1000000) - 1);
	ithSetRegBitA(ITH_TIMER_BASE + ITH_TIMER_TMUS_EN_REG, 1); // divider 2
	ithWriteRegMaskA(ITH_TIMER_BASE + ITH_TIMER_TMUS_EN_REG, 1 << 31, 1 << 31);

	// init rtc sec interrupt
	ithRtcCtrlEnable(ITH_RTC_INTR_SEC);
	ithIntrRegisterHandlerIrq(ITH_INTR_RTCSEC, RtcSecIntrHandler, NULL);
	ithIntrSetTriggerModeIrq(ITH_INTR_RTCSEC, ITH_INTR_EDGE);
	ithIntrEnableIrq(ITH_INTR_RTCSEC);

	while (1)
	{
		printf("usec: %ld\n", (long)ithReadRegA(ITH_TIMER_BASE + ITH_TIMER_TMUS_COUNTER_REG) & 0x0FFFFFFF);
		usleep(33000);
	}

	return NULL;
}

static void RtcAlarmIntrHandler(void *arg)
{
	ithPrintf("==============Alarm triggered==============\n");
}

void *TestFunc_Alarm(void *arg)
{
	long alarmTime = 5;// setting Alarm after 5 secs

	printf("Start testing: Alarm will be triggered after %ld sec.\n", alarmTime);

	// init RTC
	itpRegisterDevice(ITP_DEVICE_RTC, &itpDeviceRtc);
	ioctl(ITP_DEVICE_RTC, ITP_IOCTL_INIT, NULL);

	// setting Alarm
	ithRtcSetAlarm(ithRtcGetTime() + alarmTime);

	printf("now: %ld, alarm: %ld\n", ithRtcGetTime(), ithRtcGetAlarm());

	// init rtc sec interrupt
	ithRtcCtrlEnable(ITH_RTC_ALARM_INTR);
	ithIntrRegisterHandlerIrq(ITH_INTR_RTCALARM, RtcAlarmIntrHandler, NULL);
	ithIntrSetTriggerModeIrq(ITH_INTR_RTCALARM, ITH_INTR_EDGE);
	ithIntrEnableIrq(ITH_INTR_RTCALARM);

	while (1)
	{
		printf("sec: %ld\n", ithRtcGetTime());
		usleep(2*500*1000);
	}
	return NULL;
}
