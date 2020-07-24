/* ------------------------------------------------------------------
 * Copyright (C) 2011 Martin Storsjo
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *	  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied.
 * See the License for the specific language governing permissions
 * and limitations under the License.
 * -------------------------------------------------------------------
 */

#include <stdio.h>
//#include <stdint.h>
//#include <unistd.h>
#include <stdlib.h>
#include "voAAC.h"
#include "cmnMemory.h"
#include "wavreader.h"
#include "config.h"

#define BIT_RATE 192000

#if defined(ENABLE_CODECS_PLUGIN)
# include "mmio.h"
# include "plugin.h"
#endif

#ifdef AAC_ENCODE_PERFORMANCE_TEST_BY_TICK
#include "ticktimer.h"
#endif
#include "resample.h"
#if 0 //def AAC_ENABLE_INTERNAL_SD
uint8_t* inputBuf;
#else
unsigned char inputBuf[MAX_SUPPORT_CHANNELS*2*1024];
#endif
short convertBuf[MAX_SUPPORT_CHANNELS*1024];
unsigned char outbuf[20480];
int gnFrames;

typedef struct AACAudioContext_TAG {
    int nb_channels;
    int bitrate; /* bit rate */
    int sample_rate;
    int nOutSampleRate;
} AACAudioContext;

AACAudioContext gAACEncoder;
static AVResampleContext avResampleContext;

#if defined(WIN32)
#include "win32.h"
#endif
#ifdef AAC_ENCODE_PERFORMANCE_TEST_BY_TICK
static  int  gnNewTicks,gnTotalTicks;
static  int  gnTotalTime;
#endif
char streamBuf[READBUF_SIZE]; // wave input buffer
#ifdef ITE_RISC_FLOW
static AUDIO_ENCODE_PARAM  gAACAudioEncode;

char gOutBuf[OUTBUF_SIZE];    // encode AAC buffer 

char gOutFrameBuf[OUTFRAME_SIZE]; //prepare output aac buffer 0~3: time stamp, 4~7: data size, 8~end: data

unsigned int gTimeStamp;  // output data ,time stamp
unsigned int gDataSize; // ouput data,encoded size
unsigned int gAppendFrame; // output data,encoded frames
// microsecond 10(-6)
unsigned int TimeStamp48000[10]={21333,42666,64000,85333,106666,128000,149333,170666,192000,213333};
unsigned int TimeStamp44100[10]={23220,46440,69660,92880,116100,139320,162540,185760,208980,232200};
unsigned int TimeStamp32000[10]={32000,64000,96000,128000,160000,192000,224000,256000,288000,320000};
unsigned int OneTimeStampe48000[3] = {21000, 21000, 22000};
unsigned int gFrameCount = 0;
#define getUsedLen(rdptr, wrptr, len) (((wrptr) >= (rdptr)) ? ((wrptr) - (rdptr)) : ((len) - ((rdptr) - (wrptr))))
static int FillReadBuffer();
static void FillWriteBuffer(int nAACBytes);
void ChangeEndian(char* ptr,int size);
int getResampleEncodeBuffer(ResampleAudioContext *audioContext,short* writeptr,int nSize);
int getResampleAvailableLength(ResampleAudioContext *audioContext);
int resample(AVResampleContext *avResampleContext,ResampleAudioContext *audioContext, short *writeptr);

__inline short CLIPTOSHORT(int x)
{
    int sign;

    /* clip to [-32768, 32767] */
    sign = x >> 31;
    if (sign != (x >> 15))
        x = sign ^ ((1 << 15) - 1);

    return (short)x;
}

void resampleInit(ResampleAudioContext *audioContext){
    int nSize;
    memset(audioContext,0,sizeof(ResampleAudioContext));
    if (gAACAudioEncode.nSampleRate== 32000){
        nSize = gAACEncoder.nb_channels*2*686;//4
    }
    else if (gAACAudioEncode.nSampleRate == 44100){
        nSize = gAACEncoder.nb_channels*2*942;
        audioContext->nUseTempBuffer = 1;
    }            
    audioContext->nInSampleRate = gAACAudioEncode.nSampleRate;
    audioContext->nOutSampleRate = gAACEncoder.nOutSampleRate;
    audioContext->nInSize = nSize;
    audioContext->nInChannels = gAACEncoder.nb_channels;
    audioContext->nTempBufferLength = TEMP_BUFFER_SIZE;
    audioContext->nTempBufferRdPtr = 0;
    audioContext->nTempBufferWrPtr = 0;
    printf("[AAC Enc]resampleInit %d %d %d %d %d\n",audioContext->nInSampleRate,audioContext->nOutSampleRate,audioContext->nInSize,audioContext->nInChannels,audioContext->nUseTempBuffer);
}

int getResampleEncodeBuffer(ResampleAudioContext *audioContext,short* writeptr,int nSize){
    // copy to encode buffer
    if (audioContext->nTempBufferRdPtr+nSize < audioContext->nTempBufferLength){
        memcpy(writeptr,&audioContext->nTempBuffer[audioContext->nTempBufferRdPtr],nSize*2/* short*/);
        audioContext->nTempBufferRdPtr += nSize;
    } else if (audioContext->nTempBufferRdPtr+nSize == audioContext->nTempBufferLength) {
        memcpy(writeptr,&audioContext->nTempBuffer[audioContext->nTempBufferRdPtr],(audioContext->nTempBufferLength-audioContext->nTempBufferRdPtr)*2/* short*/);
        // reset rd ptr
        audioContext->nTempBufferRdPtr = 0;
    } else {
        memcpy(writeptr,&audioContext->nTempBuffer[audioContext->nTempBufferRdPtr],(audioContext->nTempBufferLength-audioContext->nTempBufferRdPtr)*2/* short*/);
        memcpy(&writeptr[audioContext->nTempBufferLength-audioContext->nTempBufferRdPtr],audioContext->nTempBuffer,(nSize-(audioContext->nTempBufferLength-audioContext->nTempBufferRdPtr))*2/* short*/);
        audioContext->nTempBufferRdPtr = nSize-(audioContext->nTempBufferLength-audioContext->nTempBufferRdPtr);
        // reset rd wr ptr
        memcpy(audioContext->nTempBuffer,&audioContext->nTempBuffer[audioContext->nTempBufferRdPtr],(audioContext->nTempBufferWrPtr-audioContext->nTempBufferRdPtr)*2/* short*/);
        audioContext->nTempBufferWrPtr = audioContext->nTempBufferWrPtr - (audioContext->nTempBufferWrPtr-audioContext->nTempBufferRdPtr);
        audioContext->nTempBufferRdPtr = 0;
    }
}

int getResampleAvailableLength(ResampleAudioContext *audioContext){
    if (audioContext->nTempBufferWrPtr>=audioContext->nTempBufferRdPtr){
        return (audioContext->nTempBufferWrPtr-audioContext->nTempBufferRdPtr);
    } else {
        printf("[AAC Enc]getResampleAvailableLength wr %d > rd %d ptr \n",audioContext->nTempBufferWrPtr,audioContext->nTempBufferRdPtr);
        return audioContext->nTempBufferLength-(audioContext->nTempBufferRdPtr-audioContext->nTempBufferWrPtr);
    }
}

int resample(AVResampleContext *avResampleContext,ResampleAudioContext *audioContext, short *writeptr)
{
    int nTemp,i,j;
    int consumed,lenout,nReSampleOutputSize;
    float ratio = (float)audioContext->nOutSampleRate/(float)audioContext->nInSampleRate;

    for (i=0;i<audioContext->nInChannels;i++) {
        int is_last = i + 1 == audioContext->nInChannels;

        lenout= (audioContext->nInSize/audioContext->nInChannels) *ratio + 16;  
        nTemp = (audioContext->nInSize/audioContext->nInChannels)/2+audioContext->nKeep[i];
        nReSampleOutputSize = av_resample(avResampleContext, audioContext->reSamplePcmOutput[i], audioContext->reSamplePcmInput[i],
            &consumed, nTemp, lenout, is_last);
        audioContext->nKeep[i] = nTemp - consumed;
        //printf("resample nReSampleOutputSize %d consumed %d   %d %d %d %d %d \n",nReSampleOutputSize,consumed,audioContext->reSamplePcmOutput[i][1],audioContext->reSamplePcmOutput[i][2],audioContext->reSamplePcmOutput[i][3],audioContext->reSamplePcmOutput[i][4],audioContext->reSamplePcmOutput[i][5]);
    } 
    
    nTemp = audioContext->nKeep[0];
    for (i=0;i<nTemp;i++) {
        for (j=0;j<audioContext->nInChannels;j++)
            audioContext->reSamplePcmInput[j][i] = audioContext->reSamplePcmInput[j][consumed + i];
    }
        
    if (audioContext->nUseTempBuffer == 0){
        if (audioContext->nInChannels==1) {
            for (i = 0; i < nReSampleOutputSize; i++) 
            {  // interleave channels
                *writeptr++ = audioContext->reSamplePcmOutput[0][i];
                *writeptr++ = audioContext->reSamplePcmOutput[0][i];
            }
        } else if (audioContext->nInChannels==2) {
            for (i = 0; i < nReSampleOutputSize; i++) 
            {  // interleave channels
                *writeptr++ = audioContext->reSamplePcmOutput[0][i];
                *writeptr++ = audioContext->reSamplePcmOutput[1][i];
            }
        }
    } else {
        // copy to temp buffer
        if (audioContext->nInChannels==1) {
            for (i = 0; i < nReSampleOutputSize; i++) 
            {  // interleave channels
                if (audioContext->nTempBufferWrPtr >= audioContext->nTempBufferLength){
                    audioContext->nTempBufferWrPtr = 0;
                }
                audioContext->nTempBuffer[audioContext->nTempBufferWrPtr++] = audioContext->reSamplePcmOutput[0][i];
                audioContext->nTempBuffer[audioContext->nTempBufferWrPtr++] = audioContext->reSamplePcmOutput[0][i];
            }
            // copy to encode buffer
            getResampleEncodeBuffer(audioContext,writeptr,2*MAX_FRAMESIZE);

        } else if (audioContext->nInChannels==2) {
            if (nReSampleOutputSize<MAX_FRAMESIZE){
                printf("[AAC Enc]resample use temp buffers nReSampleOutputSize %d < %d \n",nReSampleOutputSize,MAX_FRAMESIZE);
                nReSampleOutputSize = MAX_FRAMESIZE;
            }
            for (i = 0; i < nReSampleOutputSize; i++) 
            {  // interleave channels
                if (audioContext->nTempBufferWrPtr >= audioContext->nTempBufferLength){
                    audioContext->nTempBufferWrPtr = 0;
                }
                audioContext->nTempBuffer[audioContext->nTempBufferWrPtr++] = audioContext->reSamplePcmOutput[0][i];
                audioContext->nTempBuffer[audioContext->nTempBufferWrPtr++] = audioContext->reSamplePcmOutput[1][i];
            }
            // copy to encode buffer
            getResampleEncodeBuffer(audioContext,(short*)writeptr,audioContext->nInChannels*MAX_FRAMESIZE);
        }
    }
    return nReSampleOutputSize*2*sizeof(short);
}
void Get_Encode_Parameter()
{
    int mode,sampleRate;
#if defined(WIN32)
    gAACEncoder.nb_channels = 2;
    gAACEncoder.sample_rate = 44100;
#else
    if (gAACAudioEncode.nSampleRate != gAACAudioEncode.nOutSampleRate){
        gAACEncoder.sample_rate = gAACAudioEncode.nOutSampleRate;
    } else {
        gAACEncoder.sample_rate = gAACAudioEncode.nSampleRate;
    }

    gAACEncoder.nb_channels = gAACAudioEncode.nChannels;
    gAACEncoder.nOutSampleRate = gAACAudioEncode.nOutSampleRate;
    //gAACEncoder.bitrate = BIT_RATE;
    gAACEncoder.bitrate = (int)gAACAudioEncode.nBitrate;
    printf("[AAC Enc] ch %d sampleRate in %d out %d bitrate %d \n",gAACEncoder.nb_channels,gAACAudioEncode.nSampleRate,gAACAudioEncode.nOutSampleRate,gAACEncoder.bitrate);
#endif
    
}
static __inline unsigned int setStreamRdPtr(unsigned int wrPtr) 
{
    MMIO_Write(DrvDecode_RdPtr, wrPtr);
    return 0;
}
static __inline unsigned int setStreamRdPtrOtherRisc(unsigned int wrPtr) 
{
    MMIO_Write(DrvDecode_RdPtr, wrPtr);
    return 0;
}

static __inline unsigned int setStreamWrPtr(unsigned int wrPtr) 
{
    MMIO_Write(DrvDecode_WrPtr, wrPtr);
    return 0;
}
static __inline unsigned int getStreamWrPtr() 
{
    unsigned int wrPtr;
    wrPtr = MMIO_Read(DrvDecode_WrPtr);

    return wrPtr;
}

__inline unsigned int getStreamRdPtr() 
{
    unsigned int rdPtr;
    rdPtr = MMIO_Read(DrvDecode_RdPtr);

#if defined(__OR32__) && !defined(__FREERTOS__)
    if (0xffff == rdPtr) asm volatile("l.trap 15");
#endif
    return rdPtr;
}

static __inline unsigned int getStreamWrPtrOtherRisc() 
{
    unsigned int wrPtr;
    wrPtr = MMIO_Read(DrvDecode_WrPtr);

    return wrPtr;
}

__inline unsigned int getStreamRdPtrOtherRisc() 
{
    unsigned int rdPtr;
    rdPtr = MMIO_Read(DrvDecode_RdPtr);

#if defined(__OR32__) && !defined(__FREERTOS__)
    if (0xffff == rdPtr) asm volatile("l.trap 15");
#endif
    return rdPtr;
}
__inline unsigned int getOutBufRdPtr()
{
    unsigned int rdPtr;
    rdPtr = MMIO_Read(DrvEncode_RdPtr);
    return rdPtr;
}

__inline unsigned int setOutBufRdPtr(unsigned int wrPtr) 
{
    MMIO_Write(DrvEncode_RdPtr, wrPtr);   
    return 0;
}

__inline unsigned int getOutBufWrPtr() 
{
    unsigned int wrPtr;
    wrPtr = MMIO_Read(DrvEncode_WrPtr);
    return wrPtr;
}

__inline unsigned int setOutBufWrPtr(unsigned int wrPtr) 
{
    MMIO_Write(DrvEncode_WrPtr, wrPtr);   
    return 0;
}

#if 1
unsigned int I2S_AD32_GET_RP()
{
    unsigned int rPtr;
    rPtr = MMIO_Read(DrvEncode_RdPtr);
    return rPtr;

}

unsigned int I2S_AD32_GET_WP()
{
    unsigned int wPtr;
    wPtr = MMIO_Read(DrvEncode_WrPtr);
    return wPtr;

}

void I2S_AD32_SET_RP(unsigned int data32)
{
    MMIO_Write(DrvEncode_RdPtr, data32);
}

void I2S_AD32_SET_WP(unsigned int data32)
{
    MMIO_Write(DrvEncode_WrPtr, data32);
}


// getMixerReadPorinter() , must implement
unsigned int getMixerReadPorinter()
{
#ifdef TEST_MIXER
    return (unsigned int) gMixerRdptr;
    
#else
    return 0;
#endif
}

// getMixerWritePorinter() , must implement
unsigned int getMixerWritePorinter()
{
#ifdef TEST_MIXER
#if defined(__FREERTOS__)
        dc_invalidate(); // Flush Data Cache
#endif

    return (unsigned int) gMixerWrptr;
#else


    return 20000;
#endif
}

// setMixerReadPorinter() , must implement
void setMixerReadPorinter(unsigned int nReadPointer)
{
#ifdef TEST_MIXER
    gMixerRdptr = nReadPointer;
#else

#endif
}

// setMixerWritePorinter()
void setMixerWritePorinter(unsigned int nWritePointer)
{
#ifdef TEST_MIXER

    gMixerWrptr = nWritePointer;
#if defined(__FREERTOS__)
    dc_invalidate(); // Flush Data Cache
#endif

#else

#endif
}
#endif

static void occupyEncodeBuffer()
{
#if 0
    unsigned int nWriteProtect=0;
    unsigned int nProtect=0;
    do{
        nWriteProtect = getAudioReadBufferStatus();
        nProtect++;
        PalSleep(1);
    }while (nWriteProtect==1);

    if (nProtect>1)
        printf("[AAC Enc] occupyEncodeBuffer %d \n",nProtect);

    occupyAudioReadBuffer();
#endif
}

static void releaseEncodeBuffer()
{
#if 0// defined(__OR32__)
    releaseAudioReadBuffer();
#endif
}

static void occupyPCMBuffer()
{
#if 0 //defined(__OR32__)
    unsigned int nWriteProtect=0;
    unsigned int nProtect=0;

    do{
        nWriteProtect = getAudioWriteBufferStatus();
        nProtect++;
        PalSleep(1);
    }while (nWriteProtect==1);

    if (nProtect>1)
        printf("[AAC Enc] occupyPCMBuffer nProtect %d \n",nProtect);

    occupyAudioWriteBuffer();
#endif
}

static void releasePCMBuffer()
{
#if 0 //defined(__OR32__)
    releaseAudioWriteBuffer();
#endif
}

static int GetReadBufferSize() 
{
    int len = 0;
    unsigned int wavReadIdx,wavWriteIdx;
    wavReadIdx = getStreamRdPtr();
    // Wait Read Buffer avaliable
    wavWriteIdx = getStreamWrPtr();
    len = getUsedLen(wavReadIdx, wavWriteIdx, READBUF_SIZE);

    //printf("[AAC Enc] wavReadIdx %d wavWriteIdx %d len %d \n",wavReadIdx,wavWriteIdx,len);
    return len;
}

static int GetAvaliableReadBufferSizeOtherRisc() 
{
    int len = 0;
    unsigned int wavReadIdx,wavWriteIdx;
    wavReadIdx = getStreamRdPtrOtherRisc();
    // Wait Read Buffer avaliable
    wavWriteIdx = getStreamWrPtrOtherRisc();
    len = getUsedLen(wavReadIdx, wavWriteIdx, gAACAudioEncode.nBufferLength);

    //printf("[AAC Enc] wavReadIdx %d wavWriteIdx %d len %d \n",wavReadIdx,wavWriteIdx,len);
    return len;
}

static int GetAvaliableReadBufferSize_I2S() 
{
    int len = 0;
    unsigned int wavReadIdx,wavWriteIdx;
    wavReadIdx = I2S_AD32_GET_RP();
    // Wait Read Buffer avaliable
    wavWriteIdx = I2S_AD32_GET_WP();
    len = getUsedLen(wavReadIdx, wavWriteIdx, gAACAudioEncode.nBufferLength);

    //printf("[AAC Enc] wavReadIdx %d wavWriteIdx %d len %d \n",wavReadIdx,wavWriteIdx,len);
    return len;
}

static int GetAvaliableReadBufferSize_Mixer() 
{
    int len = 0;
    unsigned int mixReadIdx,mixWriteIdx;
    mixReadIdx = getMixerReadPorinter();
    // Wait Read Buffer avaliable
    mixWriteIdx = getMixerWritePorinter();
    len = getUsedLen(mixReadIdx, mixWriteIdx, gAACAudioEncode.nMixBufferLength);

    //printf("[Mp2 Enc] mixReadIdx %d mixWriteIdx %d len %d \n",mixReadIdx,mixWriteIdx,len);
    return len;
}


/**************************************************************************************
 * Function     : FillReadBuffer
 *
 * Description  : Update the read pointer of WAVE Buffer and return the valid data length
 *                of input buffer.
 *
 * Inputs       : nReadBytes: number of bytes will read
 *
 * Global var   : wavWriteIdx: write pointer of WAVE buffer
 *                wavReadIdx : read pointer of WAVE buffer
 *
 * Return       :
 *
 * TODO         :
 *
 * Note         : The WAVE buffer is circular buffer.
 *
 **************************************************************************************/
static int FillReadBuffer() 
{
    int len = 0;
    unsigned int wavReadIdx,wavWriteIdx;
    int nTemp,nUsedSize;

    // Updates the read buffer and returns the avaliable size
    // of input buffer. Wait a minimun FRAME_SIZE length.        
    nTemp = gAACEncoder.nb_channels*2*1024;        
    do
    {
        nUsedSize = GetReadBufferSize();
#if defined (__FREERTOS__)
        PalSleep(1);
#endif
    }while (nUsedSize<=nTemp && !isSTOP() && !isEOF());
#if defined(__FREERTOS__)
    dc_invalidate(); // Flush Data Cache
#endif         

    wavReadIdx = getStreamRdPtr();
    if (nTemp+wavReadIdx<=READBUF_SIZE){
        memcpy(&inputBuf[0],&streamBuf[wavReadIdx],nTemp);
    } else {
        //printf("[AAC Enc] memcpy \n");
        memcpy(&inputBuf[0],&streamBuf[wavReadIdx],READBUF_SIZE-wavReadIdx);
        memcpy(&inputBuf[READBUF_SIZE-wavReadIdx],&streamBuf[0],nTemp-(READBUF_SIZE-wavReadIdx));
    }
    //printf("[AAC Enc]  wavReadIdx  %d %d\n",wavReadIdx,nRead++);

    occupyPCMBuffer();
    
    //wavReadIdx = getStreamRdPtr();
    // Update Read Buffer
    if (nTemp > 0) {
        wavReadIdx = wavReadIdx + nTemp;
        if (wavReadIdx >= READBUF_SIZE) {
            wavReadIdx -= READBUF_SIZE;
        }
        setStreamRdPtr(wavReadIdx);
    }
#if defined(__FREERTOS__)
    dc_invalidate(); // Flush Data Cache
#endif

    releasePCMBuffer();

    return len;
}
/**************************************************************************************
 * Function     : FillReadBufferResample
 *
 * Description  : Update the read pointer of WAVE Buffer and return the valid data length
 *                of input buffer.
 *
 * Inputs       : nReadBytes: number of bytes will read
 *
 * Global var   : wavWriteIdx: write pointer of WAVE buffer
 *                wavReadIdx : read pointer of WAVE buffer
 *
 * Return       :
 *
 * TODO         :
 *
 * Note         : The WAVE buffer is circular buffer.
 *
 **************************************************************************************/
static int FillReadBufferResample(ResampleAudioContext *audioContext) 
{
    int len = 0;
    unsigned int wavReadIdx,wavWriteIdx;
    int nTmp;    
    int nTemp,bytesLeft;
    short* readBuffer;
    int i,j;

    // Updates the read buffer and returns the avaliable size
    // of input buffer. Wait a minimun FRAME_SIZE length.        
    //nTemp = MPA_FRAME_SIZE*2*gMpegAudioContext.nb_channels;        
    nTemp = audioContext->nInSize*(gAACAudioEncode.nSampleSize/16);

    // check resample temp buffers    
    if (audioContext->nUseTempBuffer == 1){
        nTmp = getResampleAvailableLength(audioContext);
        //printf("getResampleAvailableLength %d  %d  \n",nTemp,2*MAX_FRAMESIZE);
        if (nTmp>=audioContext->nInChannels*MAX_FRAMESIZE){
            printf("getResampleAvailableLength %d > %d  \n",nTmp,audioContext->nInChannels*MAX_FRAMESIZE);
            getResampleEncodeBuffer(audioContext,(short*)inputBuf,audioContext->nInChannels*MAX_FRAMESIZE);
            return 0;
        }
    }

    // buffer reserve from AAC Encoder
    if (gAACAudioEncode.nInputBufferType==0)
    {
        do
        {
            bytesLeft = GetReadBufferSize();
#ifdef ITE_RISC
            PalSleep(1);
#endif
        }while (bytesLeft<=nTemp && !isSTOP());
#if defined(__FREERTOS__)
        dc_invalidate(); // Flush Data Cache
#endif         

        wavReadIdx = getStreamRdPtr();
        if (nTemp+wavReadIdx<=READBUF_SIZE) {
            //memcpy(&paramBuf[0],&streamBuf[wavReadIdx],nTemp);
            // read data
            readBuffer = (short*)&streamBuf[wavReadIdx];
            for(i = 0,j=0; i < nTemp/2; i+=2,j++){
               audioContext->reSamplePcmInput[0][audioContext->nKeep[0]+j] = readBuffer[i];
               audioContext->reSamplePcmInput[1][audioContext->nKeep[1]+j] = readBuffer[i+1];
            }
        } else {
            //printf("[Mp2 Enc] memcpy \n");
            // read data
            readBuffer = (short*)&streamBuf[wavReadIdx];
            for(i = 0,j=0; i < (READBUF_SIZE-wavReadIdx)/2; i+=2,j++){
               audioContext->reSamplePcmInput[0][audioContext->nKeep[0]+j] = readBuffer[i];
               audioContext->reSamplePcmInput[1][audioContext->nKeep[1]+j] = readBuffer[i+1];
            }
            // read data
            readBuffer = (short*)&streamBuf[0];
            for(i = 0 ; i < (nTemp-(READBUF_SIZE-wavReadIdx))/2; i+=2,j++){
               audioContext->reSamplePcmInput[0][audioContext->nKeep[0]+j] = readBuffer[i];
               audioContext->reSamplePcmInput[1][audioContext->nKeep[1]+j] = readBuffer[i+1];
            }
        }        
        ChangeEndian((char*)&audioContext->reSamplePcmInput[0][audioContext->nKeep[0]], nTemp/2);
        ChangeEndian((char*)&audioContext->reSamplePcmInput[1][audioContext->nKeep[1]], nTemp/2);        
        // resample
        nTmp = resample(&avResampleContext,audioContext,(short*)inputBuf);

    }
    //printf("[Mp2 Enc]  wavReadIdx  %d %d\n",wavReadIdx,nRead++);

    occupyPCMBuffer();

    // Update Read Buffer
    if (nTemp > 0) {
        wavReadIdx = wavReadIdx + nTemp;
        if (wavReadIdx >= READBUF_SIZE) {
            wavReadIdx -= READBUF_SIZE;
        }
        setStreamRdPtr(wavReadIdx);
    }
#if defined(__FREERTOS__)
    dc_invalidate(); // Flush Data Cache
#endif

    releasePCMBuffer();
    
    return len;
}

/**************************************************************************************
 * Function     : FillReadBufferOtherRisc
 *
 * Description  : Update the read pointer of WAVE Buffer and return the valid data length
 *                of input buffer.
 *
 * Inputs       : nReadBytes: number of bytes will read
 *
 * Global var   : wavWriteIdx: write pointer of WAVE buffer
 *                wavReadIdx : read pointer of WAVE buffer
 *
 * Return       :
 *
 * TODO         :
 *
 * Note         : The WAVE buffer is circular buffer.
 *
 **************************************************************************************/
static int FillReadBufferOtherRisc() 
{
    int len = 0;
    unsigned int wavReadIdx,wavWriteIdx;
    int nTemp,nUsedSize;
    int i,j;
    short* buf;// = (char *)&pcmWriteBuf[pcmWriteIdx];
    int* in;//  = (char *)pcmbuf;

    // Updates the read buffer and returns the avaliable size
    // of input buffer. Wait a minimun FRAME_SIZE length.        
    nTemp = gAACEncoder.nb_channels*2*1024*(gAACAudioEncode.nSampleSize/16);
    // buffer reserve from Other Risc AP
    if (gAACAudioEncode.nInputBufferType==1){
        do
        {
            nUsedSize = GetAvaliableReadBufferSizeOtherRisc();
    #if defined (__FREERTOS__)
            PalSleep(1);
    #endif
        }while (nUsedSize<=nTemp && !isSTOP() && !isEOF());
    #if defined(__FREERTOS__)
        dc_invalidate(); // Flush Data Cache
    #endif         

        // read data into encode buffer
        wavReadIdx = getStreamRdPtrOtherRisc();
        if (gAACAudioEncode.nSampleSize==16){
            if (nTemp+wavReadIdx<=gAACAudioEncode.nBufferLength){
                memcpy(&inputBuf[0],&gAACAudioEncode.pInputBuffer[wavReadIdx],nTemp);
            } else {
                //printf("[AAC Enc] memcpy \n");
                memcpy(&inputBuf[0],&gAACAudioEncode.pInputBuffer[wavReadIdx],gAACAudioEncode.nBufferLength-wavReadIdx);
                memcpy(&inputBuf[gAACAudioEncode.nBufferLength-wavReadIdx],&gAACAudioEncode.pInputBuffer[0],nTemp-(gAACAudioEncode.nBufferLength-wavReadIdx));
            }
        } else if (gAACAudioEncode.nSampleSize==32){
            if (nTemp+wavReadIdx<=gAACAudioEncode.nBufferLength) {
                buf = (short*)inputBuf;
                in = (int*) &gAACAudioEncode.pInputBuffer[wavReadIdx];
                for (i= 0;i<1024*gAACEncoder.nb_channels;i++){
                    buf[i] = (short)in[i];
                }
            } else {
                printf("[AAC Enc] 32 memcpy\n");
                buf = (short*)inputBuf;
                in = (int*) &gAACAudioEncode.pInputBuffer[wavReadIdx];

                for (i= 0;i<(gAACAudioEncode.nBufferLength-wavReadIdx)/4;i++){
                    buf[i] = (short)in[i];
                }

                in = (int*) &gAACAudioEncode.pInputBuffer[0];
                for (j= 0;i<1024*gAACEncoder.nb_channels;i++,j++){
                    buf[i] = (short)in[j];
                }
            }
        }
        
        //printf("[AAC Enc]  wavReadIdx  %d %d\n",wavReadIdx,nRead++);

        occupyPCMBuffer();
        
        //wavReadIdx = getStreamRdPtr();
        // Update Read Buffer
        if (nTemp > 0) {
            wavReadIdx = wavReadIdx + nTemp;
            if (wavReadIdx >= gAACAudioEncode.nBufferLength) {
                wavReadIdx -= gAACAudioEncode.nBufferLength;
            }
            setStreamRdPtrOtherRisc(wavReadIdx);
        }
    #if defined(__FREERTOS__)
        dc_invalidate(); // Flush Data Cache
    #endif

        releasePCMBuffer();
    }
    return len;
}


/**************************************************************************************
 * Function     : FillReadBuffer_I2S
 *
 * Description  : Update the read pointer of WAVE Buffer and return the valid data length
 *                of input buffer.
 *
 * Inputs       : nReadBytes: number of bytes will read
 *
 * Global var   : wavWriteIdx: write pointer of WAVE buffer
 *                wavReadIdx : read pointer of WAVE buffer
 *
 * Return       :
 *
 * TODO         :
 *
 * Note         : The WAVE buffer is circular buffer.
 *
 **************************************************************************************/
static int FillReadBuffer_I2S() 
{
    int len = 0;
    unsigned int wavReadIdx,wavWriteIdx;
    unsigned int mixReadIdx,mixWriteIdx;
    int nTemp,nUsedSize;
    int i,j, k;
    short* buf;// = (char *)&pcmWriteBuf[pcmWriteIdx];
    short* pSrcbuf;
    short* pMixbuf;   
    int remain = 0;   
    int* in;//  = (char *)pcmbuf;

    // Updates the read buffer and returns the avaliable size
    // of input buffer. Wait a minimun FRAME_SIZE length.        
    nTemp = gAACEncoder.nb_channels*2*1024*(gAACAudioEncode.nSampleSize/16);

    // buffer reserve from I2S
    if (gAACAudioEncode.nInputBufferType==2 && gAACAudioEncode.nEnableMixer ==0){
        do
        {
            nUsedSize = GetAvaliableReadBufferSize_I2S();
    #if defined (__FREERTOS__)
            PalSleep(1);
    #endif
        }while (nUsedSize<=nTemp && !isSTOP() && !isEOF());
    #if defined(__FREERTOS__)
        dc_invalidate(); // Flush Data Cache
    #endif         

        // read data into encode buffer
        wavReadIdx = I2S_AD32_GET_RP();
        if (gAACAudioEncode.nSampleSize==16){
            if (nTemp+wavReadIdx<=gAACAudioEncode.nBufferLength){
                memcpy(&inputBuf[0],&gAACAudioEncode.pInputBuffer[wavReadIdx],nTemp);
            } else {
                //printf("[AAC Enc] memcpy \n");
                memcpy(&inputBuf[0],&gAACAudioEncode.pInputBuffer[wavReadIdx],gAACAudioEncode.nBufferLength-wavReadIdx);
                memcpy(&inputBuf[gAACAudioEncode.nBufferLength-wavReadIdx],&gAACAudioEncode.pInputBuffer[0],nTemp-(gAACAudioEncode.nBufferLength-wavReadIdx));
            }
        } else if (gAACAudioEncode.nSampleSize==32){
            if (nTemp+wavReadIdx<=gAACAudioEncode.nBufferLength) {
                buf = (short*)inputBuf;
                in = (int*) &gAACAudioEncode.pInputBuffer[wavReadIdx];
                for (i= 0;i<1024*gAACEncoder.nb_channels;i++){
                    buf[i] = (short)in[i];
                }
            } else {
                printf("[AAC Enc] 32 memcpy\n");
                buf = (short*)inputBuf;
                in = (int*) &gAACAudioEncode.pInputBuffer[wavReadIdx];

                for (i= 0;i<(gAACAudioEncode.nBufferLength-wavReadIdx)/4;i++){
                    buf[i] = (short)in[i];
                }

                in = (int*) &gAACAudioEncode.pInputBuffer[0];
                for (j= 0;i<1024*gAACEncoder.nb_channels;i++,j++){
                    buf[i] = (short)in[j];
                }
            }
        }        
        //printf("[AAC Enc]  wavReadIdx  %d %d\n",wavReadIdx,nRead++);

        occupyPCMBuffer();
        
        // Update Read Buffer
        if (nTemp > 0) {
            wavReadIdx = wavReadIdx + nTemp;
            if (wavReadIdx >= gAACAudioEncode.nBufferLength) {
                wavReadIdx -= gAACAudioEncode.nBufferLength;
            }
            I2S_AD32_SET_RP(wavReadIdx);
        }
    #if defined(__FREERTOS__)
        dc_invalidate(); // Flush Data Cache
    #endif
     //   releasePCMBuffer();
    } else if (gAACAudioEncode.nInputBufferType==2 && gAACAudioEncode.nEnableMixer ==1){
        // wait buffer available
        do
        {
            nUsedSize = GetAvaliableReadBufferSize_I2S();
    #if defined (__FREERTOS__)
            PalSleep(1);
    #else
            for (i = 0; i < 1024; i++) { asm(""); }
    #endif
        }while (nUsedSize<=nTemp && !isSTOP() && !isEOF());
        do
        {
            nUsedSize = GetAvaliableReadBufferSize_Mixer();
#ifdef ITE_RISC
            PalSleep(1);
#else
            for (i = 0; i < 1024; i++) { asm(""); }
#endif
        }while (nUsedSize<=nTemp && !isSTOP());             
    #if defined(__FREERTOS__)
        dc_invalidate(); // Flush Data Cache
    #endif         


#if 0
        // read data into encode buffer
        wavReadIdx = I2S_AD32_GET_RP();
        if (gAACAudioEncode.nSampleSize==16){
            if (nTemp+wavReadIdx<=gAACAudioEncode.nBufferLength){
                memcpy(&inputBuf[0],&gAACAudioEncode.pInputBuffer[wavReadIdx],nTemp);
            } else {
                //printf("[AAC Enc] memcpy \n");
                memcpy(&inputBuf[0],&gAACAudioEncode.pInputBuffer[wavReadIdx],gAACAudioEncode.nBufferLength-wavReadIdx);
                memcpy(&inputBuf[gAACAudioEncode.nBufferLength-wavReadIdx],&gAACAudioEncode.pInputBuffer[0],nTemp-(gAACAudioEncode.nBufferLength-wavReadIdx));
            }
        } else if (gAACAudioEncode.nSampleSize==32){
            if (nTemp+wavReadIdx<=gAACAudioEncode.nBufferLength) {
                buf = (short*)inputBuf;
                in = (int*) &gAACAudioEncode.pInputBuffer[wavReadIdx];
                for (i= 0;i<1024*gAACEncoder.nb_channels;i++){
                    buf[i] = (short)in[i];
                }
            } else {
                printf("[AAC Enc] 32 memcpy\n");
                buf = (short*)inputBuf;
                in = (int*) &gAACAudioEncode.pInputBuffer[wavReadIdx];

                for (i= 0;i<(gAACAudioEncode.nBufferLength-wavReadIdx)/4;i++){
                    buf[i] = (short)in[i];
                }

                in = (int*) &gAACAudioEncode.pInputBuffer[0];
                for (j= 0;i<1024*gAACEncoder.nb_channels;i++,j++){
                    buf[i] = (short)in[j];
                }
            }
        }        
        //printf("[AAC Enc]  wavReadIdx  %d %d\n",wavReadIdx,nRead++);

        // mix data
        mixReadIdx = getMixerReadPorinter();
        if (gAACAudioEncode.nSampleSize==16){
            if (nTemp+mixReadIdx<=gAACAudioEncode.nMixBufferLength) {
                //memcpy(&paramBuf[0],&gAACEncoder.pInputBuffer[wavReadIdx],nTemp);
                buf = (short*)inputBuf;
                pMixbuf = (short*) &gAACAudioEncode.pMixBuffer[mixReadIdx];
                for (i= 0;i<1024 *gAACEncoder.nb_channels;i++){
                    buf[i] = CLIPTOSHORT(buf[i] + pMixbuf[i]);
                }

            } else {
                buf = (short*)inputBuf;
                pMixbuf = (short*) &gAACAudioEncode.pMixBuffer[mixReadIdx];
                for (i= 0;i<(gAACAudioEncode.nMixBufferLength-mixReadIdx)/2;i++){
                    buf[i] = CLIPTOSHORT(buf[i] + pMixbuf[i]);
                }
                pMixbuf= (short*) &gAACAudioEncode.pMixBuffer[0];
                for (j= 0;j<1024*gAACEncoder.nb_channels-((gAACAudioEncode.nMixBufferLength-mixReadIdx)/2);i++,j++){
                    buf[i] = CLIPTOSHORT(buf[i] + pMixbuf[j]);                    
                }

            }
        }
#else
        wavReadIdx = I2S_AD32_GET_RP();
        mixReadIdx = getMixerReadPorinter();
        buf = (short*)inputBuf;        
        pSrcbuf = (short*) &gAACAudioEncode.pInputBuffer[wavReadIdx];
        pMixbuf = (short*) &gAACAudioEncode.pMixBuffer[mixReadIdx];
        if (gAACAudioEncode.nSampleSize==16){
            if (nTemp+wavReadIdx<=gAACAudioEncode.nBufferLength && nTemp+mixReadIdx<=gAACAudioEncode.nMixBufferLength)
            {
                for (i= 0;i<1024 *gAACEncoder.nb_channels;i++){
                    buf[i] = CLIPTOSHORT(pSrcbuf[i] + pMixbuf[i]);
                }
            }
            else if (nTemp+wavReadIdx<=gAACAudioEncode.nBufferLength)
            {
                for (i= 0;i<((gAACAudioEncode.nMixBufferLength-mixReadIdx) / 2);i++){
                    buf[i] = CLIPTOSHORT(pSrcbuf[i] + pMixbuf[i]);
                }
                pMixbuf= (short*) &gAACAudioEncode.pMixBuffer[0];
                remain = (nTemp - (gAACAudioEncode.nMixBufferLength-mixReadIdx)) / 2;
                for (j= 0; j< remain;i++,j++){
                    buf[i] = CLIPTOSHORT(pSrcbuf[i] + pMixbuf[j]);                    
                }
            }
            else if (nTemp+mixReadIdx<=gAACAudioEncode.nMixBufferLength)
            {
                for (i= 0;i<((gAACAudioEncode.nBufferLength-wavReadIdx) / 2);i++){
                    buf[i] = CLIPTOSHORT(pSrcbuf[i] + pMixbuf[i]);
                }
                pSrcbuf = (short*) &gAACAudioEncode.pInputBuffer[0];
                remain = (nTemp-(gAACAudioEncode.nBufferLength-wavReadIdx)) / 2;                
                for (j= 0; j< remain;i++,j++){
                    buf[i] = CLIPTOSHORT(pSrcbuf[j] + pMixbuf[i]);                    
                }
            }
            else
            {
                if (gAACAudioEncode.nBufferLength-wavReadIdx < gAACAudioEncode.nMixBufferLength-mixReadIdx)
                {             
                    for (i= 0;i<((gAACAudioEncode.nBufferLength-wavReadIdx) / 2);i++){
                        buf[i] = CLIPTOSHORT(pSrcbuf[i] + pMixbuf[i]);
                    }
                    pSrcbuf = (short*) &gAACAudioEncode.pInputBuffer[0];
                    remain = ((gAACAudioEncode.nMixBufferLength-mixReadIdx) - (gAACAudioEncode.nBufferLength-wavReadIdx)) / 2;                    
                    for (j = 0; j < remain; i++, j++) {
                        buf[i] = CLIPTOSHORT(pSrcbuf[j] + pMixbuf[i]);
                    }
                    pMixbuf= (short*) &gAACAudioEncode.pMixBuffer[0];
                    remain = (nTemp-(gAACAudioEncode.nMixBufferLength-mixReadIdx)) / 2;
                    for (k = 0; k < remain; i++, j++, k++) {
                        buf[i] = CLIPTOSHORT(pSrcbuf[j + k] + pMixbuf[k]);
                    }
                }
                else
                {
                    for (i= 0;i<((gAACAudioEncode.nMixBufferLength-mixReadIdx) / 2);i++){
                        buf[i] = CLIPTOSHORT(pSrcbuf[i] + pMixbuf[i]);
                    }
                    pMixbuf= (short*) &gAACAudioEncode.pMixBuffer[0];
                    remain = ((gAACAudioEncode.nBufferLength-wavReadIdx) - (gAACAudioEncode.nMixBufferLength-mixReadIdx)) / 2;
                    for (j = 0; j < remain; i++, j++) {
                        buf[i] = CLIPTOSHORT(pSrcbuf[i] + pMixbuf[j]);
                    }
                    pSrcbuf = (short*) &gAACAudioEncode.pInputBuffer[0];
                    remain = (nTemp-(gAACAudioEncode.nBufferLength-wavReadIdx)) / 2;
                    for (k = 0; k < remain; i++, j++, k++) {
                        buf[i] = CLIPTOSHORT(pSrcbuf[k] + pMixbuf[j + k]);
                    }
                }
            }
            
        }
#endif
      //  occupyPCMBuffer();
        
        // Update Read Buffer
        if (nTemp > 0) {
            wavReadIdx = wavReadIdx + nTemp;
            if (wavReadIdx >= gAACAudioEncode.nBufferLength) {
                wavReadIdx -= gAACAudioEncode.nBufferLength;
            }
            I2S_AD32_SET_RP(wavReadIdx);

            mixReadIdx = mixReadIdx + nTemp;
            if (mixReadIdx >= gAACAudioEncode.nMixBufferLength) {
                mixReadIdx -= gAACAudioEncode.nMixBufferLength;
            }
            //printf("[AAC Enc] setMixerReadPorinter %d \n",mixReadIdx);
            setMixerReadPorinter(mixReadIdx);

        }
    #if defined(__FREERTOS__)
        dc_invalidate(); // Flush Data Cache
    #endif
        //releasePCMBuffer();
    }

    return len;
}
/**************************************************************************************
 * Function     : FillReadBuffer_I2S_Resample
 *
 * Description  : Update the read pointer of WAVE Buffer and return the valid data length
 *                of input buffer.
 *
 * Inputs       : nReadBytes: number of bytes will read
 *
 * Global var   : wavWriteIdx: write pointer of WAVE buffer
 *                wavReadIdx : read pointer of WAVE buffer
 *
 * Return       :
 *
 * TODO         :
 *
 * Note         : The WAVE buffer is circular buffer.
 *
 **************************************************************************************/
static int FillReadBuffer_I2S_Resample(ResampleAudioContext *audioContext) 
{
    int len = 0;
    unsigned int wavReadIdx,wavWriteIdx;
    int nTmp;
    int nTemp,bytesLeft;
    int i,j;
    short* buf;// = (char *)&pcmWriteBuf[pcmWriteIdx];
    int* in;//  = (char *)pcmbuf;
    short* readBuffer;

    // Updates the read buffer and returns the avaliable size
    // of input buffer. Wait a minimun FRAME_SIZE length.        
    //nTemp = MPA_FRAME_SIZE*2*gMpegAudioContext.nb_channels*(gAACEncoder.nSampleSize/16);
    nTemp = audioContext->nInSize*(gAACAudioEncode.nSampleSize/16);

    // check resample temp buffers
    if (audioContext->nUseTempBuffer == 1){
        nTmp = getResampleAvailableLength(audioContext);
        //printf("getResampleAvailableLength %d  %d  \n",nTemp,2*MAX_FRAMESIZE);
        if (nTmp>=audioContext->nInChannels*MAX_FRAMESIZE){
            printf("getResampleAvailableLength %d > %d  \n",nTmp,audioContext->nInChannels*MAX_FRAMESIZE);
            getResampleEncodeBuffer(audioContext,(short*)inputBuf,audioContext->nInChannels*MAX_FRAMESIZE);
            return 0;
        }
    }

    // buffer reserve from I2S
    if (gAACAudioEncode.nInputBufferType==2)
    {
        do
        {
            bytesLeft = GetAvaliableReadBufferSize_I2S();
#ifdef ITE_RISC
            PalSleep(1);
#endif
        }while (bytesLeft<=nTemp && !isSTOP());
#if defined(__FREERTOS__)
        dc_invalidate(); // Flush Data Cache
#endif
        // read data into encode buffer
        wavReadIdx = I2S_AD32_GET_RP();
        if (gAACAudioEncode.nSampleSize==16){
            if (nTemp+wavReadIdx<=gAACAudioEncode.nBufferLength) {
                //memcpy(&paramBuf[0],&gAACEncoder.pInputBuffer[wavReadIdx],nTemp);
                // read data
                readBuffer = (short*)&gAACAudioEncode.pInputBuffer[wavReadIdx];
                for(i = 0,j=0; i < nTemp/2; i+=2,j++){
                   audioContext->reSamplePcmInput[0][audioContext->nKeep[0]+j] = readBuffer[i];
                   audioContext->reSamplePcmInput[1][audioContext->nKeep[1]+j] = readBuffer[i+1];
                }                
            } else {
                //printf("[Mp2 Enc] 16 memcpy \n");
                //memcpy(&paramBuf[0],&gAACEncoder.pInputBuffer[wavReadIdx],gAACEncoder.nBufferLength-wavReadIdx);
                //memcpy(&paramBuf[gAACEncoder.nBufferLength-wavReadIdx],&gAACEncoder.pInputBuffer[0],nTemp-(gAACEncoder.nBufferLength-wavReadIdx));
                // read data
                readBuffer = (short*)&gAACAudioEncode.pInputBuffer[wavReadIdx];
                for(i = 0,j=0; i < (gAACAudioEncode.nBufferLength-wavReadIdx)/2; i+=2,j++){
                   audioContext->reSamplePcmInput[0][audioContext->nKeep[0]+j] = readBuffer[i];
                   audioContext->reSamplePcmInput[1][audioContext->nKeep[1]+j] = readBuffer[i+1];
                }                
                // read data
                readBuffer = (short*)&gAACAudioEncode.pInputBuffer[0];
                for(i = 0 ; i < (nTemp-(gAACAudioEncode.nBufferLength-wavReadIdx))/2; i+=2,j++){
                   audioContext->reSamplePcmInput[0][audioContext->nKeep[0]+j] = readBuffer[i];
                   audioContext->reSamplePcmInput[1][audioContext->nKeep[1]+j] = readBuffer[i+1];
                }                
                
            }

    #if 0 //def AAC_ENCODE_PERFORMANCE_TEST_BY_TICK
            start_timer();
    #endif
           
            // resample
            nTmp = resample(&avResampleContext,audioContext,(short*)inputBuf);

    #if 0 //def AAC_ENCODE_PERFORMANCE_TEST_BY_TICK
            gnNewTicks = get_timer();            
            gnTotalTicks += gnNewTicks;
            if (gnFrames % 50 == 0 && gnFrames>0) {
                gnTotalTime += (gnTotalTicks/(PalGetSysClock()/1000));
                printf("[AAC Enc] resample (%d~%d) total %d (ms) average %d (ms) nFrames %d\n",(gnFrames+1-50),(gnFrames+1),(gnTotalTicks/(PalGetSysClock()/1000)),((gnTotalTicks/(PalGetSysClock()/1000))/50),gnFrames+1);
                gnTotalTicks=0;
            }
    #endif
            
        } else if (gAACAudioEncode.nSampleSize==32){
            printf("[AAC Enc] resample not support 32 bits sample size\n");
#if 0
            if (nTemp+wavReadIdx<=gAACEncoder.nBufferLength) {
                buf = (short*)paramBuf;
                in = (int*) &gAACEncoder.pInputBuffer[wavReadIdx];
                for (i= 0;i<MPA_FRAME_SIZE*gMpegAudioContext.nb_channels;i++){
                    buf[i] = (short)in[i];
                }
            } else {
               // printf("[Mp2 Enc] 32 memcpy\n");
                buf = (short*)paramBuf;
                in = (int*) &gAACEncoder.pInputBuffer[wavReadIdx];

                for (i= 0;i<(gAACEncoder.nBufferLength-wavReadIdx)/4;i++){
                    buf[i] = (short)in[i];
                }

                in = (int*) &gAACEncoder.pInputBuffer[0];
                for (j= 0;i<MPA_FRAME_SIZE*gMpegAudioContext.nb_channels;i++,j++){
                    buf[i] = (short)in[j];
                }

            }
#endif
        }
        //printf("[Mp2 Enc]  wavReadIdx  %d %d\n",wavReadIdx,nRead++);

        occupyPCMBuffer();        
        //wavReadIdx = getStreamRdPtr();
        // Update Read Buffer
        if (nTemp > 0) {
            wavReadIdx = wavReadIdx + nTemp;
            if (wavReadIdx >= gAACAudioEncode.nBufferLength) {
                wavReadIdx -= gAACAudioEncode.nBufferLength;
            }
            I2S_AD32_SET_RP(wavReadIdx);
        }
#if defined(__FREERTOS__)
        dc_invalidate(); // Flush Data Cache
#endif

        releasePCMBuffer();
    }
    
    return len;
}

/**************************************************************************************
 * Function     : FillReadBuffer_I2S_Resample_Mixer
 *
 * Description  : Update the read pointer of WAVE Buffer and return the valid data length
 *                of input buffer.
 *
 * Inputs       : nReadBytes: number of bytes will read
 *
 * Global var   : wavWriteIdx: write pointer of WAVE buffer
 *                wavReadIdx : read pointer of WAVE buffer
 *
 * Return       :
 *
 * TODO         :
 *
 * Note         : The WAVE buffer is circular buffer.
 *
 **************************************************************************************/
static int FillReadBuffer_I2S_Resample_Mixer(ResampleAudioContext *audioContext) 
{
    int len = 0;
    unsigned int wavReadIdx,wavWriteIdx;
    unsigned int mixReadIdx,mixWriteIdx;    
    
    int nTmp;
    int nTemp,bytesLeft;
    int i,j;
    short* buf;// = (char *)&pcmWriteBuf[pcmWriteIdx];
    short* pMixbuf;
    
    int* in;//  = (char *)pcmbuf;
    short* readBuffer;

    // Updates the read buffer and returns the avaliable size
    // of input buffer. Wait a minimun FRAME_SIZE length.        
    //nTemp = MPA_FRAME_SIZE*2*gMpegAudioContext.nb_channels*(gAACEncoder.nSampleSize/16);
    nTemp = audioContext->nInSize*(gAACAudioEncode.nSampleSize/16);

    // check resample temp buffers
    if (audioContext->nUseTempBuffer == 1){
        nTmp = getResampleAvailableLength(audioContext);
        //printf("getResampleAvailableLength %d  %d  \n",nTemp,2*MAX_FRAMESIZE);
        if (nTmp>=audioContext->nInChannels*MAX_FRAMESIZE){
            printf("getResampleAvailableLength %d > %d  \n",nTmp,audioContext->nInChannels*MAX_FRAMESIZE);
            getResampleEncodeBuffer(audioContext,(short*)inputBuf,audioContext->nInChannels*MAX_FRAMESIZE);
            return 0;
        }
    }

    // buffer reserve from I2S
    if (gAACAudioEncode.nInputBufferType==2)
    {
        do
        {
            bytesLeft = GetAvaliableReadBufferSize_I2S();
#ifdef ITE_RISC
            PalSleep(1);
#endif
        }while (bytesLeft<=nTemp && !isSTOP());
        do
        {
            bytesLeft = GetAvaliableReadBufferSize_Mixer();
#ifdef ITE_RISC
            PalSleep(1);
#endif
        }while (bytesLeft<=nTemp && !isSTOP());       
        
#if defined(__FREERTOS__)
        dc_invalidate(); // Flush Data Cache
#endif

        // read data into encode buffer
        wavReadIdx = I2S_AD32_GET_RP();
        if (gAACAudioEncode.nSampleSize==16){
            if (nTemp+wavReadIdx<=gAACAudioEncode.nBufferLength) {
                //memcpy(&paramBuf[0],&gAACEncoder.pInputBuffer[wavReadIdx],nTemp);
                // read data
                readBuffer = (short*)&gAACAudioEncode.pInputBuffer[wavReadIdx];
                for(i = 0,j=0; i < nTemp/2; i+=2,j++){
                   audioContext->reSamplePcmInput[0][audioContext->nKeep[0]+j] = readBuffer[i];
                   audioContext->reSamplePcmInput[1][audioContext->nKeep[1]+j] = readBuffer[i+1];
                }                
            } else {
                //printf("[Mp2 Enc] 16 memcpy \n");
                //memcpy(&paramBuf[0],&gAACEncoder.pInputBuffer[wavReadIdx],gAACEncoder.nBufferLength-wavReadIdx);
                //memcpy(&paramBuf[gAACEncoder.nBufferLength-wavReadIdx],&gAACEncoder.pInputBuffer[0],nTemp-(gAACEncoder.nBufferLength-wavReadIdx));
                // read data
                readBuffer = (short*)&gAACAudioEncode.pInputBuffer[wavReadIdx];
                for(i = 0,j=0; i < (gAACAudioEncode.nBufferLength-wavReadIdx)/2; i+=2,j++){
                   audioContext->reSamplePcmInput[0][audioContext->nKeep[0]+j] = readBuffer[i];
                   audioContext->reSamplePcmInput[1][audioContext->nKeep[1]+j] = readBuffer[i+1];
                }                
                // read data
                readBuffer = (short*)&gAACAudioEncode.pInputBuffer[0];
                for(i = 0 ; i < (nTemp-(gAACAudioEncode.nBufferLength-wavReadIdx))/2; i+=2,j++){
                   audioContext->reSamplePcmInput[0][audioContext->nKeep[0]+j] = readBuffer[i];
                   audioContext->reSamplePcmInput[1][audioContext->nKeep[1]+j] = readBuffer[i+1];
                }                
                
            }


            // mix data
            mixReadIdx = getMixerReadPorinter();
            
            if (nTemp+mixReadIdx<=gAACAudioEncode.nMixBufferLength) {
                //memcpy(&paramBuf[0],&gMp2AudioEncode.pInputBuffer[wavReadIdx],nTemp);
                pMixbuf = (short*) &gAACAudioEncode.pMixBuffer[mixReadIdx];
                for(i = 0,j=0; i < nTemp/2; i+=2,j++){
                   audioContext->reSamplePcmInput[0][audioContext->nKeep[0]+j] = CLIPTOSHORT(audioContext->reSamplePcmInput[0][audioContext->nKeep[0]+j]+pMixbuf[i]);
                   audioContext->reSamplePcmInput[1][audioContext->nKeep[1]+j] = CLIPTOSHORT(audioContext->reSamplePcmInput[1][audioContext->nKeep[1]+j]+pMixbuf[i+1]);
                }
            } else {
                pMixbuf = (short*) &gAACAudioEncode.pMixBuffer[mixReadIdx];                
                for(i = 0,j=0; i < (gAACAudioEncode.nMixBufferLength-mixReadIdx)/2; i+=2,j++){
                   audioContext->reSamplePcmInput[0][audioContext->nKeep[0]+j] = CLIPTOSHORT(audioContext->reSamplePcmInput[0][audioContext->nKeep[0]+j]+pMixbuf[i]);
                   audioContext->reSamplePcmInput[1][audioContext->nKeep[1]+j] = CLIPTOSHORT(audioContext->reSamplePcmInput[1][audioContext->nKeep[1]+j]+pMixbuf[i+1]);
                }
                // read data
                pMixbuf= (short*) &gAACAudioEncode.pMixBuffer[0];                
                for(i = 0 ; i < (nTemp-(gAACAudioEncode.nMixBufferLength-mixReadIdx))/2; i+=2,j++){
                   audioContext->reSamplePcmInput[0][audioContext->nKeep[0]+j] = CLIPTOSHORT(audioContext->reSamplePcmInput[0][audioContext->nKeep[0]+j]+pMixbuf[i]);
                   audioContext->reSamplePcmInput[1][audioContext->nKeep[1]+j] = CLIPTOSHORT(audioContext->reSamplePcmInput[1][audioContext->nKeep[1]+j]+pMixbuf[i+1]);
                }

            }
    #if 0 //def AAC_ENCODE_PERFORMANCE_TEST_BY_TICK
            start_timer();
    #endif
           
            // resample
            nTmp = resample(&avResampleContext,audioContext,(short*)inputBuf);

    #if 0 //def AAC_ENCODE_PERFORMANCE_TEST_BY_TICK
            gnNewTicks = get_timer();            
            gnTotalTicks += gnNewTicks;
            if (gnFrames % 50 == 0 && gnFrames>0) {
                gnTotalTime += (gnTotalTicks/(PalGetSysClock()/1000));
                printf("[AAC Enc] resample (%d~%d) total %d (ms) average %d (ms) nFrames %d\n",(gnFrames+1-50),(gnFrames+1),(gnTotalTicks/(PalGetSysClock()/1000)),((gnTotalTicks/(PalGetSysClock()/1000))/50),gnFrames+1);
                gnTotalTicks=0;
            }
    #endif
            
        } else if (gAACAudioEncode.nSampleSize==32){
            printf("[AAC Enc] resample not support 32 bits sample size\n");
#if 0
            if (nTemp+wavReadIdx<=gAACEncoder.nBufferLength) {
                buf = (short*)paramBuf;
                in = (int*) &gAACEncoder.pInputBuffer[wavReadIdx];
                for (i= 0;i<MPA_FRAME_SIZE*gMpegAudioContext.nb_channels;i++){
                    buf[i] = (short)in[i];
                }
            } else {
               // printf("[Mp2 Enc] 32 memcpy\n");
                buf = (short*)paramBuf;
                in = (int*) &gAACEncoder.pInputBuffer[wavReadIdx];

                for (i= 0;i<(gAACEncoder.nBufferLength-wavReadIdx)/4;i++){
                    buf[i] = (short)in[i];
                }

                in = (int*) &gAACEncoder.pInputBuffer[0];
                for (j= 0;i<MPA_FRAME_SIZE*gMpegAudioContext.nb_channels;i++,j++){
                    buf[i] = (short)in[j];
                }

            }
#endif
        }
        //printf("[Mp2 Enc]  wavReadIdx  %d %d\n",wavReadIdx,nRead++);

      //  occupyPCMBuffer();        
        //wavReadIdx = getStreamRdPtr();
        // Update Read Buffer
        if (nTemp > 0) {
            wavReadIdx = wavReadIdx + nTemp;
            if (wavReadIdx >= gAACAudioEncode.nBufferLength) {
                wavReadIdx -= gAACAudioEncode.nBufferLength;
            }
            I2S_AD32_SET_RP(wavReadIdx);

            mixReadIdx = mixReadIdx + nTemp;
            if (mixReadIdx >= gAACAudioEncode.nMixBufferLength) {
                mixReadIdx -= gAACAudioEncode.nMixBufferLength;
            }
            //printf("[AAC Enc] setMixerReadPorinter %d \n",mixReadIdx);
            setMixerReadPorinter(mixReadIdx);
        }
#if defined(__FREERTOS__)
        dc_invalidate(); // Flush Data Cache
#endif

    //    releasePCMBuffer();
    }
    
    return len;
}



// return 1:write output buffer,return 0: not write output buffer
static int OutputWriteBuffer(int nBytes)
{
    // frame count >=OUTPUT_FRAME  or frame size > OUTFRAME_SIZE-8 ,output
#ifdef OUTPUT_ONE_FRAME
    // get time stamp;
    if (gAACEncoder.sample_rate==48000)
    {
        //gTimeStamp = TimeStamp48000[0];
        gTimeStamp = OneTimeStampe48000[gFrameCount];
        gFrameCount = (gFrameCount + 1) % 3;
    }
    else if (gAACEncoder.sample_rate==44100)
        gTimeStamp = TimeStamp44100[0];
    else if (gAACEncoder.sample_rate==32000)
        gTimeStamp = TimeStamp32000[0];

    gDataSize=nBytes;    
    memcpy(&gOutFrameBuf[0],&gTimeStamp,sizeof(int));
    memcpy(&gOutFrameBuf[4],&gDataSize,sizeof(int));
    if (nBytes<=OUTFRAME_SIZE)
        memcpy(&gOutFrameBuf[8],&outbuf[0],nBytes);
    
    if (nBytes==0 || nBytes > OUTFRAME_SIZE)
        printf("[AAC Enc] OutputWriteBuffer %d %d %d\n",gTimeStamp,gDataSize,nBytes);        

    return 1;

#else

    if ( (gAppendFrame+1)>=10 || (gDataSize+nBytes)>=OUTFRAME_SIZE-8){
        // get time stamp;
        if (gAACEncoder.sample_rate==48000)
            gTimeStamp = TimeStamp48000[gAppendFrame-1];
        else if (gAACEncoder.sample_rate==44100)
            gTimeStamp = TimeStamp44100[gAppendFrame-1];
        else if (gAACEncoder.sample_rate==32000)
            gTimeStamp = TimeStamp32000[gAppendFrame-1];

        memcpy(&gOutFrameBuf[0],&gTimeStamp,sizeof(int));
        memcpy(&gOutFrameBuf[4],&gDataSize,sizeof(int));            
       // printf("[AAC Enc] OutputWriteBuffer %d %d %d %d\n",gTimeStamp,gDataSize,gAppendFrame,nBytes);        
        gAppendFrame=0;
        gDataSize=0;
        return 1;
    } else {
        return 0;
    }
#endif    
}

static int AppendWriteBuffer(int nBytes)
{
    memcpy(&gOutFrameBuf[8+gDataSize],&outbuf[0],nBytes);
    gAppendFrame++;
    gDataSize+=nBytes;

}


/**************************************************************************************
 * Function     : FillWriteBuffer
 *
 * Description  : Wait the avaliable length of the output buffer bigger than on
 *                frame of audio data.
 *
 * Inputs       :
 *
 * Global Var   : pcmWriteIdx: write index of output buffer.
 *                pcmReadIdx : read index of output buffer.
 *
 * Outputs      :
 *
 * Return       :
 *
 * TODO         :
 *
 * Note         : Output buffer is a circular queue.
 *
 **************************************************************************************/
static void FillWriteBuffer(int nBytes) 
{
    int len;
    int nOutReadIdx,nOutWriteIdx;

    occupyEncodeBuffer();
    
    nOutWriteIdx = getOutBufWrPtr(); 
    // Update Write Buffer
#if 1
    if (OutputWriteBuffer(nBytes) > 0) {
        if (nOutWriteIdx+OUTFRAME_SIZE > OUTBUF_SIZE) {
            // it will not happen
            memcpy(&gOutBuf[nOutWriteIdx],&gOutFrameBuf[0],OUTBUF_SIZE-nOutWriteIdx);
            memcpy(&gOutBuf[0],&gOutFrameBuf[OUTBUF_SIZE-nOutWriteIdx],OUTFRAME_SIZE-(OUTBUF_SIZE-nOutWriteIdx));
            nOutWriteIdx = 0;
        } else {
            memcpy(&gOutBuf[nOutWriteIdx],&gOutFrameBuf[0],OUTFRAME_SIZE);
            nOutWriteIdx = nOutWriteIdx + OUTFRAME_SIZE;
            if (nOutWriteIdx>=OUTBUF_SIZE)
                nOutWriteIdx=0;
        }
        //printf("[AAC Enc] set outBuffer %d \n",nOutWriteIdx);
        setOutBufWrPtr(nOutWriteIdx);
    }
    #ifndef OUTPUT_ONE_FRAME
    AppendWriteBuffer(nBytes);
    #endif
#else
    if (nMp3Bytes > 0) {
    
        if (nOutWriteIdx+nMp3Bytes >= OUTBUF_SIZE) {
            memcpy(&gOutBuf[nOutWriteIdx],&gFrame[0],OUTBUF_SIZE-nOutWriteIdx);
            memcpy(&gOutBuf[0],&gFrame[OUTBUF_SIZE-nOutWriteIdx],nMp3Bytes-(OUTBUF_SIZE-nOutWriteIdx));
            nOutWriteIdx = nMp3Bytes-(OUTBUF_SIZE-nOutWriteIdx);
        } else {
            memcpy(&gOutBuf[nOutWriteIdx],&gFrame[0],nMp3Bytes);
            nOutWriteIdx = nOutWriteIdx + nMp3Bytes;            
        }
        setOutBufWrPtr(nOutWriteIdx);
    }
#endif

#if defined(__FREERTOS__)
        dc_invalidate(); // Flush Data Cache
#endif
    releaseEncodeBuffer();

    // Wait output buffer avaliable
    do {
        nOutReadIdx = getOutBufRdPtr();    
        if (nOutReadIdx <= nOutWriteIdx) {
            len = OUTBUF_SIZE - (nOutWriteIdx - nOutReadIdx);
        } else {
            len = nOutReadIdx - nOutWriteIdx;
        }

        if ((len-2) < OUTFRAME_SIZE && !isSTOP()) {
        #if defined(__FREERTOS__)
            PalSleep(2);
        #endif
        } else {
            break;
        }
    } while(1 && !isSTOP());

    // PRINTF("pcmWriteIdx(%d) pcmReadIdx(%d) len(%d) nPCMBytes(%d)\n", pcmWriteIdx, pcmReadIdx, len, nPCMBytes);
}


static __inline void checkControl(void) {
    static int curPause = 0;
    static int prePause = 0;

   /* do {
        eofReached  = isEOF() || isSTOP();
        curPause = isPAUSE();
        if (!curPause) {  // Un-pause
            if (prePause) pauseDAC(0);
            break;
        } else { // Pause
            if (!prePause && curPause) {
                pauseDAC(1);
            }
            #if defined(__FREERTOS__)
            //taskYIELD();
            PalSleep(1);
            #else
            or32_delay(1); // delay 1ms
            #endif
        }
        prePause = curPause;
    } while(!eofReached);*/

    prePause = curPause;
}

void ClearRdBuffer(void) {

    //SetFrameNo(0);
    MMIO_Write(DrvDecode_WrPtr, 0);
    MMIO_Write(DrvDecode_RdPtr, 0);
    MMIO_Write(DrvEncode_WrPtr, 0);
    MMIO_Write(DrvEncode_RdPtr, 0);

    memset(gOutFrameBuf,0,sizeof(gOutFrameBuf));
    memset(gOutBuf,0,sizeof(gOutFrameBuf));
    
    #if defined(__OR32__)
        dc_invalidate(); // Flush DC Cache
    #endif

    if (isSTOP()) {
        MMIO_Write(DrvAudioCtrl, MMIO_Read(DrvAudioCtrl) & ~DrvDecode_STOP);
    }
    
    releasePCMBuffer();
    releaseEncodeBuffer();
}

void AACEncode_GetBufInfo(unsigned* inBuf, unsigned* inLen,unsigned* outEnBuf, unsigned* outEnLen,unsigned* pAudio)
{
#if defined(__FREERTOS__)
    dc_invalidate(); // Flush Data Cache
#endif         

    *inBuf = (unsigned)streamBuf;
    *inLen = sizeof(streamBuf);
    *outEnBuf =  (unsigned)gOutBuf;
    *outEnLen =  sizeof(gOutBuf);

    *pAudio = (unsigned)&gAACAudioEncode;
}

void ChangeEndian(char* ptr,int size)
{
    int i;
    char *buf = (char *)ptr;
    char in;
    for(i=0; i<size; i+=2)
    {
        in = buf[i];
        buf[i]   = buf[i+1];
        buf[i+1] = in;
    }
}

void ChangeEndian4(char* ptr,int size)
{
    int i;
    char *buf = (char *)ptr;
    char in , in2;
    for(i=0; i<size; i+=4)
    {
        in = buf[i];
        in2 = buf[i+1];
        buf[i]   = buf[i+3];
        buf[i+1] = buf[i+2];
        buf[i+2] =in2;
        buf[i+3] = in;
    }
}

#endif // #ifdef ITE_RISC_FLOW

void usage(const char* name) {
    printf( "[AAC Enc]%s [-r bitrate] in.wav out.aac\n", name);
}

# if defined(__FREERTOS__) && !defined(ENABLE_CODECS_PLUGIN)
portTASK_FUNCTION(mp2encode_task, params)
# else
int main(int argc, char *argv[]) 
#endif
{
    int bitrate = 128000;
    int ch;
    int format, sampleRate, channels, bitsPerSample;
    int inputSize;
    VO_AUDIO_CODECAPI codec_api = { 0 };
    VO_HANDLE handle = 0;
    VO_MEM_OPERATOR mem_operator = { 0 };
    VO_CODEC_INIT_USERDATA user_data;
    AACENC_PARAM params = { 0 };
    // resample 
    ResampleAudioContext resampleAudioContext;
    int nNeedResample = 0;
    int nTemp;
    int nSize;

#ifdef AAC_ENCODE_PERFORMANCE_TEST_BY_TICK
    int  nNewTicks,nTotalTicks;
    int  nTotalTime;

#endif

#if defined(WIN32)
    char *outfile="D:\\audio_testing\\AAC\\bird44100.aac";
    //char *outfile="D:\\audio_testing\\AAC\\bird48000_6.aac";
    FILE *out;
#endif

    int nResult;
#ifdef ITE_RISC_FLOW
    //param_struct param;
  /*    
    char *fileName[]=
	{
	 "mp3decode",
	 "-ao",
     //"D:\\audio_testing\\Mp3\\bird_48000.wav",
     //"D:\\audio_testing\\WAV\\swf_adpcm.wav",
     //"D:\\audio_testing\\WAV\\ima-adpcm.wav",     
     //"D:\\Castor3_Alpha\\test_data\\dump_data.mp3",     
     "D:\\audio_testing\\WAV\\1.bird_44100.wav",     
     //"D:\\audio_testing\\WAV\\1.bird_44100.wav",  
     //"D:\\audio_testing\\Mp3\\bird32000.wav",
     //"D:\\audio_testing\\Mp3\\BBC7.mp3",
	 //"D:\\audio_testing\\Mp3\\bird_22050.wav"
     "D:\\audio_testing\\AAC\\bird44100.aac"
	};

    argc+=3;
    argv = fileName;
    GetParam(argc, argv);
    win32_init();
    */
#else    
    char *infile="D:\\audio_testing\\WAV\\1.bird_44100.wav";
    //char *infile="D:\\audio_testing\\WAV\\1.bird_48000_6.wav";
    void *wav;
#endif    

#if 0//def AAC_ENABLE_INTERNAL_SD
    inputBuf = INTERNAL_SD;
#endif
    

#ifdef ITE_RISC_FLOW
    do{
    gTimeStamp = 0;
    gDataSize = 0;
    gAppendFrame = 0;

    memset(gOutFrameBuf,0,sizeof(gOutFrameBuf));
    memset(gOutBuf,0,sizeof(gOutFrameBuf));

    do {
        PalSleep(1);
    } while (MMIO_Read(AUDIO_DECODER_START_FALG) != 2);
#if defined(__OR32__)
    if (&gAACAudioEncode){
        printf("[AAC Enc] start , input buffer type %d nSampleSize %d %d 0x%x \n",gAACAudioEncode.nInputBufferType,gAACAudioEncode.nSampleSize,nResult,I2S_AD32_GET_RP());
    } else {
        printf("[AAC Enc] start , input buffer null pointer \n");
    }    
#endif

    if (gAACAudioEncode.nInputBufferType==0 || gAACAudioEncode.nInputBufferType==1){
        setStreamRdPtr(0);
        setStreamWrPtr(0);
    }
    setOutBufRdPtr(0);
    setOutBufWrPtr(0);

    Get_Encode_Parameter();

    bitrate = gAACEncoder.bitrate;
    channels = gAACEncoder.nb_channels;
    sampleRate = gAACEncoder.sample_rate;

#else
    wav = wav_read_open(infile);
    if (!wav) {
        printf("[AAC Enc]Unable to open wav file %s\n", infile);
        return 1;
    }
    if (!wav_get_header(wav, &format, &channels, &sampleRate, &bitsPerSample, NULL)) {
        printf("[AAC Enc]Bad wav file %s\n", infile);
        return 1;
    }
    if (format != 1) {
        printf("[AAC Enc]Unsupported WAV format %d\n", format);
        return 1;
    }
    if (bitsPerSample != 16) {
        printf("[AAC Enc]Unsupported WAV sample depth %d\n", bitsPerSample);
        return 1;
    }
#endif    
    inputSize = channels*2*1024;
    //inputBuf = (uint8_t*) malloc(inputSize);
    //convertBuf = (int16_t*) malloc(inputSize);
    gnFrames = 0;
    voGetAACEncAPI(&codec_api);

    mem_operator.Alloc = cmnMemAlloc;
    mem_operator.Copy = cmnMemCopy;
    mem_operator.Free = cmnMemFree;
    mem_operator.Set = cmnMemSet;
    mem_operator.Check = cmnMemCheck;
    user_data.memflag = VO_IMF_USERMEMOPERATOR;
    user_data.memData = &mem_operator;
    if (codec_api.Init(&handle, VO_AUDIO_CodingAAC, &user_data))
        printf("[AAC Enc] init error \n");

    params.sampleRate = sampleRate;
    params.bitRate = bitrate;
    params.nChannels = channels;
    params.adtsUsed = 1;
    printf("[AAC Enc] SetParam %d %d \n",sampleRate,channels);
    if (codec_api.SetParam(handle, VO_PID_AAC_ENCPARAM, &params) != VO_ERR_NONE) {
        printf("[AAC Enc]Unable to set encoding parameters\n");
        return 1;
    }

#if defined(WIN32)
    out = fopen(outfile, "wb");
    if (!out) {
        perror(outfile);
        return 1;
    }
#endif
#ifdef AAC_ENCODE_PERFORMANCE_TEST_BY_TICK
     nNewTicks = 0;
     nTotalTicks=0;
     nTotalTime=0;
     gnNewTicks = 0;
     gnTotalTicks=0;
     gnTotalTime=0;          
#endif        


// resample setting
        if (gAACAudioEncode.nSampleRate != gAACAudioEncode.nOutSampleRate) {            
            printf("[AAC Enc] resampleInit \n");
            resampleInit(&resampleAudioContext);
            av_resample_init(&avResampleContext,gAACAudioEncode.nOutSampleRate, gAACAudioEncode.nSampleRate, RESAMPLE_FILTER_LENGTH, 10,0, 0.8);
            printf("[AAC Enc] av_resample_init \n");            
            nNeedResample = 1;
        } else {
            nNeedResample = 0;
        }
// 
        if (gAACAudioEncode.nEnableMixer==1){
            printf("[AAC Enc] enable mixer ,Mixer buffer length %d \n",gAACAudioEncode.nMixBufferLength);
        }


    while (1) {
        VO_CODECBUFFER input = { 0 }, output = { 0 };
        VO_AUDIO_OUTPUTINFO output_info = { 0 };
        int read, i;
#if defined(__OR32__)
        // read input data
        if (gAACAudioEncode.nInputBufferType==0){
            if (nNeedResample==1) {
                FillReadBufferResample(&resampleAudioContext);                    
            } else {
                FillReadBuffer();
            }
        } else if (gAACAudioEncode.nInputBufferType==1){
            FillReadBufferOtherRisc();
        } else if (gAACAudioEncode.nInputBufferType==2){
	          if (nNeedResample==1) {
                     if (gAACAudioEncode.nEnableMixer==1){
                         // resample and mixer
                         FillReadBuffer_I2S_Resample_Mixer(&resampleAudioContext);
                     } else {
                         FillReadBuffer_I2S_Resample(&resampleAudioContext);
                     }                
	          } else {
	              FillReadBuffer_I2S();
	          }
        }

        read = inputSize;
        if (isSTOP()) {
            break;
        }
#else
        read = wav_read_data(wav, inputBuf, inputSize);
#endif
        if (read < inputSize)
            break;
        if (gAACAudioEncode.nInputBufferType!=2) {
          if (nNeedResample==0){
                for (i = 0; i < read/2; i++) {
                    const unsigned char* in = &inputBuf[2*i];
                    convertBuf[i] = in[0] | (in[1] << 8);
                }
                input.Buffer = (unsigned char*) convertBuf;            
            } else {
                input.Buffer = (unsigned char*) inputBuf;
            }          
        } else {
            input.Buffer = (unsigned char*) inputBuf;
        }

        input.Length = read;
        codec_api.SetInputData(handle, &input);
        output.Buffer = outbuf;
        output.Length = sizeof(outbuf);
    #ifdef AAC_ENCODE_PERFORMANCE_TEST_BY_TICK
        start_timer();
    #endif

        if (codec_api.GetOutputData(handle, &output, &output_info) != VO_ERR_NONE) {
            printf("[AAC Enc]Unable to encode frame\n");
            //return 1;
            break;
        }
        gnFrames++;
    #ifdef AAC_ENCODE_PERFORMANCE_TEST_BY_TICK
        nNewTicks = get_timer();            
        nTotalTicks += nNewTicks;
        if (gnFrames % 100 == 0 && gnFrames>0) {
            nTotalTime += (nTotalTicks/(PalGetSysClock()/1000));
            printf("[AAC Enc] (%d~%d) total %d (ms) average %d (ms) nFrames %d system clock %d \n",(gnFrames+1-100),(gnFrames+1),(nTotalTicks/(PalGetSysClock()/1000)),((nTotalTicks/(PalGetSysClock()/1000))/100),gnFrames+1,PalGetSysClock());
            nTotalTicks=0;
        }
    #endif

#if defined(__OR32__)
        FillWriteBuffer(output.Length);
#endif
        if (isEOF()) {
            break;
        }
        if (isSTOP()) {
            break;
        }

#if defined(WIN32)
        fwrite(outbuf, 1, output.Length, out);
#endif
    }
//  free(inputBuf);
//  free(convertBuf);
#if defined(WIN32)
    fclose(out);
#endif

#ifdef ITE_RISC_FLOW
    codec_api.Uninit(handle);

    ClearRdBuffer();
    memset(&codec_api,0,sizeof(codec_api));
    handle = 0;
    memset(&mem_operator,0,sizeof(mem_operator));
    memset(&user_data,0,sizeof(user_data));
    memset(&params,0,sizeof(params));

    dc_invalidate(); // Flush Data Cache

    MMIO_Write(AUDIO_DECODER_START_FALG,0);
    }while (1);
#else
    fclose(out);
    codec_api.Uninit(handle);
    wav_read_close(wav);
#endif

    return 0;
}

