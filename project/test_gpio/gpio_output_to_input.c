/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 *
 * @file (%project)/project/test_gpio/test_gpio_ex1.c
 *
 * @author Joseph Chang
 * @version 1.0.0
 *
 * example code for setting GPIO output status
 */
#include "ite/itp.h"	//for all ith driver (include GPIO) & MACRO

void* TestFunc(void* arg)
{
	int gpioPinIn = CFG_GPIO_TEST_INPUT_PIN;
	int gpioPinOut = CFG_GPIO_TEST_OUTPUT_PIN;

	int i = 0;

	printf("GPIO test start~~~\n");

	//initial GPIO
	ithGpioSetOut(gpioPinOut);
	ithGpioSetMode(gpioPinOut, ITH_GPIO_MODE0);

	ithGpioSetIn(gpioPinIn);
	ithGpioSetMode(gpioPinIn, ITH_GPIO_MODE0);

	//
	while (1)
	{
		if (i++ & 0x1)
		{
			ithGpioClear(gpioPinOut);
		}
		else
		{
			ithGpioSet(gpioPinOut);
		}
		printf("current GPIO[%d] state=%x, index=%d\n", gpioPinIn, ithGpioGet(gpioPinIn), i);
		usleep(1000 * 1000);	//wait for 1 second
	}

	return NULL;
}