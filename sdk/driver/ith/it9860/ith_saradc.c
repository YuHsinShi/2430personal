/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * HAL SARADC functions.
 *
 * @author Paul Chao
 * @version 1.0
 */
#include "ith_cfg.h"

//=============================================================================
//                Constant Definition
//=============================================================================

//=============================================================================
//                Macro Definition
//=============================================================================

//=============================================================================
//                Structure Definition
//=============================================================================

//=============================================================================
//                Global Data Definition
//=============================================================================

//=============================================================================
//                Private Function Definition
//=============================================================================

//=============================================================================
//                Public Function Definition
//=============================================================================
void ithSARADCEnableClock(void)
{
    ithSetRegBitA(ITH_APB_CLK2_REG, ITH_EN_W25CLK_BIT);
    ithSetRegBitA(ITH_MEM_CLK2_REG, ITH_MCLK2_M15CLK_BIT);
}

void ithSARADCDisableClock(void)
{
    ithClearRegBitA(ITH_MEM_CLK2_REG, ITH_MCLK2_M15CLK_BIT);
    ithClearRegBitA(ITH_APB_CLK2_REG, ITH_EN_W25CLK_BIT);
}

void ithSARADCResetEngine(void)
{
    ithSetRegBitA(ITH_APB_CLK1_REG, ITH_SARADC_RST_BIT);
    ithDelay(1);
    ithClearRegBitA(ITH_APB_CLK1_REG, ITH_SARADC_RST_BIT);
}

