/*
 * dmnr_ica.h
 *
 *  Created on: 2019�~5��14��
 *      Author: USER
 */

#ifndef DMNR_ICA_H_
#define DMNR_ICA_H_
#include "rfft_256.h"

typedef struct
{
	Word16 cntr, isEmpty;
	Word16 buffer_ch1[512];
	Word16 buffer_ch2[512];
	Word16 ola_buffer1[256];
	Word16 ola_buffer2[256];
	RingBuffer *ringBuffer1[256];
	RingBuffer *ringBuffer2[256];
#if !defined(_MSC_VER)
	float _Complex DataCh1[256][nIter];
	float _Complex DataCh2[256][nIter];
	float _Complex DW[256][2][2];
	float _Complex UW[256][2][2];
	float _Complex SW[256][2][2];
	float _Complex Rc[256][2][2]; //online_bss
	float _Complex Rv[256][2][2];
	float _Complex Wh[256][2];
	float _Complex Vh[256][2];
	float _Complex rXy[256][2];
	float _Complex rxy[256];
#endif
	float rS[257];
	float rY[257];
	float pX[257];
} bss_state;

void Init_BSS(bss_state *bss_config);
void BSS_Destroy(bss_state *bss_config);
void DMNR_ICA(Word16 *MIC1, Word16 *MIC2, Word16 *Sout1, Word16 *Sout2,
			  bss_state *bss_config);

#endif /* DMNR_ICA_H_ */
