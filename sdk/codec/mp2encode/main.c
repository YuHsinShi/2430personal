/////////////////////////////////////////////////////////////////
//                      Include File
/////////////////////////////////////////////////////////////////
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "mpegaudio.h"



#if defined(ENABLE_CODECS_PLUGIN)
# include "plugin.h"
#endif

#ifdef MP2_ENCODE_PERFORMANCE_TEST_BY_TICK
#include "ticktimer.h"
#endif
#include "resample.h"

#if defined(__OR32__)
#include "mmio.h"
#endif

#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)
/* Code for general_printf() */
#define BITS_PER_BYTE    8
#define MINUS_SIGN       1
#define RIGHT_JUSTIFY    2
#define ZERO_PAD         4
#define CAPITAL_HEX      8

struct parameters
{
    int   number_of_output_chars;
    short minimum_field_width;
    char  options;
    short edited_string_length;
    short leading_zeros;
};
#endif        

#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)
static unsigned int* gtAudioPluginBufferLength;
static unsigned short* gtAudioPluginBuffer;
static int gnTemp;
static unsigned char tDumpWave[] = "C:/mp2_dump.pcm";
static int gCh;
static int gSampleRate;
int gPause = 0 ;
int gPrint = 0;
unsigned char *gBuf;
#endif

#define BIT_RATE 192000
/////////////////////////////////////////////////////////////////
//                      Local Variable
/////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////
//                      Global Variable
/////////////////////////////////////////////////////////////////

MpegAudioContext gMpegAudioContext;

char gFrame[MPA_MAX_CODED_FRAME_SIZE];
char streamBuf[READBUF_SIZE]__attribute__ ((aligned(16))) ; // wave input buffer
char gOutBuf[OUTBUF_SIZE]__attribute__ ((aligned(16))); // encode mp2 buffer 
unsigned char paramBuf[MPA_FRAME_SIZE*2*2+2];  // temp buffer for wav data
int gnFrames;

char gOutFrameBuf[OUTFRAME_SIZE]; //prepare output mp3 buffer 0~3: time stamp, 4~7: data size, 8~end: data

unsigned int gTimeStamp;  // output data ,time stamp
unsigned int gDataSize; // ouput data,encoded size
unsigned int gAppendFrame; // output data,encoded frames
// microsecond 10(-6)
unsigned int TimeStamp48000[10]={24000,48000,72000,96000,120000,144000,168000,192000,216000,240000};
unsigned int TimeStamp44100[10]={26122,52245,78637,104490,130612,156734,182857,208980,235102,261224};
unsigned int TimeStamp32000[10]={36000,72000,108000,144000,180000,216000,252000,288000,324000,360000};
/////////////////////////////////////////////////////////////////
//                      Global Function
/////////////////////////////////////////////////////////////////
#define getUsedLen(rdptr, wrptr, len) (((wrptr) >= (rdptr)) ? ((wrptr) - (rdptr)) : ((len) - ((rdptr) - (wrptr))))

static AUDIO_ENCODE_PARAM  gMp2AudioEncode;
static AVResampleContext avResampleContext;
#ifdef MP2_ENCODE_PERFORMANCE_TEST_BY_TICK
static  int  gnNewTicks,gnTotalTicks;
static  int  gnTotalTime;
#endif

#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)
void AudioPluginAPI(int nType);
__inline int ithPrintf(char* control, ...);
#endif


#ifdef ITE_RISC


#endif // #ifdef ITE_RISC

__inline short CLIPTOSHORT(int x)
{
    int sign;

    /* clip to [-32768, 32767] */
    sign = x >> 31;
    if (sign != (x >> 15))
        x = sign ^ ((1 << 15) - 1);

    return (short)x;
}


void ChangeEndian(char* ptr,int size);
static int FillReadBuffer();
static int FillReadBufferOtherRisc();


static void FillWriteBuffer(int nMp3Bytes);
int getResampleEncodeBuffer(ResampleAudioContext *audioContext,short* writeptr,int nSize);
int getResampleAvailableLength(ResampleAudioContext *audioContext);
int resample(AVResampleContext *avResampleContext,ResampleAudioContext *audioContext, short *writeptr);


unsigned int I2S_AD32_GET_RP();
unsigned int I2S_AD32_GET_WP();
void I2S_AD32_SET_RP(unsigned int data32);
void I2S_AD32_SET_WP(unsigned int data32);


unsigned int I2S_AD32_GET_RP()
{

    return get_I2S_AD_RDPTR();
}

unsigned int I2S_AD32_GET_WP()
{

    return get_I2S_AD_WRPTR();
}

void I2S_AD32_SET_RP(unsigned int data32)
{
    set_I2S_AD_RDPTR(data32);
}

void resampleInit(ResampleAudioContext *audioContext){
    int nSize;
    memset(audioContext,0,sizeof(ResampleAudioContext));
    if (gMp2AudioEncode.nSampleRate == 32000){
        nSize = gMp2AudioEncode.nChannels*2*768;//4
    }
    else if (gMp2AudioEncode.nSampleRate == 44100){
        nSize = gMp2AudioEncode.nChannels*2*1060;
        audioContext->nUseTempBuffer = 1;
    }            
    audioContext->nInSampleRate = gMp2AudioEncode.nSampleRate;
    audioContext->nOutSampleRate = gMp2AudioEncode.nOutSampleRate;
    audioContext->nInSize = nSize;
    audioContext->nInChannels = gMp2AudioEncode.nChannels;
    audioContext->nTempBufferLength = TEMP_BUFFER_SIZE;
    audioContext->nTempBufferRdPtr = 0;
    audioContext->nTempBufferWrPtr = 0;
    printf("[Mp2 Enc]resampleInit %d %d %d %d %d\n",audioContext->nInSampleRate,audioContext->nOutSampleRate,audioContext->nInSize,audioContext->nInChannels,audioContext->nUseTempBuffer);
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
        printf("getResampleAvailableLength wr %d > rd %d ptr \n",audioContext->nTempBufferWrPtr,audioContext->nTempBufferRdPtr);
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
                printf("resample use temp buffers nReSampleOutputSize %d < %d \n",nReSampleOutputSize,MAX_FRAMESIZE);
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



void Get_Encode_Parameter(MpegAudioContext *s,AUDIO_CODEC_STREAM *audioStream)
{
    unsigned short mode;

#if 1  
    mode = getEncodeParam();

    if ( (mode & 0x000f) == 0)
        s->nb_channels  = 1;
    else if ( (mode & 0x000f) == 0x1)
        s->nb_channels = 2;

    
    if ( (mode & 0x00f0) == 0x0030)
        s->sample_rate = 32000;
    else if ( (mode & 0x00f0) == 0x0020)
        s->sample_rate = 44100;    
    else if ( (mode & 0x00f0) ==0x0010)
        s->sample_rate  = 48000;


    if ( (mode & 0x0f00) == 0x0200)
        gMp2AudioEncode.nInputBufferType = 2;
    else if ( (mode & 0x0f00) == 0x0100)
        gMp2AudioEncode.nInputBufferType = 1;
    else if ( (mode & 0x0f00) == 0x0000)
        gMp2AudioEncode.nInputBufferType  = 0;
   


    s->bit_rate = BIT_RATE;
#else
    s->nb_channels = gMp2AudioEncode.nChannels;
    if (gMp2AudioEncode.nSampleRate != gMp2AudioEncode.nOutSampleRate){
        s->sample_rate = gMp2AudioEncode.nOutSampleRate;
    } else {
        s->sample_rate = gMp2AudioEncode.nSampleRate;
    }

    s->bit_rate = (int)gMp2AudioEncode.nBitrate;
#endif    
//    s->bit_rate = BIT_RATE;    
 //   printf("[Mp2 Enc] ch %d sampleRate %d bitrate %d \n",s->nb_channels,s->sample_rate,s->bit_rate);

#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)
    ithPrintf("[Mp2 Enc] ch %d sampleRate %d,input type %d (0x%x), mode 0x%x , 0x%x %d rd %d ,wr %d \n",s->nb_channels,s->sample_rate,gMp2AudioEncode.nInputBufferType,(mode & 0x0f00),mode,gOutBuf,gOutBuf[0],I2S_AD32_GET_RP(),I2S_AD32_GET_WP());
    AudioPluginAPI(SMTK_AUDIO_PLUGIN_CMD_ID_PRINTF);
#else
    printf("[Mp2] Encode start \n");
#endif

#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)
    gCh = s->nb_channels;
    gSampleRate = s->sample_rate;
    gMp2AudioEncode.nChannels = gCh;
    gMp2AudioEncode.nSampleRate = gSampleRate;
    gMp2AudioEncode.nBitrate = BIT_RATE;
    //gMp2AudioEncode.nInputBufferType = 0;
    
    gMp2AudioEncode.nInputAudioType = 0;
    gMp2AudioEncode.nSampleSize = 16;
    gMp2AudioEncode.nOutSampleRate = gSampleRate;
    gMp2AudioEncode.nEnableMixer =0;
    
    if (gMp2AudioEncode.nInputBufferType == 0){
        gMp2AudioEncode.nBufferLength = READBUF_SIZE;        
        AudioPluginAPI(SMTK_AUDIO_PLUGIN_CMD_ID_I2S_INIT_ADC);
    } else if (gMp2AudioEncode.nInputBufferType == 2){

        // get i2s buffer
        gMp2AudioEncode.pInputBuffer = audioStream->codecStreamBuf1;
        gMp2AudioEncode.nBufferLength = (unsigned int *)(audioStream->codecStreamLength1);
#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)
    ithPrintf("[Mp2 Enc] input type %d , 0x%x %d, [0x%x,0x%x,0x%x,0x%x]\n",gMp2AudioEncode.nInputBufferType,gMp2AudioEncode.pInputBuffer,gMp2AudioEncode.nBufferLength,gMp2AudioEncode.pInputBuffer[0],gMp2AudioEncode.pInputBuffer[1],gMp2AudioEncode.pInputBuffer[2],gMp2AudioEncode.pInputBuffer[3]);
    AudioPluginAPI(SMTK_AUDIO_PLUGIN_CMD_ID_PRINTF);
#else
    printf("[Mp2] Encode start \n");
#endif

    }
    
#else

#endif



}
static __inline unsigned int setStreamRdPtr(unsigned int wrPtr) 
{
    //MMIO_Write(DrvDecode_RdPtr, wrPtr);
    I2S_AD32_SET_RP(wrPtr);
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
//    wrPtr = MMIO_Read(DrvDecode_WrPtr);
    wrPtr = I2S_AD32_GET_WP();

    return wrPtr;
}

__inline unsigned int getStreamRdPtr() 
{
    unsigned int rdPtr;
//    rdPtr = MMIO_Read(DrvDecode_RdPtr);
    rdPtr = I2S_AD32_GET_RP();

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

__inline unsigned int setOutBufRdPtr(unsigned int rdPtr) 
{
    //MMIO_Write(DrvEncode_RdPtr, rdPtr);   
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


void I2S_AD32_SET_WP(unsigned int data32)
{
    //MMIO_Write(DrvEncode_WrPtr, data32);
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
        printf("[Mp2 Enc] occupyEncodeBuffer %d \n",nProtect);

    occupyAudioReadBuffer();
#endif    
}

static void releaseEncodeBuffer()
{
   // releaseAudioReadBuffer();
}

static void occupyPCMBuffer()
{
#if 0
    unsigned int nWriteProtect=0;
    unsigned int nProtect=0;

    do{
        nWriteProtect = getAudioWriteBufferStatus();
        nProtect++;
        PalSleep(1);
    }while (nWriteProtect==1);

    if (nProtect>1)
        printf("[Mp2 Enc] occupyPCMBuffer nProtect %d \n",nProtect);

    occupyAudioWriteBuffer();
#endif    
}

static void releasePCMBuffer()
{
    // releaseAudioWriteBuffer();
}

static int GetAvaliableReadBufferSize() 
{
    int len = 0;
    unsigned int wavReadIdx,wavWriteIdx;
    wavReadIdx = getStreamRdPtr();
    // Wait Read Buffer avaliable
    wavWriteIdx = getStreamWrPtr();
    len = getUsedLen(wavReadIdx, wavWriteIdx, READBUF_SIZE);

    //printf("[Mp2 Enc] wavReadIdx %d wavWriteIdx %d len %d \n",wavReadIdx,wavWriteIdx,len);
    return len;
}

static int GetAvaliableReadBufferSizeOtherRisc() 
{
    int len = 0;
    unsigned int wavReadIdx,wavWriteIdx;
    wavReadIdx = getStreamRdPtrOtherRisc();
    // Wait Read Buffer avaliable
    wavWriteIdx = getStreamWrPtrOtherRisc();
    len = getUsedLen(wavReadIdx, wavWriteIdx, gMp2AudioEncode.nBufferLength);

    //printf("[Mp2 Enc] wavReadIdx %d wavWriteIdx %d len %d \n",wavReadIdx,wavWriteIdx,len);
    return len;
}

static int GetAvaliableReadBufferSize_I2S() 
{
    int len = 0;
    unsigned int wavReadIdx,wavWriteIdx;
    wavReadIdx = I2S_AD32_GET_RP();
    // Wait Read Buffer avaliable
    wavWriteIdx = I2S_AD32_GET_WP();
    len = getUsedLen(wavReadIdx, wavWriteIdx, gMp2AudioEncode.nBufferLength);

    //printf("[Mp2 Enc] wavReadIdx %d wavWriteIdx %d len %d \n",wavReadIdx,wavWriteIdx,len);
    return len;
}

static int GetAvaliableReadBufferSize_Mixer() 
{
    int len = 0;
    unsigned int mixReadIdx,mixWriteIdx;
    mixReadIdx = getMixerReadPorinter();
    // Wait Read Buffer avaliable
    mixWriteIdx = getMixerWritePorinter();
    len = getUsedLen(mixReadIdx, mixWriteIdx, gMp2AudioEncode.nMixBufferLength);

    //printf("[Mp2 Enc] mixReadIdx %d mixWriteIdx %d len %d \n",mixReadIdx,mixWriteIdx,len);
    return len;
}



/**************************************************************************************
 * Function     : AudioPluginAPI
 *
 * Description  : AudioPluginAPI
 *
 * Input        : plugin type
 *
 * Output       : None
 *
 * Note         :
 **************************************************************************************/
#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)

static void output_and_count(struct parameters *p, int c)
{
    if (p->number_of_output_chars >= 0)
    {
        int n = c;
        gBuf[gPrint++] = c;
        if (n >= 0)
            p->number_of_output_chars++;
        else
            p->number_of_output_chars = n;
    }
}

static void output_field(struct parameters *p, char *s)
{
    short justification_length =
        p->minimum_field_width - p->leading_zeros - p->edited_string_length;
    if (p->options & MINUS_SIGN)
    {
        if (p->options & ZERO_PAD)
            output_and_count(p, '-');
        justification_length--;
    }
    if (p->options & RIGHT_JUSTIFY)
        while (--justification_length >= 0)
            output_and_count(p, p->options & ZERO_PAD ? '0' : ' ');
    if (p->options & MINUS_SIGN && !(p->options & ZERO_PAD))
        output_and_count(p, '-');
    while (--p->leading_zeros >= 0)
        output_and_count(p, '0');
    while (--p->edited_string_length >= 0){
        output_and_count(p, *s++);
    }
    while (--justification_length >= 0)
        output_and_count(p, ' ');
}

int ithGPrintf(const char *control_string, va_list va)/*const int *argument_pointer)*/
{
    struct parameters p;
    char              control_char;
    p.number_of_output_chars = 0;
    control_char             = *control_string++;
    
    while (control_char != '\0')
    {
        if (control_char == '%')
        {
            short precision     = -1;
            short long_argument = 0;
            short base          = 0;
            control_char          = *control_string++;
            p.minimum_field_width = 0;
            p.leading_zeros       = 0;
            p.options             = RIGHT_JUSTIFY;
            if (control_char == '-')
            {
                p.options    = 0;
                control_char = *control_string++;
            }
            if (control_char == '0')
            {
                p.options   |= ZERO_PAD;
                control_char = *control_string++;
            }
            if (control_char == '*')
            {
                //p.minimum_field_width = *argument_pointer++;
                control_char          = *control_string++;
            }
            else
            {
                while ('0' <= control_char && control_char <= '9')
                {
                    p.minimum_field_width =
                        p.minimum_field_width * 10 + control_char - '0';
                    control_char = *control_string++;
                }
            }
            if (control_char == '.')
            {
                control_char = *control_string++;
                if (control_char == '*')
                {
                    //precision    = *argument_pointer++;
                    control_char = *control_string++;
                }
                else
                {
                    precision = 0;
                    while ('0' <= control_char && control_char <= '9')
                    {
                        precision    = precision * 10 + control_char - '0';
                        control_char = *control_string++;
                    }
                }
            }
            if (control_char == 'l')
            {
                long_argument = 1;
                control_char  = *control_string++;
            }
            if (control_char == 'd')
                base = 10;
            else if (control_char == 'x')
                base = 16;
            else if (control_char == 'X')
            {
                base       = 16;
                p.options |= CAPITAL_HEX;
            }
            else if (control_char == 'u')
                base = 10;
            else if (control_char == 'o')
                base = 8;
            else if (control_char == 'b')
                base = 2;
            else if (control_char == 'c')
            {
                base       = -1;
                p.options &= ~ZERO_PAD;
            }
            else if (control_char == 's')
            {
                base       = -2;
                p.options &= ~ZERO_PAD;
            }
            if (base == 0) /* invalid conversion type */
            {
                if (control_char != '\0')
                {
                    output_and_count(&p, control_char);
                    control_char = *control_string++;
                }
            }
            else
            {
                if (base == -1) /* conversion type c */
                {
                    //char c = *argument_pointer++;
                    char c = (char)(va_arg(va, int));
                    p.edited_string_length = 1;
                    output_field(&p, &c);
                }
                else if (base == -2) /* conversion type s */
                {
                    char *string;
                    p.edited_string_length = 0;
                    //string                 = *(char **) argument_pointer;
                    //argument_pointer      += sizeof(char *) / sizeof(int);
                    string = va_arg(va, char*);
                    while (string[p.edited_string_length] != 0)
                        p.edited_string_length++;
                    if (precision >= 0 && p.edited_string_length > precision)
                        p.edited_string_length = precision;
                    output_field(&p, string);
                }
                else /* conversion type d, b, o or x */
                {
                    unsigned long x;
                    char          buffer[BITS_PER_BYTE * sizeof(unsigned long) + 1];
                    p.edited_string_length = 0;
                    if (long_argument)
                    {
                        //x                 = *(unsigned long *) argument_pointer;
                        //argument_pointer += sizeof(unsigned long) / sizeof(int);
                        va_arg(va, unsigned int);
                    }
                    else if (control_char == 'd')
                        //x = (long) *argument_pointer++;
                        x = va_arg(va, long);
                    else
                        //x = (unsigned) *argument_pointer++;
                        x = va_arg(va, int);
                    if (control_char == 'd' && (long) x < 0)
                    {
                        p.options |= MINUS_SIGN;
                        x          = -(long) x;
                    }
                    do
                    {
                        int c;
                        c = x % base + '0';
                        if (c > '9')
                        {
                            if (p.options & CAPITAL_HEX)
                                c += 'A' - '9' - 1;
                            else
                                c += 'a' - '9' - 1;
                        }
                        buffer[sizeof(buffer) - 1 - p.edited_string_length++] = c;
                    } while ((x /= base) != 0);
                    if (precision >= 0 && precision > p.edited_string_length)
                        p.leading_zeros = precision - p.edited_string_length;
                    output_field(&p, buffer + sizeof(buffer) - p.edited_string_length);
                }
                control_char = *control_string++;
            }
        }
        else
        {
            output_and_count(&p, control_char);
            control_char = *control_string++;
        }
    }
    return p.number_of_output_chars;
}

int ithPrintf(char* control, ...)
{
    va_list va;
    va_start(va,control);
    gPrint = 0;
    gBuf = (unsigned char*)gtAudioPluginBuffer;
    ithGPrintf(control, va);
    va_end(va);    
    return 0;
}


void AudioPluginAPI(int nType)
{   
    unsigned short nRegister;
    int i;
    int nTemp,nTemp1;
    unsigned char* pBuf;

    gBuf = (unsigned char*)gtAudioPluginBuffer;

    
    nRegister = (SMTK_AUDIO_PROCESSOR_ID<<14) | nType;
    switch (nType)
    {
        case SMTK_AUDIO_PLUGIN_CMD_ID_FILE_OPEN:
            break;

        case SMTK_AUDIO_PLUGIN_CMD_ID_FILE_WRITE:
            break;

        case SMTK_AUDIO_PLUGIN_CMD_ID_FILE_CLOSE:
           
            break;

        case SMTK_AUDIO_PLUGIN_CMD_ID_I2S_INIT_ADC:
                nTemp  = streamBuf;
                nTemp1 = sizeof(streamBuf);
//                pBuf = (unsigned char*)gtAudioPluginBuffer;
                pBuf = gBuf;
                memcpy(&pBuf[0], &nTemp, sizeof(int));
                memcpy(&pBuf[4], &gCh, sizeof(int));
                memcpy(&pBuf[8], &gSampleRate, sizeof(int));
                memcpy(&pBuf[12], &nTemp1, sizeof(int));            
            break;

        case SMTK_AUDIO_PLUGIN_CMD_ID_I2S_PAUSE_DAC:
//                pBuf = (unsigned char*)gtAudioPluginBuffer;
                pBuf = gBuf;
                memcpy(&pBuf[0], &gPause, sizeof(int));

            break;
        case SMTK_AUDIO_PLUGIN_CMD_ID_I2S_DEACTIVE_DAC:
            break;
        
        case SMTK_AUDIO_PLUGIN_CMD_ID_PRINTF:

            break;

        default:
            break;

    }
    setAudioPluginMessageStatus(nRegister);
    i=200000*20;
    do
    {
        nRegister = getAudioPluginMessageStatus();
        nRegister = (nRegister & 0xc000)>>14;
        if (nRegister== SMTK_MAIN_PROCESSOR_ID)
        {
            //printf("[Mp3] get main procerror feedback \n");
            break;
        }
        i--;
    }while(i && !isSTOP());
    

}


#endif

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
    int nTemp,bytesLeft;

    // Updates the read buffer and returns the avaliable size
    // of input buffer. Wait a minimun FRAME_SIZE length.        
    nTemp = MPA_FRAME_SIZE*2*gMpegAudioContext.nb_channels;        

    // buffer reserve from Mp2 Encoder
    if (gMp2AudioEncode.nInputBufferType==0)
    {
        do
        {
            bytesLeft = GetAvaliableReadBufferSize();
#ifdef ITE_RISC
            PalSleep(1);
#endif
        }while (bytesLeft<=nTemp && !isSTOP());
#if defined(__FREERTOS__)
            dc_invalidate(); // Flush Data Cache
#endif         

        wavReadIdx = getStreamRdPtr();


#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)
//    ithPrintf("[Mp2 Enc] FillReadBuffer nTemp %d , wavReadIdx %d \n",nTemp,wavReadIdx);
//    AudioPluginAPI(SMTK_AUDIO_PLUGIN_CMD_ID_PRINTF);
#else

#endif

        if (nTemp+wavReadIdx<=READBUF_SIZE) {
            memcpy(&paramBuf[0],&streamBuf[wavReadIdx],nTemp);
        } else {
            printf("[Mp2 Enc] memcpy \n");
            memcpy(&paramBuf[0],&streamBuf[wavReadIdx],READBUF_SIZE-wavReadIdx);
            memcpy(&paramBuf[READBUF_SIZE-wavReadIdx],&streamBuf[0],nTemp-(READBUF_SIZE-wavReadIdx));
        }
    //printf("[Mp2 Enc]  wavReadIdx  %d %d\n",wavReadIdx,nRead++);

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
    }

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
    nTemp = audioContext->nInSize*(gMp2AudioEncode.nSampleSize/16);

    // check resample temp buffers
    if (audioContext->nUseTempBuffer == 1){
        nTmp = getResampleAvailableLength(audioContext);
        //printf("getResampleAvailableLength %d  %d  \n",nTemp,2*MAX_FRAMESIZE);
        if (nTmp>=audioContext->nInChannels*MAX_FRAMESIZE){
            //printf("[Mp2 Enc]getResampleAvailableLength %d > %d  \n",nTmp,audioContext->nInChannels*MAX_FRAMESIZE);
            getResampleEncodeBuffer(audioContext,(short*)paramBuf,audioContext->nInChannels*MAX_FRAMESIZE);
            return 0;
        }
    }

    // buffer reserve from Mp2 Encoder
    if (gMp2AudioEncode.nInputBufferType==0)
    {
        do
        {
            bytesLeft = GetAvaliableReadBufferSize();
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
        nTmp = resample(&avResampleContext,audioContext,(short*)paramBuf);

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
    int nTemp,bytesLeft;
    int i,j;
    short* buf;// = (char *)&pcmWriteBuf[pcmWriteIdx];
    int* in;//  = (char *)pcmbuf;

    // Updates the read buffer and returns the avaliable size
    // of input buffer. Wait a minimun FRAME_SIZE length.        
    nTemp = MPA_FRAME_SIZE*2*gMpegAudioContext.nb_channels*(gMp2AudioEncode.nSampleSize/16);

    // buffer reserve from Other Risc AP
    if (gMp2AudioEncode.nInputBufferType==1)
    {
        do
        {
            bytesLeft = GetAvaliableReadBufferSizeOtherRisc();
#ifdef ITE_RISC
            PalSleep(1);
#endif
        }while (bytesLeft<=nTemp && !isSTOP());
#if defined(__FREERTOS__)
        dc_invalidate(); // Flush Data Cache
#endif
        // read data into encode buffer
        wavReadIdx = getStreamRdPtrOtherRisc();
        if (gMp2AudioEncode.nSampleSize==16){
            if (nTemp+wavReadIdx<=gMp2AudioEncode.nBufferLength) {
                memcpy(&paramBuf[0],&gMp2AudioEncode.pInputBuffer[wavReadIdx],nTemp);
            } else {
                printf("[Mp2 Enc] 16 memcpy \n");
                memcpy(&paramBuf[0],&gMp2AudioEncode.pInputBuffer[wavReadIdx],gMp2AudioEncode.nBufferLength-wavReadIdx);
                memcpy(&paramBuf[gMp2AudioEncode.nBufferLength-wavReadIdx],&gMp2AudioEncode.pInputBuffer[0],nTemp-(gMp2AudioEncode.nBufferLength-wavReadIdx));
            }
        } else if (gMp2AudioEncode.nSampleSize==32){

            if (nTemp+wavReadIdx<=gMp2AudioEncode.nBufferLength) {
                buf = (short*)paramBuf;
                in = (int*) &gMp2AudioEncode.pInputBuffer[wavReadIdx];
                for (i= 0;i<MPA_FRAME_SIZE*gMpegAudioContext.nb_channels;i++){
                    buf[i] = (short)in[i];
                }
            } else {
                printf("[Mp2 Enc] 32 memcpy\n");
                buf = (short*)paramBuf;
                in = (int*) &gMp2AudioEncode.pInputBuffer[wavReadIdx];

                for (i= 0;i<(gMp2AudioEncode.nBufferLength-wavReadIdx)/4;i++){
                    buf[i] = (short)in[i];
                }

                in = (int*) &gMp2AudioEncode.pInputBuffer[0];
                for (j= 0;i<MPA_FRAME_SIZE*gMpegAudioContext.nb_channels;i++,j++){
                    buf[i] = (short)in[j];
                }

            }

        }
        //printf("[Mp2 Enc]  wavReadIdx  %d %d\n",wavReadIdx,nRead++);

        occupyPCMBuffer();        
        //wavReadIdx = getStreamRdPtr();
        // Update Read Buffer
        if (nTemp > 0) {
            wavReadIdx = wavReadIdx + nTemp;
            if (wavReadIdx >= gMp2AudioEncode.nBufferLength) {
                wavReadIdx -= gMp2AudioEncode.nBufferLength;
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
    int nTemp,bytesLeft;
    int i,j;
    short* buf;// = (char *)&pcmWriteBuf[pcmWriteIdx];
    short* pMixbuf;
    int* in;//  = (char *)pcmbuf;

    // Updates the read buffer and returns the avaliable size
    // of input buffer. Wait a minimun FRAME_SIZE length.        
    nTemp = MPA_FRAME_SIZE*2*gMpegAudioContext.nb_channels*(gMp2AudioEncode.nSampleSize/16);
    
    // buffer reserve from I2S
    if (gMp2AudioEncode.nInputBufferType==2 && gMp2AudioEncode.nEnableMixer ==0)
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
        if (gMp2AudioEncode.nSampleSize==16){
            if (nTemp+wavReadIdx<=gMp2AudioEncode.nBufferLength) {
                memcpy(&paramBuf[0],&gMp2AudioEncode.pInputBuffer[wavReadIdx],nTemp);


/*
    // Byte swap to little endian
    {

        {
            int i;
            char *buf2 = (char *)paramBuf;
            char cTemp;
            for(i=0; i<nTemp; i+=sizeof(short))
            {
                cTemp = buf2[i];
                buf2[i]   = buf2[i+1];
                buf2[i+1] = cTemp;
            }
        }
    }
*/


{
    int i;
    char *buf = (char *)paramBuf;
    char in , in2;
    for(i=0; i<nTemp; i+=4)
    {
        in = buf[i];
        in2 = buf[i+1];
        buf[i]   = buf[i+3];
        buf[i+1] = buf[i+2];
        buf[i+2] =in2;
        buf[i+3] = in;
    }

}


#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)
    //ithPrintf("[Mp2 Enc] FillReadBuffer_I2S nTemp %d , wavReadIdx %d ,(0x%x,0x%x,0x%x,0x%x)\n",nTemp,wavReadIdx,paramBuf[0],paramBuf[1],paramBuf[2],paramBuf[3]);
    //AudioPluginAPI(SMTK_AUDIO_PLUGIN_CMD_ID_PRINTF);
#else

#endif

                
            } else {
                //printf("[Mp2 Enc] 16 memcpy \n");
                memcpy(&paramBuf[0],&gMp2AudioEncode.pInputBuffer[wavReadIdx],gMp2AudioEncode.nBufferLength-wavReadIdx);
                memcpy(&paramBuf[gMp2AudioEncode.nBufferLength-wavReadIdx],&gMp2AudioEncode.pInputBuffer[0],nTemp-(gMp2AudioEncode.nBufferLength-wavReadIdx));
{
    int i;
    char *buf = (char *)paramBuf;
    char in , in2;
    for(i=0; i<nTemp; i+=4)
    {
        in = buf[i];
        in2 = buf[i+1];
        buf[i]   = buf[i+3];
        buf[i+1] = buf[i+2];
        buf[i+2] =in2;
        buf[i+3] = in;
    }

}
                
            }
        } else if (gMp2AudioEncode.nSampleSize==32){

            if (nTemp+wavReadIdx<=gMp2AudioEncode.nBufferLength) {
                buf = (short*)paramBuf;
                in = (int*) &gMp2AudioEncode.pInputBuffer[wavReadIdx];
                for (i= 0;i<MPA_FRAME_SIZE*gMpegAudioContext.nb_channels;i++){
                    buf[i] = (short)in[i];
                }
            } else {
               // printf("[Mp2 Enc] 32 memcpy\n");
                buf = (short*)paramBuf;
                in = (int*) &gMp2AudioEncode.pInputBuffer[wavReadIdx];

                for (i= 0;i<(gMp2AudioEncode.nBufferLength-wavReadIdx)/4;i++){
                    buf[i] = (short)in[i];
                }

                in = (int*) &gMp2AudioEncode.pInputBuffer[0];
                for (j= 0;i<MPA_FRAME_SIZE*gMpegAudioContext.nb_channels;i++,j++){
                    buf[i] = (short)in[j];
                }

            }

        }
        //printf("[Mp2 Enc]  wavReadIdx  %d %d\n",wavReadIdx,nRead++);

        occupyPCMBuffer();        
        //wavReadIdx = getStreamRdPtr();
        // Update Read Buffer
        if (nTemp > 0) {
            wavReadIdx = wavReadIdx + nTemp;
            if (wavReadIdx >= gMp2AudioEncode.nBufferLength) {
                wavReadIdx -= gMp2AudioEncode.nBufferLength;
            }
            //I2S_AD32_SET_RP(wavReadIdx);
            setStreamRdPtr(wavReadIdx);
        }
#if defined(__FREERTOS__)
        dc_invalidate(); // Flush Data Cache
#endif

        releasePCMBuffer();
    } else if (gMp2AudioEncode.nInputBufferType==2 && gMp2AudioEncode.nEnableMixer ==1) {
        // wait buffer available
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
        if (gMp2AudioEncode.nSampleSize==16){
            if (nTemp+wavReadIdx<=gMp2AudioEncode.nBufferLength) {
                memcpy(&paramBuf[0],&gMp2AudioEncode.pInputBuffer[wavReadIdx],nTemp);
            } else {
                //printf("[Mp2 Enc] 16 memcpy \n");
                memcpy(&paramBuf[0],&gMp2AudioEncode.pInputBuffer[wavReadIdx],gMp2AudioEncode.nBufferLength-wavReadIdx);
                memcpy(&paramBuf[gMp2AudioEncode.nBufferLength-wavReadIdx],&gMp2AudioEncode.pInputBuffer[0],nTemp-(gMp2AudioEncode.nBufferLength-wavReadIdx));
            }
        } else if (gMp2AudioEncode.nSampleSize==32){
            if (nTemp+wavReadIdx<=gMp2AudioEncode.nBufferLength) {
                buf = (short*)paramBuf;
                in = (int*) &gMp2AudioEncode.pInputBuffer[wavReadIdx];
                for (i= 0;i<MPA_FRAME_SIZE*gMpegAudioContext.nb_channels;i++){
                    buf[i] = (short)in[i];
                }
            } else {
               // printf("[Mp2 Enc] 32 memcpy\n");
                buf = (short*)paramBuf;
                in = (int*) &gMp2AudioEncode.pInputBuffer[wavReadIdx];
                for (i= 0;i<(gMp2AudioEncode.nBufferLength-wavReadIdx)/4;i++){
                    buf[i] = (short)in[i];
                }
                in = (int*) &gMp2AudioEncode.pInputBuffer[0];
                for (j= 0;i<MPA_FRAME_SIZE*gMpegAudioContext.nb_channels;i++,j++){
                    buf[i] = (short)in[j];
                }
            }
        }    

        // mix data
        mixReadIdx = getMixerReadPorinter();
        if (gMp2AudioEncode.nSampleSize==16){
            if (nTemp+mixReadIdx<=gMp2AudioEncode.nMixBufferLength) {
                //memcpy(&paramBuf[0],&gMp2AudioEncode.pInputBuffer[wavReadIdx],nTemp);
                buf = (short*)paramBuf;
                pMixbuf = (short*) &gMp2AudioEncode.pMixBuffer[mixReadIdx];
                for (i= 0;i<MPA_FRAME_SIZE *gMpegAudioContext.nb_channels;i++){
                    buf[i] = CLIPTOSHORT(buf[i] + pMixbuf[i]);
                }

            } else {
                buf = (short*)paramBuf;
                pMixbuf = (short*) &gMp2AudioEncode.pMixBuffer[mixReadIdx];
                for (i= 0;i<(gMp2AudioEncode.nMixBufferLength-mixReadIdx)/2;i++){
                    buf[i] = CLIPTOSHORT(buf[i] + pMixbuf[i]);
                }
                pMixbuf= (short*) &gMp2AudioEncode.pMixBuffer[0];
                for (j= 0;j<MPA_FRAME_SIZE*gMpegAudioContext.nb_channels-((gMp2AudioEncode.nMixBufferLength-mixReadIdx)/2);i++,j++){
                    buf[i] = CLIPTOSHORT(buf[i] + pMixbuf[j]);                    
                }

            }
        }

        
        // Update Read Buffer
        if (nTemp > 0) {
            wavReadIdx = wavReadIdx + nTemp;
            if (wavReadIdx >= gMp2AudioEncode.nBufferLength) {
                wavReadIdx -= gMp2AudioEncode.nBufferLength;
            }
            I2S_AD32_SET_RP(wavReadIdx);

            mixReadIdx = mixReadIdx + nTemp;
            if (mixReadIdx >= gMp2AudioEncode.nMixBufferLength) {
                mixReadIdx -= gMp2AudioEncode.nMixBufferLength;
            }
            //printf("[Mp2 Enc] setMixerReadPorinter %d \n",mixReadIdx);
            setMixerReadPorinter(mixReadIdx);
        }
#if defined(__FREERTOS__)
        dc_invalidate(); // Flush Data Cache
#endif

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
    //nTemp = MPA_FRAME_SIZE*2*gMpegAudioContext.nb_channels*(gMp2AudioEncode.nSampleSize/16);
    nTemp = audioContext->nInSize*(gMp2AudioEncode.nSampleSize/16);

    // check resample temp buffers
    if (audioContext->nUseTempBuffer == 1){
        nTmp = getResampleAvailableLength(audioContext);
        //printf("getResampleAvailableLength %d  %d  \n",nTemp,2*MAX_FRAMESIZE);
        if (nTmp>=audioContext->nInChannels*MAX_FRAMESIZE){
            printf("getResampleAvailableLength %d > %d  \n",nTmp,audioContext->nInChannels*MAX_FRAMESIZE);
            getResampleEncodeBuffer(audioContext,(short*)paramBuf,audioContext->nInChannels*MAX_FRAMESIZE);
            return 0;
        }
    }

    // buffer reserve from I2S
    if (gMp2AudioEncode.nInputBufferType==2)
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
        if (gMp2AudioEncode.nSampleSize==16){
            if (nTemp+wavReadIdx<=gMp2AudioEncode.nBufferLength) {
                //memcpy(&paramBuf[0],&gMp2AudioEncode.pInputBuffer[wavReadIdx],nTemp);
                // read data
                readBuffer = (short*)&gMp2AudioEncode.pInputBuffer[wavReadIdx];
                for(i = 0,j=0; i < nTemp/2; i+=2,j++){
                   audioContext->reSamplePcmInput[0][audioContext->nKeep[0]+j] = readBuffer[i];
                   audioContext->reSamplePcmInput[1][audioContext->nKeep[1]+j] = readBuffer[i+1];
                }                
            } else {
                //printf("[Mp2 Enc] 16 memcpy \n");
                //memcpy(&paramBuf[0],&gMp2AudioEncode.pInputBuffer[wavReadIdx],gMp2AudioEncode.nBufferLength-wavReadIdx);
                //memcpy(&paramBuf[gMp2AudioEncode.nBufferLength-wavReadIdx],&gMp2AudioEncode.pInputBuffer[0],nTemp-(gMp2AudioEncode.nBufferLength-wavReadIdx));
                // read data
                readBuffer = (short*)&gMp2AudioEncode.pInputBuffer[wavReadIdx];
                for(i = 0,j=0; i < (gMp2AudioEncode.nBufferLength-wavReadIdx)/2; i+=2,j++){
                   audioContext->reSamplePcmInput[0][audioContext->nKeep[0]+j] = readBuffer[i];
                   audioContext->reSamplePcmInput[1][audioContext->nKeep[1]+j] = readBuffer[i+1];
                }                
                // read data
                readBuffer = (short*)&gMp2AudioEncode.pInputBuffer[0];
                for(i = 0 ; i < (nTemp-(gMp2AudioEncode.nBufferLength-wavReadIdx))/2; i+=2,j++){
                   audioContext->reSamplePcmInput[0][audioContext->nKeep[0]+j] = readBuffer[i];
                   audioContext->reSamplePcmInput[1][audioContext->nKeep[1]+j] = readBuffer[i+1];
                }                
                
            }

    #if 0//def MP2_ENCODE_PERFORMANCE_TEST_BY_TICK
            start_timer();
    #endif
           
            // resample
            nTmp = resample(&avResampleContext,audioContext,(short*)paramBuf);

    #if 0 //def MP2_ENCODE_PERFORMANCE_TEST_BY_TICK
            gnNewTicks = get_timer();            
            gnTotalTicks += gnNewTicks;
            if (gnFrames % 50 == 0 && gnFrames>0) {
                gnTotalTime += (gnTotalTicks/(PalGetSysClock()/1000));
                printf("[MP2 Enc] resample (%d~%d) total %d (ms) average %d (ms) nFrames %d\n",(gnFrames+1-50),(gnFrames+1),(gnTotalTicks/(PalGetSysClock()/1000)),((gnTotalTicks/(PalGetSysClock()/1000))/50),gnFrames+1);
                gnTotalTicks=0;
            }
    #endif
            
        } else if (gMp2AudioEncode.nSampleSize==32){
            printf("[Mp2 Enc] resample not support 32 bits sample size\n");
#if 0
            if (nTemp+wavReadIdx<=gMp2AudioEncode.nBufferLength) {
                buf = (short*)paramBuf;
                in = (int*) &gMp2AudioEncode.pInputBuffer[wavReadIdx];
                for (i= 0;i<MPA_FRAME_SIZE*gMpegAudioContext.nb_channels;i++){
                    buf[i] = (short)in[i];
                }
            } else {
               // printf("[Mp2 Enc] 32 memcpy\n");
                buf = (short*)paramBuf;
                in = (int*) &gMp2AudioEncode.pInputBuffer[wavReadIdx];

                for (i= 0;i<(gMp2AudioEncode.nBufferLength-wavReadIdx)/4;i++){
                    buf[i] = (short)in[i];
                }

                in = (int*) &gMp2AudioEncode.pInputBuffer[0];
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
            if (wavReadIdx >= gMp2AudioEncode.nBufferLength) {
                wavReadIdx -= gMp2AudioEncode.nBufferLength;
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
    //nTemp = MPA_FRAME_SIZE*2*gMpegAudioContext.nb_channels*(gMp2AudioEncode.nSampleSize/16);
    nTemp = audioContext->nInSize*(gMp2AudioEncode.nSampleSize/16);

    // check resample temp buffers
    if (audioContext->nUseTempBuffer == 1){
        nTmp = getResampleAvailableLength(audioContext);
        //printf("getResampleAvailableLength %d  %d  \n",nTemp,2*MAX_FRAMESIZE);
        if (nTmp>=audioContext->nInChannels*MAX_FRAMESIZE){
            printf("getResampleAvailableLength %d > %d  \n",nTmp,audioContext->nInChannels*MAX_FRAMESIZE);
            getResampleEncodeBuffer(audioContext,(short*)paramBuf,audioContext->nInChannels*MAX_FRAMESIZE);
            return 0;
        }
    }

    // buffer reserve from I2S
    if (gMp2AudioEncode.nInputBufferType==2)
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
        if (gMp2AudioEncode.nSampleSize==16){
            if (nTemp+wavReadIdx<=gMp2AudioEncode.nBufferLength) {
                //memcpy(&paramBuf[0],&gMp2AudioEncode.pInputBuffer[wavReadIdx],nTemp);
                // read data
                readBuffer = (short*)&gMp2AudioEncode.pInputBuffer[wavReadIdx];
                for(i = 0,j=0; i < nTemp/2; i+=2,j++){
                   audioContext->reSamplePcmInput[0][audioContext->nKeep[0]+j] = readBuffer[i];
                   audioContext->reSamplePcmInput[1][audioContext->nKeep[1]+j] = readBuffer[i+1];
                }                
            } else {
                //printf("[Mp2 Enc] 16 memcpy \n");
                //memcpy(&paramBuf[0],&gMp2AudioEncode.pInputBuffer[wavReadIdx],gMp2AudioEncode.nBufferLength-wavReadIdx);
                //memcpy(&paramBuf[gMp2AudioEncode.nBufferLength-wavReadIdx],&gMp2AudioEncode.pInputBuffer[0],nTemp-(gMp2AudioEncode.nBufferLength-wavReadIdx));
                // read data
                readBuffer = (short*)&gMp2AudioEncode.pInputBuffer[wavReadIdx];
                for(i = 0,j=0; i < (gMp2AudioEncode.nBufferLength-wavReadIdx)/2; i+=2,j++){
                   audioContext->reSamplePcmInput[0][audioContext->nKeep[0]+j] = readBuffer[i];
                   audioContext->reSamplePcmInput[1][audioContext->nKeep[1]+j] = readBuffer[i+1];
                }                
                // read data
                readBuffer = (short*)&gMp2AudioEncode.pInputBuffer[0];
                for(i = 0 ; i < (nTemp-(gMp2AudioEncode.nBufferLength-wavReadIdx))/2; i+=2,j++){
                   audioContext->reSamplePcmInput[0][audioContext->nKeep[0]+j] = readBuffer[i];
                   audioContext->reSamplePcmInput[1][audioContext->nKeep[1]+j] = readBuffer[i+1];
                }                
                
            }

            // mix data
            mixReadIdx = getMixerReadPorinter();
            
            if (nTemp+mixReadIdx<=gMp2AudioEncode.nMixBufferLength) {
                //memcpy(&paramBuf[0],&gMp2AudioEncode.pInputBuffer[wavReadIdx],nTemp);
                pMixbuf = (short*) &gMp2AudioEncode.pMixBuffer[mixReadIdx];
                for(i = 0,j=0; i < nTemp/2; i+=2,j++){
                   audioContext->reSamplePcmInput[0][audioContext->nKeep[0]+j] = CLIPTOSHORT(audioContext->reSamplePcmInput[0][audioContext->nKeep[0]+j]+pMixbuf[i]);
                   audioContext->reSamplePcmInput[1][audioContext->nKeep[1]+j] = CLIPTOSHORT(audioContext->reSamplePcmInput[1][audioContext->nKeep[1]+j]+pMixbuf[i+1]);
                }
            } else {
                pMixbuf = (short*) &gMp2AudioEncode.pMixBuffer[mixReadIdx];                
                for(i = 0,j=0; i < (gMp2AudioEncode.nMixBufferLength-mixReadIdx)/2; i+=2,j++){
                   audioContext->reSamplePcmInput[0][audioContext->nKeep[0]+j] = CLIPTOSHORT(audioContext->reSamplePcmInput[0][audioContext->nKeep[0]+j]+pMixbuf[i]);
                   audioContext->reSamplePcmInput[1][audioContext->nKeep[1]+j] = CLIPTOSHORT(audioContext->reSamplePcmInput[1][audioContext->nKeep[1]+j]+pMixbuf[i+1]);
                }
                // read data
                pMixbuf= (short*) &gMp2AudioEncode.pMixBuffer[0];                
                for(i = 0 ; i < (nTemp-(gMp2AudioEncode.nMixBufferLength-mixReadIdx))/2; i+=2,j++){
                   audioContext->reSamplePcmInput[0][audioContext->nKeep[0]+j] = CLIPTOSHORT(audioContext->reSamplePcmInput[0][audioContext->nKeep[0]+j]+pMixbuf[i]);
                   audioContext->reSamplePcmInput[1][audioContext->nKeep[1]+j] = CLIPTOSHORT(audioContext->reSamplePcmInput[1][audioContext->nKeep[1]+j]+pMixbuf[i+1]);
                }

            }
    #if 0//def MP2_ENCODE_PERFORMANCE_TEST_BY_TICK
            start_timer();
    #endif
           
            // resample
            nTmp = resample(&avResampleContext,audioContext,(short*)paramBuf);

    #if 0 //def MP2_ENCODE_PERFORMANCE_TEST_BY_TICK
            gnNewTicks = get_timer();            
            gnTotalTicks += gnNewTicks;
            if (gnFrames % 50 == 0 && gnFrames>0) {
                gnTotalTime += (gnTotalTicks/(PalGetSysClock()/1000));
                printf("[MP2 Enc] resample (%d~%d) total %d (ms) average %d (ms) nFrames %d\n",(gnFrames+1-50),(gnFrames+1),(gnTotalTicks/(PalGetSysClock()/1000)),((gnTotalTicks/(PalGetSysClock()/1000))/50),gnFrames+1);
                gnTotalTicks=0;
            }
    #endif
            
        } else if (gMp2AudioEncode.nSampleSize==32){
            printf("[Mp2 Enc] resample not support 32 bits sample size\n");
#if 0
            if (nTemp+wavReadIdx<=gMp2AudioEncode.nBufferLength) {
                buf = (short*)paramBuf;
                in = (int*) &gMp2AudioEncode.pInputBuffer[wavReadIdx];
                for (i= 0;i<MPA_FRAME_SIZE*gMpegAudioContext.nb_channels;i++){
                    buf[i] = (short)in[i];
                }
            } else {
               // printf("[Mp2 Enc] 32 memcpy\n");
                buf = (short*)paramBuf;
                in = (int*) &gMp2AudioEncode.pInputBuffer[wavReadIdx];

                for (i= 0;i<(gMp2AudioEncode.nBufferLength-wavReadIdx)/4;i++){
                    buf[i] = (short)in[i];
                }

                in = (int*) &gMp2AudioEncode.pInputBuffer[0];
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
            if (wavReadIdx >= gMp2AudioEncode.nBufferLength) {
                wavReadIdx -= gMp2AudioEncode.nBufferLength;
            }
            I2S_AD32_SET_RP(wavReadIdx);

            mixReadIdx = mixReadIdx + nTemp;
            if (mixReadIdx >= gMp2AudioEncode.nMixBufferLength) {
                mixReadIdx -= gMp2AudioEncode.nMixBufferLength;
            }
            //printf("[Mp2 Enc] setMixerReadPorinter %d \n",mixReadIdx);
            setMixerReadPorinter(mixReadIdx);

        }
#if defined(__FREERTOS__)
        dc_invalidate(); // Flush Data Cache
#endif

        releasePCMBuffer();
    }
    
    return len;
}


// return 1:write output buffer,return 0: not write output buffer
static int OutputWriteBuffer(int nMp3Bytes)
{
    static int adjustCount = 0;
#ifdef OUTPUT_ONE_FRAME
    // get time stamp;
    if (gMpegAudioContext.sample_rate==48000)
    {
        gTimeStamp = TimeStamp48000[0];
    }
    else if (gMpegAudioContext.sample_rate==44100)
    {
        if (++adjustCount == 1225)
        {
            gTimeStamp = 26672;
            adjustCount = 0;
        }
        else
        {
            gTimeStamp = TimeStamp44100[0];
        }
    }
    else if (gMpegAudioContext.sample_rate==32000)
    {
        gTimeStamp = TimeStamp32000[0];
    }

    gDataSize=nMp3Bytes;
    memcpy(&gOutFrameBuf[0],&gTimeStamp,sizeof(int));
    memcpy(&gOutFrameBuf[4],&gDataSize,sizeof(int));
    if (nMp3Bytes<=OUTFRAME_SIZE)
        memcpy(&gOutFrameBuf[8],&gFrame[0],nMp3Bytes);
    
    if (nMp3Bytes==0 || nMp3Bytes >OUTFRAME_SIZE)
        printf("[Mp2 Enc] OutputWriteBuffer %d %d %d\n",gTimeStamp,gDataSize,nMp3Bytes);        

    return 1;

#else
    if ( (gAppendFrame+1)>=10 || (gDataSize+nMp3Bytes)>=OUTFRAME_SIZE-8){
        // get time stamp;
        if (gMpegAudioContext.sample_rate==48000)
            gTimeStamp = TimeStamp48000[gAppendFrame-1];
        else if (gMpegAudioContext.sample_rate==44100)
            gTimeStamp = TimeStamp44100[gAppendFrame-1];
        else if (gMpegAudioContext.sample_rate==32000)
            gTimeStamp = TimeStamp32000[gAppendFrame-1];

        memcpy(&gOutFrameBuf[0],&gTimeStamp,sizeof(int));
        memcpy(&gOutFrameBuf[4],&gDataSize,sizeof(int));            

       // printf("[Mp2 Enc] OutputWriteBuffer %d %d %d %d\n",gTimeStamp,gDataSize,gAppendFrame,nMp3Bytes);        

        gAppendFrame=0;
        gDataSize=0;

        return 1;
    } else {
        return 0;
    }
#endif
}


static int AppendWriteBuffer(int nMp3Bytes)
{
    memcpy(&gOutFrameBuf[8+gDataSize],&gFrame[0],nMp3Bytes);
    gAppendFrame++;
    gDataSize+=nMp3Bytes;

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
static void FillWriteBuffer(int nMp3Bytes) 
{
    int len;
    int nOutReadIdx,nOutWriteIdx;

    occupyEncodeBuffer();
    
    nOutWriteIdx = getOutBufWrPtr(); 
    // Update Write Buffer
#if 1
    if (OutputWriteBuffer(nMp3Bytes) > 0) {
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

        //printf("[Mp2 Enc] set outBuffer %d \n",nOutWriteIdx);
        setOutBufWrPtr(nOutWriteIdx);
    }
    #ifndef OUTPUT_ONE_FRAME    
    AppendWriteBuffer(nMp3Bytes);
    #endif

#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)
    //ithPrintf("[Mp2 Enc] nFrames %d , frameSize %d 0x%x 0x%x 0x%x 0x%x \n",(gnFrames+1),nMp3Bytes,gOutFrameBuf[0],gOutFrameBuf[1],gOutFrameBuf[2],gOutFrameBuf[3]);
    //AudioPluginAPI(SMTK_AUDIO_PLUGIN_CMD_ID_PRINTF);    
#else

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

    if (isSTOP()) {
        MMIO_Write(DrvAudioCtrl, MMIO_Read(DrvAudioCtrl) & ~DrvDecode_STOP);
    }
    
    #if defined(__OR32__)
        dc_invalidate(); // Flush DC Cache
    #endif
    releasePCMBuffer();
    releaseEncodeBuffer();
}



void MP2Encode_GetBufInfo(unsigned* inBuf, unsigned* inLen,unsigned* outEnBuf, unsigned* outEnLen,unsigned* pAudio)
{
#if defined(__FREERTOS__)
    dc_invalidate(); // Flush Data Cache
#endif         

    *inBuf = (unsigned)streamBuf;
    *inLen = sizeof(streamBuf);
    *outEnBuf =  (unsigned)gOutBuf;
    *outEnLen =  sizeof(gOutBuf);

    *pAudio = (unsigned)&gMp2AudioEncode;
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


# if defined(__FREERTOS__) && !defined(ENABLE_CODECS_PLUGIN)
portTASK_FUNCTION(mp2encode_task, params)
# else
int main(int argc, char **argv)
# endif
{
    int size;
    int bytesLeft;
    unsigned int wavReadIdx;
    int i;

    int                *codecStream;
    AUDIO_CODEC_STREAM *audioStream;
    
#ifdef MP2_ENCODE_PERFORMANCE_TEST_BY_TICK
    int  nNewTicks,nTotalTicks;
    int  nTotalTime;
#endif
    // resample 
    ResampleAudioContext resampleAudioContext;
    int nNeedResample = 0;
    int nTemp;
    int nSize;
#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)
    codecStream                    = (int *)CODEC_STREAM_START_ADDRESS;
    //printf("[Mp3] 0x%08x \n",*codecStream);
    audioStream                    = (AUDIO_CODEC_STREAM *)*codecStream;
    audioStream->codecStreamBuf    = &gOutBuf[0];
    audioStream->codecStreamLength =OUTBUF_SIZE; // streamLen;


    gtAudioPluginBuffer            = audioStream->codecAudioPluginBuf;
    gtAudioPluginBufferLength      = (unsigned int *)(audioStream->codecAudioPluginLength);


    //setAudioMessageBuffer((int)audioStream->codecAudioPluginBuf);
    MMIO_Write(AUDIO_DECODER_START_FALG, 1);
#endif

    Get_Encode_Parameter(&gMpegAudioContext,audioStream);
    
    while (1)
    {
        gTimeStamp = 0;
        gDataSize = 0;
        gAppendFrame = 0;

        memset(gOutFrameBuf,0,sizeof(gOutFrameBuf));
/*
        do {
            PalSleep(1);
        } while (MMIO_Read(AUDIO_DECODER_START_FALG) != 2);
*/
        if (&gMp2AudioEncode){
            printf("[Mp2 Enc] start , input buffer type %d nSampleSize %d start pointer 0x%x \n",gMp2AudioEncode.nInputBufferType,gMp2AudioEncode.nSampleSize,I2S_AD32_GET_RP());
        } else {
            printf("[Mp2 Enc] start , input buffer null pointer \n");
        }
    
        if (gMp2AudioEncode.nInputBufferType==0 || gMp2AudioEncode.nInputBufferType==1){
            setStreamRdPtr(0);
            setStreamWrPtr(0);
        }
        setOutBufRdPtr(0);
        setOutBufWrPtr(0);
        
        //Get_Encode_Parameter(&gMpegAudioContext);
#ifdef MP2_ENABLE_INTERNAL_SD
        gMpegAudioContext.samples_buf[0] = (short*)INTERNAL_SD;
        gMpegAudioContext.samples_buf[1] = (short*)&gMpegAudioContext.samples_buf_temp[1];
#else
        gMpegAudioContext.samples_buf[0] = (short*)&gMpegAudioContext.samples_buf_temp[0];
        gMpegAudioContext.samples_buf[1] = (short*)&gMpegAudioContext.samples_buf_temp[1];        
#endif
        MPA_encode_init(&gMpegAudioContext);
        //printf("[Mp2 Enc] 0x%x 0x%x \n",gMpegAudioContext.samples_buf[0],gMpegAudioContext.samples_buf[1]);
        
#ifdef MP2_ENCODE_PERFORMANCE_TEST_BY_TICK
         nNewTicks = 0;
         nTotalTicks=0;
         nTotalTime=0;

         gnNewTicks = 0;
         gnTotalTicks=0;
         gnTotalTime=0;         
#endif        

        gnFrames = 0;
// resample setting
        if (gMp2AudioEncode.nSampleRate != gMp2AudioEncode.nOutSampleRate) {            
            printf("[Mp2 Enc] resampleInit \n");
            resampleInit(&resampleAudioContext);
            av_resample_init(&avResampleContext,gMp2AudioEncode.nOutSampleRate, gMp2AudioEncode.nSampleRate, RESAMPLE_FILTER_LENGTH, 10,0, 0.8);
            printf("[Mp2 Enc] av_resample_init \n");            
            nNeedResample = 1;
        } else {
            nNeedResample = 0;
        }
// 
        if (gMp2AudioEncode.nEnableMixer==1){
            printf("[Mp2 Enc] enable mixer ,Mixer buffer length %d \n",gMp2AudioEncode.nMixBufferLength);
        }
        
        for(;;) // forever loop
        {
            int exitflag1 = 0;
            int i,j;
            int frameSize;
            // read input data
            if (gMp2AudioEncode.nInputBufferType==0){
                if (nNeedResample==1) {
                    FillReadBufferResample(&resampleAudioContext);                    
                } else {
                    FillReadBuffer();
                }
            } else if (gMp2AudioEncode.nInputBufferType==1){
                FillReadBufferOtherRisc();
            } else if (gMp2AudioEncode.nInputBufferType==2){
                if (nNeedResample==1) {
                    if (gMp2AudioEncode.nEnableMixer==1){
                        // resample and mixer
                        FillReadBuffer_I2S_Resample_Mixer(&resampleAudioContext);
                    } else {
                        FillReadBuffer_I2S_Resample(&resampleAudioContext);
                    }
                } else {
                    FillReadBuffer_I2S();
                }
            }

#ifdef ITE_RISC
        if (gMp2AudioEncode.nInputBufferType!=2)
            if (nNeedResample==0)
                ChangeEndian4(&paramBuf[0],MPA_FRAME_SIZE*2*gMpegAudioContext.nb_channels);
    #ifdef MP2_ENCODE_PERFORMANCE_TEST_BY_TICK
            start_timer();
    #endif
#endif
            // mp2 encode
            frameSize = MPA_encode_frame(&gMpegAudioContext,&gFrame[0],0,&paramBuf[0]);

#ifdef ITE_RISC
    #ifdef MP2_ENCODE_PERFORMANCE_TEST_BY_TICK
            nNewTicks = get_timer();            
            nTotalTicks += nNewTicks;
            if (gnFrames % 50 == 0 && gnFrames>0) {
                nTotalTime += (nTotalTicks/(PalGetSysClock()/1000));
                //printf("[MP2 Enc] (%d~%d) total %d (ms) average %d (ms) nFrames %d system clock %d\n",(gnFrames+1-50),(gnFrames+1),(nTotalTicks/(PalGetSysClock()/1000)),((nTotalTicks/(PalGetSysClock()/1000))/50),gnFrames+1,PalGetSysClock());
                nTotalTicks=0;
            }
    #endif
            ChangeEndian4(&gFrame[0],frameSize);
#endif
            gnFrames++;
            
#if defined(WIN32) || defined(__CYGWIN__)
            fwrite(&gFrame[0], 1, frameSize, fmp3);
#endif

            FillWriteBuffer(frameSize);
           
            if (isSTOP()) {
                break;
            }

        }

#if defined(WIN32)    
        fclose(fmp3);
#endif

       printf("[Mp2 Enc] ClearRdBuffer gnFrames%d\n",gnFrames);

        ClearRdBuffer();
        gnFrames = 0;
        MMIO_Write(AUDIO_DECODER_START_FALG,0);
    }


}




