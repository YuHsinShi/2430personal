/*
*  (C) Copyright 2014-2016 Shenzhen South Silicon Valley microelectronics co.,limited
*
*  All Rights Reserved
*/


#include <rtos.h>
#include <log.h>
#include <ssv_devinfo.h>
#include <ssv_dev.h>
#include "efuse.h"

ssv_type_u32 read_chip_id(void)
{
    return ssv_hal_read_chip_id();
}


void read_efuse_macaddr(ssv_type_u8* mcdr)
{
	ssv_hal_read_efuse_mac(mcdr);
}

