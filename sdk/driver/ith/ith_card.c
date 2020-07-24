/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * HAL Card functions.
 *
 * @author Jim Tan
 * @version 1.0
 */
#include <string.h>
#include <unistd.h>
#include "ith_cfg.h"

static ITHCardConfig cardCfg;

void ithCardInit(const ITHCardConfig *cfg)
{
    memcpy(&cardCfg, cfg, sizeof(ITHCardConfig));
}

void ithCardPowerOn(ITHCardPin pin)
{
#ifndef CFG_ITH_FPGA
    unsigned int num = cardCfg.powerEnablePins[pin];
    if (num == (unsigned char)-1)
        return;

    ithGpioEnable(num);
    ithGpioSetOut(num);
    ithGpioClear(num);
#endif // !CFG_ITH_FPGA
}

void ithCardPowerOff(ITHCardPin pin)
{
#ifndef CFG_ITH_FPGA
    unsigned int num = cardCfg.powerEnablePins[pin];
    if (num == (unsigned char)-1)
        return;

    ithGpioEnable(num);
    ithGpioSetOut(num);
    ithGpioSet(num);
#endif // !CFG_ITH_FPGA
}

#if defined(CFG_GPIO_SD1_WIFI_POWER_ENABLE) && defined(CFG_NET_WIFI_SDIO_NGPL)
void ithWIFICardPowerOn(void)
{
    if (CFG_GPIO_SD1_WIFI_POWER_PIN == -1)
        return;

    ithGpioEnable((unsigned int)CFG_GPIO_SD1_WIFI_POWER_PIN);
    ithGpioSetOut((unsigned int)CFG_GPIO_SD1_WIFI_POWER_PIN);
#ifdef CFG_SD1_WIFI_DETECT_ACTIVE_HIGH
    ithGpioSet((unsigned int)CFG_GPIO_SD1_WIFI_POWER_PIN);
#else
    ithGpioClear((unsigned int)CFG_GPIO_SD1_WIFI_POWER_PIN);
#endif
}

void ithWIFICardPowerOff(void)
{
    if (CFG_GPIO_SD1_WIFI_POWER_PIN == -1)
        return;

    ithGpioEnable((unsigned int)CFG_GPIO_SD1_WIFI_POWER_PIN);
    ithGpioSetOut((unsigned int)CFG_GPIO_SD1_WIFI_POWER_PIN);
#ifdef CFG_SD1_WIFI_DETECT_ACTIVE_HIGH
    ithGpioClear((unsigned int)CFG_GPIO_SD1_WIFI_POWER_PIN);
#else
    ithGpioSet((unsigned int)CFG_GPIO_SD1_WIFI_POWER_PIN);
#endif
}
#endif

bool ithCardInserted(ITHCardPin pin)
{
#ifdef CFG_ITH_FPGA
    return true;
#else
    unsigned int num = cardCfg.cardDetectPins[pin];
    if (num == (unsigned char)-1)
        return false;
    if (num == (unsigned char)-2) // always insert
        return true;

    ithGpioEnable(num);
    ithGpioSetIn(num);

    if (pin == ITH_CARDPIN_SD0)
    {
    #if defined(CFG_SD0_CARD_DETECT_ACTIVE_HIGH)
        return ithGpioGet(num);
    #endif
    }
    if (pin == ITH_CARDPIN_SD1)
    {
    #if defined(CFG_SD1_CARD_DETECT_ACTIVE_HIGH)
        return ithGpioGet(num);
    #endif
    }
    return !ithGpioGet(num);
#endif // CFG_ITH_FPGA
}

bool ithCardLocked(ITHCardPin pin)
{
#ifdef CFG_ITH_FPGA
    return false;
#else
    unsigned int num;

    num = cardCfg.writeProtectPins[pin];
    if (num == (unsigned char)-1)
        return false;

    ithGpioEnable(num);
    ithGpioSetIn(num);
    return ithGpioGet(num) ? true : false;
#endif // CFG_ITH_FPGA
}

#if (CFG_CHIP_FAMILY == 9860)

#define ITH_SDG_BASE 0xB0900000

#define SET_SD_DATA_MODE(x)    ((x & 0x7) << 0)
#define SET_SD_CLK_MODE(x)     ((x & 0x7) << 4)
#define SET_SD_PIN_IDX(x)      ((x & 0xF) << 8)  /* 0:D0, ~ 7:D7, 8:CMD */
#define SET_SDC(idx)           (idx << 12)

#define GET_SD_DATA_MODE(gpio) ((sd_gpio_tlb[gpio] >> 0) & 0x7)
#define GET_SD_CLK_MODE(gpio)  ((sd_gpio_tlb[gpio] >> 4) & 0x7)
#define GET_SD_PIN_INDEX(gpio) ((sd_gpio_tlb[gpio] >> 8) & 0xF)
#define GET_SDC(gpio)          ((sd_gpio_tlb[gpio] >> 12) & 0x1)

static uint16_t sd_gpio_tlb[64] = {
    SET_SDC(0) | SET_SD_PIN_IDX(4) | SET_SD_DATA_MODE(2),                       // gpio 0
    SET_SDC(0) | SET_SD_PIN_IDX(5) | SET_SD_DATA_MODE(2),                       // gpio 1
    SET_SDC(0) | SET_SD_PIN_IDX(6) | SET_SD_DATA_MODE(2),                       // gpio 2
    SET_SDC(0) | SET_SD_PIN_IDX(7) | SET_SD_DATA_MODE(2),                       // gpio 3
    SET_SDC(1) | SET_SD_PIN_IDX(1) | SET_SD_DATA_MODE(2),                       // gpio 4
    0,                                                                          // gpio 5
    SET_SDC(0) | SET_SD_PIN_IDX(0) | SET_SD_DATA_MODE(2),                       // gpio 6
    SET_SDC(0) | SET_SD_PIN_IDX(1) | SET_SD_DATA_MODE(2),                       // gpio 7
    SET_SDC(0) | SET_SD_PIN_IDX(2) | SET_SD_DATA_MODE(2),                       // gpio 8
    SET_SDC(0) | SET_SD_PIN_IDX(3) | SET_SD_DATA_MODE(2),                       // gpio 9
    SET_SDC(0) | SET_SD_PIN_IDX(3) | SET_SD_DATA_MODE(2),                       // gpio 10
    SET_SD_CLK_MODE(2),                                                         // gpio 11
    SET_SD_CLK_MODE(2),                                                         // gpio 12
    SET_SDC(0) | SET_SD_PIN_IDX(8) | SET_SD_DATA_MODE(2),                       // gpio 13
    SET_SD_CLK_MODE(2),                                                         // gpio 14
    SET_SDC(0) | SET_SD_PIN_IDX(0) | SET_SD_DATA_MODE(2),                       // gpio 15
    SET_SDC(0) | SET_SD_PIN_IDX(1) | SET_SD_DATA_MODE(2),                       // gpio 16
    SET_SDC(0) | SET_SD_PIN_IDX(2) | SET_SD_DATA_MODE(2),                       // gpio 17
    SET_SDC(0) | SET_SD_PIN_IDX(3) | SET_SD_DATA_MODE(2),                       // gpio 18
    SET_SDC(1) | SET_SD_PIN_IDX(1) | SET_SD_DATA_MODE(2),                       // gpio 19
    SET_SDC(1) | SET_SD_PIN_IDX(8) | SET_SD_DATA_MODE(2),                      // gpio 20
    0,                                                                          // gpio 21
    0,                                                                          // gpio 22
    0,                                                                          // gpio 23
    0,                                                                          // gpio 24
    0,                                                                          // gpio 25
    0,                                                                          // gpio 26
    SET_SDC(0) | SET_SD_PIN_IDX(1) | SET_SD_DATA_MODE(2),                       // gpio 27
    SET_SDC(0) | SET_SD_PIN_IDX(8) | SET_SD_DATA_MODE(2),                       // gpio 28
    SET_SD_CLK_MODE(2),                                                         // gpio 29
    0,                                                                          // gpio 30
    0,                                                                          // gpio 31
    SET_SD_CLK_MODE(2),                                                         // gpio 32
    SET_SDC(1) | SET_SD_PIN_IDX(8) | SET_SD_DATA_MODE(2),                       // gpio 33
    SET_SDC(1) | SET_SD_PIN_IDX(0) | SET_SD_DATA_MODE(2),                       // gpio 34
    SET_SDC(1) | SET_SD_PIN_IDX(1) | SET_SD_DATA_MODE(2),                       // gpio 35
    SET_SDC(1) | SET_SD_PIN_IDX(2) | SET_SD_DATA_MODE(2),                       // gpio 36
    SET_SDC(1) | SET_SD_PIN_IDX(3) | SET_SD_DATA_MODE(2),                       // gpio 37
    0,                                                                          // gpio 38
    0,                                                                          // gpio 39
    0,                                                                          // gpio 40
    0,                                                                          // gpio 41
    SET_SD_CLK_MODE(4),                                                         // gpio 42
    SET_SDC(1) | SET_SD_PIN_IDX(8) | SET_SD_DATA_MODE(4),                       // gpio 43
    SET_SDC(1) | SET_SD_PIN_IDX(0) | SET_SD_DATA_MODE(4),                       // gpio 44
    SET_SDC(1) | SET_SD_PIN_IDX(1) | SET_SD_DATA_MODE(4),                       // gpio 45
    SET_SDC(1) | SET_SD_PIN_IDX(2) | SET_SD_DATA_MODE(4),                       // gpio 46
    SET_SDC(1) | SET_SD_PIN_IDX(3) | SET_SD_DATA_MODE(4),                       // gpio 47
    SET_SDC(1) | SET_SD_PIN_IDX(4) | SET_SD_DATA_MODE(4),                       // gpio 48
    SET_SDC(1) | SET_SD_PIN_IDX(5) | SET_SD_DATA_MODE(4),                       // gpio 49
    0,                                                                          // gpio 50
    SET_SDC(1) | SET_SD_PIN_IDX(6) | SET_SD_DATA_MODE(4) | SET_SD_CLK_MODE(3),  // gpio 51
    SET_SDC(0) | SET_SD_PIN_IDX(8) | SET_SD_DATA_MODE(3),                       // gpio 52  // ???????
    SET_SDC(0) | SET_SD_PIN_IDX(0) | SET_SD_DATA_MODE(3),                       // gpio 53
    SET_SDC(0) | SET_SD_PIN_IDX(1) | SET_SD_DATA_MODE(3),                       // gpio 54
    SET_SDC(0) | SET_SD_PIN_IDX(2) | SET_SD_DATA_MODE(3),                       // gpio 55
    SET_SDC(0) | SET_SD_PIN_IDX(3) | SET_SD_DATA_MODE(3),                       // gpio 56
    SET_SDC(0) | SET_SD_PIN_IDX(4) | SET_SD_DATA_MODE(3),                       // gpio 57
    SET_SDC(0) | SET_SD_PIN_IDX(5) | SET_SD_DATA_MODE(3),                       // gpio 58
    SET_SDC(0) | SET_SD_PIN_IDX(6) | SET_SD_DATA_MODE(3),                       // gpio 59
    SET_SDC(0) | SET_SD_PIN_IDX(7) | SET_SD_DATA_MODE(3),                       // gpio 60
    SET_SDC(1) | SET_SD_PIN_IDX(1) | SET_SD_DATA_MODE(3),                       // gpio 61
    SET_SDC(1) | SET_SD_PIN_IDX(8) | SET_SD_DATA_MODE(3),                       // gpio 62
    SET_SD_CLK_MODE(3)                                                          // gpio 63
};

void ithStorageUnSelect(ITHStorage storage)
{
    int           i = 0;
    unsigned char num;

    switch (storage)
    {
    case ITH_STOR_SD:
        for (i = 0; i < SD_PIN_NUM; i++)
        {
            num = cardCfg.sd0Pins[i];
            if (num != (unsigned char)-1)
            {
                ithGpioSetMode(num, ITH_GPIO_MODE0);            /* set input mode */
                ithGpioCtrlDisable(num, ITH_GPIO_PULL_ENABLE);  /* sd io pull disable */
            }
        }
        break;
    case ITH_STOR_SD1:
        for (i = 0; i < SD_PIN_NUM; i++)
        {
            num = cardCfg.sd1Pins[i];
            if (num != (unsigned char)-1)
            {
                ithGpioSetMode(num, ITH_GPIO_MODE0);            /* set input mode */
                ithGpioCtrlDisable(num, ITH_GPIO_PULL_ENABLE);  /* sd io pull disable */
            }
        }
        break;
    default:
        break;
    }
}

void ithStorageSelect(ITHStorage storage)
{
    int           i = 0;
    unsigned char num, mode;
    uint32_t      sd_pin_sel = 0;

    #if defined(CFG_SD0_ENABLE)
    /* disable sd0 clock */
    num = cardCfg.sd0Pins[0];
    if (num != (unsigned char)-1)
        ithGpioSetMode(num, ITH_GPIO_MODE0);
    /* switch cmd to gpio mode, 
       avoid 2 gpio with same controller cause command response will always or 1. */
    num = cardCfg.sd0Pins[1];
    ithGpioSetMode(num, ITH_GPIO_MODE0);
    #endif

    #if defined(CFG_SD1_ENABLE)
    /* disable sd1 clock */
    num = cardCfg.sd1Pins[0];
    if (num != (unsigned char)-1)
        ithGpioSetMode(num, ITH_GPIO_MODE0);
    /* switch cmd to gpio mode,
       avoid 2 gpio with same controller cause command response will always or 1. */
    num = cardCfg.sd1Pins[1];
    ithGpioSetMode(num, ITH_GPIO_MODE0);
    #endif

    switch (storage)
    {
    case ITH_STOR_SD:
        {
    #if 0   //defined(CFG_SD1_ENABLE)
         /* disable sd1 clock */
            num = cardCfg.sd1Pins[0];
            if (num != (unsigned char)-1)
                ithGpioSetMode(num, ITH_GPIO_MODE0);
    #endif

            /* switch to sd0 io */
            //for (i = 0; i < SD_PIN_NUM; i++)
            for (i = (SD_PIN_NUM - 1); i >= 0; i--) /* clock is the last to switch back */
            {
                num = cardCfg.sd0Pins[i];
                if (num != (unsigned char)-1)
                {
                    mode = (i == 0) ? GET_SD_CLK_MODE(num) : GET_SD_DATA_MODE(num);
                    ithGpioSetMode(num, mode);
                    //ithGpioCtrlEnable(num, ITH_GPIO_PULL_ENABLE);      /* sd io pull up */
                    //ithGpioCtrlEnable(num, ITH_GPIO_PULL_UP);
                    ithGpioSetDriving(num, ITH_GPIO_DRIVING_1);
                }
                if ((i != 0) && (num != (unsigned char)-1))
                    sd_pin_sel |= (GET_SDC(num) == 0) ? 0 : (1 << GET_SD_PIN_INDEX(num));
            }
            ithWriteRegA(ITH_SDG_BASE + 0x0, sd_pin_sel);
        }
        break;
    case ITH_STOR_SD1:
        {
    #if 0   //defined(CFG_SD0_ENABLE)
         /* disable sd0 clock */
            num = cardCfg.sd0Pins[0];
            if (num != (unsigned char)-1)
                ithGpioSetMode(num, ITH_GPIO_MODE0);
    #endif

            /* switch to sd1 io */
            // for (i = 0; i < SD_PIN_NUM; i++)
            for (i = (SD_PIN_NUM - 1); i >= 0; i--) /* clock is the last to switch back */
            {
                num = cardCfg.sd1Pins[i];
                if (num != (unsigned char)-1)
                {
                    mode = (i == 0) ? GET_SD_CLK_MODE(num) : GET_SD_DATA_MODE(num);
                    ithGpioSetMode(num, mode);
                    //ithGpioCtrlEnable(num, ITH_GPIO_PULL_ENABLE);      /* sd io pull up */
                    //ithGpioCtrlEnable(num, ITH_GPIO_PULL_UP);
                    ithGpioSetDriving(num, ITH_GPIO_DRIVING_1);
                }
                if ((i != 0) && (num != (unsigned char)-1))
                    sd_pin_sel |= (GET_SDC(num) == 1) ? 0 : (1 << GET_SD_PIN_INDEX(num));
            }
            ithWriteRegA(ITH_SDG_BASE + 0x0, sd_pin_sel);
        }
        break;
    case ITH_STOR_NOR:
    #ifdef CFG_SPI0_40MHZ_ENABLE
        ithWriteRegMaskA(ITH_SSP0_BASE + 0x74, CFG_SPI0_NORCLK_PARA << 16, (0xFF << 16));
    #endif
        break;
    case ITH_STOR_NAND:
    #ifdef CFG_SPI0_40MHZ_ENABLE
        ithWriteRegMaskA(ITH_SSP0_BASE + 0x74, CFG_SPI0_NANDCLK_PARA << 16, (0xFF << 16));
    #endif
        break;
    default:
        break;
    }
}

#endif

void ithSdPowerReset(ITHStorage storage)
{
    unsigned int gpio_num;

    switch (storage)
    {
    case ITH_STOR_SD:
        gpio_num = cardCfg.powerEnablePins[ITH_CARDPIN_SD0];
        if (gpio_num == (unsigned char)-1)
            return;
        ithStorageUnSelect(ITH_STOR_SD);
        ithCardPowerOff(ITH_CARDPIN_SD0);
        usleep(30 * 1000);
        ithCardPowerOn(ITH_CARDPIN_SD0);
        usleep(35 * 1000);
        break;
    case ITH_STOR_SD1:
        gpio_num = cardCfg.powerEnablePins[ITH_CARDPIN_SD1];
        if (gpio_num == (unsigned char)-1)
            return;
        ithStorageUnSelect(ITH_STOR_SD1);
        ithCardPowerOff(ITH_CARDPIN_SD1);
        usleep(30 * 1000);
        ithCardPowerOn(ITH_CARDPIN_SD1);
        usleep(35 * 1000);
        break;
    default:
        break;
    }
}

int ithSd4bitMode(ITHStorage storage)
{
    switch (storage)
    {
    case ITH_STOR_SD:
        #if defined(CFG_SD0_CARD_1BIT)
        return 0;
        #else
        return 1;
        #endif
    case ITH_STOR_SD1:
        #if defined(CFG_SD1_CARD_1BIT)
        return 0;
        #else
        return 1;
        #endif
    default:
        return 0;
    }
}

int ithSdio4bitMode(void)
{
#if defined(CFG_SDIO_4BIT_MODE)
    return 1;
#else
    return 0;
#endif
}

int ithSdMaxClk(ITHStorage storage)
{
    switch (storage)
    {
    case ITH_STOR_SD:
        return 50000000;
    case ITH_STOR_SD1:
        return 50000000;
    default:
        return 300000;
    }
}

int ithSdNoPinShare(ITHStorage storage)
{
    switch (storage)
    {
    case ITH_STOR_SD:
        #if defined(CFG_SD0_NO_PIN_SHARE)
        return 1;
        #else
        return 0;
        #endif
    case ITH_STOR_SD1:
        #if defined(CFG_SD1_NO_PIN_SHARE)
        return 1;
        #else
        return 0;
        #endif
    default:
        return 0;
    }
}

#define SD_DELAY_REG     0xB0900004

uint32_t ithSdDelay(ITHStorage storage)
{
    uint32_t delay_offset = (storage == ITH_STOR_SD) ? 0 : 4;
	uint32_t delay_val = 0;

    switch (storage)
    {
    case ITH_STOR_SD:
        ithWriteRegMaskA(SD_DELAY_REG, (delay_val << delay_offset), (0xF << delay_offset));
        return 0x00000101;
    case ITH_STOR_SD1:
        ithWriteRegMaskA(SD_DELAY_REG, (delay_val << delay_offset), (0xF << delay_offset));
        return 0x00000101;
    default:
        return 0x00000101;
    }
}
