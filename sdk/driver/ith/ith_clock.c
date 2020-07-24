/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * HAL clock functions.
 *
 * @author Jim Tan
 * @version 1.0
 */
#include "ith_cfg.h"

static unsigned int clkCpu = 0, clkMem = 0, clkBus = 0, riscCpu = 0;
static uint32_t ClkApb2Val, ClkAhbVal, ClkAxiVal, ClkMem1Val, ClkMem2Val, ClkRiscVal;
static uint32_t ClkCapVal, ClkDPVal;
static uint16_t clkGenRegs[(ITH_GEN_DFT1_REG - ITH_HOST_CLK1_REG + 2) / 2];
static uint16_t clkPllRegs[(ITH_PLL_GEN_SET_REG - ITH_PLL1_SET1_REG + 2) / 2];

#if (CFG_CHIP_FAMILY == 970) || (CFG_CHIP_FAMILY == 9860)
#define PLL1_1_SETTING1             (ITH_HOST_BASE + 0x0100)
#define PLL1_1_SETTING2             (ITH_HOST_BASE + 0x0104)
#define PLL_MODE_OFFSET             31
#define PLL_MODE_MASK               0x1
#define PLL_MUX_OFFSET              30
#define PLL_MUX_MASK                0x1
#define PLL_NUMERATOR_OFFSET        16
#define PLL_NUMERATOR_MASK          0x7F
#define PLL_POST_DIV_OFFSET         8
#define PLL_POST_DIV_MASK           0x7F
#define PLL_PRE_DIV_OFFSET          0
#define PLL_PRE_DIV_MASK            0x1F
#define PLL_SDM_FRACTION_OFFSET     0
#define PLL_SDM_FRACTION_MASK       0x7FFFF
#define PLL_SDK_FRACTION_SIGN_MASK  0x80000

#define AXICLK_REG                  (ITH_HOST_BASE + 0x000C)
#define AXICLK_SRC_SEL_OFFSET       12
#define AXICLK_SRC_SEL_MASK         0x7
#define AXICLK_RATIO_OFFSET         0
#define AXICLK_RATIO_MASK           0x3FF

#define AHBCLK_REG                  (ITH_HOST_BASE + 0x0018)
#define AHBCLK_SRC_SEL_OFFSET       12
#define AHBCLK_SRC_SEL_MASK         0x7
#define AHBCLK_RATIO_OFFSET         0
#define AHBCLK_RATIO_MASK           0x3FF

#define APBCLK_REG                  (ITH_HOST_BASE + 0x001C)
#define APBCLK_SRC_SEL_OFFSET       12
#define APBCLK_SRC_SEL_MASK         0x7
#define APBCLK_RATIO_OFFSET         0
#define APBCLK_RATIO_MASK           0x3FF

#define MEMCLK_REG                  (ITH_HOST_BASE + 0x0010)
#define MEMCLK_SRC_SEL_OFFSET       12
#define MEMCLK_SRC_SEL_MASK         0x7
#define MEMCLK_RATIO_OFFSET         0
#if (CFG_CHIP_FAMILY == 970) || (CFG_CHIP_FAMILY == 9860)
#define MEMCLK_ECO_OFFSET           9
#define MEMCLK_ECO_MASK             0x1
#define MEMCLK_ECO_RATIO_MASK       0x3
#define MEMCLK_RATIO_MASK           0x1FF
#else
#define MEMCLK_RATIO_MASK           0x3FF
#endif

#define DEFAULT_XIN_CLK             (12000000)
#define DEFAULT_RING_CLK            (200000)

typedef enum
{
    CLK_PLL1_OUTPUT1 = 0, ///< From PLL1 output1
    CLK_PLL1_OUTPUT2 = 1, ///< From PLL1 output2
    CLK_PLL2_OUTPUT1 = 2, ///< From PLL2 output1
    CLK_PLL2_OUTPUT2 = 3, ///< From PLL2 output2
    CLK_PLL3_OUTPUT1 = 4, ///< From PLL3 output1
    CLK_PLL3_OUTPUT2 = 5, ///< From PLL3 output2
    CLK_PLL3_OUTPUT3 = 6, ///< From PLL3 output3
    CLK_CKSYS        = 7, ///< From CKSYS (12MHz)
} OUTPUTCLK_SRC;

static uint32_t GetOutputPllClk(uint32_t outputPll)
{
    bool done = false;
    uint64_t srcClk = DEFAULT_XIN_CLK;

    while (!done) {
        uint32_t regSetting1Val = ithReadRegA(((outputPll - CLK_PLL1_OUTPUT1) << 3) + PLL1_1_SETTING1);
        uint32_t regSetting2Val = ithReadRegA(((outputPll - CLK_PLL1_OUTPUT1) << 3) + PLL1_1_SETTING2);
        int32_t nMode = ((regSetting1Val >> PLL_MODE_OFFSET) & PLL_MODE_MASK);
        int32_t clkFromPll = ((regSetting1Val >> PLL_MUX_OFFSET) & PLL_MUX_MASK);
        int32_t preDiv = ((regSetting1Val >> PLL_PRE_DIV_OFFSET) & PLL_PRE_DIV_MASK);
        int32_t postDiv = ((regSetting1Val >> PLL_POST_DIV_OFFSET) & PLL_POST_DIV_MASK);
        int32_t numerator = ((regSetting1Val >> PLL_NUMERATOR_OFFSET) & PLL_NUMERATOR_MASK);
        uint32_t newNumerator;

        if (nMode)
        {
            int32_t fractionalPart;
            int32_t signBit = ((regSetting2Val >> PLL_SDM_FRACTION_OFFSET) & PLL_SDK_FRACTION_SIGN_MASK);
            fractionalPart = ((regSetting2Val >> PLL_SDM_FRACTION_OFFSET) & PLL_SDM_FRACTION_MASK) / 1024;

            if (signBit)
            {
                fractionalPart = -(1024 - fractionalPart);
            }
            newNumerator = numerator * 1024 + fractionalPart;
        }

        switch (outputPll)
        {
            case CLK_PLL1_OUTPUT1:
                if (clkFromPll)
                {
                    outputPll = CLK_PLL1_OUTPUT2;
                }
                else
                {
                    done = true;
                }
                break;
            case CLK_PLL2_OUTPUT1:
                if (clkFromPll)
                {
                    outputPll = CLK_PLL2_OUTPUT2;
                }
                else
                {
                    done = true;
                }
                break;
            case CLK_PLL3_OUTPUT1:
                if (clkFromPll)
                {
                    outputPll = CLK_PLL3_OUTPUT2;
                }
                else
                {
                    done = true;
                }
                break;
            case CLK_PLL3_OUTPUT2:
                if (clkFromPll)
                {
                    outputPll = CLK_PLL3_OUTPUT3;
                }
                else
                {
                    done = true;
                }
                break;
            default:
                done = true;
                break;
        }

        if (preDiv == 0)
        {
            preDiv = 1;
        }

        if (postDiv == 0)
        {
            postDiv = 1;
        }

        if (nMode)
        {
            srcClk = ((srcClk / preDiv * newNumerator / postDiv) / 1024);
        }
        else
        {
            srcClk = ((srcClk / preDiv * numerator / postDiv));
        }
    }

    return (uint32_t)srcClk;
}

uint32_t GetSrcClk(uint32_t clkSrc)
{
    if (clkSrc == CLK_CKSYS)
    {
        return DEFAULT_XIN_CLK;
    }
    else
    {
        return GetOutputPllClk(clkSrc);
    }
}

uint32_t GetArmCpuClock(void)
{

    uint32_t src = ((ithReadRegA(AXICLK_REG) >> AXICLK_SRC_SEL_OFFSET) & AXICLK_SRC_SEL_MASK);
    uint32_t div = ((ithReadRegA(AXICLK_REG) >> AXICLK_RATIO_OFFSET) & AXICLK_RATIO_MASK);

    //000: From PLL1 output1 (default)
    //001: From PLL1 output2
    //010: From PLL2 output1
    //011: From PLL2 output2
    //100: From PLL3 output1
    //101: From PLL3 output2
    //111: From PLL3 output3
    //111: From CKSYS (12MHz)
    if (div == 0)
    {
        div = 1;
    }
    return GetSrcClk(src) / div;
}

uint32_t GetRiscCpuClock(void)
{
    uint32_t src = ((ithReadRegA(AHBCLK_REG) >> AHBCLK_SRC_SEL_OFFSET) & AHBCLK_SRC_SEL_MASK);
    uint32_t div = ((ithReadRegA(AHBCLK_REG) >> AHBCLK_RATIO_OFFSET) & AHBCLK_RATIO_MASK);

    //000: From PLL1 output1 (default)
    //001: From PLL1 output2
    //010: From PLL2 output1
    //011: From PLL2 output2
    //100: From PLL3 output1
    //101: From PLL3 output2
    //111: From PLL3 output3
    //111: From CKSYS (12MHz)
    if (div == 0)
    {
        div = 1;
    }
    return GetSrcClk(src) / div;
}

uint32_t GetBusClock(void)
{
    uint32_t src = ((ithReadRegA(APBCLK_REG) >> APBCLK_SRC_SEL_OFFSET) & APBCLK_SRC_SEL_MASK);
    uint32_t div = ((ithReadRegA(APBCLK_REG) >> APBCLK_RATIO_OFFSET) & APBCLK_RATIO_MASK);
    //000: From PLL1 output1 (default)
    //001: From PLL1 output2
    //010: From PLL2 output1
    //011: From PLL2 output2
    //100: From PLL3 output1
    //101: From PLL3 output2
    //111: From PLL3 output3
    //111: From CKSYS (12MHz)
    if (div == 0)
    {
        div = 1;
    }
    return GetSrcClk(src) / div;
}

uint32_t GetMemoryClock(void)
{
    uint32_t src = 0;
    uint32_t div = 0;
    uint32_t regVal = ithReadRegA(MEMCLK_REG);
    //000: From PLL1 output1 (default)
    //001: From PLL1 output2
    //010: From PLL2 output1
    //011: From PLL2 output2
    //100: From PLL3 output1
    //101: From PLL3 output2
    //111: From PLL3 output3
    //111: From CKSYS (12MHz)

    src = (regVal >> MEMCLK_SRC_SEL_OFFSET) & MEMCLK_SRC_SEL_MASK;
#if (CFG_CHIP_FAMILY == 970) || (CFG_CHIP_FAMILY == 9860)
    if ((regVal >> MEMCLK_ECO_OFFSET) & MEMCLK_ECO_MASK)
    {
        switch ((regVal >> MEMCLK_RATIO_OFFSET) & MEMCLK_ECO_RATIO_MASK)
        {
            case 0:
            {
                div = 2;
                break;
            }
            case 1:
            {
                div = 3;
                break;
            }
            case 2:
            default:
            {
                div = 4;
                break;
            }
        }
    }
    else
    {
        div = (regVal >> MEMCLK_RATIO_OFFSET) & MEMCLK_RATIO_MASK;
        if (div == 0)
        {
            div = 1;
        }
    }
#else
    uint32_t div = ((ithReadRegA(MEMCLK_REG) >> MEMCLK_RATIO_OFFSET) & MEMCLK_RATIO_MASK);
    if (div == 0)
    {
        div = 1;
    }
#endif
    return (GetSrcClk(src) / div);
}

#else
static unsigned int GetPllFreq(int n)
{
    unsigned int pll = 0;
    uint32_t pllBase = n * 0x10;
    uint32_t reg1 = ithReadRegH(pllBase + ITH_PLL1_SET1_REG);
    uint32_t sdmSel    = (reg1 & (1<<15)) ? 1 : 0;
    uint32_t sdmBypass = (reg1 & (1<<14)) ? 1 : 0;
    uint32_t sdmFix    = ((reg1 & (3<<12)) >> 12) + 3;
    uint32_t preDiv    = (reg1 & 0x1f);

    if (sdmSel)
    { // SDM divider
        uint32_t reg4   = ithReadRegH(pllBase + ITH_PLL1_SET4_REG);
        uint32_t sdm    = (reg4 & 0x7ff);
        uint32_t sdm_dv = (reg4 & (3<<12)) >> 12;

        if (sdm & (1<<10))
            sdm = sdm | 0xfffff800;

        switch(sdm_dv)
        {
        case 0:
            sdm += 16 * 1024;
            break;

        case 1:
            sdm += 17 * 1024;
            break;

        case 2:
            sdm += 18 * 1024;
            break;

        case 3:
            sdm += (uint32_t)(16.5f * 2048);
            break;
        }

        if (sdm_dv != 3)
            pll = (unsigned int)((float)(CFG_OSC_CLK / preDiv) * (sdmFix * sdm / 1024.0f));
        else
            pll = (unsigned int)((float)(CFG_OSC_CLK / preDiv) * (sdmFix * sdm / 2048.0f));

    }
    else
    {
        // fix divider
        uint32_t reg3 = ithReadRegH(pllBase + ITH_PLL1_SET3_REG);
        uint32_t num = reg3 & 0x3ff;
        pll = (unsigned int)((float)(CFG_OSC_CLK / preDiv) * num);
    }
    return pll;
}


static unsigned int GetPllFreqOut1(int n)
{
    uint32_t pllBase = n * 0x10;
    unsigned int srcclk = GetPllFreq(n);
    unsigned int clk = srcclk / (ithReadRegH(pllBase + ITH_PLL1_SET2_REG) & 0x7f);
    return clk;
}

static unsigned int GetPllFreqOut2(int n)
{
    uint32_t pllBase = n * 0x10;
    unsigned int srcclk = GetPllFreq(n);
    unsigned int clk = srcclk / ((ithReadRegH(pllBase + ITH_PLL1_SET2_REG) >> 8) & 0x7f);
    return clk;
}
#endif

void ithClockInit(void)
{
    clkCpu = clkMem = clkBus = riscCpu = 0;
}

void ithClockSleep(void)
{
	ClkAxiVal = ithReadRegA(ITH_AXI_CLK_REG);
	ClkMem2Val = ithReadRegA(ITH_MEM_CLK2_REG);
	ClkAhbVal = ithReadRegA(ITH_AHB_CLK_REG);
	ClkApb2Val = ithReadRegA(ITH_APB_CLK2_REG);
	ClkRiscVal = ithReadRegA(ITH_RISC_CLK_REG);
	ClkCapVal = ithReadRegA(ITH_HOST_BASE + ITH_CAP_CLK_REG);

	//AXI
	ithClearRegBitA(ITH_AXI_CLK_REG, ITH_AXICLK_A1CLK_BIT);
	//ithClearRegBitA(ITH_AXI_CLK_REG, ITH_AXICLK_A0CLK_BIT); //Mem control

	//MEM2
	//ithClearRegBitA(ITH_MEM_CLK2_REG, ITH_MCLK2_W11CLK_BIT); //Mem control
	ithClearRegBitA(ITH_MEM_CLK2_REG, ITH_MCLK2_M15CLK_BIT);

	//AHB
	ithClearRegBitA(ITH_AHB_CLK_REG, ITH_EN_N11CLK_BIT);
	ithClearRegBitA(ITH_AHB_CLK_REG, ITH_EN_N10CLK_BIT);

	//APB2
    ithClearRegBitA(ITH_APB_CLK2_REG, ITH_EN_W17CLK_BIT);
    ithClearRegBitA(ITH_APB_CLK2_REG, ITH_EN_W16CLK_BIT);
	ithClearRegBitA(ITH_APB_CLK2_REG, ITH_EN_W10CLK_BIT);
	ithClearRegBitA(ITH_APB_CLK2_REG, ITH_EN_W9CLK_BIT);
	ithClearRegBitA(ITH_APB_CLK2_REG, ITH_EN_W8CLK_BIT);
	ithClearRegBitA(ITH_APB_CLK2_REG, ITH_EN_W7CLK_BIT);
#if !defined(CFG_POWER_WAKEUP_UART)
	ithClearRegBitA(ITH_APB_CLK2_REG, ITH_EN_W6CLK_BIT);
#endif
#if !defined(CFG_POWER_WAKEUP_UART) && !defined(CFG_WATCHDOG_ENABLE)
	ithClearRegBitA(ITH_APB_CLK2_REG, ITH_EN_W5CLK_BIT);
#endif
	ithClearRegBitA(ITH_APB_CLK2_REG, ITH_EN_W4CLK_BIT);
	ithClearRegBitA(ITH_APB_CLK2_REG, ITH_EN_W3CLK_BIT);
	ithClearRegBitA(ITH_APB_CLK2_REG, ITH_EN_W2CLK_BIT);
	//ithClearRegBitA(ITH_APB_CLK2_REG, ITH_EN_W1CLK_BIT);  //GPIO

	//DP
	ClkDPVal = ithReadRegA(ITH_DPU_CLK_REG);
	ithClearRegBitA(ITH_DPU_CLK_REG, ITH_EN_W19CLK_BIT);
	ithClearRegBitA(ITH_DPU_CLK_REG, ITH_EN_M14CLK_BIT);
	ithClearRegBitA(ITH_DPU_CLK_REG, ITH_EN_DPCLK_BIT);
	ithClearRegBitA(ITH_DPU_CLK_REG, ITH_EN_DIV_DPCLK_BIT);

	//RISC
	ithClearRegBitA(ITH_RISC_CLK_REG, ITH_EN_N8CLK_BIT);
	ithClearRegBitA(ITH_RISC_CLK_REG, ITH_EN_N4CLK_BIT);

	/************************************************************/
	/* Workaround: Before power down Video-domain(H264&VP&CAP), */
	/*             MUST close domain's CLK.                     */
	/************************************************************/
	// Disable CAP CLK
	ithClearRegBitA(ITH_HOST_BASE + ITH_CAP_CLK_REG, ITH_EN_M12CLK_BIT);
	ithClearRegBitA(ITH_HOST_BASE + ITH_CAP_CLK_REG, ITH_EN_W18CLK_BIT);
	ithClearRegBitA(ITH_HOST_BASE + ITH_CAP_CLK_REG, ITH_EN_DIV_CAPCLK_BIT);
	// Disable VP CLK, use by ithIspDisableClock()
	// Disable H264 CLK, use by ithVideoDisableClock()


}

void ithClockWakeup(void)
{
	// restore original register's values
	ithWriteRegA(ITH_AXI_CLK_REG, ClkAxiVal);
	ithWriteRegA(ITH_MEM_CLK2_REG, ClkMem2Val);
	ithWriteRegA(ITH_AHB_CLK_REG, ClkAhbVal);
	ithWriteRegA(ITH_APB_CLK2_REG, ClkApb2Val);
	ithWriteRegA(ITH_DPU_CLK_REG, ClkDPVal);
	ithWriteRegA(ITH_RISC_CLK_REG, ClkRiscVal);

	/************************************************************/
	/* Workaround: After power up Video-domain(H264&VP&CAP)     */
	/*             Follow the following step:                   */
	/*             1.Reset domain's engine and register         */
	/*             2.Eanble domain's CLK                        */
	/*             To solve that VP&CAP don't fire, but they    */
	/*             still access memory.                         */
	/************************************************************/
	// Reset VP
	ithSetRegBitA(ITH_HOST_BASE + ITH_ISP_CLK_REG, ITH_ISPQ_RST_BIT);
	ithSetRegBitA(ITH_HOST_BASE + ITH_ISP_CLK_REG, ITH_ISP0_REG_RST_BIT);
	ithSetRegBitA(ITH_HOST_BASE + ITH_ISP_CLK_REG, ITH_ISP0_RST_BIT);
	ithSetRegBitA(ITH_HOST_BASE + ITH_ISP_CLK_REG, ITH_ISP_ALLREG_RST_BIT);
	ithSetRegBitA(ITH_HOST_BASE + ITH_ISP_CLK_REG, ITH_ISP1_REG_RST_BIT);
	ithSetRegBitA(ITH_HOST_BASE + ITH_ISP_CLK_REG, ITH_ISP1_RST_BIT);
	// Reset H264
	ithSetRegBitA(ITH_HOST_BASE + ITH_VIDEO_CLK1_REG, ITH_VIDEO_RST_BIT);
	// Reset CAP
	ithSetRegBitA(ITH_HOST_BASE + ITH_CAP_CLK_REG, ITH_EN_CAPC_RST_BIT);
	ithDelay(200);
	ithClearRegBitA(ITH_HOST_BASE + ITH_ISP_CLK_REG, ITH_ISPQ_RST_BIT);
	ithClearRegBitA(ITH_HOST_BASE + ITH_ISP_CLK_REG, ITH_ISP0_REG_RST_BIT);
	ithClearRegBitA(ITH_HOST_BASE + ITH_ISP_CLK_REG, ITH_ISP0_RST_BIT);
	ithClearRegBitA(ITH_HOST_BASE + ITH_ISP_CLK_REG, ITH_ISP_ALLREG_RST_BIT);
	ithClearRegBitA(ITH_HOST_BASE + ITH_ISP_CLK_REG, ITH_ISP1_REG_RST_BIT);
	ithClearRegBitA(ITH_HOST_BASE + ITH_ISP_CLK_REG, ITH_ISP1_RST_BIT);

	ithClearRegBitA(ITH_HOST_BASE + ITH_VIDEO_CLK1_REG, ITH_VIDEO_RST_BIT);

	ithClearRegBitA(ITH_HOST_BASE + ITH_CAP_CLK_REG, ITH_EN_CAPC_RST_BIT);
	// Enable VP CLK, use by ithIspEnableClock()
	// Enable H264 CLK, use by ithVideoEnableClock()
	// Restore CAP CLK
	ithWriteRegA(ITH_HOST_BASE + ITH_CAP_CLK_REG, ClkCapVal);
}

void ithClockSuspend(void)
{
    int i;

    for (i = 0; i < ITH_COUNT_OF(clkGenRegs); i++)
        clkGenRegs[i] = ithReadRegH(ITH_HOST_CLK1_REG + i * 2);

    for (i = 0; i < ITH_COUNT_OF(clkPllRegs); i++)
        clkPllRegs[i] = ithReadRegH(ITH_PLL1_SET1_REG + i * 2);
}

void ithClockResume(void)
{
    int i;

    for (i = 0; i < ITH_COUNT_OF(clkGenRegs); i++)
        ithWriteRegH(ITH_HOST_CLK1_REG + i * 2, clkGenRegs[i]);

    for (i = 0; i < ITH_COUNT_OF(clkPllRegs); i++)
        ithWriteRegH(ITH_PLL1_SET1_REG + i * 2, clkPllRegs[i]);
}

/*
 * Get CPU clock in Hz
 */
unsigned int ithGetCpuClock(void)
{
#ifdef CFG_ITH_FPGA
    return CFG_ITH_FPGA_CLK_CPU;
#elif defined(_WIN32)
	return 100 * 1024 * 1024;
#else
#if (CFG_CHIP_FAMILY == 970) || (CFG_CHIP_FAMILY == 9860)
    if (clkCpu == 0)
    {
        clkCpu = GetArmCpuClock();
    }
#else
    if (clkCpu == 0)
    {
    #if defined(__SM32__)
        unsigned int src = (ithReadRegH(ITH_AHB_CLK1_REG) & ITH_NCLK_SRC_SEL_MASK) >> ITH_NCLK_SRC_SEL_BIT;
        unsigned int div = (ithReadRegH(ITH_AHB_CLK1_REG) & ITH_NCLK_RATIO_MASK) >> ITH_NCLK_RATIO_BIT;
    #elif defined(__arm__)
        unsigned int src = (ithReadRegH(ITH_ARM_CLK1_REG) & ITH_FCLK_SRC_SEL_MASK) >> ITH_FCLK_SRC_SEL_BIT;
        unsigned int div = (ithReadRegH(ITH_ARM_CLK1_REG) & ITH_FCLK_RATIO_MASK) >> ITH_FCLK_RATIO_BIT;
    #elif defined(__riscv)
        unsigned int src = 0; // TODO: RISCV
        unsigned int div = 0; // TODO: RISCV
    #warning "the clock source of RISCV is not yet defined."
    #else
    #error "No CPU type defined"
    #endif
        unsigned int clk;

        switch (src)
        {
        case 0x1: // From PLL1 output2
            clk = GetPllFreqOut2(0);
            break;

        case 0x2: // From PLL2 output1
            clk = GetPllFreqOut1(1);
            break;

        case 0x3: // From PLL2 output2
            clk = GetPllFreqOut2(1);
            break;

        case 0x4: // From PLL3 output1
            clk = GetPllFreqOut1(2);
            break;

        case 0x5: // From PLL3 output2
            clk = GetPllFreqOut2(2);
            break;

        case 0x6: // From CKSYS (12MHz/30MHz)
            clk = CFG_OSC_CLK;
            break;

        case 0x7: // From Ring OSC (200KHz)
            clk = 200000;
            break;

        case 0x00: // From PLL1 output1 (default)
        default:
            clk = GetPllFreqOut1(0);
            break;
        }
        clkCpu = clk / (div + 1);
    }
#endif
    return clkCpu;
#endif // CFG_ITH_FPGA
}

/*
 * Get RISC CPU clock in Hz
 */
unsigned int ithGetRiscCpuClock(void)
{
#ifdef CFG_ITH_FPGA
    return CFG_ITH_FPGA_CLK_CPU;
#elif defined(_WIN32)
	return 100 * 1024 * 1024;
#else
#if (CFG_CHIP_FAMILY == 970) || (CFG_CHIP_FAMILY == 9860)
    if (riscCpu == 0)
    {
        riscCpu = GetRiscCpuClock();
    }
#else
    if (riscCpu == 0)
    {
        unsigned int src = (ithReadRegH(ITH_AHB_CLK1_REG) & ITH_NCLK_SRC_SEL_MASK) >> ITH_NCLK_SRC_SEL_BIT;
        unsigned int div = (ithReadRegH(ITH_AHB_CLK1_REG) & ITH_NCLK_RATIO_MASK) >> ITH_NCLK_RATIO_BIT;
        unsigned int clk;
        switch (src)
        {
        case 0x1: // From PLL1 output2
            clk = GetPllFreqOut2(0);
            break;

        case 0x2: // From PLL2 output1
            clk = GetPllFreqOut1(1);
            break;

        case 0x3: // From PLL2 output2
            clk = GetPllFreqOut2(1);
            break;

        case 0x4: // From PLL3 output1
            clk = GetPllFreqOut1(2);
            break;

        case 0x5: // From PLL3 output2
            clk = GetPllFreqOut2(2);
            break;

        case 0x6: // From CKSYS (12MHz/30MHz)
            clk = CFG_OSC_CLK;
            break;

        case 0x7: // From Ring OSC (200KHz)
            clk = 200000;
            break;

        case 0x00: // From PLL1 output1 (default)
        default:
            clk = GetPllFreqOut1(0);
            break;
        }
        riscCpu = clk / (div + 1);
    }
#endif
    return riscCpu;
#endif // CFG_ITH_FPGA
}

/*
 * Get memory clock in Hz
 */
unsigned int ithGetMemClock(void)
{
#ifdef CFG_ITH_FPGA
    return CFG_ITH_FPGA_CLK_MEM;

#else
#if (CFG_CHIP_FAMILY == 970) || (CFG_CHIP_FAMILY == 9860)
    if (clkMem == 0)
    {
        clkMem = GetMemoryClock();
    }
#else

    if (clkMem == 0)
    {
        unsigned int src = (ithReadRegH(ITH_MEM_CLK1_REG) & ITH_MCLK_SRC_SEL_MASK) >> ITH_MCLK_SRC_SEL_BIT;
        unsigned int div = (ithReadRegH(ITH_MEM_CLK1_REG) & ITH_MCLK_RATIO_MASK) >> ITH_MCLK_RATIO_BIT;
        unsigned int clk;

        switch (src)
        {
        case 0x1: // From PLL1 output2
            clk = GetPllFreqOut2(0);
            break;

        case 0x2: // From PLL2 output1
            clk = GetPllFreqOut1(1);
            break;

        case 0x3: // From PLL2 output2
            clk = GetPllFreqOut2(1);
            break;

        case 0x4: // From PLL3 output1
            clk = GetPllFreqOut1(2);
            break;

        case 0x5: // From PLL3 output2
            clk = GetPllFreqOut2(2);
            break;

        case 0x6: // From CKSYS (12MHz/30MHz)
            clk = CFG_OSC_CLK;
            break;

        case 0x7: // From Ring OSC (200KHz)
            clk = 200000;
            break;

        case 0x00: // From PLL1 output1 (default)
        default:
            clk = GetPllFreqOut1(0);
            break;
        }
        clkMem = clk / (div + 1);
    }
#endif
    return clkMem;
#endif // CFG_ITH_FPGA
}

/*
 * Get bus clock in Hz
 */
unsigned int ithGetBusClock(void)
{
#ifdef CFG_ITH_FPGA
    return CFG_ITH_FPGA_CLK_BUS;

#else
#if (CFG_CHIP_FAMILY == 970) || (CFG_CHIP_FAMILY == 9860)
    if (clkBus == 0)
    {
        clkBus = GetBusClock();
    }
#else
    if (clkBus == 0)
    {
        unsigned int src = (ithReadRegH(ITH_APB_CLK1_REG) & ITH_WCLK_SRC_SEL_MASK) >> ITH_WCLK_SRC_SEL_BIT;
        unsigned int div = (ithReadRegH(ITH_APB_CLK1_REG) & ITH_WCLK_RATIO_MASK) >> ITH_WCLK_RATIO_BIT;
        unsigned int clk;

        switch (src)
        {
        case 0x1: // From PLL1 output2
            clk = GetPllFreqOut2(0);
            break;

        case 0x2: // From PLL2 output1
            clk = GetPllFreqOut1(1);
            break;

        case 0x3: // From PLL2 output2
            clk = GetPllFreqOut2(1);
            break;

        case 0x4: // From PLL3 output1
            clk = GetPllFreqOut1(2);
            break;

        case 0x5: // From PLL3 output2
            clk = GetPllFreqOut2(2);
            break;

        case 0x6: // From CKSYS (12MHz/30MHz)
            clk = CFG_OSC_CLK;
            break;

        case 0x7: // From Ring OSC (200KHz)
            clk = 200000;
            break;

        case 0x00: // From PLL1 output1 (default)
        default:
            clk = GetPllFreqOut1(0);
            break;
        }
        clkBus = clk / (div + 1);
    }
#endif
    return clkBus;
#endif // CFG_ITH_FPGA
}

/*
 * Get NCLK in Hz
 */
unsigned int ithGetNclk(void)
{
    uint32_t src = ((ithReadRegA(AHBCLK_REG) >> AHBCLK_SRC_SEL_OFFSET) & AHBCLK_SRC_SEL_MASK);
    uint32_t div = ((ithReadRegA(AHBCLK_REG) >> AHBCLK_RATIO_OFFSET) & AHBCLK_RATIO_MASK);

    //000: From PLL1 output1 (default)
    //001: From PLL1 output2
    //010: From PLL2 output1
    //011: From PLL2 output2
    //100: From PLL3 output1
    //101: From PLL3 output2
    //111: From PLL3 output3
    //111: From CKSYS (12MHz)
    if (div == 0)
    {
        div = 1;
    }
    return GetSrcClk(src) / div;
}

/*
 * Get A0CLK ~ A4CLK in Hz
 */
unsigned int ithGetA0_4clk(void)
{
    return GetArmCpuClock() / 2;
}

void ithClockSetSource(ITHClock clk, ITHClockSource src)
{
    ithWriteRegMaskH(clk,
        (0x1 << ITH_MCLK_UPD_BIT) | (src << ITH_MCLK_SRC_SEL_BIT),
        ITH_MCLK_UPD_MASK | ITH_MCLK_SRC_SEL_MASK);

    //ithClearRegBitH(clk, ITH_MCLK_UPD_BIT);
    ithClockInit();
}

void ithClockSetRatio(ITHClock clk, unsigned int ratio)
{
    ithWriteRegMaskA(clk,
        (0x1 << ITH_AXICLK_UPD_BIT) | (ratio << ITH_AXICLK_RATIO_BIT),
        ITH_AXICLK_UPD_MASK | ITH_AXICLK_RATIO_MASK);

    //ithClearRegBitH(clk, ITH_MCLK_UPD_BIT);
    ithClockInit();
}

void ithClockEnablePll(ITHPll pll)
{
    ithWriteRegMaskH(ITH_PLL1_SET3_REG + pll * 0x10,
        (0x0 << ITH_PLL1_PWR_STATE_POS) | (0x1 << ITH_PLL1_UPDATE_BIT),
        ITH_PLL1_PWR_STATE_MSK | ITH_PLL1_UPDATE_MASK);

    //ithClearRegBitH(pll, ITH_PLL1_UPDATE_BIT);
}

void ithClockDisablePll(ITHPll pll)
{
    ithWriteRegMaskH(ITH_PLL1_SET3_REG + pll * 0x10,
        (0x1 << ITH_PLL1_PWR_STATE_POS) | (0x1 << ITH_PLL1_UPDATE_BIT),
        ITH_PLL1_PWR_STATE_MSK | ITH_PLL1_UPDATE_MASK);

    //ithClearRegBitH(pll, ITH_PLL1_UPDATE_BIT);
}

void ithSetSpreadSpectrum(ITHPll pll, ITHSpreadSpectrumMode mode, uint32_t width, uint32_t freq)
{
    #define DIV_ROUND(a,b) (((a) + ((b)/2)) / (b))

    int sp_stepy, sp_stepx, sp_level, sp_n, n1, n2, SDM_Sel, sdm, sdm_dv;
    uint32_t PLL_BASE, reg1, reg4, reg5, reg6;

    PLL_BASE = pll * 0x10;
    reg1 = ithReadRegH(PLL_BASE + ITH_PLL1_SET1_REG);
    SDM_Sel = (reg1 & (1<<15)) ? 1 : 0;
    if (!SDM_Sel)
    {
        LOG_ERR "it dose not support spread spectrum on non-SDM PLL; reg 0x%X=0x%X\n",
            PLL_BASE + ITH_PLL1_SET1_REG, reg1
        LOG_END
        return;
    }

    reg4   = ithReadRegH(PLL_BASE + ITH_PLL1_SET4_REG);
    sdm    = (reg4 & 0x7FF);
    sdm_dv = (reg4 & (3<<12)) >> 12;
    switch(sdm_dv)
    {
        case 0: sdm += (int)(16 * 1024); break;
        case 1: sdm += (int)(17 * 1024); break;
        case 2: sdm += (int)(18 * 1024); break;
        case 3: sdm += (int)(16.5f * 2048); break;
    }

    switch(mode)
    {
        case 0: sp_n = 2; break; // up-spread
        case 1: sp_n = 2; break; // down-spread
        case 2: sp_n = 4; break; // center-spread
        default:
            LOG_ERR "unknown mode of spread spectrum\n" LOG_END
            return;
    }

    n1 = DIV_ROUND(width * sdm, 1000);
    n2 = DIV_ROUND(CFG_OSC_CLK, freq * sp_n);

    // search step x and level by step y.
    for(sp_stepy=1; sp_stepy<=63; sp_stepy++)
    {
        sp_stepx = 0;
        sp_level = DIV_ROUND(n1, sp_stepy);
        if (sp_level < 2 || sp_level > 511) continue;

        sp_stepx = DIV_ROUND(n2, sp_level) - 1;
        if (sp_stepx < 0 || sp_stepx > 63) continue;

        break;
    }

    // Is it out of range on step y?
    if (sp_stepy >= 63)
    {
        LOG_ERR "setting spread spectrum fails\n" LOG_END
        return;
    }

    // setting step x, step y and level
    reg5 = sp_level;
    reg6 = (1 << 15) + (mode << 12) + (sp_stepx << 6) + sp_stepy;
    ithWriteRegMaskH(PLL_BASE + ITH_PLL1_SET6_REG, (0 << 15), (1 << 15));
    ithWriteRegH(PLL_BASE + ITH_PLL1_SET5_REG, reg5);
    ithWriteRegH(PLL_BASE + ITH_PLL1_SET6_REG, reg6);

    LOG_INFO "PLL %d spread spectrum: modulation width=%4.2f%%, modulation frequency=%d\n", pll, (float)sp_stepy*sp_level*100.0f/sdm, CFG_OSC_CLK/(sp_n*sp_level*(sp_stepx+1)) LOG_END
}

void ithClockStats(void)
{
#if defined(CFG_ITH_FPGA) || (CFG_CHIP_FAMILY == 970) || (CFG_CHIP_FAMILY == 9860)
    PRINTF("CLK: cpu=%lu hz,mem=%lu hz,bus=%lu hz, risc:%lu hz\r\n", ithGetCpuClock(), ithGetMemClock(), ithGetBusClock(), ithGetRiscCpuClock());
#else
    PRINTF("CLK: cpu=%lu,mem=%lu,bus=%lu,pll1=%lu,pll2=%lu,pll3=%lu\r\n",
        ithGetCpuClock(), ithGetMemClock(), ithGetBusClock(), GetPllFreq(0), GetPllFreq(1), GetPllFreq(2));
#endif // CFG_ITH_FPGA

    //ithPrintRegH(ITH_HTRAP_REG, ITH_OPVG_CLK3_REG - ITH_HTRAP_REG + sizeof (uint16_t));
    //ithPrintRegH(ITH_PLL1_SET1_REG, ITH_PLL3_SET3_REG - ITH_PLL1_SET1_REG + sizeof (uint16_t));
}
