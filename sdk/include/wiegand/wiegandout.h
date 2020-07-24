#ifndef _WIEGANDOUT_H
#define _WIEGANDOUT_H

#include <sys/ioctl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include "ite/itp.h"

#define WIEGANDOUT_API

#ifdef __cplusplus
extern "C" {
#endif

	typedef enum WIEGAND_OUT_ID_TAG
	{
		wiegand_out_0 = 0x30,
		wiegand_out_1 = 0x40,
	}WIEGAND_OUT_ID;

	typedef enum WIEGAND_LOOPBACK_DIRECTION_TAG
	{
		I2O,
		O2I,
		BYPASS
	}WIEGAND_LOOPBACK_DIRECTION;

	typedef enum WIEGAND_BIT_ORDER_TAG
	{
		MSB = 0,
		LSB = 1
	}WIEGAND_OUT_BIT_ORDER;

	WIEGANDOUT_API void init_wiegandout_controller(WIEGAND_OUT_ID id);
	WIEGANDOUT_API void wiegandout_set_bitcnt(WIEGAND_OUT_ID id, int bitcnt);
	WIEGANDOUT_API void wiegandout_set_loopback(WIEGAND_OUT_ID id, WIEGAND_LOOPBACK_DIRECTION dir);
	WIEGANDOUT_API void wiegandout_set_bit_order(WIEGAND_OUT_ID id, WIEGAND_OUT_BIT_ORDER order);
	WIEGANDOUT_API void wiegandout_controller_enable(WIEGAND_OUT_ID id, int bitcnt, WIEGAND_LOOPBACK_DIRECTION dir, WIEGAND_OUT_BIT_ORDER order);
	

#ifdef __cplusplus
}
#endif

#endif
