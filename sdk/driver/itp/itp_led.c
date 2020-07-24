/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * PAL LED functions.
 *
 * @author Jim Tan
 * @version 1.0
 */
#include <errno.h>
#include <time.h>
#include "itp_cfg.h"

static const unsigned int ledGpioTable[] = { CFG_GPIO_LED };
typedef struct
{
    timer_t timerId;
    bool on;
} LedData;

static LedData ledData[ITH_COUNT_OF(ledGpioTable)];

static void LedInit(void)
{
    int i;

    for (i = 0; i < ITH_COUNT_OF(ledGpioTable); i++)
    {
        ledData[i].timerId = 0;
        ledData[i].on = false;
		
		ithGpioSetOut(ledGpioTable[i]);
		ithGpioClear(ledGpioTable[i]);
		ithGpioEnable(ledGpioTable[i]);
    }
}

static void LedOn(int index)
{
    if (ledData[index].timerId)
    {
        timer_delete(ledData[index].timerId);
        ledData[index].timerId = 0;
    }
    
	ithGpioSet(ledGpioTable[index]);
	    
    ledData[index].on = true;
}

static void LedFlickerFunc(timer_t timerid, int arg)
{
    LedData* data = &ledData[arg];

    if (data->on)
    {
    	ithGpioClear(ledGpioTable[arg]);
        	
        data->on = false;
    }
    else
    {
		ithGpioSet(ledGpioTable[arg]);

        data->on = true;
    }
}

static void LedOff(int index)
{
    if (ledData[index].timerId)
    {
        timer_delete(ledData[index].timerId);
        ledData[index].timerId = 0;
    }
    
	ithGpioClear(ledGpioTable[index]);
       	
    ledData[index].on = false;
}

static void LedFlicker(int index, int ms)
{
    struct itimerspec value;

    if (ledData[index].timerId)
    {
        timer_delete(ledData[index].timerId);
        ledData[index].timerId = 0;
    }

    value.it_value.tv_sec       = ms / 1000;
    value.it_value.tv_nsec      = (ms % 1000) * 1000000;
    value.it_interval.tv_sec    = value.it_value.tv_sec;
    value.it_interval.tv_nsec   = value.it_value.tv_nsec;

    timer_create(CLOCK_REALTIME, NULL, &ledData[index].timerId);
    timer_connect(ledData[index].timerId, LedFlickerFunc, index);
    timer_settime(ledData[index].timerId, 0, &value, NULL);
}

static int LedOpen(const char* name, int flags, int mode, void* info)
{
    int index = atoi(name);
    return index;
}

static int LedClose(int file, void* info)
{
    LedOff(file);
    return 0;
}

static int LedIoctl(int file, unsigned long request, void* ptr, void* info)
{
    switch (request)
    {
    case ITP_IOCTL_INIT:
        LedInit();
        break;

    case ITP_IOCTL_ON:
        LedOn(file);
        break;

    case ITP_IOCTL_OFF:
        LedOff(file);
        break;

    case ITP_IOCTL_FLICKER:
        LedFlicker(file, (int)ptr);
        break;

    default:
        errno = (ITP_DEVICE_LED << ITP_DEVICE_ERRNO_BIT) | __LINE__;
        return -1;
    }
    return 0;
}

const ITPDevice itpDeviceLed =
{
    ":led",
    LedOpen,
    LedClose,
    itpReadDefault,
    itpWriteDefault,
    itpLseekDefault,
    LedIoctl,
    NULL
};
