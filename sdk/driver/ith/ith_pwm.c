/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * HAL PWM functions.
 *
 * @author Jim Tan
 * @version 1.0
 */
#include "ith_cfg.h"
#include <pthread.h>

static uint32_t blCounts[6], blMatchs[6];

static unsigned int oldmatch1 = 0;
static pthread_mutex_t PWM_MUTEX  = PTHREAD_MUTEX_INITIALIZER;

void ithPwmInit(ITHPwm pwm, unsigned int freq, unsigned int duty)
{
    blCounts[pwm]   = ithGetBusClock() / freq;
    blMatchs[pwm]    = (uint64_t) blCounts[pwm] * duty / 100;
}

static void ithPwmSetGpioMode(ITHPwm pwm, unsigned int pin)
{
#if (CFG_CHIP_FAMILY == 9860)
		if (pin >= 21 && pin <= 24)
		{
			ithPrintf("Hit fully mux special case, pin: %d change to %d\n", pin, pin-21);
			pin -= 21;
		}
	
		if (pin >= 68 && pin <= 77) // using MIPI GPIO
			ithWriteRegMaskA(ITH_MIPI_DPHY_BASE, 0x2 << 22, 0x3 << 22); // set MIPI DPHY R0 to TTL
#endif

	switch (pwm) {
		case ITH_PWM1:
			ithWriteRegMaskA(ITH_GPIO_BASE + ITH_GPIO_PWMSEL01_REG, pin << ITH_GPIO_PWM_LOW_BIT, ITH_GPIO_PWM_LOW_MASK);
			ithSetRegBitA(ITH_GPIO_BASE + ITH_GPIO_PWMSEL01_REG, ITH_GPIO_PWM_LOW_EN_BIT);
			break;
		case ITH_PWM2:
			ithWriteRegMaskA(ITH_GPIO_BASE + ITH_GPIO_PWMSEL01_REG, pin << ITH_GPIO_PWM_HIGH_BIT, ITH_GPIO_PWM_HIGH_MASK);
			ithSetRegBitA(ITH_GPIO_BASE + ITH_GPIO_PWMSEL01_REG, ITH_GPIO_PWM_HIGH_EN_BIT);
			break;
		case ITH_PWM3:
			ithWriteRegMaskA(ITH_GPIO_BASE + ITH_GPIO_PWMSEL23_REG, pin << ITH_GPIO_PWM_LOW_BIT, ITH_GPIO_PWM_LOW_MASK);
			ithSetRegBitA(ITH_GPIO_BASE + ITH_GPIO_PWMSEL23_REG, ITH_GPIO_PWM_LOW_EN_BIT);
			break;
		case ITH_PWM4:
			ithWriteRegMaskA(ITH_GPIO_BASE + ITH_GPIO_PWMSEL23_REG, pin << ITH_GPIO_PWM_HIGH_BIT, ITH_GPIO_PWM_HIGH_MASK);
			ithSetRegBitA(ITH_GPIO_BASE + ITH_GPIO_PWMSEL23_REG, ITH_GPIO_PWM_HIGH_EN_BIT);
			break;
		case ITH_PWM5:
			ithWriteRegMaskA(ITH_GPIO_BASE + ITH_GPIO_PWMSEL45_REG, pin << ITH_GPIO_PWM_LOW_BIT, ITH_GPIO_PWM_LOW_MASK);
			ithSetRegBitA(ITH_GPIO_BASE + ITH_GPIO_PWMSEL45_REG, ITH_GPIO_PWM_LOW_EN_BIT);
			break;
		case ITH_PWM6:
			ithWriteRegMaskA(ITH_GPIO_BASE + ITH_GPIO_PWMSEL45_REG, pin << ITH_GPIO_PWM_HIGH_BIT, ITH_GPIO_PWM_HIGH_MASK);
			ithSetRegBitA(ITH_GPIO_BASE + ITH_GPIO_PWMSEL45_REG, ITH_GPIO_PWM_HIGH_EN_BIT);
			break;
#if (CFG_CHIP_FAMILY == 9860)
		case ITH_PWM7:
			ithWriteRegMaskA(ITH_GPIO_BASE + ITH_GPIO_PWMSEL67_REG, pin << ITH_GPIO_PWM_LOW_BIT, ITH_GPIO_PWM_LOW_MASK);
			ithSetRegBitA(ITH_GPIO_BASE + ITH_GPIO_PWMSEL67_REG, ITH_GPIO_PWM_LOW_EN_BIT);
			break;
		case ITH_PWM8:
			ithWriteRegMaskA(ITH_GPIO_BASE + ITH_GPIO_PWMSEL67_REG, pin << ITH_GPIO_PWM_HIGH_BIT, ITH_GPIO_PWM_HIGH_MASK);
			ithSetRegBitA(ITH_GPIO_BASE + ITH_GPIO_PWMSEL67_REG, ITH_GPIO_PWM_HIGH_EN_BIT);
			break;
#endif
	}
}

static void ithPwmClearGpioMode(ITHPwm pwm)
{
	switch(pwm) {
		case ITH_PWM1:
			ithClearRegBitA(ITH_GPIO_BASE + ITH_GPIO_PWMSEL01_REG, ITH_GPIO_PWM_LOW_EN_BIT);
			break;
		case ITH_PWM2:
			ithClearRegBitA(ITH_GPIO_BASE + ITH_GPIO_PWMSEL01_REG, ITH_GPIO_PWM_HIGH_EN_BIT);
			break;
		case ITH_PWM3:
			ithClearRegBitA(ITH_GPIO_BASE + ITH_GPIO_PWMSEL23_REG, ITH_GPIO_PWM_LOW_EN_BIT);
			break;
		case ITH_PWM4:
			ithClearRegBitA(ITH_GPIO_BASE + ITH_GPIO_PWMSEL23_REG, ITH_GPIO_PWM_HIGH_EN_BIT);
			break;
		case ITH_PWM5:
			ithClearRegBitA(ITH_GPIO_BASE + ITH_GPIO_PWMSEL45_REG, ITH_GPIO_PWM_LOW_EN_BIT);
			break;
		case ITH_PWM6:
			ithClearRegBitA(ITH_GPIO_BASE + ITH_GPIO_PWMSEL45_REG, ITH_GPIO_PWM_HIGH_EN_BIT);
			break;
#if (CFG_CHIP_FAMILY == 9860)
		case ITH_PWM7:
			ithClearRegBitA(ITH_GPIO_BASE + ITH_GPIO_PWMSEL67_REG, ITH_GPIO_PWM_LOW_EN_BIT);
			break;
		case ITH_PWM8:
			ithClearRegBitA(ITH_GPIO_BASE + ITH_GPIO_PWMSEL67_REG, ITH_GPIO_PWM_HIGH_EN_BIT);
			break;
#endif
	}
}

void ithPwmReset(ITHPwm pwm, unsigned int pin, unsigned int gpio_mode)
{
#if (CFG_CHIP_FAMILY == 9860)
    ithPwmSetGpioMode(pwm, pin);
#endif
	ithGpioSetMode(pin, gpio_mode);
    ithTimerReset(pwm);
    ithTimerSetCounter(pwm, 0);
    ithTimerSetPwmMatch(pwm, blMatchs[pwm], blCounts[pwm]);

    ithTimerCtrlEnable(pwm, ITH_TIMER_UPCOUNT);
    ithTimerCtrlEnable(pwm, ITH_TIMER_PERIODIC);
    ithTimerCtrlEnable(pwm, ITH_TIMER_PWM);
    ithTimerEnable(pwm);
}

void ithPwmSetDutyCycle(ITHPwm pwm, unsigned int duty)
{
	uint32_t count, newmatch1,newcounter;

	ithTimerCtrlDisable(pwm, ITH_TIMER_EN);
	count = ithTimerGetCounter(pwm);
	newmatch1 = ((uint64_t) blCounts[pwm] * duty / 100);

	pthread_mutex_lock(&PWM_MUTEX);
	if ((oldmatch1 > count) && count > newmatch1)
	{
		if (newmatch1 - (oldmatch1 - count) >= 0)
		{
			newcounter = newmatch1 - (oldmatch1 - count);
		}
		else
		{
			newcounter =  blCounts[pwm] - ((oldmatch1 - count)- newmatch1);
		}

    	ithTimerSetPwmMatch(pwm, newmatch1, blCounts[pwm]);
		ithTimerSetCounter(pwm, newcounter);
	}
	else
		ithTimerSetPwmMatch(pwm, newmatch1, blCounts[pwm]);

	ithTimerCtrlEnable(pwm, ITH_TIMER_EN);
	oldmatch1 = newmatch1;
	pthread_mutex_unlock(&PWM_MUTEX);
}

void ithPwmEnable(ITHPwm pwm, unsigned int pin, unsigned int gpio_mode)
{
#if (CFG_CHIP_FAMILY == 9860)
	ithPwmSetGpioMode(pwm, pin);
#endif
    ithGpioSetMode(pin, gpio_mode);
    ithTimerCtrlEnable(pwm, ITH_TIMER_PWM);
    ithTimerCtrlEnable(pwm, ITH_TIMER_EN);
}

void ithPwmDisable(ITHPwm pwm, unsigned int pin)
{
#if (CFG_CHIP_FAMILY == 9860)
	ithPwmClearGpioMode(pwm);
#endif
    ithGpioClear(pin);
    ithGpioEnable(pin);
    ithGpioSetOut(pin);
    ithTimerCtrlDisable(pwm, ITH_TIMER_EN);
    ithTimerCtrlDisable(pwm, ITH_TIMER_PWM);
}
