#ifndef ITH_CODEC_H
#define ITH_CODEC_H

#include "ith/ith_utility.h"

/** @addtogroup ith ITE Hardware Library
 *  @{
 */
/** @addtogroup ith_codec Codec
 *  @{
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Command for codec on risc.
 */
bool ithCodecCommand(int command, int parameter0, int parameter1, int parameter2);

/**
 * Open engine for codec on risc.
 */
//void ithCodecOpenEngine(void);

/**
 * Read card id from wiegand.
 */
int ithCodecWiegandReadCard(int index, unsigned long long* card_id);

/**
 * Redirect printf message to codec of sw uart.
 */
void ithCodecPrintfWrite(char* string, int length);

void ithCodecCtrlBoardWrite(uint8_t* data, int length);

void ithCodecCtrlBoardRead(uint8_t* data, int length);

void ithCodecHeartBeatRead(uint8_t* data,int length);

#ifdef __cplusplus
}
#endif

#endif // ITH_CODEC_H
/** @} */ // end of ith_codec
/** @} */ // end of ith