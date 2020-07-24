#ifndef ITH_IR_GENERIC_H
#define ITH_IR_GENERIC_H

#ifndef ITH_IR_H
    #error "Never include this file directly. Use ith/ith_ir.h instead."
#endif

/** @addtogroup ith ITE Hardware Library
 *  @{
 */
/** @addtogroup ith_ir IR
 *  @{
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * IR control definition.
 */
typedef enum
{
    ITH_IR_EN        = 0,    ///< Capture Enable
    ITH_IR_DEBOUNCE  = 1,    ///< Enable De-bounce circuit
    ITH_IR_INT       = 2,    ///< Enable Interrupt
    ITH_IR_PIO       = 3,    ///< PIO Mode
    ITH_IR_SIGINVESE = 8,    ///< Inverse the signal
    ITH_IR_TMRST     = 10,   ///< Reset the timer after sampled event
    ITH_IR_WARP      = 11    ///< Do not stop capture even time out interrupt occurs
} ITHIrCtrl;

/**
 * IR TX control definition.
 */
typedef enum
{
    ITH_IR_TX_EN        = 0,    ///< IR sender Enable
    ITH_IR_TX_LOOPBACK  = 1,    ///< Loopback  to the Receiver
    ITH_IR_TX_INT       = 2,    ///< Enable Interrupt
    ITH_IR_TX_PIO       = 3,    ///< PIO Mode
    ITH_IR_TX_SIGINVESE = 8,    ///< Inverse the signal
} ITHIrTxCtrl;

typedef enum
{
    ITH_IR_TRIGGER_LEVEL_1  = 0,      ///< 1
    ITH_IR_TRIGGER_LEVEL_32 = 1,      ///< (CFG_DEPTH/8)*4
    ITH_IR_TRIGGER_LEVEL_48 = 2,      ///< (CFG_DEPTH/8)*6
    ITH_IR_TRIGGER_LEVEL_64 = 3       ///< (CFG_DEPTH/8)*8
} ITHIrSenderFifoIntrTriggerLevel;

typedef enum
{
    ITH_IR_RISING  = 0,      ///< Rising Edge
    ITH_IR_FALLING = 1,      ///< Falling Edge
    ITH_IR_BOTH    = 2       ///< Both Edge
} ITHIrCaptureMode;

typedef enum
{
    ITH_IR_DATA    = 8,      ///< Data trigger level available interrupt
    ITH_IR_FULL    = 9,      ///< FIFO Full Interrupt
    ITH_IR_EMPTY   = 10,     ///< FIFO Empty Interrupt
    ITH_IR_OE      = 11,     ///< Overrun Interrupt
    ITH_IR_TIMEOUT = 12      ///< Timeout interrupt
} ITHIrIntrCtrl;

/**
 * Sets IR RX GPIO pin.
 *
 * @param pin the IR RX GPIO pin.
 */
static inline void ithIrRxSetGpio(ITHIrPort port, unsigned int pin)
{
    ithWriteRegA(port + ITH_IR_GPIO_SEL_REG, pin | (1 << 7));
}

/**
 * Enables specified IR control.
 *
 * @param ctrl the control to enable.
 */
static inline void ithIrRxCtrlEnable(ITHIrPort port, ITHIrCtrl ctrl)
{
    ithSetRegBitA(port + ITH_IR_RX_CTRL_REG, ctrl);
}

/**
 * Disables specified IR control.
 *
 * @param ctrl the control to disable.
 */
static inline void ithIrRxCtrlDisable(ITHIrPort port, ITHIrCtrl ctrl)
{
    ithClearRegBitA(port + ITH_IR_RX_CTRL_REG, ctrl);
}

/**
 * Sets IR capture mode.
 *
 * @param mode the capture mode to set.
 */
static inline void ithIrRxSetCaptureEdge(ITHIrPort port, ITHIrCaptureMode mode)
{
    ithWriteRegMaskA(port + ITH_IR_RX_CTRL_REG, mode << ITH_IR_RX_CAPEDGE_BIT, ITH_IR_RX_CAPEDGE_MASK);
}

/**
 * Enables specified IR interrupt control.
 *
 * @param ctrl the control to enable.
 */
static inline void ithIrRxIntrCtrlEnable(ITHIrPort port, ITHIrCtrl ctrl)
{
    ithSetRegBitA(port + ITH_IR_RX_STATUS_REG, ctrl);
}

/**
 * Disables specified IR interrupt control.
 *
 * @param ctrl the control to disable.
 */
static inline void ithIrRxIntrCtrlDisable(ITHIrPort port, ITHIrCtrl ctrl)
{
    ithClearRegBitA(port + ITH_IR_RX_STATUS_REG, ctrl);
}

/**
 * Clears the receiver FIFO and reset control logic.
 */
static inline void ithIrRxClear(ITHIrPort port)
{
    ithSetRegBitA(port + ITH_IR_RX_CTRL_REG, ITH_IR_CLEAR_BIT);
}

/**
 * Transmit IR code.
 *
 * int code: the IR code
 */
static inline void ithIrTxTransmit(ITHIrPort port, int code)
{
    ithWriteRegA(port + ITH_IR_TX_DATA_REG, code);
}

/**
 * Get IR TX Fifo available length
 *
 * @param port the IR base address
 */
static inline uint32_t ithIrTxFifoLen(ITHIrPort port)
{
    return (ithReadRegA(port + ITH_IR_TX_CTRL_REG) >> 16);
}

/**
 * Get IR TX Fifo available length
 *
 * @param port the IR base address
 */
static inline uint32_t ithIrTxFifoEmpty(ITHIrPort port)
{
    return (ithReadRegA(port + ITH_IR_TX_STATUS_REG) & 0x4) >> 2;
}

/**
 * Set IR TX modulation frequency.
 *
 * @param freqDiv the modulation frequency.
 */
static inline void ithIrTxSetModFreq(ITHIrPort port, int freqDiv)
{
    ithWriteRegA(port + ITH_IR_TX_MOD_REG, 0x8000 | freqDiv);
}

/**
 * Sets IR capture mode.
 *
 * @param mode the capture mode to set.
 */
static inline void ithIrTxSetCaptureMode(ITHIrPort port, ITHIrCaptureMode mode)
{
    ithWriteRegMaskA(port + ITH_IR_RX_CTRL_REG, mode << ITH_IR_RX_CAPEDGE_BIT, ITH_IR_RX_CAPEDGE_MASK);
}

/**
 * Enables specified IR interrupt control.
 *
 * @param ctrl the control to enable.
 */
static inline void ithIrTxIntrCtrlEnable(ITHIrPort port, ITHIrCtrl ctrl)
{
    ithSetRegBitA(port + ITH_IR_TX_STATUS_REG, ctrl);
}

/**
 * Enables specified IR control.
 *
 * @param ctrl the control to enable.
 */
static inline void ithIrTxCtrlEnable(ITHIrPort port, ITHIrCtrl ctrl)
{
    ithSetRegBitA(port + ITH_IR_TX_CTRL_REG, ctrl);
}

/**
 * Disables specified IR control.
 *
 * @param ctrl the control to disable.
 */
static inline void ithIrTxCtrlDisable(ITHIrPort port, ITHIrCtrl ctrl)
{
    ithClearRegBitA(port + ITH_IR_TX_CTRL_REG, ctrl);
}

/**
 * Disables specified IR interrupt control.
 *
 * @param ctrl the control to disable.
 */
static inline void ithIrTxIntrCtrlDisable(ITHIrPort port, ITHIrCtrl ctrl)
{
    ithClearRegBitA(port + ITH_IR_TX_STATUS_REG, ctrl);
}

/**
 * Clears the receiver FIFO and reset control logic.
 */
static inline void ithIrTxClear(ITHIrPort port)
{
    ithSetRegBitA(port + ITH_IR_TX_CTRL_REG, ITH_IR_CLEAR_BIT);
}

#ifdef __cplusplus
}
#endif

#endif // ITH_IR_GENERIC_H
/** @} */ // end of ith_ir
/** @} */ // end of ith