/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * HAL UIEnc functions.
 *
 * @author Jim Tan
 * @version 1.0
 */
#include "ith_cfg.h"

void ithUiEncEnableClock(void)
{
    // enable clock
#if (CFG_CHIP_FAMILY == 970)
    ithSetRegBitA(ITH_HOST_BASE + ITH_UIENC_CLK_REG, ITH_EN_M16CLK_BIT);
#else
    ithSetRegBitH(ITH_UIENC_CLK_REG, ITH_EN_M14CLK_BIT);
#endif
}

void ithUiEncDisableClock(void)
{
    // disable clock
#if (CFG_CHIP_FAMILY == 970)
    ithClearRegBitA(ITH_HOST_BASE + ITH_UIENC_CLK_REG, ITH_EN_M16CLK_BIT);
#else
    ithClearRegBitH(ITH_UIENC_CLK_REG, ITH_EN_M14CLK_BIT);
#endif
}

void ithUiEncResetReg(void)
{

}

void ithUiEncResetEngine(void)
{
#if (CFG_CHIP_FAMILY == 970)
    ithSetRegBitA(ITH_HOST_BASE + ITH_UIENC_CLK_REG, ITH_EN_UIENC_RST_BIT);
    ithClearRegBitA(ITH_HOST_BASE + ITH_UIENC_CLK_REG, ITH_EN_UIENC_RST_BIT);
#else
    ithSetRegBitH(ITH_UIENC_CLK_REG, ITH_EN_UIENC_RST_BIT);
    ithClearRegBitH(ITH_UIENC_CLK_REG, ITH_EN_UIENC_RST_BIT);
#endif
}
