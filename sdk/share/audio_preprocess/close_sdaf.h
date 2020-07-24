/*
 * close_sdaf.h
 *
 *  Created on: 2017¦~8¤ë25¤é
 *      Author: ite01527
 */

#ifndef CLOSE_SDAF_H_
#define CLOSE_SDAF_H_
#include "type_def.h"
#include "basic_op.h"
#include <pthread.h>

typedef struct {
	Word16 W[256];
	Word16 bufferTime[256];
	Word16 buffern[512];
	Word16 bufferf[512];
	Complex16_t phi[33][16];
	Complex16_t wght[33][32];
	Complex32_t FiltBuffer[33][16];
	Word16 bufferSB[64][32];
	Word16 qDomain[32];
	Word16 Gain[33];
	Word16 GainLinear[33];
	Word16 Cntr;
	Word16 flag;
	Word32 ca[33]; //attack counter
	Word32 cr[33]; //release counter
	Word32 preSNR[33];
	Word32 gs[33];
	Word64 Sf[33];
	Word64 Snn[33];
	pthread_mutex_t mutex;
} sdaf_config;

void WightTransform(sdaf_config *s);
void CloseLoop_SDAFInit(sdaf_config *s);
void CloseLoop_SDAF(Word16 * __restrict nearEnds, Word16 * __restrict farEnds,
		Word16 *output, sdaf_config * __restrict s);
extern sdaf_config sdaf_state;

#endif /* CLOSE_SDAF_H_ */
