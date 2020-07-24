/*
*  (C) Copyright 2014-2016 Shenzhen South Silicon Valley microelectronics co.,limited
*
*  All Rights Reserved
*/


#ifndef _SSV_EFUSE_H_
#define _SSV_EFUSE_H_

ssv_type_u32 read_chip_id(void);
void read_efuse_macaddr(ssv_type_u8* mcdr);

#endif // _SSV_EFUSE_H_
