#ifndef ITH_UART_H
#define ITH_UART_H

/** @addtogroup ith ITE Hardware Library
 *  @{
 */
/** @addtogroup ith_uart UART
 *  @{
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * UART port definition.
 */
typedef enum
{
    ITH_UART0 = ITH_UART0_BASE,     ///< UART port #0
    ITH_UART1 = ITH_UART1_BASE,     ///< UART port #1
    ITH_UART2 = ITH_UART2_BASE,     ///< UART port #2
    ITH_UART3 = ITH_UART3_BASE,     ///< UART port #3
    ITH_UART4 = ITH_UART4_BASE,     ///< UART port #4
    ITH_UART5 = ITH_UART5_BASE,     ///< UART port #5
} ITHUartPort;

/**
 * UART mode definition
 */
typedef enum
{
    ITH_UART_DEFAULT = 0x0,      ///< UART TX/RX mode
    ITH_UART_SIR     = 0x1,      ///< SIR mode
    ITH_UART_FIR     = 0x2,      ///< FIR mode
    ITH_UART_TX      = 0x3       ///< UART TX only mode
} ITHUartMode;

/**
 * UART parity definition
 */
typedef enum
{
    ITH_UART_NONE  = 0,    ///< None
    ITH_UART_ODD   = 1,    ///< Odd
    ITH_UART_EVEN  = 2,    ///< Even
    ITH_UART_MARK  = 3,    ///< Mark
    ITH_UART_SPACE = 4     ///< Space
} ITHUartParity;

/**
 * UART FIFO depth definition
 */
typedef enum
{
    ITH_UART_FIFO16  = 0x1,     ///< TX/RX FIFOs are 16-byte deep
    ITH_UART_FIFO32  = 0x2,     ///< TX/RX FIFOs are 32-byte deep
    ITH_UART_FIFO64  = 0x4,     ///< TX/RX FIFOs are 64-byte deep
    ITH_UART_FIFO128 = 0x8      ///< TX/RX FIFOs are 128-byte deep
} ITHUartFifoDepth;

/**
 * UART FIFO control definition
 */
typedef enum
{
    ITH_UART_FIFO_EN  = 0,          ///< Set this bit to logic 1 enables both transmit and receive FIFOs (And Status FIFO)
    ITH_UART_RX_RESET = 1,          ///< Set this bit to clear all bytes in RX FIFO
    ITH_UART_TX_RESET = 2,          ///< Set this bit to clear all bytes in TX FIFO
    ITH_UART_DMA      = 3           ///< This bit selects the UART DMA mode
} ITHUartFifoCtrl;

/**
 * UART trigger level of the TX/RX FIFO interrupt
 */
typedef enum
{
    ITH_UART_TRGL0 = 0x0,   ///< FIFO Trigger Level0
    ITH_UART_TRGL1 = 0x1,   ///< FIFO Trigger Level1
    ITH_UART_TRGL2 = 0x2,   ///< FIFO Trigger Level2
    ITH_UART_TRGL3 = 0x3    ///< FIFO Trigger Level3
} ITHUartTriggerLevel;

/**
 * UART interrupt definition
 */
typedef enum
{
    ITH_UART_RX_READY     = 0,     ///< Receiver Data Available
    ITH_UART_TX_READY     = 1,     ///< THR Empty
    ITH_UART_RECV_STATUS  = 2,     ///< Receiver Line Status
    ITH_UART_MODEM_STATUS = 3      ///< MODEM Status
} ITHUartIntr;

/**
 * UART config info structure
 */
typedef struct
{
    unsigned int  TxGpio;
    unsigned int  RxGpio;
    unsigned int  BaudRate;
    ITHUartParity Parity;
} ITHUartConfig;

/**
 * Sets UART mode.
 *
 * @param port The UART port
 * @param mode The UART mode
 * @param txPin The UART TX GPIO pin. for UART1 only.
 * @param rxPin The UART RX GPIO pin.
 */
void ithUartSetMode(
    ITHUartPort  port,
    ITHUartMode  mode,
    unsigned int txPin, // UART1 only
    unsigned int rxPin);

/**
 * Only sets UART parity.
 *
 * @param port The UART port.
 * @param parity The UART parity.
 * @param stop The UART stop bit.
 * @param len The UART word length.
 */
void ithUartSetParity(
    ITHUartPort   port,
    ITHUartParity parity,
    unsigned int  stop,
    unsigned int  len);

/**
 * Only sets UART baudrate.
 *
 * @param port The UART port.
 * @param baud The UART baudrate.
 */
void ithUartSetBaudRate(
    ITHUartPort  port,
    unsigned int baud);

/**
 * Resets UART.
 *
 * @param port The UART port.
 * @param baud The UART baudrate.
 * @param parity The UART parity.
 * @param stop The UART stop bit.
 * @param len The UART word length.
 */
void ithUartReset(
    ITHUartPort   port,
    unsigned int  baud,
    ITHUartParity parity,
    unsigned int  stop,
    unsigned int  len);

/**
 * Enables specified FIFO controls.
 *
 * @param port The UART port.
 * @param ctrl the controls to enable.
 */
void ithUartFifoCtrlEnable(ITHUartPort port, ITHUartFifoCtrl ctrl);

/**
 * Disables specified FIFO controls.
 *
 * @param port The UART port.
 * @param ctrl the controls to disable.
 */
void ithUartFifoCtrlDisable(ITHUartPort port, ITHUartFifoCtrl ctrl);

/**
 * Sets UART TX interrupt trigger level.
 *
 * @param port The UART port
 * @param level The UART TX trigger level
 */
void ithUartSetTxTriggerLevel(ITHUartPort port, ITHUartTriggerLevel level);

/**
 * Sets UART RX interrupt trigger level.
 *
 * @param port The UART port
 * @param level The UART RX trigger level
 */
void ithUartSetRxTriggerLevel(ITHUartPort port, ITHUartTriggerLevel level);

/**
 * Whether RX is ready.
 *
 * @return the result of RX is ready or not
 */
static inline ITHUartFifoDepth ithUartGetFifoDepth(ITHUartPort port)
{
    return (ITHUartFifoDepth)((ithReadRegA(port + ITH_UART_FEATURE_REG) & ITH_UART_FIFO_DEPTH_MASK) >> ITH_UART_FIFO_DEPTH_BIT);
}

/**
 * Whether RX is ready.
 *
 * @return the result of RX is ready or not
 */
static inline bool ithUartIsRxReady(ITHUartPort port)
{
    return (ithReadRegA(port + ITH_UART_LSR_REG) & ITH_UART_LSR_DR) == ITH_UART_LSR_DR;
}

/**
 * Gets a character from UART RX.
 *
 * @param port The UART port.
 */
static inline char ithUartGetChar(ITHUartPort port)
{
    return (char)ithReadRegA(port + ITH_UART_RBR_REG);
}

/**
 * Whether TX FIFO is empty.
 *
 * @return the result of TX FIFO is empty or not
 */
static inline bool ithUartIsTxEmpty(ITHUartPort port)
{
    return (ithReadRegA(port + ITH_UART_LSR_REG) & ITH_UART_LSR_THRE) == ITH_UART_LSR_THRE;
}

static inline bool ithUartIsTxWIthFifoEmpty(ITHUartPort port)
{
    return (ithReadRegA(port + ITH_UART_LSR_REG) & ITH_UART_LSR_TRANS_EMPTY) == ITH_UART_LSR_TRANS_EMPTY;
}

/**
 * Whether TX FIFO is full.
 *
 * @return the result of TX FIFO is full or not
 */
static inline bool ithUartIsTxFull(ITHUartPort port)
{
    return (ithReadRegA(port + ITH_UART_IIR_REG) & ITH_UART_IIR_TXFIFOFULL) == ITH_UART_IIR_TXFIFOFULL;
}

/**
 * Puts character to UART TX.
 *
 * @param port The UART port.
 * @param c The character.
 */
static inline void ithUartPutChar(ITHUartPort port, char c)
{
    ithWriteRegA(port + ITH_UART_THR_REG, c);
}

/**
 * Enables UART interrupt.
 *
 * @param port The UART port.
 * @param intr The UART interrupt.
 */
static inline void ithUartEnableIntr(ITHUartPort port, ITHUartIntr intr)
{
    ithSetRegBitA(port + ITH_UART_IER_REG, intr);
}

/**
 * Disables UART interrupt.
 *
 * @param port The UART port.
 * @param intr The UART interrupt.
 */
static inline void ithUartDisableIntr(ITHUartPort port, ITHUartIntr intr)
{
    ithClearRegBitA(port + ITH_UART_IER_REG, intr);
}

/**
 * Clears UART interrupt.
 *
 * @param port The UART port.
 */
static inline uint32_t ithUartClearIntr(ITHUartPort port)
{
    return ithReadRegA(port + ITH_UART_IIR_REG);
}

/**
 * Enables UART DMA MODE2.
 *
 * @param port The UART port.
 */
static inline void ithUartEnableDmaMode2(ITHUartPort port)
{
    ithWriteRegMaskA(port + ITH_UART_MCR_REG, ITH_UART_MCR_DMAMODE2, ITH_UART_MCR_DMAMODE2);
}

/**
 * Disables UART DMA MODE2.
 *
 * @param port The UART port.
 */
static inline void ithUartDisableDmaMode2(ITHUartPort port)
{
    ithWriteRegMaskA(port + ITH_UART_MCR_REG, 0, ITH_UART_MCR_DMAMODE2);
}

/**
 * Set UART BREAK.
 *
 * @param port The UART port.
 */
static inline void ithUartSetBreak(ITHUartPort port)
{
    ithWriteRegMaskA(port + ITH_UART_LCR_REG, ITH_UART_LCR_SETBREAK, ITH_UART_LCR_SETBREAK);
}

/**
 * Clear UART BREAK.
 *
 * @param port The UART port.
 */
static inline void ithUartClearBreak(ITHUartPort port)
{
    ithWriteRegMaskA(port + ITH_UART_LCR_REG, 0, ITH_UART_LCR_SETBREAK);
}

/**
 * RESET UART TX FIFO
 *
 * @param port The UART port.
 */
static inline void ithUartResetTxFIFO(ITHUartPort port)
{
    ithSetRegBitA(port + ITH_UART_FCR_REG, ITH_UART_TX_RESET);
}

/**
 * RESET UART RX FIFO
 *
 * @param port The UART port.
 */
static inline void ithUartResetRxFIFO(ITHUartPort port)
{
    ithSetRegBitA(port + ITH_UART_FCR_REG, ITH_UART_RX_RESET);
}

/**
 * Marco whether TX FIFO is full.
 *
 * @param port The UART port.
 */
#define ITH_UART_IS_TX_FULL(port) \
    (ITH_READ_REG_A(port + ITH_UART_IIR_REG) & ITH_UART_IIR_TXFIFOFULL) == ITH_UART_IIR_TXFIFOFULL

/**
 * Marco whether RX is ready.
 *
 * @param port The UART port.
 */
#define ITH_UART_IS_RX_READY(port) \
    (ITH_READ_REG_A(port + ITH_UART_LSR_REG) & ITH_UART_LSR_DR) == ITH_UART_LSR_DR

#ifdef __cplusplus
}
#endif

#endif // ITH_UART_H
/** @} */ // end of ith_uart
/** @} */ // end of ith