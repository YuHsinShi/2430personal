/*
 * pvsola.h
 *
 *  Created on: 2020�~4��8��
 *      Author: USER
 */

#ifndef PVSOLA_H_
#define PVSOLA_H_

#include "type_def.h"
#include "webrtc_lib/include/ring_buffer.h"
#include "rfft_256.h"

typedef struct
{
	Word32 phi[PartLen1];
	RingBuffer *outputs;
	RingBuffer *inputs;
	Word32 acc;
	Word32 vad_cntr;
	Word32 status;
	Word32 R_ana;
	Word32 counter;
	Word16 Prf[PartLen1];
	Word16 Reset;
} pvsola_state;

#endif /* PVSOLA_H_ */
