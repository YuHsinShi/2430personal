/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * Castor3 keypad module.
 *
 * @author Jim Tan
 * @version 1.0
 */
#include "../itp_cfg.h"

#ifdef	CFG_TOUCH_KEY_NUM
	#if	(CFG_TOUCH_KEY_NUM > 32)
		#error "unsupport TOUCH_KEY_NUM over 32"
	#endif
static const unsigned int gkpTotalNumber = CFG_TOUCH_KEY_NUM;
#else
	#error "Must define TOUCH_KEY_NUM"
#endif

static unsigned int ts_read_kp_default(void)
{
	return 0;
}
unsigned int (*ts_read_kp_callback)(void) = ts_read_kp_default;

int itpKeypadProbe(void)
{
    unsigned int i;
    uint32_t value = 0;
    
    value = ts_read_kp_callback();

    if(!value)	return -1;

    for (i = 0; i < gkpTotalNumber; i++)  
    {
        if ( ((value>>i) & 0x01 ) == 1 )
        {
        	//printf("send KP:%x\n",i);
        	return i;
        }
    }
    return -1;
}

void itpKeypadInit(void)
{
	//don't care if the touch driver ready
	//gTpKeypadValue will be 0 if touch driver is not ready	
}

int itpKeypadGetMaxLevel(void)
{
    return gkpTotalNumber;
}