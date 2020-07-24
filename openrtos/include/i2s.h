/*
 * Copyright (c) 2006 ITE Technology Corp. All Rights Reserved.
 */
/* @file
 * Include file for I2S Interface
 *
 * @author Kuoping Hsu
 * @date 2006.07.26.
 * @version 1.0
 *
 */

#ifndef __I2S_H__
#define __I2S_H__

#include "mmio.h"

//#define I2S_TEST_MODE

/* Constan Declaration for I2S Interface */
#define MMIO_I2S_BASE               0xD0100000
#define MMIO_I2S_OUT_BUF_WRPTR      (MMIO_I2S_BASE + 0xC0)
#define MMIO_I2S_OUT_BUF_RRPTR      (MMIO_I2S_BASE + 0xC4)

#define MMIO_I2S_IN_BUF_RDPTR       (MMIO_I2S_BASE + 0x50)
#define MMIO_I2S_IN_BUF_WRPTR       (MMIO_I2S_BASE + 0x54)

#define MMIO_I2S_OUT_BUF_LEN        (MMIO_I2S_BASE + 0x90)

#if !defined(WIN32) && !defined(__CYGWIN__)
/* For PCM Mixer Input */
    #define SetMixRdPtr(i)        MMIO_Write(DrvMixerPCM_RdPtr, (short int)i)
    #define GetMixWrPtr()         MMIO_Read(DrvMixerPCM_WrPtr)
    #define GetMixRdPtr()         MMIO_Read(DrvMixerPCM_RdPtr)

/* For I2S ADC Input */
    #ifdef DUMP_PCM_DATA
        #define SetInRdPtr(i)     MMIO_Write(DrvEncodePCM_RdPtr, (short int)i)
        #define SetInWrPtr(i)     MMIO_Write(DrvEncodePCM_WrPtr, (short int)i)
        #define GetInWrPtr()      MMIO_Read(DrvEncodePCM_WrPtr)
    #else
        #define SetInRdPtr(i)     MMIO_Write(MMIO_I2S_IN_BUF_RDPTR, (short int)i)
        #define SetInWrPtr(i)     MMIO_Write(MMIO_I2S_IN_BUF_WRPTR, (short int)i)
        #define GetInWrPtr()      MMIO_Read(MMIO_I2S_IN_BUF_WRPTR)
    #endif

/* For I2S DAC Input */
    #if defined(DUMP_PCM_DATA)
        #define SetOutWrPtr(i)    MMIO_Write(DrvDecodePCM_WrPtr, (short int)i)
        #define SetOutRdPtr(i)    MMIO_Write(DrvDecodePCM_RdPtr, (short int)i)
        #define GetOutRdPtr()     MMIO_Read(DrvDecodePCM_RdPtr)
        #define GetOutWrPtr()     MMIO_Read(DrvDecodePCM_WrPtr)
    #else
        #define SetOutWrPtr(i)    CODEC_I2S_SET_OUTWR_PTR(i);
        #define SetOutRdPtr(i)
        #if defined(I2S_TEST_MODE) || defined(ENABLE_PERFORMANCE_MEASURE)
            #define GetOutRdPtr() GetOutWrPtr()
        #else
            #define GetOutRdPtr() CODEC_I2S_GET_OUTRD_PTR()
        #endif
        #define GetOutWrPtr()     CODEC_I2S_GET_OUTRD_PTR()
    #endif // defined(DUMP_PCM_DATA)
#endif     // !defined(WIN32) && !defined(__CYGWIN__)

static void CODEC_I2S_SET_OUTWR_PTR(unsigned int data);
static unsigned int CODEC_I2S_GET_OUTRD_PTR(void);

static void CODEC_I2S_SET_OUTWR_PTR(unsigned int data)
{
    MMIO_Write(MMIO_I2S_OUT_BUF_WRPTR, data);
}

static unsigned int CODEC_I2S_GET_OUTRD_PTR(void)
{
    return MMIO_Read(MMIO_I2S_OUT_BUF_RRPTR);
}

static unsigned int CODEC_I2S_GET_OUT_BUF_LEN(void){
    return MMIO_Read(MMIO_I2S_OUT_BUF_LEN)+1;
}

static unsigned int CODEC_I2S_GET_OUT_FREE_LEN(void)
{
    unsigned int buf_len=CODEC_I2S_GET_OUT_BUF_LEN();
    unsigned int RP=MMIO_Read(MMIO_I2S_OUT_BUF_RRPTR);
    unsigned int WP=MMIO_Read(MMIO_I2S_OUT_BUF_WRPTR);
    return (WP >= RP) ? (buf_len - WP + RP) : (RP - WP);
}

/*void pauseDAC(int flag);
   void deactiveDAC(void);
   void pauseADC(int flag);
   void deactiveADC(void);
   void initDAC(unsigned char *bufptr, int nChannel, int sample_rate, int buflen, int isBigEndian);
   void initADC(unsigned char *bufptr, int nChannel, int sample_rate, int buflen, int isBigEndian);
   void initCODEC(unsigned char *enc_buf, unsigned char *dec_buf, int nChannel, int sample_rate,
               int enc_buflen, int dec_buflen, int isBigEndian);
   void initCODEC2(unsigned char *enc_buf, unsigned char *dec_buf,
                int nDecChannel, int nEncChannel, int sample_rate,
               int enc_buflen, int dec_buflen, int isBigEndian);
   void enableFadeInOut();
   void disableFadeInOut();*/
#endif /* __I2S_H__ */