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


void spi_main(void)
{
int fd = 0;
ITPSpiInfo SpiInfo = {0};
uint8_t command[5] = {0};
uint8_t id2[3]	= {0};

itpRegisterDevice(ITP_DEVICE_SPI, &itpDeviceSpi0);
ioctl(ITP_DEVICE_SPI, ITP_IOCTL_INIT, NULL);

printf("Start SPI test!\n");

fd = open(":spi0", O_RDONLY);
if (!fd)
printf("--- open device spi0 fail ---\n");
else
printf("fd = %d\n", fd);

command[0] = 0x9F;

SpiInfo.readWriteFunc = ITP_SPI_PIO_WRITE; //決定讀寫行為
SpiInfo.cmdBuffer = &command;
SpiInfo.cmdBufferSize = 1;
SpiInfo.dataBuffer = &id2;
SpiInfo.dataBufferSize = 0;

while (1)
{
write(fd, &SpiInfo, 1);
printf("SpiInfo 0x%x, 0x%x, 0x%x\n", id2[0],id2[1], id2[2]);
sleep(1);
}

close(fd);
}



void* TestFunc(void* arg)
{
	printf("spi_main test start~~~\n");
	spi_main();
	while(1);
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