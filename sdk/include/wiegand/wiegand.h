#ifndef _WIEGAND_H
#define _WIEGAND_H

#include <sys/ioctl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include "ite/itp.h"

#define WIEGAND_API

#define WIEGAND_BASE    0xD1300000
#define WIEGAND_UART_BAUD 115200

#ifdef __cplusplus
extern "C" {
#endif

	typedef enum WIEGANDID_IN_TAG
	{
		wiegand_in_0 = 0x00,
		wiegand_in_1 = 0x10,
	}WIEGAND_IN_ID;

	WIEGAND_API unsigned int wiegand_get_uart_base(WIEGAND_IN_ID id);
	WIEGAND_API void init_wiegand_controller(WIEGAND_IN_ID id);
	WIEGAND_API void wiegand_controller_enable(WIEGAND_IN_ID id, int d0pin, int d1pin, int bitcnt);
	WIEGAND_API void wiegand_verify_enable(WIEGAND_IN_ID id, int enable);
	WIEGAND_API void wiegand_suspend(WIEGAND_IN_ID id);
	WIEGAND_API void wiegand_resume(WIEGAND_IN_ID id);

#ifdef __cplusplus
}
#endif

#endif
