﻿/*
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
	printf("spi_main test start~~~\n");
	//spi_main();
	//while(1);
	int gpioPinIn = CFG_GPIO_TEST_INPUT_PIN;
	int gpioPinOut = CFG_GPIO_TEST_OUTPUT_PIN;

	int i = 0;

	printf("GPIOv  test start~~~\n");
	ithGpioSetOut(48);
	ithGpioSetMode(48, ITH_GPIO_MODE0);
	ithGpioClear(48);
	ithGpioSetOut(49);
	ithGpioSetMode(49, ITH_GPIO_MODE0);
	ithGpioClear(49);

	ithGpioSetOut(50);
	ithGpioSetMode(50, ITH_GPIO_MODE0);
	ithGpioClear(50);
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
			printf("output gpio %d low~~~\n",gpioPinOut);

		}
		else
		{
			ithGpioSet(gpioPinOut);
			printf("output gpio %d high~~~\n",gpioPinOut);

		}
		printf("current GPIO[%d] state=%x, index=%d\n", gpioPinIn, ithGpioGet(gpioPinIn), i);
		usleep(1000 * 1000);	//wait for 1 second
	}

	return NULL;
}