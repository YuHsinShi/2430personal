#include "ite/ith.h"
#include "saradc/saradc_reg.h"
#include "saradc/saradc_hw.h"

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
//=============================================================================
/**
 * Fire SARADC Engine
 */
//=============================================================================
void
SARADC_DriverFire_Reg(
    void)
{
    ithWriteRegMaskA(SARADC_REG_CR0, (0x1 & SARADC_BIT_DRIVER_FIRE_EN) << SARADC_SHT_DRIVER_FIRE_EN,
            SARADC_BIT_DRIVER_FIRE_EN << SARADC_SHT_DRIVER_FIRE_EN);

    ithWriteRegMaskA(SARADC_REG_CR0, (0x1 & SARADC_BIT_PHY_FIRE_EN) << SARADC_SHT_PHY_FIRE_EN,
            SARADC_BIT_PHY_FIRE_EN << SARADC_SHT_PHY_FIRE_EN);
}

//=============================================================================
/**
 * Stop SARADC Engine
 */
//=============================================================================
void
SARADC_DriverStop_Reg(
    void)
{
    ithWriteRegMaskA(SARADC_REG_CR0, (0x0 & SARADC_BIT_PHY_FIRE_EN) << SARADC_SHT_PHY_FIRE_EN,
            SARADC_BIT_PHY_FIRE_EN << SARADC_SHT_PHY_FIRE_EN);

    ithWriteRegMaskA(SARADC_REG_CR0, (0x0 & SARADC_BIT_DRIVER_FIRE_EN) << SARADC_SHT_DRIVER_FIRE_EN,
            SARADC_BIT_DRIVER_FIRE_EN << SARADC_SHT_DRIVER_FIRE_EN);
}

//=============================================================================
/**
 * Set Clock Divider of SARADC
 */
//=============================================================================
void
SARADC_SetCLKDIV_Reg(
    SARADC_CLK_DIV divider)
{
    ithWriteRegMaskA(SARADC_REG_CR0, (divider & SARADC_BIT_CLK_DIV) << SARADC_SHT_CLK_DIV,
            SARADC_BIT_CLK_DIV << SARADC_SHT_CLK_DIV);
}

//=============================================================================
/**
 * Set the Average Mode of SARADC
 */
//=============================================================================
void
SARADC_SetModeAVG_Reg(
    SARADC_MODE_AVG modeAVG)
{
    ithWriteRegMaskA(SARADC_REG_CR0, (modeAVG & SARADC_BIT_MODE_AVG) << SARADC_SHT_MODE_AVG,
            SARADC_BIT_MODE_AVG << SARADC_SHT_MODE_AVG);
}

//=============================================================================
/**
 * Set the Store Mode of SARADC
 */
//=============================================================================
void
SARADC_SetModeStore_Reg(
    SARADC_MODE_STORE modeStore)
{
    if (modeStore == SARADC_MODE_STORE_DISABLE)
    {
        ithWriteRegMaskA(SARADC_REG_CR0, (0x0 & SARADC_BIT_MODE_STORE_EN) << SARADC_SHT_MODE_STORE_EN,
                SARADC_BIT_MODE_STORE_EN << SARADC_SHT_MODE_STORE_EN);
    }
    else
    {
        ithWriteRegMaskA(SARADC_REG_CR0, (0x1 & SARADC_BIT_MODE_STORE_EN) << SARADC_SHT_MODE_STORE_EN,
                SARADC_BIT_MODE_STORE_EN << SARADC_SHT_MODE_STORE_EN);

        if (modeStore == SARADC_MODE_STORE_RAW_ENABLE)
            ithWriteRegMaskA(SARADC_REG_CR0, (0x0 & SARADC_BIT_MODE_STORE_TYPE) << SARADC_SHT_MODE_STORE_TYPE,
                    SARADC_BIT_MODE_STORE_TYPE << SARADC_SHT_MODE_STORE_TYPE);
        else
            ithWriteRegMaskA(SARADC_REG_CR0, (0x1 & SARADC_BIT_MODE_STORE_TYPE) << SARADC_SHT_MODE_STORE_TYPE,
                    SARADC_BIT_MODE_STORE_TYPE << SARADC_SHT_MODE_STORE_TYPE);
    }
}

//=============================================================================
/**
 * Set GAIN Driving of SARADC
 */
//=============================================================================
void
SARADC_SetGAIN_Reg(
    SARADC_AMPLIFY_GAIN driving)
{
    ithWriteRegMaskA(SARADC_REG_CR0, (driving & SARADC_BIT_GAIN_SETTING) << SARADC_SHT_GAIN_SETTING,
            SARADC_BIT_GAIN_SETTING << SARADC_SHT_GAIN_SETTING);
}

//=============================================================================
/**
 * Set Write Buffer Size of SARADC
 */
//=============================================================================
void
SARADC_SetWBSize_Reg(
    uint16_t wbSize)
{
    ithWriteRegMaskA(SARADC_REG_CR1, ((uint32_t)wbSize & SARADC_BIT_BUFFER_LEN) << SARADC_SHT_BUFFER_LEN,
            SARADC_BIT_BUFFER_LEN << SARADC_SHT_BUFFER_LEN);
}

//=============================================================================
/**
 * Set Average Calculation Count of SARADC
 */
//=============================================================================
void
SARADC_SetAVGCALCount_Reg(
    SARADC_AVG_CAL_COUNT avgCalCount)
{
    ithWriteRegMaskA(SARADC_REG_CR0, (avgCalCount & SARADC_BIT_AVG_COUNT) << SARADC_SHT_AVG_COUNT,
            SARADC_BIT_AVG_COUNT << SARADC_SHT_AVG_COUNT);
}

//=============================================================================
/**
 * Set Trigger Store Count of SARADC
 */
//=============================================================================
void
SARADC_SetTRIGStoreCount_Reg(
    uint8_t trigStoreCount)
{
    ithWriteRegMaskA(SARADC_REG_CR1, (trigStoreCount & SARADC_BIT_TRIG_STORE_COUNT) << SARADC_SHT_TRIG_STORE_COUNT,
            SARADC_BIT_TRIG_STORE_COUNT << SARADC_SHT_TRIG_STORE_COUNT);
}

//=============================================================================
/**
 * Select XAIN Signal of SARADC
 */
//=============================================================================
void
SARADC_SelectXAINSignal_Reg(
    SARADC_XAIN_SIGNAL signal)
{
    if (signal == SARADC_XAIN_SIGNAL_0)
        ithWriteRegMaskA(SARADC_REG_CR1, (signal & SARADC_BIT_XAIN_SIGNAL) << SARADC_SHT_XAIN_SIGNAL,
                SARADC_BIT_XAIN_SIGNAL << SARADC_SHT_XAIN_SIGNAL);
    else
        ithWriteRegMaskA(SARADC_REG_CR1, (0x1 << (signal - 1) & SARADC_BIT_XAIN_SIGNAL) << SARADC_SHT_XAIN_SIGNAL,
                SARADC_BIT_XAIN_SIGNAL << SARADC_SHT_XAIN_SIGNAL);
}

//=============================================================================
/**
 * Set Valid XAIN of SARADC
 */
//=============================================================================
void
SARADC_SetValidXAIN_Reg(
    uint8_t validXAIN)
{
    ithWriteRegMaskA(SARADC_REG_CR0, (validXAIN & SARADC_BIT_XAIN_VALID) << SARADC_SHT_XAIN_VALID,
            SARADC_BIT_XAIN_VALID << SARADC_SHT_XAIN_VALID);
}

//=============================================================================
/**
 * Enable XAIN Interrupt of SARADC
 */
//=============================================================================
void
SARADC_EnableXAININTR_Reg(
    SARADC_PORT hwPort)
{
    switch (hwPort)
    {
    case SARADC_0:
        ithWriteRegMaskA(SARADC_REG_XAIN_INTR_CR, (0x1 & SARADC_BIT_XAIN_OVERWT_EN_0) << SARADC_SHT_XAIN_OVERWT_EN_0,
                SARADC_BIT_XAIN_OVERWT_EN_0 << SARADC_SHT_XAIN_OVERWT_EN_0);
        ithWriteRegMaskA(SARADC_REG_XAIN_INTR_CR, (0x1 & SARADC_BIT_XAIN_EVENT_EN_0) << SARADC_SHT_XAIN_EVENT_EN_0,
                SARADC_BIT_XAIN_EVENT_EN_0 << SARADC_SHT_XAIN_EVENT_EN_0);
        ithWriteRegMaskA(SARADC_REG_XAIN_INTR_CR, (0x1 & SARADC_BIT_XAIN_ABOMAX_EN_0) << SARADC_SHT_XAIN_ABOMAX_EN_0,
                SARADC_BIT_XAIN_ABOMAX_EN_0 << SARADC_SHT_XAIN_ABOMAX_EN_0);
        ithWriteRegMaskA(SARADC_REG_XAIN_INTR_CR, (0x1 & SARADC_BIT_XAIN_UNDMIN_EN_0) << SARADC_SHT_XAIN_UNDMIN_EN_0,
                SARADC_BIT_XAIN_UNDMIN_EN_0 << SARADC_SHT_XAIN_UNDMIN_EN_0);
        break;

    case SARADC_1:
        ithWriteRegMaskA(SARADC_REG_XAIN_INTR_CR, (0x1 & SARADC_BIT_XAIN_OVERWT_EN_1) << SARADC_SHT_XAIN_OVERWT_EN_1,
                SARADC_BIT_XAIN_OVERWT_EN_1 << SARADC_SHT_XAIN_OVERWT_EN_1);
        ithWriteRegMaskA(SARADC_REG_XAIN_INTR_CR, (0x1 & SARADC_BIT_XAIN_EVENT_EN_1) << SARADC_SHT_XAIN_EVENT_EN_1,
                SARADC_BIT_XAIN_EVENT_EN_1 << SARADC_SHT_XAIN_EVENT_EN_1);
        ithWriteRegMaskA(SARADC_REG_XAIN_INTR_CR, (0x1 & SARADC_BIT_XAIN_ABOMAX_EN_1) << SARADC_SHT_XAIN_ABOMAX_EN_1,
                SARADC_BIT_XAIN_ABOMAX_EN_1 << SARADC_SHT_XAIN_ABOMAX_EN_1);
        ithWriteRegMaskA(SARADC_REG_XAIN_INTR_CR, (0x1 & SARADC_BIT_XAIN_UNDMIN_EN_1) << SARADC_SHT_XAIN_UNDMIN_EN_1,
                SARADC_BIT_XAIN_UNDMIN_EN_1 << SARADC_SHT_XAIN_UNDMIN_EN_1);
        break;

    case SARADC_2:
        ithWriteRegMaskA(SARADC_REG_XAIN_INTR_CR, (0x1 & SARADC_BIT_XAIN_OVERWT_EN_2) << SARADC_SHT_XAIN_OVERWT_EN_2,
                SARADC_BIT_XAIN_OVERWT_EN_2 << SARADC_SHT_XAIN_OVERWT_EN_2);
        ithWriteRegMaskA(SARADC_REG_XAIN_INTR_CR, (0x1 & SARADC_BIT_XAIN_EVENT_EN_2) << SARADC_SHT_XAIN_EVENT_EN_2,
                SARADC_BIT_XAIN_EVENT_EN_2 << SARADC_SHT_XAIN_EVENT_EN_2);
        ithWriteRegMaskA(SARADC_REG_XAIN_INTR_CR, (0x1 & SARADC_BIT_XAIN_ABOMAX_EN_2) << SARADC_SHT_XAIN_ABOMAX_EN_2,
                SARADC_BIT_XAIN_ABOMAX_EN_2 << SARADC_SHT_XAIN_ABOMAX_EN_2);
        ithWriteRegMaskA(SARADC_REG_XAIN_INTR_CR, (0x1 & SARADC_BIT_XAIN_UNDMIN_EN_2) << SARADC_SHT_XAIN_UNDMIN_EN_2,
                SARADC_BIT_XAIN_UNDMIN_EN_2 << SARADC_SHT_XAIN_UNDMIN_EN_2);
        break;

    case SARADC_3:
        ithWriteRegMaskA(SARADC_REG_XAIN_INTR_CR, (0x1 & SARADC_BIT_XAIN_OVERWT_EN_3) << SARADC_SHT_XAIN_OVERWT_EN_3,
                SARADC_BIT_XAIN_OVERWT_EN_3 << SARADC_SHT_XAIN_OVERWT_EN_3);
        ithWriteRegMaskA(SARADC_REG_XAIN_INTR_CR, (0x1 & SARADC_BIT_XAIN_EVENT_EN_3) << SARADC_SHT_XAIN_EVENT_EN_3,
                SARADC_BIT_XAIN_EVENT_EN_3 << SARADC_SHT_XAIN_EVENT_EN_3);
        ithWriteRegMaskA(SARADC_REG_XAIN_INTR_CR, (0x1 & SARADC_BIT_XAIN_ABOMAX_EN_3) << SARADC_SHT_XAIN_ABOMAX_EN_3,
                SARADC_BIT_XAIN_ABOMAX_EN_3 << SARADC_SHT_XAIN_ABOMAX_EN_3);
        ithWriteRegMaskA(SARADC_REG_XAIN_INTR_CR, (0x1 & SARADC_BIT_XAIN_UNDMIN_EN_3) << SARADC_SHT_XAIN_UNDMIN_EN_3,
                SARADC_BIT_XAIN_UNDMIN_EN_3 << SARADC_SHT_XAIN_UNDMIN_EN_3);
        break;

    case SARADC_4:
        ithWriteRegMaskA(SARADC_REG_XAIN_INTR_CR, (0x1 & SARADC_BIT_XAIN_OVERWT_EN_4) << SARADC_SHT_XAIN_OVERWT_EN_4,
                SARADC_BIT_XAIN_OVERWT_EN_4 << SARADC_SHT_XAIN_OVERWT_EN_4);
        ithWriteRegMaskA(SARADC_REG_XAIN_INTR_CR, (0x1 & SARADC_BIT_XAIN_EVENT_EN_4) << SARADC_SHT_XAIN_EVENT_EN_4,
                SARADC_BIT_XAIN_EVENT_EN_4 << SARADC_SHT_XAIN_EVENT_EN_4);
        ithWriteRegMaskA(SARADC_REG_XAIN_INTR_CR, (0x1 & SARADC_BIT_XAIN_ABOMAX_EN_4) << SARADC_SHT_XAIN_ABOMAX_EN_4,
                SARADC_BIT_XAIN_ABOMAX_EN_4 << SARADC_SHT_XAIN_ABOMAX_EN_4);
        ithWriteRegMaskA(SARADC_REG_XAIN_INTR_CR, (0x1 & SARADC_BIT_XAIN_UNDMIN_EN_4) << SARADC_SHT_XAIN_UNDMIN_EN_4,
                SARADC_BIT_XAIN_UNDMIN_EN_4 << SARADC_SHT_XAIN_UNDMIN_EN_4);
        break;

    case SARADC_5:
        ithWriteRegMaskA(SARADC_REG_XAIN_INTR_CR, (0x1 & SARADC_BIT_XAIN_OVERWT_EN_5) << SARADC_SHT_XAIN_OVERWT_EN_5,
                SARADC_BIT_XAIN_OVERWT_EN_5 << SARADC_SHT_XAIN_OVERWT_EN_5);
        ithWriteRegMaskA(SARADC_REG_XAIN_INTR_CR, (0x1 & SARADC_BIT_XAIN_EVENT_EN_5) << SARADC_SHT_XAIN_EVENT_EN_5,
                SARADC_BIT_XAIN_EVENT_EN_5 << SARADC_SHT_XAIN_EVENT_EN_5);
        ithWriteRegMaskA(SARADC_REG_XAIN_INTR_CR, (0x1 & SARADC_BIT_XAIN_ABOMAX_EN_5) << SARADC_SHT_XAIN_ABOMAX_EN_5,
                SARADC_BIT_XAIN_ABOMAX_EN_5 << SARADC_SHT_XAIN_ABOMAX_EN_5);
        ithWriteRegMaskA(SARADC_REG_XAIN_INTR_CR, (0x1 & SARADC_BIT_XAIN_UNDMIN_EN_5) << SARADC_SHT_XAIN_UNDMIN_EN_5,
                SARADC_BIT_XAIN_UNDMIN_EN_5 << SARADC_SHT_XAIN_UNDMIN_EN_5);
        break;

    case SARADC_6:
        ithWriteRegMaskA(SARADC_REG_XAIN_INTR_CR, (0x1 & SARADC_BIT_XAIN_OVERWT_EN_6) << SARADC_SHT_XAIN_OVERWT_EN_6,
                SARADC_BIT_XAIN_OVERWT_EN_6 << SARADC_SHT_XAIN_OVERWT_EN_6);
        ithWriteRegMaskA(SARADC_REG_XAIN_INTR_CR, (0x1 & SARADC_BIT_XAIN_EVENT_EN_6) << SARADC_SHT_XAIN_EVENT_EN_6,
                SARADC_BIT_XAIN_EVENT_EN_6 << SARADC_SHT_XAIN_EVENT_EN_6);
        ithWriteRegMaskA(SARADC_REG_XAIN_INTR_CR, (0x1 & SARADC_BIT_XAIN_ABOMAX_EN_6) << SARADC_SHT_XAIN_ABOMAX_EN_6,
                SARADC_BIT_XAIN_ABOMAX_EN_6 << SARADC_SHT_XAIN_ABOMAX_EN_6);
        ithWriteRegMaskA(SARADC_REG_XAIN_INTR_CR, (0x1 & SARADC_BIT_XAIN_UNDMIN_EN_6) << SARADC_SHT_XAIN_UNDMIN_EN_6,
                SARADC_BIT_XAIN_UNDMIN_EN_6 << SARADC_SHT_XAIN_UNDMIN_EN_6);
        break;

    case SARADC_7:
        ithWriteRegMaskA(SARADC_REG_XAIN_INTR_CR, (0x1 & SARADC_BIT_XAIN_OVERWT_EN_7) << SARADC_SHT_XAIN_OVERWT_EN_7,
                SARADC_BIT_XAIN_OVERWT_EN_7 << SARADC_SHT_XAIN_OVERWT_EN_7);
        ithWriteRegMaskA(SARADC_REG_XAIN_INTR_CR, (0x1 & SARADC_BIT_XAIN_EVENT_EN_7) << SARADC_SHT_XAIN_EVENT_EN_7,
                SARADC_BIT_XAIN_EVENT_EN_7 << SARADC_SHT_XAIN_EVENT_EN_7);
        ithWriteRegMaskA(SARADC_REG_XAIN_INTR_CR, (0x1 & SARADC_BIT_XAIN_ABOMAX_EN_7) << SARADC_SHT_XAIN_ABOMAX_EN_7,
                SARADC_BIT_XAIN_ABOMAX_EN_7 << SARADC_SHT_XAIN_ABOMAX_EN_7);
        ithWriteRegMaskA(SARADC_REG_XAIN_INTR_CR, (0x1 & SARADC_BIT_XAIN_UNDMIN_EN_7) << SARADC_SHT_XAIN_UNDMIN_EN_7,
                SARADC_BIT_XAIN_UNDMIN_EN_7 << SARADC_SHT_XAIN_UNDMIN_EN_7);
        break;

    default:
        break;
    }
}

//=============================================================================
/**
 * Disable XAIN Interrupt of SARADC
 */
//=============================================================================
void
SARADC_DisableXAININTR_Reg(
    SARADC_PORT hwPort)
{
    switch (hwPort)
    {
    case SARADC_0:
        ithWriteRegMaskA(SARADC_REG_XAIN_INTR_CR, (0x0 & SARADC_BIT_XAIN_OVERWT_EN_0) << SARADC_SHT_XAIN_OVERWT_EN_0,
                SARADC_BIT_XAIN_OVERWT_EN_0 << SARADC_SHT_XAIN_OVERWT_EN_0);
        ithWriteRegMaskA(SARADC_REG_XAIN_INTR_CR, (0x0 & SARADC_BIT_XAIN_EVENT_EN_0) << SARADC_SHT_XAIN_EVENT_EN_0,
                SARADC_BIT_XAIN_EVENT_EN_0 << SARADC_SHT_XAIN_EVENT_EN_0);
        ithWriteRegMaskA(SARADC_REG_XAIN_INTR_CR, (0x0 & SARADC_BIT_XAIN_ABOMAX_EN_0) << SARADC_SHT_XAIN_ABOMAX_EN_0,
                SARADC_BIT_XAIN_ABOMAX_EN_0 << SARADC_SHT_XAIN_ABOMAX_EN_0);
        ithWriteRegMaskA(SARADC_REG_XAIN_INTR_CR, (0x0 & SARADC_BIT_XAIN_UNDMIN_EN_0) << SARADC_SHT_XAIN_UNDMIN_EN_0,
                SARADC_BIT_XAIN_UNDMIN_EN_0 << SARADC_SHT_XAIN_UNDMIN_EN_0);
        break;

    case SARADC_1:
        ithWriteRegMaskA(SARADC_REG_XAIN_INTR_CR, (0x0 & SARADC_BIT_XAIN_OVERWT_EN_1) << SARADC_SHT_XAIN_OVERWT_EN_1,
                SARADC_BIT_XAIN_OVERWT_EN_1 << SARADC_SHT_XAIN_OVERWT_EN_1);
        ithWriteRegMaskA(SARADC_REG_XAIN_INTR_CR, (0x0 & SARADC_BIT_XAIN_EVENT_EN_1) << SARADC_SHT_XAIN_EVENT_EN_1,
                SARADC_BIT_XAIN_EVENT_EN_1 << SARADC_SHT_XAIN_EVENT_EN_1);
        ithWriteRegMaskA(SARADC_REG_XAIN_INTR_CR, (0x0 & SARADC_BIT_XAIN_ABOMAX_EN_1) << SARADC_SHT_XAIN_ABOMAX_EN_1,
                SARADC_BIT_XAIN_ABOMAX_EN_1 << SARADC_SHT_XAIN_ABOMAX_EN_1);
        ithWriteRegMaskA(SARADC_REG_XAIN_INTR_CR, (0x0 & SARADC_BIT_XAIN_UNDMIN_EN_1) << SARADC_SHT_XAIN_UNDMIN_EN_1,
                SARADC_BIT_XAIN_UNDMIN_EN_1 << SARADC_SHT_XAIN_UNDMIN_EN_1);
        break;

    case SARADC_2:
        ithWriteRegMaskA(SARADC_REG_XAIN_INTR_CR, (0x0 & SARADC_BIT_XAIN_OVERWT_EN_2) << SARADC_SHT_XAIN_OVERWT_EN_2,
                SARADC_BIT_XAIN_OVERWT_EN_2 << SARADC_SHT_XAIN_OVERWT_EN_2);
        ithWriteRegMaskA(SARADC_REG_XAIN_INTR_CR, (0x0 & SARADC_BIT_XAIN_EVENT_EN_2) << SARADC_SHT_XAIN_EVENT_EN_2,
                SARADC_BIT_XAIN_EVENT_EN_2 << SARADC_SHT_XAIN_EVENT_EN_2);
        ithWriteRegMaskA(SARADC_REG_XAIN_INTR_CR, (0x0 & SARADC_BIT_XAIN_ABOMAX_EN_2) << SARADC_SHT_XAIN_ABOMAX_EN_2,
                SARADC_BIT_XAIN_ABOMAX_EN_2 << SARADC_SHT_XAIN_ABOMAX_EN_2);
        ithWriteRegMaskA(SARADC_REG_XAIN_INTR_CR, (0x0 & SARADC_BIT_XAIN_UNDMIN_EN_2) << SARADC_SHT_XAIN_UNDMIN_EN_2,
                SARADC_BIT_XAIN_UNDMIN_EN_2 << SARADC_SHT_XAIN_UNDMIN_EN_2);
        break;

    case SARADC_3:
        ithWriteRegMaskA(SARADC_REG_XAIN_INTR_CR, (0x0 & SARADC_BIT_XAIN_OVERWT_EN_3) << SARADC_SHT_XAIN_OVERWT_EN_3,
                SARADC_BIT_XAIN_OVERWT_EN_3 << SARADC_SHT_XAIN_OVERWT_EN_3);
        ithWriteRegMaskA(SARADC_REG_XAIN_INTR_CR, (0x0 & SARADC_BIT_XAIN_EVENT_EN_3) << SARADC_SHT_XAIN_EVENT_EN_3,
                SARADC_BIT_XAIN_EVENT_EN_3 << SARADC_SHT_XAIN_EVENT_EN_3);
        ithWriteRegMaskA(SARADC_REG_XAIN_INTR_CR, (0x0 & SARADC_BIT_XAIN_ABOMAX_EN_3) << SARADC_SHT_XAIN_ABOMAX_EN_3,
                SARADC_BIT_XAIN_ABOMAX_EN_3 << SARADC_SHT_XAIN_ABOMAX_EN_3);
        ithWriteRegMaskA(SARADC_REG_XAIN_INTR_CR, (0x0 & SARADC_BIT_XAIN_UNDMIN_EN_3) << SARADC_SHT_XAIN_UNDMIN_EN_3,
                SARADC_BIT_XAIN_UNDMIN_EN_3 << SARADC_SHT_XAIN_UNDMIN_EN_3);
        break;

    case SARADC_4:
        ithWriteRegMaskA(SARADC_REG_XAIN_INTR_CR, (0x0 & SARADC_BIT_XAIN_OVERWT_EN_4) << SARADC_SHT_XAIN_OVERWT_EN_4,
                SARADC_BIT_XAIN_OVERWT_EN_4 << SARADC_SHT_XAIN_OVERWT_EN_4);
        ithWriteRegMaskA(SARADC_REG_XAIN_INTR_CR, (0x0 & SARADC_BIT_XAIN_EVENT_EN_4) << SARADC_SHT_XAIN_EVENT_EN_4,
                SARADC_BIT_XAIN_EVENT_EN_4 << SARADC_SHT_XAIN_EVENT_EN_4);
        ithWriteRegMaskA(SARADC_REG_XAIN_INTR_CR, (0x0 & SARADC_BIT_XAIN_ABOMAX_EN_4) << SARADC_SHT_XAIN_ABOMAX_EN_4,
                SARADC_BIT_XAIN_ABOMAX_EN_4 << SARADC_SHT_XAIN_ABOMAX_EN_4);
        ithWriteRegMaskA(SARADC_REG_XAIN_INTR_CR, (0x0 & SARADC_BIT_XAIN_UNDMIN_EN_4) << SARADC_SHT_XAIN_UNDMIN_EN_4,
                SARADC_BIT_XAIN_UNDMIN_EN_4 << SARADC_SHT_XAIN_UNDMIN_EN_4);
        break;

    case SARADC_5:
        ithWriteRegMaskA(SARADC_REG_XAIN_INTR_CR, (0x0 & SARADC_BIT_XAIN_OVERWT_EN_5) << SARADC_SHT_XAIN_OVERWT_EN_5,
                SARADC_BIT_XAIN_OVERWT_EN_5 << SARADC_SHT_XAIN_OVERWT_EN_5);
        ithWriteRegMaskA(SARADC_REG_XAIN_INTR_CR, (0x0 & SARADC_BIT_XAIN_EVENT_EN_5) << SARADC_SHT_XAIN_EVENT_EN_5,
                SARADC_BIT_XAIN_EVENT_EN_5 << SARADC_SHT_XAIN_EVENT_EN_5);
        ithWriteRegMaskA(SARADC_REG_XAIN_INTR_CR, (0x0 & SARADC_BIT_XAIN_ABOMAX_EN_5) << SARADC_SHT_XAIN_ABOMAX_EN_5,
                SARADC_BIT_XAIN_ABOMAX_EN_5 << SARADC_SHT_XAIN_ABOMAX_EN_5);
        ithWriteRegMaskA(SARADC_REG_XAIN_INTR_CR, (0x0 & SARADC_BIT_XAIN_UNDMIN_EN_5) << SARADC_SHT_XAIN_UNDMIN_EN_5,
                SARADC_BIT_XAIN_UNDMIN_EN_5 << SARADC_SHT_XAIN_UNDMIN_EN_5);
        break;

    case SARADC_6:
        ithWriteRegMaskA(SARADC_REG_XAIN_INTR_CR, (0x0 & SARADC_BIT_XAIN_OVERWT_EN_6) << SARADC_SHT_XAIN_OVERWT_EN_6,
                SARADC_BIT_XAIN_OVERWT_EN_6 << SARADC_SHT_XAIN_OVERWT_EN_6);
        ithWriteRegMaskA(SARADC_REG_XAIN_INTR_CR, (0x0 & SARADC_BIT_XAIN_EVENT_EN_6) << SARADC_SHT_XAIN_EVENT_EN_6,
                SARADC_BIT_XAIN_EVENT_EN_6 << SARADC_SHT_XAIN_EVENT_EN_6);
        ithWriteRegMaskA(SARADC_REG_XAIN_INTR_CR, (0x0 & SARADC_BIT_XAIN_ABOMAX_EN_6) << SARADC_SHT_XAIN_ABOMAX_EN_6,
                SARADC_BIT_XAIN_ABOMAX_EN_6 << SARADC_SHT_XAIN_ABOMAX_EN_6);
        ithWriteRegMaskA(SARADC_REG_XAIN_INTR_CR, (0x0 & SARADC_BIT_XAIN_UNDMIN_EN_6) << SARADC_SHT_XAIN_UNDMIN_EN_6,
                SARADC_BIT_XAIN_UNDMIN_EN_6 << SARADC_SHT_XAIN_UNDMIN_EN_6);
        break;

    case SARADC_7:
        ithWriteRegMaskA(SARADC_REG_XAIN_INTR_CR, (0x0 & SARADC_BIT_XAIN_OVERWT_EN_7) << SARADC_SHT_XAIN_OVERWT_EN_7,
                SARADC_BIT_XAIN_OVERWT_EN_7 << SARADC_SHT_XAIN_OVERWT_EN_7);
        ithWriteRegMaskA(SARADC_REG_XAIN_INTR_CR, (0x0 & SARADC_BIT_XAIN_EVENT_EN_7) << SARADC_SHT_XAIN_EVENT_EN_7,
                SARADC_BIT_XAIN_EVENT_EN_7 << SARADC_SHT_XAIN_EVENT_EN_7);
        ithWriteRegMaskA(SARADC_REG_XAIN_INTR_CR, (0x0 & SARADC_BIT_XAIN_ABOMAX_EN_7) << SARADC_SHT_XAIN_ABOMAX_EN_7,
                SARADC_BIT_XAIN_ABOMAX_EN_7 << SARADC_SHT_XAIN_ABOMAX_EN_7);
        ithWriteRegMaskA(SARADC_REG_XAIN_INTR_CR, (0x0 & SARADC_BIT_XAIN_UNDMIN_EN_7) << SARADC_SHT_XAIN_UNDMIN_EN_7,
                SARADC_BIT_XAIN_UNDMIN_EN_7 << SARADC_SHT_XAIN_UNDMIN_EN_7);
        break;

    default:
        break;
    }
}

//=============================================================================
/**
 * Set XAIN memory base of SARADC
 */
//=============================================================================
void
SARADC_SetXAINMEMBase_Reg(
    SARADC_PORT hwPort,
    uint8_t     *baseAddr)
{
    switch (hwPort)
    {
    case SARADC_0:
        ithWriteRegA(SARADC_REG_XAIN_BASEADDR_0,
                ((uint32_t)baseAddr & SARADC_BIT_BASEADDR_0) << SARADC_SHT_BASEADDR_0);
        break;

    case SARADC_1:
        ithWriteRegA(SARADC_REG_XAIN_BASEADDR_1,
                ((uint32_t)baseAddr & SARADC_BIT_BASEADDR_1) << SARADC_SHT_BASEADDR_1);
        break;

    case SARADC_2:
        ithWriteRegA(SARADC_REG_XAIN_BASEADDR_2,
                ((uint32_t)baseAddr & SARADC_BIT_BASEADDR_2) << SARADC_SHT_BASEADDR_2);
        break;

    case SARADC_3:
        ithWriteRegA(SARADC_REG_XAIN_BASEADDR_3,
                ((uint32_t)baseAddr & SARADC_BIT_BASEADDR_3) << SARADC_SHT_BASEADDR_3);
        break;

    case SARADC_4:
        ithWriteRegA(SARADC_REG_XAIN_BASEADDR_4,
                ((uint32_t)baseAddr & SARADC_BIT_BASEADDR_4) << SARADC_SHT_BASEADDR_4);
        break;

    case SARADC_5:
        ithWriteRegA(SARADC_REG_XAIN_BASEADDR_5,
                ((uint32_t)baseAddr & SARADC_BIT_BASEADDR_5) << SARADC_SHT_BASEADDR_5);
        break;

    case SARADC_6:
        ithWriteRegA(SARADC_REG_XAIN_BASEADDR_6,
                ((uint32_t)baseAddr & SARADC_BIT_BASEADDR_6) << SARADC_SHT_BASEADDR_6);
        break;

    case SARADC_7:
        ithWriteRegA(SARADC_REG_XAIN_BASEADDR_7,
                ((uint32_t)baseAddr & SARADC_BIT_BASEADDR_7) << SARADC_SHT_BASEADDR_7);
        break;

    default:
        break;
    }
}

//=============================================================================
/**
 * Set XAIN trigger rule of SARADC
 */
//=============================================================================
void
SARADC_SetXAINTriggerRule_Reg(
    SARADC_PORT     hwPort,
    SARADC_TRIG_AVG trigAVG,
    uint16_t        maxAVGTrigger,
    uint16_t        minAVGTrigger)
{
    switch (hwPort)
    {
    case SARADC_0:
        ithWriteRegMaskA(SARADC_REG_XAIN_TRIGRULE_0, (trigAVG & SARADC_BIT_XAIN_TRIGRULE_TYPE_0) << SARADC_SHT_XAIN_TRIGRULE_TYPE_0,
                SARADC_BIT_XAIN_TRIGRULE_TYPE_0 << SARADC_SHT_XAIN_TRIGRULE_TYPE_0);
        ithWriteRegMaskA(SARADC_REG_XAIN_TRIGRULE_0,
                ((uint32_t)maxAVGTrigger & SARADC_BIT_XAIN_TRIGRULE_MAX_0) << SARADC_SHT_XAIN_TRIGRULE_MAX_0,
                SARADC_BIT_XAIN_TRIGRULE_MAX_0 << SARADC_SHT_XAIN_TRIGRULE_MAX_0);
        ithWriteRegMaskA(SARADC_REG_XAIN_TRIGRULE_0,
                ((uint32_t)minAVGTrigger & SARADC_BIT_XAIN_TRIGRULE_MIN_0) << SARADC_SHT_XAIN_TRIGRULE_MIN_0,
                SARADC_BIT_XAIN_TRIGRULE_MIN_0 << SARADC_SHT_XAIN_TRIGRULE_MIN_0);
        break;

    case SARADC_1:
        ithWriteRegMaskA(SARADC_REG_XAIN_TRIGRULE_1, (trigAVG & SARADC_BIT_XAIN_TRIGRULE_TYPE_1) << SARADC_SHT_XAIN_TRIGRULE_TYPE_1,
                SARADC_BIT_XAIN_TRIGRULE_TYPE_1 << SARADC_SHT_XAIN_TRIGRULE_TYPE_1);
        ithWriteRegMaskA(SARADC_REG_XAIN_TRIGRULE_1,
                ((uint32_t)maxAVGTrigger & SARADC_BIT_XAIN_TRIGRULE_MAX_1) << SARADC_SHT_XAIN_TRIGRULE_MAX_1,
                SARADC_BIT_XAIN_TRIGRULE_MAX_1 << SARADC_SHT_XAIN_TRIGRULE_MAX_1);
        ithWriteRegMaskA(SARADC_REG_XAIN_TRIGRULE_1,
                ((uint32_t)minAVGTrigger & SARADC_BIT_XAIN_TRIGRULE_MIN_1) << SARADC_SHT_XAIN_TRIGRULE_MIN_1,
                SARADC_BIT_XAIN_TRIGRULE_MIN_1 << SARADC_SHT_XAIN_TRIGRULE_MIN_1);
        break;

    case SARADC_2:
        ithWriteRegMaskA(SARADC_REG_XAIN_TRIGRULE_2, (trigAVG & SARADC_BIT_XAIN_TRIGRULE_TYPE_2) << SARADC_SHT_XAIN_TRIGRULE_TYPE_2,
                SARADC_BIT_XAIN_TRIGRULE_TYPE_2 << SARADC_SHT_XAIN_TRIGRULE_TYPE_2);
        ithWriteRegMaskA(SARADC_REG_XAIN_TRIGRULE_2,
                ((uint32_t)maxAVGTrigger & SARADC_BIT_XAIN_TRIGRULE_MAX_2) << SARADC_SHT_XAIN_TRIGRULE_MAX_2,
                SARADC_BIT_XAIN_TRIGRULE_MAX_2 << SARADC_SHT_XAIN_TRIGRULE_MAX_2);
        ithWriteRegMaskA(SARADC_REG_XAIN_TRIGRULE_2,
                ((uint32_t)minAVGTrigger & SARADC_BIT_XAIN_TRIGRULE_MIN_2) << SARADC_SHT_XAIN_TRIGRULE_MIN_2,
                SARADC_BIT_XAIN_TRIGRULE_MIN_2 << SARADC_SHT_XAIN_TRIGRULE_MIN_2);
        break;

    case SARADC_3:
        ithWriteRegMaskA(SARADC_REG_XAIN_TRIGRULE_3, (trigAVG & SARADC_BIT_XAIN_TRIGRULE_TYPE_3) << SARADC_SHT_XAIN_TRIGRULE_TYPE_3,
                SARADC_BIT_XAIN_TRIGRULE_TYPE_3 << SARADC_SHT_XAIN_TRIGRULE_TYPE_3);
        ithWriteRegMaskA(SARADC_REG_XAIN_TRIGRULE_3,
                ((uint32_t)maxAVGTrigger & SARADC_BIT_XAIN_TRIGRULE_MAX_3) << SARADC_SHT_XAIN_TRIGRULE_MAX_3,
                SARADC_BIT_XAIN_TRIGRULE_MAX_3 << SARADC_SHT_XAIN_TRIGRULE_MAX_3);
        ithWriteRegMaskA(SARADC_REG_XAIN_TRIGRULE_3,
                ((uint32_t)minAVGTrigger & SARADC_BIT_XAIN_TRIGRULE_MIN_3) << SARADC_SHT_XAIN_TRIGRULE_MIN_3,
                SARADC_BIT_XAIN_TRIGRULE_MIN_3 << SARADC_SHT_XAIN_TRIGRULE_MIN_3);
        break;

    case SARADC_4:
        ithWriteRegMaskA(SARADC_REG_XAIN_TRIGRULE_4, (trigAVG & SARADC_BIT_XAIN_TRIGRULE_TYPE_4) << SARADC_SHT_XAIN_TRIGRULE_TYPE_4,
                SARADC_BIT_XAIN_TRIGRULE_TYPE_4 << SARADC_SHT_XAIN_TRIGRULE_TYPE_4);
        ithWriteRegMaskA(SARADC_REG_XAIN_TRIGRULE_4,
                ((uint32_t)maxAVGTrigger & SARADC_BIT_XAIN_TRIGRULE_MAX_4) << SARADC_SHT_XAIN_TRIGRULE_MAX_4,
                SARADC_BIT_XAIN_TRIGRULE_MAX_4 << SARADC_SHT_XAIN_TRIGRULE_MAX_4);
        ithWriteRegMaskA(SARADC_REG_XAIN_TRIGRULE_4,
                ((uint32_t)minAVGTrigger & SARADC_BIT_XAIN_TRIGRULE_MIN_4) << SARADC_SHT_XAIN_TRIGRULE_MIN_4,
                SARADC_BIT_XAIN_TRIGRULE_MIN_4 << SARADC_SHT_XAIN_TRIGRULE_MIN_4);
        break;

    case SARADC_5:
        ithWriteRegMaskA(SARADC_REG_XAIN_TRIGRULE_5, (trigAVG & SARADC_BIT_XAIN_TRIGRULE_TYPE_5) << SARADC_SHT_XAIN_TRIGRULE_TYPE_5,
                SARADC_BIT_XAIN_TRIGRULE_TYPE_5 << SARADC_SHT_XAIN_TRIGRULE_TYPE_5);
        ithWriteRegMaskA(SARADC_REG_XAIN_TRIGRULE_5,
                ((uint32_t)maxAVGTrigger & SARADC_BIT_XAIN_TRIGRULE_MAX_5) << SARADC_SHT_XAIN_TRIGRULE_MAX_5,
                SARADC_BIT_XAIN_TRIGRULE_MAX_5 << SARADC_SHT_XAIN_TRIGRULE_MAX_5);
        ithWriteRegMaskA(SARADC_REG_XAIN_TRIGRULE_5,
                ((uint32_t)minAVGTrigger & SARADC_BIT_XAIN_TRIGRULE_MIN_5) << SARADC_SHT_XAIN_TRIGRULE_MIN_5,
                SARADC_BIT_XAIN_TRIGRULE_MIN_5 << SARADC_SHT_XAIN_TRIGRULE_MIN_5);
        break;

    case SARADC_6:
        ithWriteRegMaskA(SARADC_REG_XAIN_TRIGRULE_6, (trigAVG & SARADC_BIT_XAIN_TRIGRULE_TYPE_6) << SARADC_SHT_XAIN_TRIGRULE_TYPE_6,
                SARADC_BIT_XAIN_TRIGRULE_TYPE_6 << SARADC_SHT_XAIN_TRIGRULE_TYPE_6);
        ithWriteRegMaskA(SARADC_REG_XAIN_TRIGRULE_6,
                ((uint32_t)maxAVGTrigger & SARADC_BIT_XAIN_TRIGRULE_MAX_6) << SARADC_SHT_XAIN_TRIGRULE_MAX_6,
                SARADC_BIT_XAIN_TRIGRULE_MAX_6 << SARADC_SHT_XAIN_TRIGRULE_MAX_6);
        ithWriteRegMaskA(SARADC_REG_XAIN_TRIGRULE_6,
                ((uint32_t)minAVGTrigger & SARADC_BIT_XAIN_TRIGRULE_MIN_6) << SARADC_SHT_XAIN_TRIGRULE_MIN_6,
                SARADC_BIT_XAIN_TRIGRULE_MIN_6 << SARADC_SHT_XAIN_TRIGRULE_MIN_6);
        break;

    case SARADC_7:
        ithWriteRegMaskA(SARADC_REG_XAIN_TRIGRULE_7, (trigAVG & SARADC_BIT_XAIN_TRIGRULE_TYPE_7) << SARADC_SHT_XAIN_TRIGRULE_TYPE_7,
                SARADC_BIT_XAIN_TRIGRULE_TYPE_7 << SARADC_SHT_XAIN_TRIGRULE_TYPE_7);
        ithWriteRegMaskA(SARADC_REG_XAIN_TRIGRULE_7,
                ((uint32_t)maxAVGTrigger & SARADC_BIT_XAIN_TRIGRULE_MAX_7) << SARADC_SHT_XAIN_TRIGRULE_MAX_7,
                SARADC_BIT_XAIN_TRIGRULE_MAX_7 << SARADC_SHT_XAIN_TRIGRULE_MAX_7);
        ithWriteRegMaskA(SARADC_REG_XAIN_TRIGRULE_7,
                ((uint32_t)minAVGTrigger & SARADC_BIT_XAIN_TRIGRULE_MIN_7) << SARADC_SHT_XAIN_TRIGRULE_MIN_7,
                SARADC_BIT_XAIN_TRIGRULE_MIN_7 << SARADC_SHT_XAIN_TRIGRULE_MIN_7);
        break;

    default:
        break;
    }
}

//=============================================================================
/**
 * Set XAIN detection rule of SARADC
 */
//=============================================================================
void
SARADC_SetXAINDetectRule_Reg(
    SARADC_PORT hwPort,
    uint16_t    maxAVGDetect,
    uint16_t    minAVGDetect)
{
    switch (hwPort)
    {
    case SARADC_0:
        ithWriteRegMaskA(SARADC_REG_XAIN_DECTRULE_0,
                ((uint32_t)maxAVGDetect & SARADC_BIT_XAIN_DECTRULE_MAX_0) << SARADC_SHT_XAIN_DECTRULE_MAX_0,
                SARADC_BIT_XAIN_DECTRULE_MAX_0 << SARADC_SHT_XAIN_DECTRULE_MAX_0);
        ithWriteRegMaskA(SARADC_REG_XAIN_DECTRULE_0,
                ((uint32_t)minAVGDetect & SARADC_BIT_XAIN_DECTRULE_MIN_0) << SARADC_SHT_XAIN_DECTRULE_MIN_0,
                SARADC_BIT_XAIN_DECTRULE_MIN_0 << SARADC_SHT_XAIN_DECTRULE_MIN_0);
        break;

    case SARADC_1:
        ithWriteRegMaskA(SARADC_REG_XAIN_DECTRULE_1,
                ((uint32_t)maxAVGDetect & SARADC_BIT_XAIN_DECTRULE_MAX_1) << SARADC_SHT_XAIN_DECTRULE_MAX_1,
                SARADC_BIT_XAIN_DECTRULE_MAX_1 << SARADC_SHT_XAIN_DECTRULE_MAX_1);
        ithWriteRegMaskA(SARADC_REG_XAIN_DECTRULE_1,
                ((uint32_t)minAVGDetect & SARADC_BIT_XAIN_DECTRULE_MIN_1) << SARADC_SHT_XAIN_DECTRULE_MIN_1,
                SARADC_BIT_XAIN_DECTRULE_MIN_1 << SARADC_SHT_XAIN_DECTRULE_MIN_1);
        break;

    case SARADC_2:
        ithWriteRegMaskA(SARADC_REG_XAIN_DECTRULE_2,
                ((uint32_t)maxAVGDetect & SARADC_BIT_XAIN_DECTRULE_MAX_2) << SARADC_SHT_XAIN_DECTRULE_MAX_2,
                SARADC_BIT_XAIN_DECTRULE_MAX_2 << SARADC_SHT_XAIN_DECTRULE_MAX_2);
        ithWriteRegMaskA(SARADC_REG_XAIN_DECTRULE_2,
                ((uint32_t)minAVGDetect & SARADC_BIT_XAIN_DECTRULE_MIN_2) << SARADC_SHT_XAIN_DECTRULE_MIN_2,
                SARADC_BIT_XAIN_DECTRULE_MIN_2 << SARADC_SHT_XAIN_DECTRULE_MIN_2);
        break;

    case SARADC_3:
        ithWriteRegMaskA(SARADC_REG_XAIN_DECTRULE_3,
                ((uint32_t)maxAVGDetect & SARADC_BIT_XAIN_DECTRULE_MAX_3) << SARADC_SHT_XAIN_DECTRULE_MAX_3,
                SARADC_BIT_XAIN_DECTRULE_MAX_3 << SARADC_SHT_XAIN_DECTRULE_MAX_3);
        ithWriteRegMaskA(SARADC_REG_XAIN_DECTRULE_3,
                ((uint32_t)minAVGDetect & SARADC_BIT_XAIN_DECTRULE_MIN_3) << SARADC_SHT_XAIN_DECTRULE_MIN_3,
                SARADC_BIT_XAIN_DECTRULE_MIN_3 << SARADC_SHT_XAIN_DECTRULE_MIN_3);
        break;

    case SARADC_4:
        ithWriteRegMaskA(SARADC_REG_XAIN_DECTRULE_4,
                ((uint32_t)maxAVGDetect & SARADC_BIT_XAIN_DECTRULE_MAX_4) << SARADC_SHT_XAIN_DECTRULE_MAX_4,
                SARADC_BIT_XAIN_DECTRULE_MAX_4 << SARADC_SHT_XAIN_DECTRULE_MAX_4);
        ithWriteRegMaskA(SARADC_REG_XAIN_DECTRULE_4,
                ((uint32_t)minAVGDetect & SARADC_BIT_XAIN_DECTRULE_MIN_4) << SARADC_SHT_XAIN_DECTRULE_MIN_4,
                SARADC_BIT_XAIN_DECTRULE_MIN_4 << SARADC_SHT_XAIN_DECTRULE_MIN_4);
        break;

    case SARADC_5:
        ithWriteRegMaskA(SARADC_REG_XAIN_DECTRULE_5,
                ((uint32_t)maxAVGDetect & SARADC_BIT_XAIN_DECTRULE_MAX_5) << SARADC_SHT_XAIN_DECTRULE_MAX_5,
                SARADC_BIT_XAIN_DECTRULE_MAX_5 << SARADC_SHT_XAIN_DECTRULE_MAX_5);
        ithWriteRegMaskA(SARADC_REG_XAIN_DECTRULE_5,
                ((uint32_t)minAVGDetect & SARADC_BIT_XAIN_DECTRULE_MIN_5) << SARADC_SHT_XAIN_DECTRULE_MIN_5,
                SARADC_BIT_XAIN_DECTRULE_MIN_5 << SARADC_SHT_XAIN_DECTRULE_MIN_5);
        break;

    case SARADC_6:
        ithWriteRegMaskA(SARADC_REG_XAIN_DECTRULE_6,
                ((uint32_t)maxAVGDetect & SARADC_BIT_XAIN_DECTRULE_MAX_6) << SARADC_SHT_XAIN_DECTRULE_MAX_6,
                SARADC_BIT_XAIN_DECTRULE_MAX_6 << SARADC_SHT_XAIN_DECTRULE_MAX_6);
        ithWriteRegMaskA(SARADC_REG_XAIN_DECTRULE_6,
                ((uint32_t)minAVGDetect & SARADC_BIT_XAIN_DECTRULE_MIN_6) << SARADC_SHT_XAIN_DECTRULE_MIN_6,
                SARADC_BIT_XAIN_DECTRULE_MIN_6 << SARADC_SHT_XAIN_DECTRULE_MIN_6);
        break;

    case SARADC_7:
        ithWriteRegMaskA(SARADC_REG_XAIN_DECTRULE_7,
                ((uint32_t)maxAVGDetect & SARADC_BIT_XAIN_DECTRULE_MAX_7) << SARADC_SHT_XAIN_DECTRULE_MAX_7,
                SARADC_BIT_XAIN_DECTRULE_MAX_7 << SARADC_SHT_XAIN_DECTRULE_MAX_7);
        ithWriteRegMaskA(SARADC_REG_XAIN_DECTRULE_7,
                ((uint32_t)minAVGDetect & SARADC_BIT_XAIN_DECTRULE_MIN_7) << SARADC_SHT_XAIN_DECTRULE_MIN_7,
                SARADC_BIT_XAIN_DECTRULE_MIN_7 << SARADC_SHT_XAIN_DECTRULE_MIN_7);
        break;

    default:
        break;
    }
}

//=============================================================================
/**
 * Set XAIN Read pointer of SARADC
 */
//=============================================================================
void
SARADC_SetXAINRPTR_Reg(
    SARADC_PORT hwPort,
    uint16_t    rptr)
{
    switch (hwPort)
    {
    case SARADC_0:
        ithWriteRegMaskA(SARADC_REG_XAIN_RWPTR_0,
                ((uint32_t)rptr & SARADC_BIT_XAIN_RPTR_0) << SARADC_SHT_XAIN_RPTR_0,
                SARADC_BIT_XAIN_RPTR_0 << SARADC_SHT_XAIN_RPTR_0);
        break;

    case SARADC_1:
        ithWriteRegMaskA(SARADC_REG_XAIN_RWPTR_1,
                ((uint32_t)rptr & SARADC_BIT_XAIN_RPTR_1) << SARADC_SHT_XAIN_RPTR_1,
                SARADC_BIT_XAIN_RPTR_1 << SARADC_SHT_XAIN_RPTR_1);
        break;

    case SARADC_2:
        ithWriteRegMaskA(SARADC_REG_XAIN_RWPTR_2,
                ((uint32_t)rptr & SARADC_BIT_XAIN_RPTR_2) << SARADC_SHT_XAIN_RPTR_2,
                SARADC_BIT_XAIN_RPTR_2 << SARADC_SHT_XAIN_RPTR_2);
        break;

    case SARADC_3:
        ithWriteRegMaskA(SARADC_REG_XAIN_RWPTR_3,
                ((uint32_t)rptr & SARADC_BIT_XAIN_RPTR_3) << SARADC_SHT_XAIN_RPTR_3,
                SARADC_BIT_XAIN_RPTR_3 << SARADC_SHT_XAIN_RPTR_3);
        break;

    case SARADC_4:
        ithWriteRegMaskA(SARADC_REG_XAIN_RWPTR_4,
                ((uint32_t)rptr & SARADC_BIT_XAIN_RPTR_4) << SARADC_SHT_XAIN_RPTR_4,
                SARADC_BIT_XAIN_RPTR_4 << SARADC_SHT_XAIN_RPTR_4);
        break;

    case SARADC_5:
        ithWriteRegMaskA(SARADC_REG_XAIN_RWPTR_5,
                ((uint32_t)rptr & SARADC_BIT_XAIN_RPTR_5) << SARADC_SHT_XAIN_RPTR_5,
                SARADC_BIT_XAIN_RPTR_5 << SARADC_SHT_XAIN_RPTR_5);
        break;

    case SARADC_6:
        ithWriteRegMaskA(SARADC_REG_XAIN_RWPTR_6,
                ((uint32_t)rptr & SARADC_BIT_XAIN_RPTR_6) << SARADC_SHT_XAIN_RPTR_6,
                SARADC_BIT_XAIN_RPTR_6 << SARADC_SHT_XAIN_RPTR_6);
        break;

    case SARADC_7:
        ithWriteRegMaskA(SARADC_REG_XAIN_RWPTR_7,
                ((uint32_t)rptr & SARADC_BIT_XAIN_RPTR_7) << SARADC_SHT_XAIN_RPTR_7,
                SARADC_BIT_XAIN_RPTR_7 << SARADC_SHT_XAIN_RPTR_7);
        break;

    default:
        break;
    }
}

//=============================================================================
/**
 * Read Write pointer that XAIN recorded of SARADC
 */
//=============================================================================
void
SARADC_ReadXAINWPTR_Reg(
    SARADC_PORT hwPort,
    uint16_t    *wptr)
{
    switch (hwPort)
    {
    case SARADC_0:
        *wptr = (uint16_t)((ithReadRegA(SARADC_REG_XAIN_RWPTR_0) & SARADC_BIT_XAIN_WPTR_0 << SARADC_SHT_XAIN_WPTR_0) >> SARADC_SHT_XAIN_WPTR_0);
        break;

    case SARADC_1:
        *wptr = (uint16_t)((ithReadRegA(SARADC_REG_XAIN_RWPTR_1) & SARADC_BIT_XAIN_WPTR_1 << SARADC_SHT_XAIN_WPTR_1) >> SARADC_SHT_XAIN_WPTR_1);
        break;

    case SARADC_2:
        *wptr = (uint16_t)((ithReadRegA(SARADC_REG_XAIN_RWPTR_2) & SARADC_BIT_XAIN_WPTR_2 << SARADC_SHT_XAIN_WPTR_2) >> SARADC_SHT_XAIN_WPTR_2);
        break;

    case SARADC_3:
        *wptr = (uint16_t)((ithReadRegA(SARADC_REG_XAIN_RWPTR_3) & SARADC_BIT_XAIN_WPTR_3 << SARADC_SHT_XAIN_WPTR_3) >> SARADC_SHT_XAIN_WPTR_3);
        break;

    case SARADC_4:
        *wptr = (uint16_t)((ithReadRegA(SARADC_REG_XAIN_RWPTR_4) & SARADC_BIT_XAIN_WPTR_4 << SARADC_SHT_XAIN_WPTR_4) >> SARADC_SHT_XAIN_WPTR_4);
        break;

    case SARADC_5:
        *wptr = (uint16_t)((ithReadRegA(SARADC_REG_XAIN_RWPTR_5) & SARADC_BIT_XAIN_WPTR_5 << SARADC_SHT_XAIN_WPTR_5) >> SARADC_SHT_XAIN_WPTR_5);
        break;

    case SARADC_6:
        *wptr = (uint16_t)((ithReadRegA(SARADC_REG_XAIN_RWPTR_6) & SARADC_BIT_XAIN_WPTR_6 << SARADC_SHT_XAIN_WPTR_6) >> SARADC_SHT_XAIN_WPTR_6);
        break;

    case SARADC_7:
        *wptr = (uint16_t)((ithReadRegA(SARADC_REG_XAIN_RWPTR_7) & SARADC_BIT_XAIN_WPTR_7 << SARADC_SHT_XAIN_WPTR_7) >> SARADC_SHT_XAIN_WPTR_7);
        break;

    default:
        break;
    }
}

//=============================================================================
/**
 * Read average value that XAIN recorded of SARADC
 */
//=============================================================================
void
SARADC_ReadXAINAVGREG_Reg(
    SARADC_PORT hwPort,
    uint16_t    *avg)
{
    switch (hwPort)
    {
    case SARADC_0:
        *avg = (uint16_t)((ithReadRegA(SARADC_REG_XAIN_AVG_0) & SARADC_BIT_XAIN_AVG_0 << SARADC_SHT_XAIN_AVG_0) >> SARADC_SHT_XAIN_AVG_0);
        break;

    case SARADC_1:
        *avg = (uint16_t)((ithReadRegA(SARADC_REG_XAIN_AVG_0) & SARADC_BIT_XAIN_AVG_1 << SARADC_SHT_XAIN_AVG_1) >> SARADC_SHT_XAIN_AVG_1);
        break;

    case SARADC_2:
        *avg = (uint16_t)((ithReadRegA(SARADC_REG_XAIN_AVG_1) & SARADC_BIT_XAIN_AVG_2 << SARADC_SHT_XAIN_AVG_2) >> SARADC_SHT_XAIN_AVG_2);
        break;

    case SARADC_3:
        *avg = (uint16_t)((ithReadRegA(SARADC_REG_XAIN_AVG_1) & SARADC_BIT_XAIN_AVG_3 << SARADC_SHT_XAIN_AVG_3) >> SARADC_SHT_XAIN_AVG_3);
        break;

    case SARADC_4:
        *avg = (uint16_t)((ithReadRegA(SARADC_REG_XAIN_AVG_2) & SARADC_BIT_XAIN_AVG_4 << SARADC_SHT_XAIN_AVG_4) >> SARADC_SHT_XAIN_AVG_4);
        break;

    case SARADC_5:
        *avg = (uint16_t)((ithReadRegA(SARADC_REG_XAIN_AVG_2) & SARADC_BIT_XAIN_AVG_5 << SARADC_SHT_XAIN_AVG_5) >> SARADC_SHT_XAIN_AVG_5);
        break;

    case SARADC_6:
        *avg = (uint16_t)((ithReadRegA(SARADC_REG_XAIN_AVG_3) & SARADC_BIT_XAIN_AVG_6 << SARADC_SHT_XAIN_AVG_6) >> SARADC_SHT_XAIN_AVG_6);
        break;

    case SARADC_7:
        *avg = (uint16_t)((ithReadRegA(SARADC_REG_XAIN_AVG_3) & SARADC_BIT_XAIN_AVG_7 << SARADC_SHT_XAIN_AVG_7) >> SARADC_SHT_XAIN_AVG_7);
        break;

    default:
        break;
    }
}

//=============================================================================
/**
 * Reset XAIN OVERWT Interrupt of SARADC
 */
//=============================================================================
void
SARADC_ResetXAINOVERWT_Reg(
    SARADC_PORT hwPort)
{
    switch (hwPort)
    {
    case SARADC_0:
        ithSetRegBitA(SARADC_REG_XAIN_INTR, SARADC_SHT_XAIN_OVERWT_0);
        break;

    case SARADC_1:
        ithSetRegBitA(SARADC_REG_XAIN_INTR, SARADC_SHT_XAIN_OVERWT_1);
        break;

    case SARADC_2:
        ithSetRegBitA(SARADC_REG_XAIN_INTR, SARADC_SHT_XAIN_OVERWT_2);
        break;

    case SARADC_3:
        ithSetRegBitA(SARADC_REG_XAIN_INTR, SARADC_SHT_XAIN_OVERWT_3);
        break;

    case SARADC_4:
        ithSetRegBitA(SARADC_REG_XAIN_INTR, SARADC_SHT_XAIN_OVERWT_4);
        break;

    case SARADC_5:
        ithSetRegBitA(SARADC_REG_XAIN_INTR, SARADC_SHT_XAIN_OVERWT_5);
        break;

    case SARADC_6:
        ithSetRegBitA(SARADC_REG_XAIN_INTR, SARADC_SHT_XAIN_OVERWT_6);
        break;

    case SARADC_7:
        ithSetRegBitA(SARADC_REG_XAIN_INTR, SARADC_SHT_XAIN_OVERWT_7);
        break;

    default:
        break;
    }
}

//=============================================================================
/**
 * Reset average detection Interrupt of SARADC
 */
//=============================================================================
void
SARADC_ResetXAINAVGDetect_Reg(
    SARADC_PORT hwPort)
{
    switch (hwPort)
    {
    case SARADC_0:
        ithSetRegBitA(SARADC_REG_XAIN_INTR, SARADC_SHT_XAIN_ABOMAX_0);
        ithSetRegBitA(SARADC_REG_XAIN_INTR, SARADC_SHT_XAIN_UNDMIN_0);
        break;

    case SARADC_1:
        ithSetRegBitA(SARADC_REG_XAIN_INTR, SARADC_SHT_XAIN_ABOMAX_1);
        ithSetRegBitA(SARADC_REG_XAIN_INTR, SARADC_SHT_XAIN_UNDMIN_1);
        break;

    case SARADC_2:
        ithSetRegBitA(SARADC_REG_XAIN_INTR, SARADC_SHT_XAIN_ABOMAX_2);
        ithSetRegBitA(SARADC_REG_XAIN_INTR, SARADC_SHT_XAIN_UNDMIN_2);
        break;

    case SARADC_3:
        ithSetRegBitA(SARADC_REG_XAIN_INTR, SARADC_SHT_XAIN_ABOMAX_3);
        ithSetRegBitA(SARADC_REG_XAIN_INTR, SARADC_SHT_XAIN_UNDMIN_3);
        break;

    case SARADC_4:
        ithSetRegBitA(SARADC_REG_XAIN_INTR, SARADC_SHT_XAIN_ABOMAX_4);
        ithSetRegBitA(SARADC_REG_XAIN_INTR, SARADC_SHT_XAIN_UNDMIN_4);
        break;

    case SARADC_5:
        ithSetRegBitA(SARADC_REG_XAIN_INTR, SARADC_SHT_XAIN_ABOMAX_5);
        ithSetRegBitA(SARADC_REG_XAIN_INTR, SARADC_SHT_XAIN_UNDMIN_5);
        break;

    case SARADC_6:
        ithSetRegBitA(SARADC_REG_XAIN_INTR, SARADC_SHT_XAIN_ABOMAX_6);
        ithSetRegBitA(SARADC_REG_XAIN_INTR, SARADC_SHT_XAIN_UNDMIN_6);
        break;

    case SARADC_7:
        ithSetRegBitA(SARADC_REG_XAIN_INTR, SARADC_SHT_XAIN_ABOMAX_7);
        ithSetRegBitA(SARADC_REG_XAIN_INTR, SARADC_SHT_XAIN_UNDMIN_7);
        break;

    default:
        break;
    }
}

//=============================================================================
/**
 * Reset XAIN EVENT Interrupt of SARADC
 */
//=============================================================================
void
SARADC_ResetXAINEVENT_Reg(
    SARADC_PORT hwPort)
{
    switch (hwPort)
    {
    case SARADC_0:
        ithSetRegBitA(SARADC_REG_XAIN_INTR, SARADC_SHT_XAIN_EVENT_0);
        break;

    case SARADC_1:
        ithSetRegBitA(SARADC_REG_XAIN_INTR, SARADC_SHT_XAIN_EVENT_1);
        break;

    case SARADC_2:
        ithSetRegBitA(SARADC_REG_XAIN_INTR, SARADC_SHT_XAIN_EVENT_2);
        break;

    case SARADC_3:
        ithSetRegBitA(SARADC_REG_XAIN_INTR, SARADC_SHT_XAIN_EVENT_3);
        break;

    case SARADC_4:
        ithSetRegBitA(SARADC_REG_XAIN_INTR, SARADC_SHT_XAIN_EVENT_4);
        break;

    case SARADC_5:
        ithSetRegBitA(SARADC_REG_XAIN_INTR, SARADC_SHT_XAIN_EVENT_5);
        break;

    case SARADC_6:
        ithSetRegBitA(SARADC_REG_XAIN_INTR, SARADC_SHT_XAIN_EVENT_6);
        break;

    case SARADC_7:
        ithSetRegBitA(SARADC_REG_XAIN_INTR, SARADC_SHT_XAIN_EVENT_7);
        break;

    default:
        break;
    }
}

//=============================================================================
/**
 * Check XAIN OVERWT Interrupt of SARADC
 */
//=============================================================================
bool
SARADC_IsXAINOVERWT_Reg(
    SARADC_PORT hwPort)
{
    switch (hwPort)
    {
    case SARADC_0:
        if ((ithReadRegA(SARADC_REG_XAIN_INTR) & SARADC_BIT_XAIN_OVERWT_0 << SARADC_SHT_XAIN_OVERWT_0) >> SARADC_SHT_XAIN_OVERWT_0)
            return true;
        else
            return false;
        break;

    case SARADC_1:
        if ((ithReadRegA(SARADC_REG_XAIN_INTR) & SARADC_BIT_XAIN_OVERWT_1 << SARADC_SHT_XAIN_OVERWT_1) >> SARADC_SHT_XAIN_OVERWT_1)
            return true;
        else
            return false;
        break;

    case SARADC_2:
        if ((ithReadRegA(SARADC_REG_XAIN_INTR) & SARADC_BIT_XAIN_OVERWT_2 << SARADC_SHT_XAIN_OVERWT_2) >> SARADC_SHT_XAIN_OVERWT_2)
            return true;
        else
            return false;
        break;

    case SARADC_3:
        if ((ithReadRegA(SARADC_REG_XAIN_INTR) & SARADC_BIT_XAIN_OVERWT_3 << SARADC_SHT_XAIN_OVERWT_3) >> SARADC_SHT_XAIN_OVERWT_3)
            return true;
        else
            return false;
        break;

    case SARADC_4:
        if ((ithReadRegA(SARADC_REG_XAIN_INTR) & SARADC_BIT_XAIN_OVERWT_4 << SARADC_SHT_XAIN_OVERWT_4) >> SARADC_SHT_XAIN_OVERWT_4)
            return true;
        else
            return false;
        break;

    case SARADC_5:
        if ((ithReadRegA(SARADC_REG_XAIN_INTR) & SARADC_BIT_XAIN_OVERWT_5 << SARADC_SHT_XAIN_OVERWT_5) >> SARADC_SHT_XAIN_OVERWT_5)
            return true;
        else
            return false;
        break;

    case SARADC_6:
        if ((ithReadRegA(SARADC_REG_XAIN_INTR) & SARADC_BIT_XAIN_OVERWT_6 << SARADC_SHT_XAIN_OVERWT_6) >> SARADC_SHT_XAIN_OVERWT_6)
            return true;
        else
            return false;
        break;

    case SARADC_7:
        if ((ithReadRegA(SARADC_REG_XAIN_INTR) & SARADC_BIT_XAIN_OVERWT_7 << SARADC_SHT_XAIN_OVERWT_7) >> SARADC_SHT_XAIN_OVERWT_7)
            return true;
        else
            return false;
        break;

    default:
        break;
    }
}

//=============================================================================
/**
 * Check XAIN average Interrupt of SARADC
 */
//=============================================================================
SARADC_INTR_AVG
SARADC_IsXAINOutOfRange_Reg(
    SARADC_PORT hwPort)
{
    switch (hwPort)
    {
    case SARADC_0:
        if ((ithReadRegA(SARADC_REG_XAIN_INTR) & SARADC_BIT_XAIN_ABOMAX_0 << SARADC_SHT_XAIN_ABOMAX_0) >> SARADC_SHT_XAIN_ABOMAX_0)
            return SARADC_INTR_AVG_ABOMAX;
        else if ((ithReadRegA(SARADC_REG_XAIN_INTR) & SARADC_BIT_XAIN_UNDMIN_0 << SARADC_SHT_XAIN_UNDMIN_0) >> SARADC_SHT_XAIN_UNDMIN_0)
            return SARADC_INTR_AVG_UNDMIN;
        else
            return SARADC_INTR_AVG_VALID;
        break;

    case SARADC_1:
        if ((ithReadRegA(SARADC_REG_XAIN_INTR) & SARADC_BIT_XAIN_ABOMAX_1 << SARADC_SHT_XAIN_ABOMAX_1) >> SARADC_SHT_XAIN_ABOMAX_1)
            return SARADC_INTR_AVG_ABOMAX;
        else if ((ithReadRegA(SARADC_REG_XAIN_INTR) & SARADC_BIT_XAIN_UNDMIN_1 << SARADC_SHT_XAIN_UNDMIN_1) >> SARADC_SHT_XAIN_UNDMIN_1)
            return SARADC_INTR_AVG_UNDMIN;
        else
            return SARADC_INTR_AVG_VALID;
        break;

    case SARADC_2:
        if ((ithReadRegA(SARADC_REG_XAIN_INTR) & SARADC_BIT_XAIN_ABOMAX_2 << SARADC_SHT_XAIN_ABOMAX_2) >> SARADC_SHT_XAIN_ABOMAX_2)
            return SARADC_INTR_AVG_ABOMAX;
        else if ((ithReadRegA(SARADC_REG_XAIN_INTR) & SARADC_BIT_XAIN_UNDMIN_2 << SARADC_SHT_XAIN_UNDMIN_2) >> SARADC_SHT_XAIN_UNDMIN_2)
            return SARADC_INTR_AVG_UNDMIN;
        else
            return SARADC_INTR_AVG_VALID;
        break;

    case SARADC_3:
        if ((ithReadRegA(SARADC_REG_XAIN_INTR) & SARADC_BIT_XAIN_ABOMAX_3 << SARADC_SHT_XAIN_ABOMAX_3) >> SARADC_SHT_XAIN_ABOMAX_3)
            return SARADC_INTR_AVG_ABOMAX;
        else if ((ithReadRegA(SARADC_REG_XAIN_INTR) & SARADC_BIT_XAIN_UNDMIN_3 << SARADC_SHT_XAIN_UNDMIN_3) >> SARADC_SHT_XAIN_UNDMIN_3)
            return SARADC_INTR_AVG_UNDMIN;
        else
            return SARADC_INTR_AVG_VALID;
        break;

    case SARADC_4:
        if ((ithReadRegA(SARADC_REG_XAIN_INTR) & SARADC_BIT_XAIN_ABOMAX_4 << SARADC_SHT_XAIN_ABOMAX_4) >> SARADC_SHT_XAIN_ABOMAX_4)
            return SARADC_INTR_AVG_ABOMAX;
        else if ((ithReadRegA(SARADC_REG_XAIN_INTR) & SARADC_BIT_XAIN_UNDMIN_4 << SARADC_SHT_XAIN_UNDMIN_4) >> SARADC_SHT_XAIN_UNDMIN_4)
            return SARADC_INTR_AVG_UNDMIN;
        else
            return SARADC_INTR_AVG_VALID;
        break;

    case SARADC_5:
        if ((ithReadRegA(SARADC_REG_XAIN_INTR) & SARADC_BIT_XAIN_ABOMAX_5 << SARADC_SHT_XAIN_ABOMAX_5) >> SARADC_SHT_XAIN_ABOMAX_5)
            return SARADC_INTR_AVG_ABOMAX;
        else if ((ithReadRegA(SARADC_REG_XAIN_INTR) & SARADC_BIT_XAIN_UNDMIN_5 << SARADC_SHT_XAIN_UNDMIN_5) >> SARADC_SHT_XAIN_UNDMIN_5)
            return SARADC_INTR_AVG_UNDMIN;
        else
            return SARADC_INTR_AVG_VALID;
        break;

    case SARADC_6:
        if ((ithReadRegA(SARADC_REG_XAIN_INTR) & SARADC_BIT_XAIN_ABOMAX_6 << SARADC_SHT_XAIN_ABOMAX_6) >> SARADC_SHT_XAIN_ABOMAX_6)
            return SARADC_INTR_AVG_ABOMAX;
        else if ((ithReadRegA(SARADC_REG_XAIN_INTR) & SARADC_BIT_XAIN_UNDMIN_6 << SARADC_SHT_XAIN_UNDMIN_6) >> SARADC_SHT_XAIN_UNDMIN_6)
            return SARADC_INTR_AVG_UNDMIN;
        else
            return SARADC_INTR_AVG_VALID;
        break;

    case SARADC_7:
        if ((ithReadRegA(SARADC_REG_XAIN_INTR) & SARADC_BIT_XAIN_ABOMAX_7 << SARADC_SHT_XAIN_ABOMAX_7) >> SARADC_SHT_XAIN_ABOMAX_7)
            return SARADC_INTR_AVG_ABOMAX;
        else if ((ithReadRegA(SARADC_REG_XAIN_INTR) & SARADC_BIT_XAIN_UNDMIN_7 << SARADC_SHT_XAIN_UNDMIN_7) >> SARADC_SHT_XAIN_UNDMIN_7)
            return SARADC_INTR_AVG_UNDMIN;
        else
            return SARADC_INTR_AVG_VALID;
        break;

    default:
        break;
    }
}

//=============================================================================
/**
 * Check XAIN EVENT Interrupt of SARADC
 */
//=============================================================================
bool
SARADC_IsXAINEVENTOccurrence_Reg(
    SARADC_PORT hwPort)
{
    switch (hwPort)
    {
    case SARADC_0:
        if ((ithReadRegA(SARADC_REG_XAIN_INTR) & SARADC_BIT_XAIN_EVENT_0 << SARADC_SHT_XAIN_EVENT_0) >> SARADC_SHT_XAIN_EVENT_0)
            return true;
        else
            return false;
        break;

    case SARADC_1:
        if ((ithReadRegA(SARADC_REG_XAIN_INTR) & SARADC_BIT_XAIN_EVENT_1 << SARADC_SHT_XAIN_EVENT_1) >> SARADC_SHT_XAIN_EVENT_1)
            return true;
        else
            return false;
        break;

    case SARADC_2:
        if ((ithReadRegA(SARADC_REG_XAIN_INTR) & SARADC_BIT_XAIN_EVENT_2 << SARADC_SHT_XAIN_EVENT_2) >> SARADC_SHT_XAIN_EVENT_2)
            return true;
        else
            return false;
        break;

    case SARADC_3:
        if ((ithReadRegA(SARADC_REG_XAIN_INTR) & SARADC_BIT_XAIN_EVENT_3 << SARADC_SHT_XAIN_EVENT_3) >> SARADC_SHT_XAIN_EVENT_3)
            return true;
        else
            return false;
        break;

    case SARADC_4:
        if ((ithReadRegA(SARADC_REG_XAIN_INTR) & SARADC_BIT_XAIN_EVENT_4 << SARADC_SHT_XAIN_EVENT_4) >> SARADC_SHT_XAIN_EVENT_4)
            return true;
        else
            return false;
        break;

    case SARADC_5:
        if ((ithReadRegA(SARADC_REG_XAIN_INTR) & SARADC_BIT_XAIN_EVENT_5 << SARADC_SHT_XAIN_EVENT_5) >> SARADC_SHT_XAIN_EVENT_5)
            return true;
        else
            return false;
        break;

    case SARADC_6:
        if ((ithReadRegA(SARADC_REG_XAIN_INTR) & SARADC_BIT_XAIN_EVENT_6 << SARADC_SHT_XAIN_EVENT_6) >> SARADC_SHT_XAIN_EVENT_6)
            return true;
        else
            return false;
        break;

    case SARADC_7:
        if ((ithReadRegA(SARADC_REG_XAIN_INTR) & SARADC_BIT_XAIN_EVENT_7 << SARADC_SHT_XAIN_EVENT_7) >> SARADC_SHT_XAIN_EVENT_7)
            return true;
        else
            return false;
        break;

    default:
        break;
    }
}

