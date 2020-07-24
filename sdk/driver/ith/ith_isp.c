/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * HAL ISP functions.
 *
 * @author Jim Tan
 * @version 1.0
 */
#include "ith_cfg.h"

#if (CFG_CHIP_FAMILY == 970 || CFG_CHIP_FAMILY == 9860)

void ithIspEnableSharedClock(void)
{
    ithWriteRegMaskA(ITH_HOST_BASE + ITH_ISP_CLK_REG,
        (0x1 << ITH_EN_M4CLK_BIT) | (0x1 << ITH_EN_W13CLK_BIT),
        (0x1 << ITH_EN_M4CLK_BIT) | (0x1 << ITH_EN_W13CLK_BIT));
    ithSetRegBitA(ITH_HOST_BASE + ITH_ISP_CLK_REG, ITH_EN_DIV_ICLK_BIT);
}

void ithIspDisableSharedClock(void)
{
    ithWriteRegMaskA(ITH_HOST_BASE + ITH_ISP_CLK_REG,
        (0x0 << ITH_EN_M4CLK_BIT) | (0x0 << ITH_EN_W13CLK_BIT),
        (0x1 << ITH_EN_M4CLK_BIT) | (0x1 << ITH_EN_W13CLK_BIT));
    ithClearRegBitA(ITH_HOST_BASE + ITH_ISP_CLK_REG, ITH_EN_DIV_ICLK_BIT);
}

void ithIspEnableClock()
{
    ithWriteRegMaskA(ITH_HOST_BASE + ITH_ISP_CLK_REG,
        (0x1 << ITH_EN_DIV_ICLK_BIT) | (0x1 << ITH_EN_I0CLK_BIT) | (0x1 << ITH_EN_I1CLK_BIT) |
        (0x1 << ITH_EN_M4CLK_BIT) | (0x1 << ITH_EN_W13CLK_BIT),
        (0x1 << ITH_EN_DIV_ICLK_BIT) | (0x1 << ITH_EN_I0CLK_BIT) | (0x1 << ITH_EN_I1CLK_BIT) |
        (0x1 << ITH_EN_M4CLK_BIT) | (0x1 << ITH_EN_W13CLK_BIT));
}

void ithIspDisableClock()
{
    ithWriteRegMaskA(ITH_HOST_BASE + ITH_ISP_CLK_REG,
        (0x0 << ITH_EN_DIV_ICLK_BIT) | (0x0 << ITH_EN_I0CLK_BIT) | (0x0 << ITH_EN_I1CLK_BIT) |
        (0x0 << ITH_EN_M4CLK_BIT) | (0x0 << ITH_EN_W13CLK_BIT),
        (0x1 << ITH_EN_DIV_ICLK_BIT) | (0x1 << ITH_EN_I0CLK_BIT) | (0x1 << ITH_EN_I1CLK_BIT) |
        (0x1 << ITH_EN_M4CLK_BIT) | (0x1 << ITH_EN_W13CLK_BIT));
}

void ithIspSetClockSource(ITHClockSource src)
{
    ithClearRegBitA(ITH_HOST_BASE + ITH_ISP_CLK_REG, ITH_EN_DIV_ICLK_BIT);
    ithWriteRegMaskA(ITH_HOST_BASE + ITH_ISP_CLK_REG, src << ITH_ICLK_SRC_SEL_BIT, ITH_ICLK_SRC_SEL_MASK);
    ithSetRegBitA(ITH_HOST_BASE + ITH_ISP_CLK_REG, ITH_EN_DIV_ICLK_BIT);
}

void ithIspResetReg(ITHIspCore ispcore)
{
    if (ispcore == ITH_ISP_CORE0)
    {
        ithSetRegBitA(ITH_HOST_BASE + ITH_ISP_CLK_REG, ITH_ISP0_REG_RST_BIT);
        ithClearRegBitA(ITH_HOST_BASE + ITH_ISP_CLK_REG, ITH_ISP0_REG_RST_BIT);
    }
    else if (ispcore == ITH_ISP_CORE1)
    {
        ithSetRegBitA(ITH_HOST_BASE + ITH_ISP_CLK_REG, ITH_ISP1_REG_RST_BIT);
        ithClearRegBitA(ITH_HOST_BASE + ITH_ISP_CLK_REG, ITH_ISP1_REG_RST_BIT);
    }
}

void ithIspResetAllReg(void)
{
    ithSetRegBitA(ITH_HOST_BASE + ITH_ISP_CLK_REG, ITH_ISP_ALLREG_RST_BIT);
    ithClearRegBitA(ITH_HOST_BASE + ITH_ISP_CLK_REG, ITH_ISP_ALLREG_RST_BIT);
}

void ithIspQResetReg(void)
{
    ithSetRegBitA(ITH_HOST_BASE + ITH_ISP_CLK_REG, ITH_ISPQ_RST_BIT);
    ithClearRegBitA(ITH_HOST_BASE + ITH_ISP_CLK_REG, ITH_ISPQ_RST_BIT);
}

void ithIspResetEngine(ITHIspCore ispcore)
{
    if (ispcore == ITH_ISP_CORE0)
    {
        ithSetRegBitA(ITH_HOST_BASE + ITH_ISP_CLK_REG, ITH_ISP0_RST_BIT);
        ithClearRegBitA(ITH_HOST_BASE + ITH_ISP_CLK_REG, ITH_ISP0_RST_BIT);
    }
    else if (ispcore == ITH_ISP_CORE1)
    {
        ithSetRegBitA(ITH_HOST_BASE + ITH_ISP_CLK_REG, ITH_ISP1_RST_BIT);
        ithClearRegBitA(ITH_HOST_BASE + ITH_ISP_CLK_REG, ITH_ISP1_RST_BIT);
    }
}

void ithIspEnableTiling(ITHIspTilingMode mode, ITHIspTilingWidth width, ITHIspTilingPitch pitch)
{
    if (mode == ITH_ISP_TILING_READ)
    {
        ithWriteRegMaskA(ITH_MEM_ISP_TILING_READ_REG,
        (pitch << ITH_ISP_TILING_READ_PITCH_BIT) | (width << ITH_ISP_TILING_READ_WIDTH_BIT) | (0x1 << ITH_ISP_TILING_READ_EN_BIT),
        ITH_ISP_TILING_READ_PITCH_MASK | ITH_ISP_TILING_READ_WIDTH_MASK | ITH_ISP_TILING_READ_EN_MASK);
    }
    else if (mode == ITH_ISP_TILING_WRITE)
    {
        ithWriteRegMaskA(ITH_MEM_ISP_TILING_WRITE_REG,
        (pitch << ITH_ISP_TILING_WRITE_PITCH_BIT) | (width << ITH_ISP_TILING_WRITE_WIDTH_BIT) | (0x1 << ITH_ISP_TILING_WRITE_EN_BIT),
        ITH_ISP_TILING_WRITE_PITCH_MASK | ITH_ISP_TILING_WRITE_WIDTH_MASK | ITH_ISP_TILING_WRITE_EN_MASK);
    }
}

void ithIspDisableTiling(ITHIspTilingMode mode)
{
    if (mode == ITH_ISP_TILING_READ)
        ithClearRegBitA(ITH_MEM_ISP_TILING_READ_REG, ITH_ISP_TILING_READ_EN_BIT);
    else if (mode == ITH_ISP_TILING_WRITE)
        ithClearRegBitA(ITH_MEM_ISP_TILING_WRITE_REG, ITH_ISP_TILING_WRITE_EN_BIT);
}

#else

void ithIspEnableClock(void)
{
    ithWriteRegMaskH(ITH_ISP_CLK2_REG,
        (0x1 << ITH_EN_N5CLK_BIT) | (0x1 << ITH_EN_M5CLK_BIT) | (0x1 << ITH_EN_ICLK_BIT) | (0x1 << ITH_EN_I1CLK_BIT),
        (0x1 << ITH_EN_N5CLK_BIT) | (0x1 << ITH_EN_M5CLK_BIT) | (0x1 << ITH_EN_ICLK_BIT) | (0x1 << ITH_EN_I1CLK_BIT));
    ithSetRegBitH(ITH_ISP_CLK1_REG, ITH_EN_DIV_ICLK_BIT);
    ithSetRegBitH(ITH_EN_MMIO_REG, ITH_EN_ISP_MMIO_BIT);
}

void ithIspDisableClock(void)
{
    ithWriteRegMaskH(ITH_ISP_CLK2_REG,
        (0x0 << ITH_EN_M5CLK_BIT) | (0x0 << ITH_EN_ICLK_BIT) | (0x0 << ITH_EN_I1CLK_BIT),
        (0x1 << ITH_EN_M5CLK_BIT) | (0x1 << ITH_EN_ICLK_BIT) | (0x1 << ITH_EN_I1CLK_BIT));

    if ((ithReadRegH(ITH_CQ_CLK_REG) & (0x1 << ITH_EN_M3CLK_BIT)) == 0)
        ithClearRegBitH(ITH_ISP_CLK2_REG, ITH_EN_N5CLK_BIT);   // disable N5 clock safely

    ithClearRegBitH(ITH_ISP_CLK1_REG, ITH_EN_DIV_ICLK_BIT);
    ithClearRegBitH(ITH_EN_MMIO_REG, ITH_EN_ISP_MMIO_BIT);
}

void ithIspSetClockSource(ITHClockSource src)
{
    ithClearRegBitH(ITH_ISP_CLK1_REG, ITH_EN_DIV_ICLK_BIT);
    ithWriteRegMaskH(ITH_ISP_CLK1_REG, src << ITH_ICLK_SRC_SEL_BIT, ITH_ICLK_SRC_SEL_MASK);
    ithSetRegBitH(ITH_ISP_CLK1_REG, ITH_EN_DIV_ICLK_BIT);
}

void ithIspResetReg(void)
{
    ithSetRegBitH(ITH_ISP_CLK2_REG, ITH_ISP_REG_RST_BIT);
    ithClearRegBitH(ITH_ISP_CLK2_REG, ITH_ISP_REG_RST_BIT);
}

void ithIspQResetReg(void)
{
    ithSetRegBitH(ITH_ISP_CLK2_REG, ITH_ISPQ_RST_BIT);
    ithClearRegBitH(ITH_ISP_CLK2_REG, ITH_ISPQ_RST_BIT);
}

void ithIspResetEngine(void)
{
    ithSetRegBitH(ITH_ISP_CLK2_REG, ITH_ISP_RST_BIT);
    ithClearRegBitH(ITH_ISP_CLK2_REG, ITH_ISP_RST_BIT);
}

#endif

