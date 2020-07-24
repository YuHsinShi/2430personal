#ifndef _SSV6006_HW_REG_H_
#define _SSV6006_HW_REG_H_

#include <host_config.h>

#if(CONFIG_CHIP_ID==SSV6006B)
#include "regs/SSV6006B/ssv6006_reg.h"
#include "regs/SSV6006B/ssv6006_aux.h"	

#ifdef SSV6006_HAL_C
#include "regs/SSV6006B/ssv6006_configuration.h"
//#include "regs/SSV6006B/turismoB_rf_reg.h"
//#include "regs/SSV6006B/turismoB_wifi_phy_reg.h"
#endif	

#elif(CONFIG_CHIP_ID==SSV6006C)

#include "regs/SSV6006C/ssv6006_reg.h"
#include "regs/SSV6006C/ssv6006_aux.h"	

#ifdef SSV6006_HAL_C
#include "regs/SSV6006C/ssv6006_configuration.h"
//#include "regs/SSV6006C/turismoC_rf_reg.h"
//#include "regs/SSV6006C/turismoC_wifi_phy_reg.h"
#endif	

#endif

#endif

