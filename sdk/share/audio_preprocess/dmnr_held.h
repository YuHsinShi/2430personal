/*
 * dmnr_held.h
 *
 *  Created on: 2019¦~1¤ë14¤é
 *      Author: USER
 */

#ifndef DMNR_HELD_H_
#define DMNR_HELD_H_

#if defined(_MSC_VER)
  #define __restrict 
#endif

typedef struct {
	Word64 real;
	Word64 imag;
} Complex64_t;

typedef struct {
	Word16 nk[257];
	Word16 buffer_ch1[512];
	Word16 buffer_ch2[512];
	Word16 buffer_olpa[256];
	Word16 buffer_olpa1[256];
	Word32 reset;
	Word32 Gain[257];
	Word32 preSNR[257];
	Word64 pk1[257];
	Word64 pk2[257];
	Word64 vk1[257];
	Word64 uk1[257];
	Word64 noise[257];
	Word64 phi_z1z1[257];
	Complex64_t phi_z2z1[256];
	Word64 phiEst1[257][16];
	Complex64_t phiEst2[256][16];
#if !defined(_MSC_VER)
	double _Complex h[256];
	double _Complex rCh1[256][2];
	double _Complex rCh2[256][2];
#endif
} dmnr_state;

void dmnr_hand_held(Word16 * __restrict MIC1, Word16 * __restrict MIC2,
		Word16 * __restrict Sout, Word16 * __restrict Sout1,
		dmnr_state * __restrict dmnr_config);

#endif /* DMNR_HELD_H_ */
