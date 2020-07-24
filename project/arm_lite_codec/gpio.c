/*
 * Copyright ITE Tech. Inc. All Rights Reserved.
 */
/* @file
 * Include file for GPIO Control
 *
 * @author Steven Hsiao
 * @date 2018.06.13.
 * @version 1.0
 *
 */
#define GPIO_BASE               0xD1000000
#define GPIO_PINDIR_OFFSET      0x08
#define GPIO_MODE_OFFSET        0x60
#define GPIO_DATASET_OFFSET     0xC
#define GPIO_DATACLR_OFFSET     0x10
#define GPIO_DATAIN_OFFSET      0x4
#define GPIO_DATAOUT_OFFSET     0x0

#define GPIO_DIR_BASE       (GPIO_BASE + GPIO_PINDIR_OFFSET)
#define GPIO_MODE_BASE      (GPIO_BASE + GPIO_MODE_OFFSET)
#define GPIO_SET_BASE       (GPIO_BASE + GPIO_DATASET_OFFSET)
#define GPIO_CLEAR_BASE     (GPIO_BASE + GPIO_DATACLR_OFFSET)
#define GPIO_IN_DATA_BASE   (GPIO_BASE + GPIO_DATAIN_OFFSET)
#define GPIO_OUT_DATA_BASE  (GPIO_BASE + GPIO_DATAOUT_OFFSET)

static __inline void RegWrite(unsigned long addr, unsigned long data) {
    *(volatile unsigned long *) (addr) = data;
}

static __inline unsigned long RegRead(unsigned long addr) {
    return *(volatile unsigned long *) (addr);
}

static __inline void RegWriteMask(unsigned long addr, unsigned long data, unsigned long mask) {
    RegWrite(addr, ((RegRead(addr) & ~mask) | (data & mask)));
}

void setGpioDir(unsigned long gpioPin, unsigned long bIn)
{
    unsigned long regAddr = GPIO_DIR_BASE + ((gpioPin >> 5) * 0x80);
    unsigned long bitOffset = (gpioPin & 0x1F);

    if (bIn)
    {
        RegWriteMask(regAddr, (0x0 << bitOffset), (0x1 << bitOffset));
    }
    else
    {
        RegWriteMask(regAddr, (0x1 << bitOffset), (0x1 << bitOffset));
    }
}

void setGpioMode(unsigned long gpioPin, unsigned long mode)
{
    unsigned long regAddr = GPIO_MODE_BASE + ((gpioPin >> 5) * 0x80) + (((gpioPin & 0x1F) >> 3) * 0x4);
    unsigned long bitOffset = ((gpioPin & 0x7) << 2);

    RegWriteMask(regAddr, (mode << bitOffset), (0xF << bitOffset));
}

unsigned long getGpioValue(unsigned long gpioPin, unsigned long bIn)
{
    unsigned long regAddr = 0;
    unsigned long bitOffset = (gpioPin & 0x1F);
    unsigned long gpioValue = 0;
    if (bIn)
    {
        regAddr = GPIO_IN_DATA_BASE + ((gpioPin >> 5) * 0x80);
    }
    else
    {
        regAddr = GPIO_OUT_DATA_BASE + ((gpioPin >> 5) * 0x80);
    } 

    gpioValue = RegRead(regAddr);
    if ((gpioValue & (0x1 << bitOffset)))
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

void setGpioValue(unsigned long gpioPin, unsigned long bHigh)
{
    unsigned long regAddr = 0;
    unsigned long bitOffset = (gpioPin & 0x1F);
    if (bHigh)
    {
        regAddr = GPIO_SET_BASE + ((gpioPin >> 5) * 0x80);
    }
    else
    {
        regAddr = GPIO_CLEAR_BASE + ((gpioPin >> 5) * 0x80);
    }

    RegWrite(regAddr, (0x1 << bitOffset));
}
