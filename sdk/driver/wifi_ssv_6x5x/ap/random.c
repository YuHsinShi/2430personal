/*
*  (C) Copyright 2014-2016 Shenzhen South Silicon Valley microelectronics co.,limited
*
*  All Rights Reserved
*/


//#include "utils/includes.h"
//#ifdef __linux__
//#include <fcntl.h>
//#endif /* __linux__ */

#include <ssv_types.h>
#include "crypto/sha1.h"
#include "random.h"
#include "wpa_debug.h"

#if (AP_MODE_ENABLE == 1)

//#define os_get_random OS_Random

int os_get_random(unsigned char *buf, int len)
{
    ssv_type_u16 count = (ssv_type_u32)len >> 2;
    ssv_type_u32 tmp;
    ssv_type_u32 *des = (ssv_type_u32 *)buf;
    
    tmp = OS_Random(); //The value is always zero in the first time because this action is for updating seed.
    while (count)
    {
        tmp = OS_Random();
        OS_MemCPY((void*)des, (void*)&tmp, 4);
        des++;
        count--;
    }
    return 0;
}
#endif
