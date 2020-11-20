/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * HAL UART functions.
 *
 * @author Jim Tan
 * @version 1.0
 */
#include "ith_cfg.h"

static uint32_t gFCRvalue[6] = { 0 };

static uint32_t *getFCRvalue(ITHUartPort port)
{
    if (port == ITH_UART0)
        return &gFCRvalue[0];
    else if (port == ITH_UART1)
        return &gFCRvalue[1];
    else if (port == ITH_UART2)
        return &gFCRvalue[2];
	else if (port == ITH_UART3)
		return &gFCRvalue[3];
	else if (port == ITH_UART4)
		return &gFCRvalue[4];
	else
		return &gFCRvalue[5];
}

/**
 * Enables specified FIFO controls.
 *
 * @param ctrl the controls to enable.
 */
void ithUartFifoCtrlEnable(ITHUartPort port, ITHUartFifoCtrl ctrl)
{
    uint32_t *pValue = getFCRvalue(port);

    *pValue |= (1 << ITH_UART_FCR_FIFO_EN_BIT);
    ithWriteRegA(port + ITH_UART_FCR_REG, *pValue);
    //ithWriteRegA(0xde600108, 1);
    //ithWriteRegH(0x16B0 ,port + ITH_UART_FCR_REG);
}

/**
 * Disables specified FIFO controls.
 *
 * @param ctrl the controls to disable.
 */
void ithUartFifoCtrlDisable(ITHUartPort port, ITHUartFifoCtrl ctrl)
{
    uint32_t *pValue = getFCRvalue(port);

    *pValue &= ~(0x1 << ITH_UART_FCR_FIFO_EN_BIT);
    ithWriteRegA(port + ITH_UART_FCR_REG, *pValue);
}

/**
 * Sets UART TX interrupt trigger level.
 *
 * @param port The UART port
 * @param level The UART TX trigger level
 */
void ithUartSetTxTriggerLevel(
    ITHUartPort         port,
    ITHUartTriggerLevel level)
{
    uint32_t *pValue = getFCRvalue(port);

    *pValue &= ~ITH_UART_FCR_TXFIFO_TRGL_MASK;
    *pValue |= (level << ITH_UART_FCR_TXFIFO_TRGL_BIT);
    ithWriteRegA(port + ITH_UART_FCR_REG, (*pValue | (0x1 << ITH_UART_FCR_TXFIFO_RESET_BIT)));
}

/**
 * Sets UART RX interrupt trigger level.
 *
 * @param port The UART port
 * @param level The UART RX trigger level
 */
void ithUartSetRxTriggerLevel(
    ITHUartPort         port,
    ITHUartTriggerLevel level)
{
    uint32_t *pValue = getFCRvalue(port);

    *pValue &= ~ITH_UART_FCR_RXFIFO_TRGL_MASK;
    *pValue |= (level << ITH_UART_FCR_RXFIFO_TRGL_BIT);
    ithWriteRegA(port + ITH_UART_FCR_REG, (*pValue | (0x1 << ITH_UART_FCR_RXFIFO_RESET_BIT)));
}

void ithUartSetMode(
    ITHUartPort     port,
    ITHUartMode     mode,
    unsigned int    txPin,
    unsigned int    rxPin)
{
    int txgpiomode = -1, rxgpiomode = -1;

    switch (port)
    {
    case ITH_UART0:
#if ((CFG_CHIP_FAMILY == 9070) || (CFG_CHIP_FAMILY == 9910))
        ithWriteRegMaskA(ITH_GPIO_BASE + ITH_GPIO_MISC_SET_REG, ITH_GPIO_HOSTSEL_GPIO << ITH_GPIO_HOST_SEL_POS, ITH_GPIO_HOST_SEL_MSK);

    #if (CFG_CHIP_FAMILY == 9910)
        if (mode != ITH_UART_TX)
            ithGpioSetMode(0, ITH_GPIO_MODE1);

        ithGpioSetMode(1, ITH_GPIO_MODE1);
    #else
        if (mode != ITH_UART_TX)
            ithGpioSetMode(2, ITH_GPIO_MODE1);

        ithGpioSetMode(3, ITH_GPIO_MODE1);

        // IrDA
        if (mode != ITH_UART_DEFAULT)
        {
            ithGpioSetMode(6, ITH_GPIO_MODE1);

            if (mode == ITH_UART_FIR)
                ithSetRegBitH(ITH_UART_CLK_REG, ITH_UART_CLK_SRC_BIT);
        }
    #endif
#endif
        txgpiomode  = ITH_GPIO_MODE_TX0;
        rxgpiomode  = ITH_GPIO_MODE_RX0;
        break;
    case ITH_UART1:
        txgpiomode  = ITH_GPIO_MODE_TX1;
        rxgpiomode  = ITH_GPIO_MODE_RX1;
        break;
    case ITH_UART2:
        txgpiomode  = ITH_GPIO_MODE_TX2;
        rxgpiomode  = ITH_GPIO_MODE_RX2;
        break;
    case ITH_UART3:
        txgpiomode  = ITH_GPIO_MODE_TX3;
        rxgpiomode  = ITH_GPIO_MODE_RX3;
        break;
    case ITH_UART4:
        txgpiomode  = ITH_GPIO_MODE_TX4;
        rxgpiomode  = ITH_GPIO_MODE_RX4;
        break;
    case ITH_UART5:
        txgpiomode  = ITH_GPIO_MODE_TX5;
        rxgpiomode  = ITH_GPIO_MODE_RX5;
        break;
    default:
        txgpiomode  = ITH_GPIO_MODE_TX1;
        rxgpiomode  = ITH_GPIO_MODE_RX1;
        break;
    }

#if (CFG_CHIP_FAMILY == 9860)
	if (txPin >= 21 && txPin <= 24)
	{
		ithPrintf("Hit fully mux special case, tx pin: %d change to %d\n", txPin, txPin-21);
		txPin -= 21;
	}
	if (rxPin >= 21 && rxPin <= 24)
		ithPrintf("Hit fully mux special case, rx pin: %d won't work properly.\n", rxPin);

	if ((txPin >= 68 && txPin <= 77) || (rxPin >= 68 && rxPin <= 77)) // using MIPI GPIO
		ithWriteRegMaskA(ITH_MIPI_DPHY_BASE, 0x2 << 22, 0x3 << 22); // set MIPI DPHY R0 to TTL
#endif

    // cannot be IrDA mode
    if (mode != ITH_UART_TX && rxPin != -1)
    {
        ithGpioSetMode(rxPin, rxgpiomode);
        ithGpioSetIn(rxPin);
    }
	// ignore gpio -1
	if(txPin != -1)
	{
    	ithGpioSetMode(txPin, txgpiomode);
    	ithGpioSetOut(txPin);
	}

    ithWriteRegMaskA(port + ITH_UART_MDR_REG, mode, ITH_UART_MDR_MODE_SEL_MASK);
}

void ithUartSetParity(
    ITHUartPort     port,
    ITHUartParity   parity,
    unsigned int    stop,
    unsigned int    len)
{
    uint32_t lcr;
    lcr = ithReadRegA(port + ITH_UART_LCR_REG) & ~ITH_UART_LCR_DLAB;

    // Clear orignal parity setting
    lcr &= 0xC0;

    switch (parity)
    {
    case ITH_UART_ODD:
        lcr |= ITH_UART_LCR_ODD;
        break;

    case ITH_UART_EVEN:
        lcr |= ITH_UART_LCR_EVEN;
        break;

    case ITH_UART_MARK:
        lcr |= ITH_UART_LCR_STICKPARITY | ITH_UART_LCR_ODD;
        break;

    case ITH_UART_SPACE:
        lcr |= ITH_UART_LCR_STICKPARITY | ITH_UART_LCR_EVEN;
        break;

    default:
        break;
    }

    if (stop == 2)
        lcr |= ITH_UART_LCR_STOP;

    lcr |= len - 5;
    ithWriteRegA(port + ITH_UART_LCR_REG, lcr);
}

void ithUartSetBaudRate(ITHUartPort     port,
                        unsigned int    baud)
{
    unsigned int    totalDiv, intDiv, fDiv;
    uint32_t        lcr;
    lcr = ithReadRegA(port + ITH_UART_LCR_REG) & ~ITH_UART_LCR_DLAB;

    // Set DLAB = 1
    ithWriteRegA(port + ITH_UART_LCR_REG, ITH_UART_LCR_DLAB);

    totalDiv    = ithGetBusClock() / baud;
    intDiv      = totalDiv >> 4;
    fDiv        = totalDiv & 0xF;

    // Set baud rate
    ithWriteRegA(   port + ITH_UART_DLM_REG,    (intDiv & 0xF00) >> 8);
    ithWriteRegA(   port + ITH_UART_DLL_REG,    intDiv & 0xFF);

    // Set fraction rate
    ithWriteRegA(   port + ITH_UART_DLH_REG,    fDiv & 0xF);

    ithWriteRegA(   port + ITH_UART_LCR_REG,    lcr);
}

void ithUartReset(
    ITHUartPort     port,
    unsigned int    baud,
    ITHUartParity   parity,
    unsigned int    stop,
    unsigned int    len)
{
    unsigned int    totalDiv, intDiv, fDiv;
    uint32_t        lcr;

    // Power on clock
	ithSetRegBitA(ITH_APB_CLK2_REG, ITH_EN_W6CLK_BIT);

    // Temporarily setting?
#if ((CFG_CHIP_FAMILY == 9070) || (CFG_CHIP_FAMILY == 9910))
    if (port == ITH_UART0)
        ithWriteRegMaskA(ITH_GPIO_BASE + ITH_GPIO_MISC_SET_REG, ITH_GPIO_HOSTSEL_GPIO << ITH_GPIO_HOST_SEL_POS, ITH_GPIO_HOST_SEL_MSK);
#endif

    totalDiv    = ithGetBusClock() / baud;
    intDiv      = totalDiv >> 4;
    fDiv        = totalDiv & 0xF;

    lcr         = ithReadRegA(port + ITH_UART_LCR_REG) & ~ITH_UART_LCR_DLAB;

    // Set DLAB = 1
    ithWriteRegA(   port + ITH_UART_LCR_REG,    ITH_UART_LCR_DLAB);

    // Set baud rate
    ithWriteRegA(   port + ITH_UART_DLM_REG,    (intDiv & 0xFF00) >> 8);
    ithWriteRegA(   port + ITH_UART_DLL_REG,    intDiv & 0xFF);

    // Set fraction rate
    ithWriteRegA(   port + ITH_UART_DLH_REG,    fDiv & 0xF);

    // Clear orignal parity setting
    lcr &= 0xC0;

    switch (parity)
    {
    case ITH_UART_ODD:
        lcr |= ITH_UART_LCR_ODD;
        break;

    case ITH_UART_EVEN:
        lcr |= ITH_UART_LCR_EVEN;
        break;

    case ITH_UART_MARK:
        lcr |= ITH_UART_LCR_STICKPARITY | ITH_UART_LCR_ODD;
        break;

    case ITH_UART_SPACE:
        lcr |= ITH_UART_LCR_STICKPARITY | ITH_UART_LCR_EVEN;
        break;

    default:
        break;
    }

    if (stop == 2)
        lcr |= ITH_UART_LCR_STOP;

    lcr |= len - 5;

    ithWriteRegA(port + ITH_UART_LCR_REG, lcr);

    ithUartFifoCtrlEnable(port, ITH_UART_FIFO_EN);  // enable fifo as default
    ithUartSetTxTriggerLevel(port, ITH_UART_TRGL2); // default to maximum level
    ithUartSetRxTriggerLevel(port, ITH_UART_TRGL0); // default to maximum level
}