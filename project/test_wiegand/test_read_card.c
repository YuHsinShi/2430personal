#include <sys/ioctl.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include "ite/ith.h"
#include "ite/itp.h"
#include "ith/ith_defs.h"

int getWiegandIn_Handle()
{
#ifdef CFG_TEST_FROM_WIEGAND0
	return ITP_DEVICE_WIEGAND0;
#else
	return ITP_DEVICE_WIEGAND1;
#endif
}

void* getWiegandIn_Setting()
{
#ifdef CFG_TEST_FROM_WIEGAND0
	return &itpDeviceWiegand0;
#else
	return &itpDeviceWiegand1;
#endif
}

int getWiegandIn_BitCount()
{
#ifdef CFG_TEST_FROM_WIEGAND0
	return WIEGAND0_BIT_COUNT;
#else
	return WIEGAND1_BIT_COUNT;
#endif
}

void getWiegandIn_Gpio(int* gpio)
{
	int wiegand_gpio[2];

#ifdef CFG_TEST_FROM_WIEGAND0
	gpio[0] = CFG_WIEGAND0_GPIO0;
	gpio[1] = CFG_WIEGAND0_GPIO1;
#else
	gpio[0] = CFG_WIEGAND1_GPIO0;
	gpio[1] = CFG_WIEGAND1_GPIO1;
#endif

	return wiegand_gpio;
}

void *TestFunc(void *arg)
{
	//9850 lobby:
	//wiegand 0: 7,8
	//wiegand 1: 5,6

#if defined(_WIN32)
	ithWriteRegMaskA(ITH_HOST_BASE + ITH_APB_CLK2_REG, 0x80000000, 0x80000000);	// CLK48M_EN
	ithWriteRegMaskA(ITH_HOST_BASE + ITH_APB_CLK1_REG, 0x00C00000, 0x00C00000);	// Reset wiegand input and output
	ithWriteRegMaskA(ITH_HOST_BASE + ITH_APB_CLK1_REG, 0x00000000, 0x00C00000);
#endif

	/****************************************************************************/
	// Setting test properties
	int WIEGNAD_HANDLE = getWiegandIn_Handle();
	void* WIEGNAD_SETTING = getWiegandIn_Setting();
	int WIEGAND_BITCOUNT = getWiegandIn_BitCount();
	int WIEGAND_GPIO[2];
	getWiegandIn_Gpio(WIEGAND_GPIO);

	/****************************************************************************/
	//Set wiegand in receive from goip to uart rx
	itpRegisterDevice(WIEGNAD_HANDLE, WIEGNAD_SETTING);
	ioctl(WIEGNAD_HANDLE, ITP_IOCTL_INIT, NULL);
	ioctl(WIEGNAD_HANDLE, ITP_IOCTL_SET_BIT_COUNT, &WIEGAND_BITCOUNT);
	ioctl(WIEGNAD_HANDLE, ITP_IOCTL_SET_GPIO_PIN, WIEGAND_GPIO); // must set before enable
	ioctl(WIEGNAD_HANDLE, ITP_IOCTL_ENABLE, NULL);

	/****************************************************************************/
	// Try to readout
	while (true)
	{
		char card_id[17];
		read(WIEGNAD_HANDLE, &card_id, 0);	// Auto print result if readed
		usleep(300000);
	}
}