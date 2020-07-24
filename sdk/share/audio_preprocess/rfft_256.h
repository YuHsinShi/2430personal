/*
 * rfft_256.h
 *
 *  Created on: 2015�~7��20��
 *      Author: ych
 */
#pragma once
#ifndef RFFT_256_H_
#define RFFT_256_H_

#include "type_def.h"
#if !defined(_MSC_VER)
#include <float.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include "binary_libraryC.h"
#endif
#include "webrtc_lib/include/ring_buffer.h"

#define NFFT (256)
#define U_Size (8)
#define V_Size (15)
#define PartLen ((NFFT) >> 1)
#define PartLen1 ((PartLen) + 1)
#define Ra (64)
#define Rs (62)
#define nIter (48)
#define Fs (8000)

typedef enum
{
	kMaxFFTOrder = 10
} FFT_ORDER;

extern Word32 CB_FREQ_INDICES_256[];
extern Word32 CB_OFST[];
extern const Word32 sprd_table[][18];
extern Word32 T_ABS[];

typedef enum
{
	Wiener_Mode = 0,
	Auditory_Mask_Mode,
	Wind_Mode
} MONO_NR_MODE;

typedef struct
{
	Word16 buffer[NFFT - Ra]; // prev
	Word16 sc_buffer[NFFT];	  // single channel
	Word16 mOnSet[PartLen1];
	Word16 mEdge[PartLen1];
	Word16 mInterFere[PartLen1];

	Word32 nrStartup;
	Word32 cntr;
	Word32 mode;
	Word32 beta;
	Word32 gamma_prev[PartLen1];
	Word32 xi[PartLen1];
	Word32 Gh1[PartLen1];
	Word32 bufferGamma[3][PartLen1];

	Word64 S[PartLen1];
	Word64 S_tilt[PartLen1];
	Word64 Smin[PartLen1];
	Word64 Smin_sw[PartLen1];
	Word64 Smin_Lists[PartLen1][U_Size];
	Word64 Smin_tilt[PartLen1];
	Word64 Smin_sw_tilt[PartLen1];
	Word64 Smin_tilt_Lists[PartLen1][U_Size];
	Word64 lambda[PartLen1];
	Word64 lambda_prev[PartLen1];
	Word64 Sii[PartLen1];

} nr_state;

#if !defined(_MSC_VER)
typedef struct
{
	Word16 mic1Buf[1024];
	Word16 mic2Buf[1024];
	Word16 buffer_Olpa[768];
	Word16 buffer_Olpa1[768];
	Word16 pca_flag[513];
	Word16 Gain[513];
	Word16 Gain1[513];
	Word16 cntr;
	Word16 Iters;

	RingBuffer *ringBuffer1[512];
	RingBuffer *ringBuffer2[512];

	float _Complex DataCh1[512][nIter];
	float _Complex DataCh2[512][nIter];
	float _Complex IcaCh1[512][nIter]; //block(m-2)
	float _Complex IcaCh2[512][nIter];
	float _Complex PcaCh1[512][nIter]; //whitening data
	float _Complex PcaCh2[512][nIter];
	float _Complex WCh1[512][2]; //ICA-DeMix
	float _Complex WCh2[512][2];
	float _Complex PCh1[512][2]; //PCA-White
	float _Complex PCh2[512][2];
	float _Complex PWC1[512][2]; //Fast_ICA,z*z.'/L
	float _Complex PWC2[512][2];
	float _Complex DCh1[512][2]; //ICA,unmixing
	float _Complex DCh2[512][2];
	float _Complex uCh1[512][2]; //project
	float _Complex uCh2[512][2];
	float _Complex aCh1[512][2]; //DS
	float _Complex aCh2[512][2];

	Complex32_t ICAD1[512][nIter];
	Complex32_t ICAD2[512][nIter];
	Complex32_t ICAD3[512][nIter];
	Complex32_t ICAD4[512][nIter];

	float eivalues[513][2];
	Word32 doas[513][2];
	Word64 noise[513];
	Word64 noise1[513];
	Word32 preSNR[513];
	Word32 preSNR1[513];
	sig_atomic_t isEmpty;

	pthread_mutex_t state_mutex;
	pthread_mutex_t filter_mutex;
	pthread_mutex_t queue_mutex;
} ica_state;

typedef struct
{
	ica_state *ica_config;
	pthread_mutex_t *mx;
} ARG;

#endif

typedef struct
{
	Word16 MonoBuffer[NFFT];
	Word16 SynsBuffer[NFFT - Rs];
	Word32 ResetPhase;
	Word32 Cntr;

	Word32 alf[PartLen1];
	Word32 arf[PartLen1];

	Word32 phi[PartLen1];
	Word16 plf[PartLen1];
	Word16 prf[PartLen1];

	RingBuffer *outFrameBuf;
	RingBuffer *farFrameBuf;
} sound_stretch;

extern Word16 sqrt_hann_win[256];
extern nr_state anr_config[];
extern sound_stretch sndstrh_config[];
#if !defined(_MSC_VER)
extern ica_state ica_config[];
#endif

Word32 Complex_FFT(ComplexInt16 *frfi, Word32 stages);
Word32 Real_FFT(ComplexInt16 *RealDataIn, Word32 Order);
void HS_IFFT(ComplexInt16 *complex_data_in, Word16 *real_data_out, Word32 Order);
void NR_Create(nr_state *st, Word32 mode);
void NoiseSuppression(nr_state *Inst, ComplexInt16 *Sin, Word32 blk_exponent);
void MaskThresholds(Word64 *, Word64 *const, Word32, Word32 *);
void Overlap_Add(Word16 *Sin, Word16 *Sout, nr_state *NRI);
void TimeStretch(Word16 *Sin, Word16 *Out, sound_stretch *src);
void TimeStretch_Create(sound_stretch *src);
void TimeStretch_Init(sound_stretch *src);
void TimeStretch_Destroy(sound_stretch *src);
Word32 atan2Cordic(Word16 y, Word16 x);
#if !defined(_MSC_VER)
void *pca_ica_thread_func(void *arg);
void ICA_SeparateInit(ica_state *ica_config);
void ICA_SeparateDestroy(ica_state *ica_config);
void ICA_Separation(Word16 *MIC1, Word16 *MIC2, Word16 *Sout, Word16 *Sout1,
					ica_state *ica_config);
#endif

#endif /* RFFT_256_H_ */
