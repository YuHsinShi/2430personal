#ifndef ITH_IR_H
#define ITH_IR_H

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
 * IR port definition.
 */
typedef enum
{
    ITH_IR0 = ITH_IR0_BASE,      ///< IR Port #0
    ITH_IR1 = ITH_IR1_BASE,     ///< IR Port #1
    ITH_IR2 = ITH_IR2_BASE,     ///< IR port #2
    ITH_IR3 = ITH_IR3_BASE      ///< IR port #3
} ITHIrPort;

typedef enum
{
    ITH_IR_RX_DEFAULT   = 0,        ///< Receive data without modulation filter
	ITH_SAMPLE_CLK		= 1,        ///< Sample clock function enable
	ITH_IR_RX_MODFILTER = 2,        ///< Receive data with modulation filter
} ITHIrRXMode;

typedef enum
{
    ITH_FIFO_MODE	 = 0,       ///< FIFO default mode
    ITH_INTR_MODE    = 1,       ///< Interrupt mode
    ITH_DMA_MODE     = 2,       ///< DMA mode
} ITHIrMode;

void ithIrRxInit(ITHIrPort port, int pin, unsigned long extClk, int sampleRate, unsigned int precision);
void ithIrTxInit(ITHIrPort port, int pin, unsigned long extClk, int sampleRate, unsigned int precision);

/**
 * Probes IR code.
 *
 * @param port the IR base address
 * @return the code. -1 indicates no key input.
 */
int ithIrProbe(ITHIrPort port);

/**
 * Set IR RX modulation frequency.
 *
 * @param freqDiv the modulation frequency.
 */
static inline void ithIrRxSetModFilter(ITHIrPort port, int minFreqDiv, int maxFreqDiv)
{
	ithWriteRegA(port + ITH_IR_RX_MOD_FILTER_REG, (minFreqDiv & 0x7FFF) << 16 | (maxFreqDiv & 0x7FFF));
}

/**
 * Set IR RX Receiver Mode
 *
 * @param mode the receiver mode
 */
static inline void ithIrRxMode(ITHIrPort port, ITHIrRXMode mode) // equal to ithIrRxSetCaptureMode need to be modified
{
	ithWriteRegMaskA(port + ITH_IR_RX_CTRL_REG, mode << ITH_IR_RX_CAPMODE_BIT, ITH_IR_RX_CAPMODE_MASK);
}

/**
 * Initializes Clock Sample Function
 *
 * @param port Ir port 0~3
 * @param pin the GPIO pin
 * @param extClk external clock frequency
 * @param sample rate factor
 */
void ithClkSampleInit(ITHIrPort port, int pin, unsigned long extClk, int sampleRate, unsigned int precision);

/**
 * Get the slowest Freq
 */
static inline uint32_t ithGetFreqSlow(ITHIrPort port)
{
    return (ithReadRegA(port + ITH_IR_FS1_REG) & 0x7FFF0000) >> 16;
}

/**
 * Get the fastest Freq
 */
static inline uint32_t ithGetFreqFast(ITHIrPort port)
{
    return (ithReadRegA(port + ITH_IR_FS1_REG) & 0x00007FFF);
}

/**
 * Get the newest Freq
 */
static inline uint32_t ithGetFreqNew(ITHIrPort port)
{
    return (ithReadRegA(port + ITH_IR_FS2_REG) & 0x7FFF0000) >> 16;
}

/**
 * Get the average Freq
 */
static inline uint32_t ithGetFreqAvg(ITHIrPort port)
{
    return (ithReadRegA(port + ITH_IR_FS2_REG) & 0x00007FFF);
}

/**
 * Get the fastest Freq's high-level duty cycle
 */
static inline uint32_t ithGetHighDCFast(ITHIrPort port)
{
    return (ithReadRegA(port + ITH_IR_FDC1_REG) & 0x7FFF0000) >> 16;
}

/**
 * Get the fastest Freq's low-level duty cycle
 */
static inline uint32_t ithGetLowDCFast(ITHIrPort port)
{
    return (ithReadRegA(port + ITH_IR_FDC1_REG) & 0x00007FFF);
}

/**
 * Get the slowest Freq's high-level duty cycle
 */
static inline uint32_t ithGetHighDCSlow(ITHIrPort port)
{
    return (ithReadRegA(port + ITH_IR_FDC2_REG) & 0x7FFF0000) >> 16;
}

/**
 * Get the slowest Freq's low-level duty cycle
 */
static inline uint32_t ithGetLowDCSlow(ITHIrPort port)
{
    return (ithReadRegA(port + ITH_IR_FDC2_REG) & 0x00007FFF);
}

/**
 * Get the average Freq's high-level duty cycle
 */
static inline uint32_t ithGetHighDCAvg(ITHIrPort port)
{
    return (ithReadRegA(port + ITH_IR_FDC3_REG) & 0x7FFF0000) >> 16;
}

/**
 * Get the average Freq's low-level duty cycle
 */
static inline uint32_t ithGetLowDCAvg(ITHIrPort port)
{
    return (ithReadRegA(port + ITH_IR_FDC3_REG) & 0x00007FFF);
}

/**
 * Get the newest Freq's high-level duty cycle
 */
static inline uint32_t ithGetHighDCNew(ITHIrPort port)
{
    return (ithReadRegA(port + ITH_IR_FDC4_REG) & 0x7FFF0000) >> 16;
}

/**
 * Get the newest Freq's low-level duty cycle
 */
static inline uint32_t ithGetLowDCNew(ITHIrPort port)
{
    return (ithReadRegA(port + ITH_IR_FDC4_REG) & 0x00007FFF);
}

#include "ith/ith_ir_generic.h"

#ifdef __cplusplus
}
#endif

#endif // ITH_IR_H
/** @} */ // end of ith_ir
/** @} */ // end of ith