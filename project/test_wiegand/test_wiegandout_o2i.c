#include <sys/ioctl.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include "ite/ith.h"
#include "ite/itp.h"
#include "ith/ith_defs.h"
#include "wiegand/wiegandout.h"

void getTargetCardId(char* sourceId, int idlen, char* idNum)
{
	char tmp[17];
	unsigned long long value = 0;										// Ex sourceId = { 0x30, 0xB4, 0x73 }
	for (int i = 0; i < idlen; i++) tmp[i] = sourceId[idlen - 1 - i];	// tmp = { 0x73, 0xB4, 0x30 }
	memcpy(&value, tmp, idlen);											// Value = 0x30B473
	sprintf(idNum, "%08X%03d%05d", (unsigned long)(value >> 32), ((unsigned long)value & 0xFFFF0000) >> 16, (unsigned long)value & 0xFFFF);
}

int getWiegandIn_Handle()
{
#ifdef CFG_TEST_FROM_WIEGAND0
	return ITP_DEVICE_WIEGAND0;
#else
	return ITP_DEVICE_WIEGAND1;
#endif
}

int getWiegandOut_Handle()
{
#ifdef CFG_TEST_FROM_WIEGAND0
	return ITP_DEVICE_WIEGANDOUT0;
#else
	return ITP_DEVICE_WIEGANDOUT1;
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

void* getWiegandOutSource_Setting()
{
#ifdef CFG_TEST_FROM_WIEGAND0
	return &itpDeviceWiegandout0;
#else
	return &itpDeviceWiegandout1;
#endif
}

int getWiegandUart()
{
#ifdef CFG_TEST_FROM_WIEGAND0
	return ITH_UART4;
#else
	return ITH_UART5;
#endif
}

int getBitCount()
{
#ifdef CFG_TEST_FROM_WIEGAND0
	return WIEGAND0_BIT_COUNT;
#else
	return WIEGAND1_BIT_COUNT;
#endif
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
	int WIEGANDIN_HANDLE = getWiegandIn_Handle();
	int WIEGANDOUT_HANDLE = getWiegandOut_Handle();
	void* WIEGANDIN_SETTING = getWiegandIn_Setting();
	void* WIEGANDOUT_SETTING = getWiegandOutSource_Setting();
	int WIEGAND_UART = getWiegandUart();
	int WIEGAND_BITCOUNT = getBitCount();

	char cardid[] = { 0x30, 0xB4, 0x73 };	// Wiegand26: 3 byte, Wiegand34: 4 byte
	int idlen = sizeof(cardid);
	char cardidnum[17];
	char cardidnum_readout[17];

	/****************************************************************************/
	//Set wiegand in signal to uart rx
	itpRegisterDevice(WIEGANDIN_HANDLE, WIEGANDIN_SETTING);
	ioctl(WIEGANDIN_HANDLE, ITP_IOCTL_INIT, NULL);
	ioctl(WIEGANDIN_HANDLE, ITP_IOCTL_SET_BIT_COUNT, &WIEGAND_BITCOUNT);
	//ioctl(ITP_DEVICE_WIEGAND0, ITP_IOCTL_CUSTOM_CTL_ID0, NULL);
	ioctl(WIEGANDIN_HANDLE, ITP_IOCTL_ENABLE, NULL);

	/****************************************************************************/
	// Wiegand out signal from uart rx to gpio
	ioctl(WIEGANDOUT_HANDLE, ITP_IOCTL_INIT, NULL);
	itpRegisterDevice(WIEGANDOUT_HANDLE, WIEGANDOUT_SETTING);
	ioctl(WIEGANDOUT_HANDLE, ITP_IOCTL_SET_BIT_COUNT, &WIEGAND_BITCOUNT);
	ioctl(WIEGANDOUT_HANDLE, ITP_IOCTL_CUSTOM_CTL_ID0, O2I);
	ioctl(WIEGANDOUT_HANDLE, ITP_IOCTL_ENABLE, NULL);

	/****************************************************************************/
	// Build & notify real id number
	getTargetCardId(cardid, idlen, cardidnum);
	printf("Target Id Number: %s, idlen: %d\n", cardidnum, idlen);

	/****************************************************************************/
	// Try to readout (Header will auto added by wiegand in)
	//ithUartPutChar(ITH_UART4, 0xAA);
	//ithUartPutChar(ITH_UART4, 0x03);
	ithUartPutChar(WIEGAND_UART, cardid[0]);
	ithUartPutChar(WIEGAND_UART, cardid[1]);
	ithUartPutChar(WIEGAND_UART, cardid[2]);
	//ithUartPutChar(ITH_UART4, 0x0D);

	usleep(300000);
	read(WIEGANDIN_HANDLE, &cardidnum_readout, 0);	// Auto print result if readed

}