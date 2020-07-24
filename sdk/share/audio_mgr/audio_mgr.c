#include "audio_mgr.h"
#include "ite/audio.h"
#include <malloc.h>
#include <string.h>

#define MULTISECTION_SIZE           SMTK_AUDIO_SPECIAL_CASE_BUFFER_SIZE // (64*1024)

#define HAVE_WMA                    0
#define HAVE_WAV                    1

#define AUDIO_MGR_POLL_STACK_SIZE           40000   //(255 * 1024)

#define AUDIOMGR_NETWORKPLAY_EOF_SLEEP_US   1000000 // usec
#define AUDIOMGR_NETWORKPLAY_EOF_COUNT_MAX  600
#define I2S_BUFFER_LENGTH   3 * 1024 * 1024

//#define DEBUG_PRINT printf
#define DEBUG_PRINT(...)

SMTK_AUDIO_MGR                          audioMgr;
SMTK_AUDIO_PARAM_NETWORK                gpNetwork;
static PAL_FILE                         *currFile;
static MMP_UINT32                       audioTime;
static MMP_ULONG                        audioRemainSize;
//static SMTK_AUDIO_FILE_INFO             gtFileInfo;

static pthread_t                        gAudioMgrthreadID;
static pthread_t                        gAudioReadthreadID;

#ifdef HAVE_WAV
static unsigned char                    ptWaveHeader[48];
static MMP_UINT                         gnWavByte;
#endif
#if 1 //def MUSIC_PLAYER_SUPPORT_WAV
static SMTK_AUDIO_RECORD_WAVE_WAVEINFO  gtWaveInfo;
#endif

static struct timeval                   gStartRemoveCardT, gEndRemoveCardT;
static struct timeval                   gStartDebugT, gEndDebugT;

static int                              gnAudioWrite    = 0;
static sem_t                            gAudioMgrSemaphore;
static sem_t                            *gpAudioMgrSemaphore = &gAudioMgrSemaphore;

static char                 *gOutBuffer             = NULL; //[I2S_BUFFER_LENGTH]__attribute__ ((aligned(16)));
static unsigned int         gOutReadPointer         = 0;
static unsigned int         gOutWritePointer        = 0;
static char                 ptBuffer[6];
static int                  gBufferIndex            = 0;

// 1: open, 2:get data 3:put data
static int                  gnAudioMgrStop          = 0;
static int                  gnAudioMgrPlayNetwork   = 0;

// instering audio when playing vedio ,video audio codec need save.
static int nVideoAudioCodec = 0;
static FILE *AL_Local_player_http_handle = NULL;

#ifdef AUDIO_MGR_DUMP
static FILE *fout = NULL;
#endif

#ifdef DUMP_MUSIC
static FILE         *fI2SOut                    = NULL;
static unsigned int gnAudioMgrI2sRead           = 0;
static unsigned int gnAudioMgrI2sWrite          = 0;
static unsigned int gnAudioMgrI2sLastRead       = 0;
static unsigned int gnAudioMgrI2sLastWrite      = 0;

static int          gnAudioMgrWriteWAVHeader    = 0;
static char         *gFilepath[128];

#endif

// check interrupt sound remove card start
static int gnCheckRemoveCardStart = 0;

static int audio_write_stream(char *pBuf, unsigned long bufSize);
static int audio_get_available_buffer_size(unsigned long *bufSize);

static int audioCheckInterruptSoundRemoveCard();

#ifdef SMTK_AUDIO_READ_BUFFER_THREAD
static AUDIO_READ_BUFFER audio_buffer[SMTK_AUDIO_READ_BUFFER_FRAMES];
static void init_audio_buffer(void);
static void free_audio_buffer(void);
static void audio_buffer_reset(void);
//static void audio_buffer_read_data_from_network(int nNext);
static void audio_buffer_read_data_from_local(int nNext);
static int audio_buffer_write_complete(void);
static void audio_buffer_read_data(void);
static void *audio_read_thread_func(void *arg);
static pthread_mutex_t  audio_callback_mutex;

static pthread_mutex_t  audio_read_buffer_mutex;
static int checkNetworkRead(int nReadSize, int nResult);
static void checkMusic(int nNext);
static void checkMusic_MP3(int nNext);
static void checkMusic_WAV(int nNext);
static void checkMusic_Local(int nNext);
static void checkMusic_MP3_Local(int nNext);
static void checkMusic_WAV_Local(int nNext);

#endif

//extern  MP3_CODECS;
//extern  WAVE_CODECS;
//extern  AAC_CODECS;
//extern  WMA_CODECS;

//=============================================================================
//                              Constant Definition
//=============================================================================

//=============================================================================
//                              Private Function Declaration
//=============================================================================
#ifdef HAVE_WAV
// wav
static __inline int ENDIAN_LE32(unsigned char *n)
{
    int num = (((unsigned int)n[0]) + ((unsigned int)n[1] << 8) +
               ((unsigned int)n[2] << 16) + ((unsigned int)n[3] << 24));
    return num;
}

static __inline short ENDIAN_LE16(unsigned char *n)
{
    short num = (short)(((unsigned short)n[0]) + ((unsigned short)n[1] << 8));
    return num;
}
#endif

static MMP_UINT32 audioGetTotoalTime()
{
    // unit : milliseconds
    MMP_INT nResult = 1;
    if (audioTime > 0)
    {
        return audioTime;
    }

    return nResult;
}

static int audioSetTotoalTime(MMP_UINT32 time)
{   // uint : milliseconds
  //if (sameFile == MMP_FALSE)
    audioTime = time;

    /*LOG_DEBUG
    "Set Audio Time %d lns %d func %s \n", audioTime, __LINE__, __FUNCTION__
    LOG_END*/
    return 0;
}

// JL, 10012016
MMP_INT
Audio_Printf(
    MMP_CHAR *strPtr)
{
    MMP_CHAR    *tempPtr    = strPtr;
    MMP_INT     result      = 0;
    /*#if AUDIO_DEBUG
    while (*tempPtr)
    {
        DEBUG_PRINT("%c", *tempPtr);
        *tempPtr++;
    }

    DEBUG_PRINT("\n");
    #endif*/
    return result;
}

MMP_INT
AudioPrintf(
    MMP_WCHAR *strPtr)
{
    MMP_WCHAR   *tempPtr    = strPtr;
    MMP_INT     result      = 0;
    /*#if AUDIO_DEBUG && !defined(_WIN32)
    while (*tempPtr)
    {
        DEBUG_PRINT("%c", *tempPtr);
        *tempPtr++;
    }

    DEBUG_PRINT("\n");
    #endif*/
    return result;
}

#ifdef HAVE_WMA
static MMP_BOOL audioCheckWmaHeaderExist()
{
    return MMP_FALSE;
}

static MMP_INT audioGetWmaInfo()
{
    return SMTK_AUDIO_ERROR_NO_ERROR;
}
#endif

///////////////////////////////////////////////////////////////////////////

static MMP_BOOL audioIsId3V1Tag(MMP_LONG dwEnd, MMP_INT *tagSize)
{
    int         result;
    // stands 128 byte before file end
    MMP_ULONG   dwOffset = dwEnd - 128;
    MMP_UINT8   pbBuffer[4];
    *tagSize    = 0;

    result      = fseek(currFile, dwOffset + audioMgr.nOffset, SEEK_SET);
    //PalAssert(result == 0);
    result      = fread(pbBuffer, 1, 4, currFile);
    //PalAssert((MMP_UINT) result == 4);

    if (memcmp("TAG", pbBuffer, 3) == 0)
    {
        *tagSize = 128;
        return MMP_TRUE;
    }

    return MMP_FALSE;
}

static MMP_BOOL audioIsId3V2Tag(MMP_LONG dwEnd, MMP_INT *tagSize)
{
    int         result;
    MMP_ULONG   dwOffset = 0;
    MMP_UINT8   pbBuffer[10];
    *tagSize    = 0;

    result      = fseek(currFile, dwOffset + audioMgr.nOffset, SEEK_SET);
    //PalAssert(result == 0);

    result      = fread(pbBuffer, 1, 10, currFile);
    //PalAssert((MMP_UINT) result == 10);

    if (memcmp("ID3", pbBuffer, 3) == 0)
    {
        /* high bit is not used */
        *tagSize    = (pbBuffer[6] << 21) | (pbBuffer[7] << 14) | (pbBuffer[8] << 7) | (pbBuffer[9] << 0);
        *tagSize    += 10;
        return MMP_TRUE;
    }

    return MMP_FALSE;
}

static MMP_BOOL audioIsId3V2Tag_Buffer(unsigned char *pBuffer, MMP_INT *tagSize)
{
    int result;

    if (pBuffer == 0 || tagSize == 0)
    {
        DEBUG_PRINT("[Audio Mgr] buffer null #line %d \n", __LINE__);
    }
    *tagSize = 0;

    // result = fread(pbBuffer, 1, 10, currFile);

    if (memcmp("ID3", pBuffer, 3) == 0)
    {
        /* high bit is not used */
        *tagSize    = (pBuffer[6] << 21) | (pBuffer[7] << 14) | (pBuffer[8] << 7) | (pBuffer[9] << 0);
        *tagSize    += 10;
        return MMP_TRUE;
    }

    return MMP_FALSE;
}

static MMP_BOOL audioIsApeTag(MMP_LONG dwEnd, MMP_INT *tagSize)
{
    int         result;
    // stands 32 byte before file end
    MMP_ULONG   dwOffset = dwEnd - 32;
    MMP_UINT8   pbBuffer[8];

    *tagSize    = 0;

    result      = fseek(currFile, dwOffset + audioMgr.nOffset, SEEK_SET);
    //PalAssert(result == 0);

    result      = fread(pbBuffer, 1, 8, currFile);
    //PalAssert((MMP_UINT) result == 8);

    if (memcmp("APETAGEX", pbBuffer, 8) == 0)
    {
        *tagSize = 32;
        return MMP_TRUE;
    }

    return MMP_FALSE;
}

static void
audioSetRemainSize(MMP_ULONG remainSize)
{
    audioRemainSize = remainSize;
}

#if HAVE_WMA
static MMP_INT
audioSetWmaTime(MMP_INT nTime)
{
    return SMTK_AUDIO_SEEK_OK;
}
#endif

#ifdef HAVE_WAV
static MMP_INT
audioSetWavTime(MMP_INT nSecond)
{
    return SMTK_AUDIO_SEEK_OK;
}
#endif

#define _f_toupper(ch) (((ch) >= 'a' && (ch) <= 'z') ? ((ch) - 'a' + 'A') : (ch))

static MMP_BOOL
check_Extention(MMP_CHAR *ext, MMP_CHAR *filename)
{
    MMP_INT     wchCnt  = 0;
    MMP_CHAR    wch;
    MMP_CHAR    ch;
    MMP_INT     i       = 0;

    for (;;)
    {
        wch = (MMP_CHAR)_f_toupper(*ext);

        if ((!wch))
        {
            break;
        }
        wchCnt++;
        ext++;
    }

    for (;;)
    {
        ch = (MMP_CHAR)_f_toupper(*filename);

        if ((!ch))
        {
            break;
        }
        filename++;
    }

    for (i = 0; i < wchCnt; i++)
    {
        ext--;
        filename--;
        wch = (MMP_CHAR)_f_toupper(*ext);
        ch  = (MMP_CHAR)_f_toupper(*filename);

        if ((wch == '*') && (ch))
        {
            break;
        }

        if (wch != ch)
        {
            return MMP_FALSE;
        }
    }

    return MMP_TRUE;
}

static void *AudioMgrPollTask(void *arg)
{
    for (;;)
    {
        AudioThreadFunc();
#if defined(__OPENRTOS__)
        usleep(1000);
#endif
    }
    return NULL;
}

static void AudioMgrInit()
{
    pthread_t       task;
    pthread_attr_t  attr;
    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, AUDIO_MGR_POLL_STACK_SIZE);
    pthread_create(&task, &attr, AudioMgrPollTask, NULL);
    gAudioMgrthreadID = task;
}

#ifdef CFG_AUDIO_MGR_PARSING_MP3
    #define READ_SIZE 64 * 1024

// buffer

static int audioReadMp3Frame()
{
    int     nSize   = 0;
    int     done    = 0;
    int     nTemp   = 0;
    char    readBuffer[READ_SIZE];
    char    *filename=audioMgr.ptNetwork.pFilename;
    FILE    *finput = NULL;
    nSize = READ_SIZE;

    if ((finput = fopen(audioMgr.ptNetwork.pFilename, "rb")) == NULL)
    {
        printf("[Audio Mgr] Can not open file \n");
        return -1;
    }

    do
    {
        // read input audio
        nSize = fread(readBuffer, 1, READ_SIZE, finput);
        if (nSize != READ_SIZE)
        {
            //printf("eof \n");
            done = 1;
        }
        if(stricmp(filename,audioMgr.ptNetwork.pFilename)!=0) break;

        nTemp += parsing_data(readBuffer, nSize, 1);
        usleep(1000);
    } while (!done);
    
    if(done){
        nTemp = parsing_data_get_current_time();
        printf("[Audio Mgr] duration = %d , %d:%d \n", nTemp, (nTemp / 1000) / 60, (nTemp / 1000) % 60);
        smtkAudioMgrSetTotalTime(nTemp);
    }
    fclose(finput);
}

static void audioReadWavTime(void)
{
    FILE *finput = NULL;
    unsigned char streamBuf[512];
    unsigned char* header;
    int nWaitBytes;
    int done;
    int header_size;
    unsigned int format;
    unsigned int nChans;
    unsigned int sampRate;
    unsigned int avgBytesPerSec;
    unsigned int blockAlign;
    unsigned int bitsPerSample;
    unsigned int samplesPerBlock;
    unsigned int dataSize;
    uint32_t time;
    
    enum
    {
        INIT_STATE = 0,
        CHK_TYPE   = 1,
        FMT_TYPE   = 2,
        FACT_TYPE  = 3,
        DATA_TYPE  = 4,
        UNKNOW_TYPE= 5,
    } readState;  
    
    readState   = INIT_STATE;
    nWaitBytes  = 12;    // Wait the wave header
    done        = 0;
    header_size = 0;
      
    
    if ((finput = fopen(audioMgr.ptNetwork.pFilename, "rb")) == NULL)
    {
        printf("[Audio Mgr] Can not open file \n");
        return -1;
    }
    
    //header = malloc(512);    
    fread(streamBuf, 1, 512, finput);
    header = &streamBuf[0];
    while(!done)
    {
    
    switch(readState) {
        case INIT_STATE : if (!(header[ 0] == 'R' && header[ 1] == 'I' && header[ 2] == 'F' && header[ 3] == 'F' &&
                                header[ 8] == 'W' && header[ 9] == 'A' && header[10] == 'V' && header[11] == 'E')) {
                              printf("Not .wav file\n");         
                              return 0;
                          }
                          header      += 12;
                          header_size += 12;
                          nWaitBytes   = 8;
                          readState = CHK_TYPE;
                          break;
        case CHK_TYPE   : if (!strncmp(&header[0], "fmt ", 4)) {
                                readState  = FMT_TYPE;
                                nWaitBytes = ENDIAN_LE32(&header[4]);
                          } else if(!strncmp(&header[0], "data", 4)) {
                                readState  = DATA_TYPE;
                                nWaitBytes = 0;
                                dataSize =(unsigned int)ENDIAN_LE32(&header[4]);
                                done       = 1;
                          } else if(!strncmp(&header[0], "fact", 4)) {
                                readState  = FACT_TYPE;
                                nWaitBytes = ENDIAN_LE32(&header[4]);
                          } else {
                                readState  = UNKNOW_TYPE;
                                nWaitBytes = ENDIAN_LE32(&header[4]);
                          }
                          header += 8;
                          header_size += (8+nWaitBytes);
                          break;
        case FMT_TYPE   : readState = CHK_TYPE;
                          format         = (unsigned int)ENDIAN_LE16(&header[0]);
                          nChans         = (unsigned int)header[2];
                          sampRate       = (unsigned int)ENDIAN_LE32(&header[4]);
                          avgBytesPerSec = (unsigned int)ENDIAN_LE32(&header[8]);
                          blockAlign     = (unsigned int)ENDIAN_LE16(&header[12]);
                          bitsPerSample  = (unsigned int)header[14];
                          if (nWaitBytes > 16) { // extension header
                              samplesPerBlock = (unsigned int)ENDIAN_LE16(&header[18]);
                          } else {
                              samplesPerBlock = 0;
                          }
                          header    += nWaitBytes;
                          nWaitBytes = 8;
                          break;
        case FACT_TYPE  : readState  = CHK_TYPE;
                          header    += nWaitBytes;
                          nWaitBytes = 8;
                          break;
        case UNKNOW_TYPE: readState  = CHK_TYPE;
                          header    += nWaitBytes;
                          nWaitBytes = 8;
                          break;                          
        default         : break;
        }

    } // while(!done)
    time = dataSize/(avgBytesPerSec/1000);
    
    printf("[Audio Mgr] duration = %d(ms) , (min:sec)(%d:%d) \n", time, (time / 1000) / 60, (time / 1000) % 60);
    //printf("sampRate %d nChans %d bitsPerSample %d dataSize %d\n",sampRate,nChans,bitsPerSample,dataSize);
    smtkAudioMgrSetTotalTime(time);
    fclose(finput);
}

#endif

// parsing data for upnp
static void audio_parsing_data(int nNext)
{
    int nResult = 0;
    int nFrames = 0;
    smtkAudioMgrSetTotalTime(0);
#ifdef CFG_AUDIO_MGR_PARSING_MP3
    if (audioMgr.ptNetwork.nType == SMTK_AUDIO_MP3)
    {
        pthread_t       task;
        pthread_attr_t  attr;

        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
        pthread_attr_setstacksize(&attr, (255 * 1024));
        pthread_create(&task, &attr, audioReadMp3Frame, NULL);
    }
    else if (audioMgr.ptNetwork.nType == SMTK_AUDIO_WAV)
    {
        audioReadWavTime();
    }
    else if (audioMgr.ptNetwork.nType == SMTK_AUDIO_AAC && audioMgr.ptNetwork.nM4A == 0)
    {
        printf("not support now\n");
    }
    else
    {
        printf("error audio_parsing_data()\n");
    }

#endif
}


#ifdef DUMP_MUSIC
int32_t
audio_wave_header()
{
    uint32_t    nSamplesPerSec = 44100;
    uint16_t    wFormatTag;
    uint16_t    i;
    uint8_t     wave_header[48];
    int         nTemp               = 0;
    int         nSampleRate;
    int         nCh;
    int         nI2SBufferLength    = 0;

    iteAudioGetAttrib(  ITE_AUDIO_CODEC_SET_CHANNEL,        &nCh);
    iteAudioGetAttrib(  ITE_AUDIO_CODEC_SET_SAMPLE_RATE,    &nSampleRate);
    iteAudioGetAttrib(  ITE_AUDIO_CODEC_SET_BUFFER_LENGTH,  &nI2SBufferLength);

    //ckID
    wave_header[0]  = 'R';
    wave_header[1]  = 'I';
    wave_header[2]  = 'F';
    wave_header[3]  = 'F';
    //cksize
    wave_header[4]  = 0x24;
    wave_header[5]  = 0x60;
    wave_header[6]  = 0x01;
    wave_header[7]  = 0x00;
    //WAVEID
    wave_header[8]  = 'W';
    wave_header[9]  = 'A';
    wave_header[10] = 'V';
    wave_header[11] = 'E';
    //ckID
    wave_header[12] = 'f';
    wave_header[13] = 'm';
    wave_header[14] = 't';
    wave_header[15] = ' ';
    //cksize ADPCM is 20, others is 16
    {
        wave_header[16] = 0x10;
    }
    wave_header[17] = 0;
    wave_header[18] = 0;
    wave_header[19] = 0;

    //wFormatTag
    // set format
    wFormatTag      = 1;
    wave_header[34] = 0x10;

    wave_header[21] = (uint8_t) ((wFormatTag >> 8) & 0xff);
    wave_header[20] = (uint8_t) ((wFormatTag) & 0xff);

    if (nCh <= 6)
    {
        wave_header[23] = 0;
        wave_header[22] = nCh;
    }

    if (nSampleRate <= 96000)
    {
        nSamplesPerSec = nSampleRate;
    }

    wave_header[27]     = (uint8_t) ((nSamplesPerSec >> 24) & 0xff);
    wave_header[26]     = (uint8_t) ((nSamplesPerSec >> 16) & 0xff);
    wave_header[25]     = (uint8_t) ((nSamplesPerSec >> 8) & 0xff);
    wave_header[24]     = (uint8_t) ((nSamplesPerSec) & 0xff);

    wave_header[36 ]    = 'd';
    wave_header[37 ]    = 'a';
    wave_header[38 ]    = 't';
    wave_header[39 ]    = 'a';

    wave_header[40 ]    = 0;
    wave_header[41 ]    = 0;
    wave_header[42 ]    = 0;
    wave_header[43 ]    = 0;

    //write_waveHeader = 1;
    nTemp               = fwrite(wave_header, 1, 44, fI2SOut);

    return 0;
}

static int audio_dump_I2S()
{
    //wirte wav header
    int nI2S    = 0;
    int nResult = 1;
    int I2SBuf;
    int nBufferLength;

    nI2S = i2s_get_DA_running();

    if (nI2S && gnAudioMgrWriteWAVHeader == 0)
    {
        audio_wave_header();
        printf("[Audio mgr] dump debug, audio_wave_header() \n");
        gnAudioMgrWriteWAVHeader = 1;
    }

    // dump data
    if (gnAudioMgrWriteWAVHeader == 1)
    {
        iteAudioGetAttrib(  ITE_AUDIO_CODEC_SET_BUFFER_LENGTH,  &nBufferLength);
        iteAudioGetAttrib(  ITE_AUDIO_I2S_PTR,                  &I2SBuf);

        gnAudioMgrI2sWrite = I2S_DA32_GET_WP();
        // first time
        if ((gnAudioMgrI2sWrite > gnAudioMgrI2sLastWrite) && gnAudioMgrI2sLastWrite == 0 && gnAudioMgrI2sWrite == 0)
        {
            //gnAudioMgrI2sWrite = I2S_DA32_GET_WP();
            nResult                 = fwrite(I2SBuf, 1, gnAudioMgrI2sWrite, fI2SOut);
            gnAudioMgrI2sLastWrite  = gnAudioMgrI2sWrite;
            printf("[Audio mgr]dump I2S buffer first (%d)  0x%x #line %d \n", (gnAudioMgrI2sWrite), I2SBuf, __LINE__);
        }
        else if ((gnAudioMgrI2sWrite > gnAudioMgrI2sLastWrite) && (gnAudioMgrI2sWrite != gnAudioMgrI2sLastWrite))
        {
            //gnAudioMgrI2sWrite = I2S_DA32_GET_WP();
            nResult                 = fwrite(I2SBuf + gnAudioMgrI2sLastWrite, 1, gnAudioMgrI2sWrite - gnAudioMgrI2sLastWrite, fI2SOut);
           
            gnAudioMgrI2sLastWrite  = gnAudioMgrI2sWrite;
        }
        else if (gnAudioMgrI2sWrite < gnAudioMgrI2sLastWrite && gnAudioMgrI2sWrite != gnAudioMgrI2sLastWrite)
        {
            //gnAudioMgrI2sWrite = I2S_DA32_GET_WP();
            nResult                 = fwrite(I2SBuf + gnAudioMgrI2sLastWrite, 1, nBufferLength - gnAudioMgrI2sLastWrite, fI2SOut);
            nResult                 = fwrite(I2SBuf, 1, gnAudioMgrI2sWrite, fI2SOut);
           
            gnAudioMgrI2sLastWrite  = gnAudioMgrI2sWrite;
        }
        else
        {
            printf("[Audio mgr]dump error %d %d  #line %d \n", (gnAudioMgrI2sWrite), gnAudioMgrI2sLastWrite, __LINE__);
        }
    }
}

#endif

void AudioThreadFunc()
{
    MMP_INT         nResult;
    MMP_ULONG       buffSize    = 0;
    MMP_UINT        i           = 0;
    MMP_ULONG       space;
    struct timeval  tv;
    struct timeval  stv;
    MMP_UINT        nTotal              = 0;
    int             nRemoveCard         = 0;
    int             nI2SBufferLength    = 0;

    sem_wait(gpAudioMgrSemaphore);
    PalAssert(nResult == 0);
    audioMgr.bAudioEos  = MMP_FALSE;
    audioMgr.nAudioSeek = MMP_FALSE;
    if (audioMgr.destroying == MMP_TRUE)
    {
        goto end;
    }
    nRemoveCard         = 0;
    audioMgr.mute       = MMP_FALSE;

    //fileSecPosComplete  = MMP_FALSE;
    for (;;)
    {
        MMP_ULONG       remainSize;
        MMP_ULONG       nTemp1;
        MMP_UINT        nCurrTime = 0;
        MMP_UINT        nTemp = 0;
        struct timeval  /*StartPlayT,*/ CurrPlayT;

        if (audioMgr.stop == MMP_TRUE)
        {
            goto stop;
        }

        if (audioMgr.bNetworkPlay == MMP_TRUE)
        {
            remainSize = 0;
            audioSetRemainSize(remainSize);
        }
        else
        {
            remainSize = audioMgr.streamSize;
            audioSetRemainSize(remainSize);
            memcpy(audioMgr.sampleBuf, audioMgr.streamBuf, remainSize);
        }

        gettimeofday(   &gStartDebugT,  NULL);
        //gettimeofday(   &StartPlayT,    NULL);

#ifdef SMTK_AUDIO_READ_BUFFER_THREAD
        do
        {
            usleep(20000);
            //audioMgr.playing=false : idle state ,audioMgr.bCheckMusicComplete = false :playing state
        } while (audioMgr.bCheckMusicComplete == MMP_FALSE || audioMgr.playing == MMP_FALSE);
#endif

        for (;;)
        {
            MMP_ULONG       bufSize;
            MMP_ULONG       readSize;
            MMP_ULONG       writtenSize;
            MMP_INT         type = 0, error = 0;
            int             nM4Aindex;
            int             nM4AStop;
            unsigned int    nTest;

            if (audioMgr.ptNetwork.nSpecialCase == 1)
            {
                I2S_DA32_WAIT_RP_EQUAL_WP(1);
                i2s_pause_DAC(1);
                goto stop;
            }

ReadData:

#ifdef SMTK_AUDIO_READ_BUFFER_THREAD
            readSize    = audio_buffer[audioMgr.nCurrentWriteBuffer].size;
            //smtkAudioMgrGetNetworkState(&type,&error);

            nTest       = smtkAudioMgrGetTime();

            // read error,stop
            if (audioMgr.nReadBufferStop == 1 && audio_buffer_write_complete() == 1)
            {
                {
                    // avoid small ring can't play
                    i = 0;
                    do
                    {
                        usleep(5000);
                        i++;
                        if (i > 10)
                            break;
                    } while (audioMgr.stop == MMP_FALSE && i2s_get_DA_running() == 0);
                    usleep(15000);
                    
                    if (audioMgr.ptNetwork.nType == SMTK_AUDIO_WAV) I2S_DA32_WAIT_RP_EQUAL_WP(1); //wait wp=rp
                    
                    goto stop;
                }
            }
            if (audioMgr.stop == MMP_TRUE)
            {
                //DEBUG_PRINT("[Audio mgr] stop #line %d \n",__LINE__);
                goto stop;
            }

            if (readSize <= 0)
                readSize = SMTK_AUDIO_READ_BUFFER_SIZE;
#else
            // READ DATA FROM FILE TO SAMPLE BUFFER
            if (audioMgr.filePlay == MMP_TRUE || audioMgr.bFilePlayEx == MMP_TRUE)
            {
                readSize = (remainSize >= MULTISECTION_SIZE) ? MULTISECTION_SIZE : remainSize;
                if (audioMgr.bMp4Parsing == MMP_FALSE)
                {
                    nResult = fread(audioMgr.sampleBuf, 1, readSize, currFile);
                }
                else
                {
                    nResult     = audioM4AFileRead();
                    readSize    = nResult;
                }

                {
                    if ((MMP_UINT) nResult != readSize)
                    {
                        audioMgr.playing = MMP_FALSE;
                        //LOG_DEBUG "currFile  PalFileRead null   #line %d \n", __LINE__ LOG_END
                        goto stop;
                    }
                }
            }

#endif      // SMTK_AUDIO_READ_BUFFER_THREAD

            bufSize = writtenSize = 0;
            while (writtenSize < readSize)
            {   //  writtenSize < readSize
                MMP_ULONG remain = (readSize - writtenSize);
                if (audioMgr.bMp4Parsing == MMP_TRUE)
                {
                    remain = readSize - writtenSize;
                }
                //checkAndroidSyncPlay();
                //sleep, release task time
                usleep(50000);
AudioPause:
                if (audioMgr.pause == MMP_TRUE)
                {
                    audioMgr.playing = MMP_FALSE;
                    sem_wait(gpAudioMgrSemaphore);
                    PalAssert(nResult == 0);
                    DEBUG_PRINT("[Audio mgr] continue  #line %d \n",__LINE__);
                }
                if (audioMgr.playing == MMP_FALSE)
                {
                    DEBUG_PRINT("[Audio mgr]audioMgr.playing == MMP_FALSE \n");
                    break;
                }

GetBuffer:
                nResult = audio_get_available_buffer_size(&bufSize);
                if (audioMgr.stop == MMP_TRUE)
                {
                    DEBUG_PRINT("[Audio mgr] stop #line %d \n",__LINE__);
                    goto stop;
                }
                if (bufSize == 0)
                {
                    gettimeofday(&gEndDebugT, NULL);
                    if (itpTimevalDiff(&gStartDebugT, &gEndDebugT) > 1000)
                    {
                        audio_get_available_buffer_size(&nTemp1);

                        gettimeofday(&gStartDebugT, NULL);
                    }

                    if (audioMgr.ptNetwork.nLocalFileSize <= gnAudioWrite && audioMgr.ptNetwork.bLocalPlay == 1 && audioMgr.ptNetwork.nType == SMTK_AUDIO_WAV)
                    {
                        //DEBUG_PRINT("[Audio mgr] wav eof size %d write %d  #line %d \n",audioMgr.ptNetwork.nLocalFileSize,gnAudioWrite,__LINE__);
                        goto stop;
                    }

                    if (audioMgr.playing == MMP_FALSE)
                    {
                        //DEBUG_PRINT("[Audio mgr] bufSize == 0 , audioMgr.playing == MMP_FALSE \n");
                        break;
                    }
                    usleep(200);

                    continue;
                }
                // if inturrupt sound playing , and remove music
                if (smtkAudioMgrInterruptSoundGetRemoveCard() == 1)
                {
                    nRemoveCard = audioCheckInterruptSoundRemoveCard();
                    if (nRemoveCard % 20 == 0)
                        //DEBUG_PRINT("remove card %d \n",nRemoveCard);

                        if (nRemoveCard > 50)
                        {
                            //DEBUG_PRINT("remove card %d stop\n",nRemoveCard);
                            smtkAudioMgrInterruptSoundSetRemoveCard(0);
                            gnCheckRemoveCardStart = 0;
                            goto stop;
                        }
                }
#ifdef SMTK_AUDIO_READ_BUFFER_THREAD
                // check audio buffer prepared
                if (audio_buffer[audioMgr.nCurrentWriteBuffer].ready)
                {
                    // write data
                    bufSize = bufSize > remain ? remain : bufSize;
                    nResult = audio_write_stream(audio_buffer[audioMgr.nCurrentWriteBuffer].data + writtenSize, bufSize);
                }
                else
                {
                    // audio buffer empty
                    //DEBUG_PRINT("[Audio mgr] audio buffer %d emtpy #line %d \n",audioMgr.nCurrentWriteBuffer,__LINE__);
                    if (audioMgr.nReadBufferStop == 1 && smtkAudioMgrInterruptSoundGetRemoveCard() == 0)
                    {
                        //DEBUG_PRINT("[Audio mgr] read error,stop #line %d \n",__LINE__);
                        goto stop;
                    }
                    usleep(200);
                    continue;
                }

#else
                bufSize = bufSize > remain ? remain : bufSize;
                nResult = audio_write_stream(audio_buffer[audioMgr.nCurrentWriteBuffer].data + writtenSize, bufSize);

#endif          //SMTK_AUDIO_READ_BUFFER_THREAD
                iteAudioGetAttrib(ITE_AUDIO_CODEC_SET_SAMPLE_RATE, &nTemp);
                if (audioMgr.nDriverSampleRate != nTemp)
                {
                    audioMgr.nDriverSampleRate = nTemp;
                }
                PalAssert(nResult == 0);

                writtenSize += bufSize;

                iteAudioGetDecodeTime((uint32_t *)&nTemp);
                if (nTemp > 0 && nTemp - nCurrTime > 1000)
                {
                    nCurrTime = nTemp;
                    //LOG_INFO "Current Time %d \n", nCurrTime / 1000 + audioMgr.jumpSecond LOG_END
                }
#ifdef SMTK_AUDIO_READ_BUFFER_THREAD
                if (remain == 0 || writtenSize >= readSize)
                {
                    if (audioMgr.nCurrentWriteBuffer == audioMgr.nCurrentReadBuffer)
                    {
                        //DEBUG_PRINT("[Audio mgr] audio write buffer[%d] == read buffer[%d] ,there is no ready buffer , #line %d \n",audioMgr.nCurrentWriteBuffer,audioMgr.nCurrentReadBuffer,__LINE__);
                        pthread_mutex_lock(&audio_read_buffer_mutex);
                        audio_buffer[audioMgr.nCurrentWriteBuffer].ready = 0;
                        audioMgr.nCurrentWriteBuffer++;
                        if (audioMgr.nCurrentWriteBuffer >= SMTK_AUDIO_READ_BUFFER_FRAMES)
                            audioMgr.nCurrentWriteBuffer = 0;
                        pthread_mutex_unlock(&audio_read_buffer_mutex);
                    }
                    else
                    {
                        audio_buffer[audioMgr.nCurrentWriteBuffer].ready = 0;
                        //DEBUG_PRINT("[Audio mgr] audio_buffer[%d] set 0, #line %d \n",audioMgr.nCurrentWriteBuffer,__LINE__);
                        audioMgr.nCurrentWriteBuffer++;
                        if (audioMgr.nCurrentWriteBuffer >= SMTK_AUDIO_READ_BUFFER_FRAMES)
                            audioMgr.nCurrentWriteBuffer = 0;
                    }
                    break;
                }
#else
                if (remain == 0)
                {
                    break;
                }
#endif
            }//  writtenSize < readSize

            remainSize -= readSize;
            audioSetRemainSize(remainSize);
            if (audioMgr.pause != MMP_TRUE)
            {
                if (remainSize == 0 || audioMgr.playing == MMP_FALSE)
                {
                    //LOG_DEBUG "remainSize == %d  || audioMgr.playing == %d  \n", remainSize, audioMgr.playing LOG_END
                    break;
                }
            }
        }

stop:
        if (audioMgr.stop == MMP_TRUE)
        {
            //DEBUG_PRINT("[Audio mgr] stop audioMgr.stop == MMP_TRUE lns %d  \n",__LINE__);
        }
        else
        {
            audioMgr.bAudioEos  = MMP_FALSE;
            audioMgr.jumpSecond = 0;

            if (audioMgr.playing == MMP_FALSE)
            {
                if (audioMgr.destroying == MMP_TRUE)
                {
                    goto end;
                }
            }
        }

        if (audioMgr.mode == SMTK_AUDIO_NORMAL)
        {
            unsigned int remain;
            unsigned int WP;
            unsigned int RP;
            unsigned int ncount = 0;
            iteAudioGetAttrib(ITE_AUDIO_CODEC_SET_BUFFER_LENGTH, &nI2SBufferLength);
            do
            {   /*wait audio play over */
                if (audioMgr.ptNetwork.nSpecialCase == 1)
                {
                    DEBUG_PRINT("[Audio mgr] short wav(nSpecialCase) end #line %d \n", __LINE__);
                    break;
                }
                usleep(20000);
                WP=I2S_DA32_GET_WP();
                RP = I2S_DA32_GET_RP();
                remain = (WP >= RP) ? (WP - RP):((nI2SBufferLength-RP)+WP);
                ncount++;
                if(ncount>200) break;
                //printf("(%d)remain=%d WP=%d RP=%d length(%d)\n",ncount,remain,WP,RP,nI2SBufferLength);
            } while (remain >= 1000 && audioMgr.stop != MMP_TRUE);      

            if (smtkAudioMgrInterruptSoundGetRemoveCard() == 1)
            {
                smtkAudioMgrInterruptSoundSetRemoveCard(0);
            }

            //audioMgr.playing = MMP_FALSE;
            
            // if interrupt sound playing , then delete interrupt task
            if (smtkAudioMgrGetInterruptStatus() == 1 && smtkAudioMgrGetInterruptOverwriteStatus() == 0)
            {
#ifdef CFG_BUILD_FFMPEG
                if (audioMgr.playing /*i2s_get_DA_running()*/ && !mtal_pb_check_fileplayer_with_audio())
                {
                    //DEBUG_PRINT("[Audio mgr] i2s_deinit_DAC \n");
                    //i2s_deinit_DAC();
                    i2s_pause_DAC(1);
                }
#endif

                pthread_mutex_lock(&audio_callback_mutex);
                if (audioMgr.ptNetwork.audioMgrCallback)
                {
                    audioMgr.ptNetwork.audioMgrCallback(AUDIOMGR_STATE_CALLBACK_GET_FINISH_NAME);
                }
                pthread_mutex_unlock(&audio_callback_mutex);
                usleep(100000);
            }
            else
            {

                if (audioMgr.playing /*i2s_get_DA_running()*/ && audioMgr.ptNetwork.nSpecialCase == 0 
#ifdef CFG_BUILD_FFMPEG
                    && mtal_pb_check_fileplayer_with_audio() == 0
#endif
                    )
                {
                    //DEBUG_PRINT("[Audio mgr] i2s_deinit_DAC 0\n");
                    //i2s_deinit_DAC();
                    while(I2S_DA32_GET_WP() != I2S_DA32_GET_RP())
                    {
                        // wait to playback end completely
                        I2S_DA32_WAIT_RP_EQUAL_WP(1); //wait wp=rp
                        // usleep(15000);
                    }
                    i2s_pause_DAC(1);
                }

            }

            audioMgr.playing = MMP_FALSE;
            audioMgr.ptNetwork.bEOP = MMP_TRUE;            
#ifdef CFG_AUDIO_FILTER_ENABLE
            audioFilterProcessExit();
#endif
            if (smtkAudioMgrGetInterruptOverwriteStatus() == 1)
            {
                smtkAudioMgrSetInterruptStatus(0);

                pthread_mutex_lock(&audio_callback_mutex);
                if (audioMgr.ptNetwork.audioMgrCallback)
                {
                    audioMgr.ptNetwork.audioMgrCallback(AUDIOMGR_STATE_CALLBACK_GET_FINISH_NAME);
                }
                pthread_mutex_unlock(&audio_callback_mutex);
            }
            else
            {
                if (audioMgr.stop != MMP_TRUE)
                {
                    pthread_mutex_lock(&audio_callback_mutex);
                    if (audioMgr.ptNetwork.audioMgrCallback)
                    {
                        DEBUG_PRINT("[Audio mgr] AUDIOMGR_STATE_CALLBACK_GET_FINISH_NAME #%d \n",audioMgr.ptNetwork.pFilename,__LINE__);
                        audioMgr.ptNetwork.audioMgrCallback(AUDIOMGR_STATE_CALLBACK_GET_FINISH_NAME);
                    }

                    if (audioMgr.ptNetwork.audioMgrCallback)
                    {
                        DEBUG_PRINT("[Audio mgr] AUDIOMGR_STATE_CALLBACK_PLAYING_FINISH #%d \n",audioMgr.ptNetwork.pFilename,__LINE__);
                        audioMgr.ptNetwork.audioMgrCallback(AUDIOMGR_STATE_CALLBACK_PLAYING_FINISH);
                    }
                    
                    if (audioMgr.ptNetwork.audioMgrCallback && audioMgr.ptNetwork.nSpecialCase == 1)
                    {
                        DEBUG_PRINT("[Audio mgr] AUDIOMGR_STATE_CALLBACK_PLAYING_SPECIAL_CASE_FINISH #%d \n",audioMgr.ptNetwork.pFilename,__LINE__);
                        audioMgr.ptNetwork.audioMgrCallback(AUDIOMGR_STATE_CALLBACK_PLAYING_SPECIAL_CASE_FINISH);
                    }
                    pthread_mutex_unlock(&audio_callback_mutex);
                }
            }

            // disable ffmpeg pause audio
#ifdef CFG_BUILD_FFMPEG
            FFmpeg_pause(0);//resume vedio's audio , and reload audio codec.
#endif
            sem_wait(gpAudioMgrSemaphore);    
            
            //printf("[Audio mgr] sem_wait exit #line %d \n",__LINE__);

            PalAssert(nResult == 0);
            /*LOG_DEBUG
            " audio mgr stop eventAudioMgrToThread %d \n", __LINE__
            LOG_END*/
        }
        else if (audioMgr.mode == SMTK_AUDIO_REPEAT)
        {
            //DEBUG_PRINT(" audio mgr stop eventAudioMgrToThread %d \n",__LINE__);
            nResult = 0;
            nTemp   = 0;
            do
            {
                usleep(2000);
                if (nResult == 10)
                {
                    //DEBUG_PRINT("[Audio mgr] wait to end of playing %d %d #line %d \n",I2S_DA32_GET_RP(),I2S_DA32_GET_WP(),__LINE__);
                    nResult = 0;
                    nTemp++;
                    // flac can not wp == rp
                    if ((I2S_DA32_GET_WP() > I2S_DA32_GET_RP()) && (I2S_DA32_GET_WP() - I2S_DA32_GET_RP() < 128))
                    {
                        //DEBUG_PRINT("[Audio mgr] wait to end of playing breaking %d %d #line %d \n",I2S_DA32_GET_RP(),I2S_DA32_GET_WP(),__LINE__);
                        break;
                    }

                    if (nTemp > 20)
                    {
                        //DEBUG_PRINT("[Audio mgr] wait to end of playing breaking %d %d #line %d \n",I2S_DA32_GET_RP(),I2S_DA32_GET_WP(),__LINE__);
                        break;
                    }
                }
                nResult++;
            } while (I2S_DA32_GET_RP() != I2S_DA32_GET_WP() && audioMgr.stop != MMP_TRUE);
            audio_buffer_reset();
            fseek(audioMgr.ptNetwork.pHandle, 0, SEEK_SET);
            audioMgr.playing = MMP_TRUE;
            //audioMgr.stop = MMP_FALSE;
        }
    }

end:
    DEBUG_PRINT(" audio mgr stop ithAudioTerminate %d \n", __LINE__);
    nResult = iteAudioTerminate();
    PalAssert(nResult == 0);
    sem_post(gpAudioMgrSemaphore);
    PalAssert(nResult == 0);

    //return MMP_NULL;
}

MMP_INT
smtkAudioMgrInitialize(
    void)
{
    MMP_INT         result = 0;
    MMP_UINT        i;
#ifdef SMTK_AUDIO_READ_BUFFER_THREAD
    pthread_t       task;
    pthread_attr_t  attr;
#endif

    // Set the CODEC plugins path
    if (audioMgr.nInit == 1)
    {
        /*LOG_DEBUG
        "audio mar has init %d \n", __LINE__
        LOG_END*/
        return result;
    }
    memset(&audioMgr, 0, sizeof(audioMgr));

    audioMgr.sampleBuf = malloc(MULTISECTION_SIZE);
    if (!audioMgr.sampleBuf)
    {
        result = SMTK_AUDIO_ERROR_INIT_FAIL;
        goto end;
    }

    result = sem_init(gpAudioMgrSemaphore, 0, 0);
    if (result == -1)
    {
        result = SMTK_AUDIO_ERROR_INIT_FAIL;
        goto end;
    }
    
    PalAssert(result == 0);
    audioMgr.playing        = MMP_FALSE;
    audioMgr.pause          = MMP_FALSE;
    audioMgr.mode           = SMTK_AUDIO_NORMAL;
    audioMgr.stop           = MMP_TRUE;
    audioMgr.Nfilequeque    = 0;
    audioMgr.bInsertSound= MMP_FALSE;

#ifdef SMTK_AUDIO_READ_BUFFER_THREAD
    smtkAudioMgrSetNetworkMusicTagSize(0);
    init_audio_buffer();
    result = pthread_mutex_init(&audio_read_buffer_mutex, NULL);
    if (result != 0)
        DEBUG_PRINT("[Audio mgr] Can not open ab_mutex %d \n", result);

    // create audio read data task
    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, SMTK_AUDIO_MGR_READ_BUFFER_STACK_SIZE);
    pthread_create(&task, &attr, audio_read_thread_func, NULL);
    gAudioReadthreadID              = task;

    audioMgr.nCurrentReadBuffer     = 0;
    audioMgr.nCurrentWriteBuffer    = 0;
#endif

    if (result != 0)
        DEBUG_PRINT("[Audio mgr] Can not open sxadmx mutex %d \n", result);

    result = pthread_mutex_init(&audio_callback_mutex, NULL);
    if (result != 0)
        DEBUG_PRINT("[Audio mgr] Can not open callback mutex %d \n", result);

    AudioMgrInit();

    //memset(&gtFileInfo, 0, sizeof(gtFileInfo));

    audioMgr.nInit  = 1;
    // init wave buffer here
    gOutBuffer      = memalign(64, SMTK_AUDIO_READ_BUFFER_SIZE);



    memset(&gpNetwork, 0, sizeof(gpNetwork));
    gpNetwork.pFilename = (char *)malloc(256);
    iteAudioSetAttrib(ITE_AUDIO_I2S_INIT, &audioMgr.nInit);//set init value
    //iteAudioStopQuick();

end:
    return result;
}

MMP_INT
smtkAudioMgrTerminate(
    void)
{
    MMP_INT result = 0;


    audioMgr.nInit = 0;
    DEBUG_PRINT("[Audio mgr] smtkAudioMgrTerminate\n");
    smtkAudioMgrQuickStop();

    audioMgr.playing    = MMP_FALSE;
    audioMgr.pause      = MMP_FALSE;
    audioMgr.destroying = MMP_TRUE;
    usleep(20000);


    if (gOutBuffer)
        free(gOutBuffer);
    gOutBuffer = NULL;

    if (audioMgr.sampleBuf)
    {
        free(audioMgr.sampleBuf);
        audioMgr.sampleBuf = MMP_NULL;
    }
    free_audio_buffer();

#if defined(__OPENRTOS__)
    if (gAudioMgrthreadID)
    {
        pthread_cancel(gAudioMgrthreadID);
        gAudioMgrthreadID = 0;
    }

    if (gAudioReadthreadID)
    {
        pthread_cancel(gAudioReadthreadID);
        gAudioReadthreadID = 0;
    }
#endif
    pthread_mutex_destroy(  &audio_callback_mutex);
    pthread_mutex_destroy(  &audio_read_buffer_mutex);

    return result;
}

static int audioCheckInterruptSoundRemoveCard()
{
    if (gnCheckRemoveCardStart == 0)
    {
        DEBUG_PRINT("[Audio mgr]start remove card buffering... \n");
        gnCheckRemoveCardStart++;
        gettimeofday(&gStartRemoveCardT, NULL);
    }
    if (smtkAudioMgrGetInterruptStatus() == 0)
        gnCheckRemoveCardStart++;

    usleep(20 * 1000);
    gettimeofday(&gEndRemoveCardT, NULL);

    if (itpTimevalDiff(&gStartRemoveCardT, &gEndRemoveCardT) > 10 * 1000)
    {
        DEBUG_PRINT("[Audio mgr]stoping remove card buffering...  %d \n", itpTimevalDiff(&gStartRemoveCardT, &gEndRemoveCardT));

        return 100;
    }

    return gnCheckRemoveCardStart;
}

static void audioCheckInterruptSound()
{
    int i;
    int nTime   = 80; // 16000 ms
    int nTemp   = 0;
    // check interrupt sound
    if (smtkAudioMgrGetInterruptStatus() == 1 && smtkAudioMgrGetInterruptOverwriteStatus() == 0)
    {
        DEBUG_PRINT("[Audio mgr] interrupt sound playing, need to wait start #line %d \n", __LINE__);
        do
        {
            usleep(200 * 1000);
            nTime--;

            if (nTime % 10 == 0)
                DEBUG_PRINT("[Audio mgr] interrupt sound playing, wait %d #line %d \n", nTime, __LINE__);

            if (smtkAudioMgrGetInterruptStatus() == 0)
                nTemp++;
        } while (nTemp <= 5 && nTime > 0);
        DEBUG_PRINT("[Audio mgr] interrupt sound not playing %d %d #line %d \n", nTemp, nTime, __LINE__);
    }
    usleep(20 * 1000);
}

MMP_INT smtkSetFileQueue(SMTK_AUDIO_PARAM_NETWORK tmpNetwork)
{
    pthread_mutex_lock(&audio_read_buffer_mutex);

    if (gpNetwork.pHandle) fclose(gpNetwork.pHandle);

    gpNetwork.audioMgrCallback  = tmpNetwork.audioMgrCallback;
    gpNetwork.nType             = tmpNetwork.nType;
    gpNetwork.LocalRead         = tmpNetwork.LocalRead;
    gpNetwork.nReadLength       = tmpNetwork.nReadLength;
    gpNetwork.bSeek             = tmpNetwork.bSeek;
    gpNetwork.nM4A              = tmpNetwork.nM4A;
    gpNetwork.bLocalPlay        = tmpNetwork.bLocalPlay;
    strcpy(gpNetwork.pFilename, tmpNetwork.pFilename);

    audioMgr.Nfilequeque++;
    pthread_mutex_unlock(&audio_read_buffer_mutex);
    return 0;
}

void 
smtkAudioMgrSetInitPlay(SMTK_AUDIO_PARAM_NETWORK *audiomgr_local,char *filename)
{
    char* ext;
    MMP_INT nType = 0;
    ext = strrchr(filename, '.');
    if (!ext){
        printf("Invalid file name: %s\n", filename);
        return ;
    }
    
    if (AL_Local_player_http_handle)
    {
        fclose(AL_Local_player_http_handle);
        AL_Local_player_http_handle = NULL;
    }
    
    if(filename) AL_Local_player_http_handle = fopen(filename, "rb");    
    
    ext++;
    if      (stricmp(ext, "wav") == 0){nType = SMTK_AUDIO_WAV;}
    else if (stricmp(ext, "mp3") == 0){nType = SMTK_AUDIO_MP3;}
    else if (stricmp(ext, "wma") == 0){nType = SMTK_AUDIO_WMA;}
    else    {printf("Unsupport file format: %s\n", ext);}
    
    audiomgr_local->audioMgrCallback = NULL;
    audiomgr_local->pHandle     = AL_Local_player_http_handle;
    audiomgr_local->nType       = nType;
    audiomgr_local->LocalRead   = fread;
    audiomgr_local->nReadLength = SMTK_AUDIO_READ_BUFFER_SIZE;
    audiomgr_local->bSeek       = 0;
    audiomgr_local->nM4A        = 0;
    audiomgr_local->bLocalPlay  = 1;
    audiomgr_local->pFilename   = filename;    
      
}

MMP_INT
smtkAudioMgrPlayNetwork(
    SMTK_AUDIO_PARAM_NETWORK *pNetwork)
{
    MMP_INT         nResult = 0;
    MMP_INT         i       = 0;
    ITE_WaveInfo    tWavInfo;

    int             nFfmpegPauseAudio = 0;
    nResult             = MMP_TRUE;
    iteAudioSetAttrib(ITE_AUDIO_MUSIC_PLAYER_ENABLE, &nResult);
    nResult             = 0;
    audioMgr.nOffset    = 0;
    // write to audio driver
    gnAudioWrite        = 0;
#ifdef CFG_AUDIO_ENABLE
    smtkAudioMgrSetTotalTime(0);

    if (audioMgr.nInit == 0)
    {
        DEBUG_PRINT("[Audio mgr] audio mgr has terminated , return #line %d \n", __LINE__);
        return 0;
    }
    if (gnAudioMgrPlayNetwork)
    {
        DEBUG_PRINT("[Audio mgr] play network setting  , return #line %d \n", __LINE__);
        smtkAudioMgrSetNetworkError(1);
        return SMTK_AUDIO_ERROR_UNKNOW_FAIL;
    }

    gnAudioMgrPlayNetwork = 1;

    //check interrupt sound
    audioCheckInterruptSound();

    if (smtkAudioMgrInterruptSoundGetRemoveCard() == 1)
    {
        smtkAudioMgrInterruptSoundSetRemoveCard(0);
    }
    gnCheckRemoveCardStart = 0;

    // check  pNetwork paramater
    if (!pNetwork)
    {
        DEBUG_PRINT("[Audio mgr] smtkAudioMgrPlayNetwork init error %d \n", SMTK_AUDIO_ERROR_UNKNOW_FAIL);
        gnAudioMgrPlayNetwork = 0;
        smtkAudioMgrSetNetworkError(1);
        return SMTK_AUDIO_ERROR_UNKNOW_FAIL;
    }
    if (pNetwork->nReadLength < 0 || pNetwork->nReadLength > MULTISECTION_SIZE)
    {
        gnAudioMgrPlayNetwork = 0;
        smtkAudioMgrSetNetworkError(1);
        return SMTK_AUDIO_ERROR_UNKNOW_FAIL;
    }
    if (pNetwork->nType > SMTK_AUDIO_FLAC || pNetwork->nType < SMTK_AUDIO_MP3)
    {
        gnAudioMgrPlayNetwork = 0;
        smtkAudioMgrSetNetworkError(1);
        return SMTK_AUDIO_ERROR_UNKNOW_FAIL;
    }
    if (pNetwork->bLocalPlay == 1)
    {
        if (pNetwork->pHandle == 0 || pNetwork->LocalRead == 0)
        {
            gnAudioMgrPlayNetwork = 0;
            smtkAudioMgrSetNetworkError(1);
            return SMTK_AUDIO_ERROR_UNKNOW_FAIL;
        }
    }
    else
    {
        if (pNetwork->pHandle == 0 || pNetwork->NetworkRead == 0)
        {
            gnAudioMgrPlayNetwork = 0;
            smtkAudioMgrSetNetworkError(1);
            return SMTK_AUDIO_ERROR_UNKNOW_FAIL;
        }
    }

    if (pNetwork->pHandle && pNetwork->bLocalPlay && pNetwork->pFilename)
    {
        fseek(  pNetwork->pHandle,  0,  SEEK_END);
        audioMgr.ptNetwork.nLocalFileSize = ftell(pNetwork->pHandle);
        fseek(  pNetwork->pHandle,  0,  SEEK_SET);
        //DEBUG_PRINT("[Audio mgr] smtkAudioMgrPlayNetwork local file size %d \n",audioMgr.ptNetwork.nLocalFileSize);
    }

    if (audioMgr.ptNetwork.nSpecialCase == 1 && pNetwork->nType != SMTK_AUDIO_WAV)
    {
        audioMgr.ptNetwork.nSpecialCase = 0;
        if (i2s_get_DA_running())
            smtkAudioMgrStop();
    }
    audioMgr.ptNetwork.nSpecialCase = 0;
    

#if defined(__OPENRTOS__)
    // check if show spectrum
    iteAudioSetMusicShowSpectrum((int)pNetwork->bSpectrum);
#endif

#ifdef CFG_BUILD_FFMPEG
        FFmpeg_pause(1);//if video exit,pause video's audio ,insert short audio(mp3 or wav..) to play, 
#endif

    // open engine
    if (audioMgr.playing == MMP_FALSE)
    {
        if (pNetwork->nType == SMTK_AUDIO_MP3)
        {
            //DEBUG_PRINT("[Audio mgr] mp3 play \n");
            nResult = iteAudioOpenEngine(SMTK_AUDIO_MP3);
        }
        else if (pNetwork->nType == SMTK_AUDIO_FLAC)
        {
            DEBUG_PRINT("[Audio mgr] Flac play \n");
            nResult = iteAudioOpenEngine(SMTK_AUDIO_FLAC);
        }
        else if (pNetwork->nType == SMTK_AUDIO_AAC)
        {
                nResult = iteAudioOpenEngine(SMTK_AUDIO_AAC);
        }
        else if (pNetwork->nType == SMTK_AUDIO_WMA)
        {
            DEBUG_PRINT("[Audio mgr] wma play %d  \n", pNetwork->bSeek);
            if (pNetwork->bSeek)
            {
                iteAudioSetMusicWithoutASFHeader(1);
            }
            else
            {
                iteAudioSetMusicWithoutASFHeader(0);
            }
            nResult = iteAudioOpenEngine(SMTK_AUDIO_WMA);
            if (pNetwork->bSeek)
            {
                iteAudioWriteWmaInfo();
            }
        }
        else if (pNetwork->nType == SMTK_AUDIO_WAV)
        {
            //DEBUG_PRINT("[Audio mgr] wav play %d \n",pNetwork->bSeek);
            pNetwork->bFlowConrol = 0;
            if (pNetwork->pHandle && pNetwork->bLocalPlay)
            {
                fseek(  pNetwork->pHandle,  0,  SEEK_END);
                audioMgr.ptNetwork.nLocalFileSize = ftell(pNetwork->pHandle);
                fseek(  pNetwork->pHandle,  0,  SEEK_SET);
            }
            if (pNetwork->bLocalPlay && audioMgr.ptNetwork.nLocalFileSize <= SMTK_AUDIO_SPECIAL_CASE_BUFFER_SIZE && smtkAudioMgrGetInterruptOverwriteStatus() == 0)
            {
                iteAudioStopEngine();//need not Audioprocessor (stop it)
                audioMgr.ptNetwork.nSpecialCase = 1;
                //DEBUG_PRINT("[Audio mgr] special case enable \n");
            }
            else
            {
                audioMgr.ptNetwork.nSpecialCase = 0;
                nResult                         = iteAudioOpenEngine(SMTK_AUDIO_WAV);
                if (pNetwork->bSeek)
                {
                    iteAudioWriteWavInfo();
                }
            }
        }
        else
        {
            //DEBUG_PRINT("[Audio mgr] smtkAudioMgrPlayNetwork unknow engine %d #line %d \n",SMTK_AUDIO_ERROR_UNKNOW_FAIL,__LINE__);
            gnAudioMgrPlayNetwork = 0;
            smtkAudioMgrSetNetworkError(1);
            return SMTK_AUDIO_ERROR_UNKNOW_FAIL;
        }
    }

    audioMgr.filePlay                   = MMP_FALSE;
    audioMgr.pause                      = MMP_FALSE;
    audioMgr.stop                       = MMP_FALSE;

    audioMgr.bFilePlayEx                = MMP_FALSE;
    audioMgr.bNetworkPlay               = MMP_TRUE;
    audioMgr.ptNetwork.pHandle          = pNetwork->pHandle;
    audioMgr.ptNetwork.NetworkRead      = pNetwork->NetworkRead;
    audioMgr.ptNetwork.LocalRead        = pNetwork->LocalRead;
    audioMgr.ptNetwork.nReadLength      = pNetwork->nReadLength;
    audioMgr.ptNetwork.nType            = pNetwork->nType;
    audioMgr.ptNetwork.bEOF             = MMP_FALSE;
    audioMgr.ptNetwork.bEOP             = MMP_FALSE;
    audioMgr.ptNetwork.bFlowConrol      = pNetwork->bFlowConrol;
    audioMgr.ptNetwork.nError           = 0;
    audioMgr.ptNetwork.nEOFWaitCount    = AUDIOMGR_NETWORKPLAY_EOF_COUNT_MAX;
    audioMgr.ptNetwork.bLocalPlay       = pNetwork->bLocalPlay;
    audioMgr.ptNetwork.nM4A             = pNetwork->nM4A;
    audioMgr.ptNetwork.nM4ASeekSize     = pNetwork->nM4ASeekSize;
    audioMgr.ptNetwork.nM4AIndex        = pNetwork->nM4AIndex;
    audioMgr.ptNetwork.bSeek            = pNetwork->bSeek;
    audioMgr.ptNetwork.audioMgrCallback = pNetwork->audioMgrCallback;

    audioMgr.ptNetwork.pFilename        = pNetwork->pFilename;

    // drop audio
    audioMgr.ptNetwork.nDropTimeEnable  = pNetwork->nDropTimeEnable;
   // if (audioMgr.ptNetwork.nDropTimeEnable)
    //{
        //DEBUG_PRINT("[Audio mgr] audio linker drop audio #line %d \n",__LINE__);
    //}

    smtkAudioMgrSetI2sPostpone(0);

    audioMgr.ptNetwork.nARMDecode   = 0;
    audioMgr.ptNetwork.nDecodeTime  = 0;


/*#ifdef CFG_AUDIO_MGR_PARSING_MP3
    //parsing, not seek
    parsing_data_init();
    audio_parsing_data(0);
#endif
*/
    if (pNetwork->bSeek)
    {
        DEBUG_PRINT("[Audio mgr] smtkAudioMgrPlayNetwork seek #line %d \n", __LINE__);
        audioMgr.bCheckMusicComplete = MMP_TRUE;
    }
    else
    {
        audioMgr.bCheckMusicComplete = MMP_FALSE;
    }

    if (audioMgr.ptNetwork.bLocalPlay == 1)
    {
        if (pNetwork->nType == SMTK_AUDIO_MP3 || pNetwork->nType == SMTK_AUDIO_WMA)
        {
            // get metadata
            //smtkAudioMgrGetFileInfo(&gtFileInfo);

            pthread_mutex_lock(&audio_callback_mutex);
            if (audioMgr.ptNetwork.audioMgrCallback)
            {
                audioMgr.ptNetwork.audioMgrCallback(AUDIOMGR_STATE_CALLBACK_GET_METADATA);
            }
            pthread_mutex_unlock(&audio_callback_mutex);

            // get lrc
/*            if (pNetwork->pFilename)
            {
                gpLrcInfo = smtkAudioMgrGetLrc(pNetwork->pFilename);
                //DEBUG_PRINT("smtkAudioMgrGetLrc gpLrcInfo %d 0x%x \n",gpLrcInfo[0].line,gpLrcInfo);
                if (gpLrcInfo > 0)
                {
                    audioMgr.ptNetwork.audioMgrCallback(AUDIOMGR_STATE_CALLBACK_GET_LRC);
                }
            }*/
        }
    }

    //ithInvalidateDCache();
    audioMgr.playing        = MMP_TRUE;
    usleep(1000);
    gnAudioMgrPlayNetwork   = 0;

#if defined(__OPENRTOS__)
    iteAudioSetOutputEmpty(0);
#endif

#ifdef DUMP_MUSIC
    if (!fI2SOut)
    {
        sprintf(gFilepath, "%s.wav", pNetwork->pFilename);

        fI2SOut = fopen(gFilepath, "wb");
        printf("[Audio mgr] debug  dump, file  open  \n");

        if (fI2SOut == NULL)
        {
            printf("[Audio mgr] debug  dump,fopen null \n");
        }
    }
#endif

    sem_post(gpAudioMgrSemaphore);
    PalAssert(nResult == 0);
    nResult = MMP_TRUE;
#endif
    return nResult;
}

void smtkAudioParsingTime(SMTK_AUDIO_PARAM_NETWORK *pNetwork){
    audioMgr.ptNetwork.pFilename        = pNetwork->pFilename;
    audioMgr.ptNetwork.nType            = pNetwork->nType;
#ifdef CFG_AUDIO_MGR_PARSING_MP3
    //parsing, not seek
    parsing_data_init();
    audio_parsing_data(0);
    return;
#endif 
}

MMP_INT
smtkAudioMgrPlay(
    MMP_CHAR *filename)
{
    MMP_INT nResult = 0;
    return nResult;
}


#ifdef CFG_BUILD_FFMPEG
void FFmpeg_pause(int pause)
{
    int nFfmpegPauseAudio;
    pthread_mutex_lock(&audio_callback_mutex);
    #if defined(__OPENRTOS__)
    iteAudioGetAttrib(ITE_AUDIO_FFMPEG_PAUSE_AUDIO, &nFfmpegPauseAudio);
    
    if (mtal_pb_check_fileplayer_with_audio() && nFfmpegPauseAudio != pause)
    {
        if (pause)
        {
            iteAudioGetEngineAttrib(ITE_AUDIO_ENGINE_TYPE,&nVideoAudioCodec);
            i2s_enable_fading(0);//no fading
        }
        else
        {
            i2s_enable_fading(1);               //fading
            iteAudioOpenEngine(nVideoAudioCodec); //reinit i2s ,clear buffer data
        }
        itp_codec_playback_mute();
        iteAudioSetAttrib(ITE_AUDIO_FFMPEG_PAUSE_AUDIO, &pause);
        iteAudioSetMp3RdBufPointer(pause);
        smtkAudioSetInsertSoundStatus(pause);
        printf("%s FFmpeg_pause(%d)\n", __FUNCTION__, pause);
    }else{
        i2s_enable_fading(0);//no fading
        if(!pause){
            iteAudioSetAttrib(ITE_AUDIO_FFMPEG_PAUSE_AUDIO, &pause);
        }
    }
    #endif
    pthread_mutex_unlock(&audio_callback_mutex);
}
#endif

MMP_BOOL smtkAudioGetInsertSoundStatus(void){
    return audioMgr.bInsertSound;
}

void smtkAudioSetInsertSoundStatus(MMP_BOOL pause){
    audioMgr.bInsertSound = pause;
}

void smtkAudioWaitAudioStop(int sec){
    int timeout = sec*1000000;
    int usec = 100000;
    while(!audioMgr.ptNetwork.bEOP){//wait inturrupt sound auto stop
        usleep(usec);
        timeout-=usec;
        //printf("timeout = %d\n",timeout);
        if(timeout <= 0) { // force inturrupt sound time out stop,InturruptSound =FALSE 
#ifdef CFG_BUILD_FFMPEG        
            FFmpeg_pause(0);
#endif
            smtkAudioMgrQuickStop();
            printf("smtkAudioWaitAudioStop timeout = %d\n",timeout);
            //audioMgr.bInsertSound = false;
            break;
        }
    }
}

// check if playing,then stop
MMP_INT
smtkAudioMgrQuickStop()
{
    int nTemp = 0;
    if (smtkAudioMgrInterruptSoundGetRemoveCard() == 1)
    {
        DEBUG_PRINT("[Audio mgr] smtkAudioMgrQuickStop remove card, not stop now \n");
        return 0;
    }
    pthread_mutex_lock(&audio_callback_mutex);
    if (audioMgr.ptNetwork.audioMgrCallback)
        audioMgr.ptNetwork.audioMgrCallback = NULL;

    pthread_mutex_unlock(&audio_callback_mutex);

    if (audioMgr.playing == MMP_TRUE)
    {
        nTemp = 0;
        smtkAudioMgrStop();
    }
    else
    {
        nTemp = 0;
        do
        {
            usleep(1000);
            nTemp++;
        } while (gnAudioMgrStop == 1 && nTemp < 20);
        smtkAudioMgrStop();
    }

#if defined(__OPENRTOS__)
    // disable show spectrum
    iteAudioSetMusicShowSpectrum(0);
#endif

#ifdef DUMP_MUSIC
    if (fI2SOut)
    {
        printf("[Audio mgr] debug dump, fclose \n");
        fclose(fI2SOut);
        fI2SOut                     = NULL;
        gnAudioMgrI2sRead           = 0;
        gnAudioMgrI2sWrite          = 0;
        gnAudioMgrI2sLastWrite      = 0;
        gnAudioMgrWriteWAVHeader    = 0;
        memset(gFilepath, 0, sizeof(gFilepath));
    }

#endif
}

MMP_INT
smtkAudioMgrStop(
    void)
{
    MMP_INT result      = 0;
    MMP_INT timeout     = 0;
    int     i           = 0;
    int     al_state    = 0;

    if (gnAudioMgrStop)
    {
        return 0;
    }
    
    I2S_DA32_WAIT_RP_EQUAL_WP(0);
    
    gnAudioMgrStop = 1;
    if (audioMgr.pause == MMP_TRUE)
    {
        // avoid noise
        i2s_deinit_DAC();

        audioMgr.pause  = MMP_FALSE;
        result          = iteAudioPause(MMP_FALSE);

        sem_post(gpAudioMgrSemaphore);
        PalAssert(result == 0);
    }
    audioMgr.playing                = MMP_FALSE;
    audioMgr.stop                   = MMP_TRUE;
    audioMgr.jumpSecond             = 0;
    audioMgr.bCheckMusicComplete    = MMP_FALSE;
    timeout                         = 0;
    do
    {
        usleep(200);
        timeout++;
    } while (audioMgr.ptNetwork.bEOP == MMP_FALSE && timeout <= 10);

    smtkAudioMgrSetNetworkMusicTagSize(0);
    audioMgr.mode = SMTK_AUDIO_NORMAL;

    smtkAudioMgrSetNetworkError(0);
    if (audioMgr.ptNetwork.nARMDecode == 0 && audioMgr.ptNetwork.nSpecialCase == 0 
        #ifdef CFG_BUILD_FFMPEG
        && mtal_pb_check_fileplayer_with_audio() == 0
        #endif
        )
    {
        iteAudioStopQuick();
        i2s_deinit_DAC();
    }

#ifdef SMTK_AUDIO_READ_BUFFER_THREAD
    timeout = 0;
    do
    {
        usleep(200);
        timeout++;
    } while (audioMgr.nReading == 1 && timeout <= 10);
    audio_buffer_reset();

#endif
    result = MMP_FALSE;
    if (audioMgr.bAudioEos == MMP_TRUE)
    {
        audioMgr.bAudioEos = MMP_FALSE;
    }
    iteAudioSetAttrib(ITE_AUDIO_MUSIC_PLAYER_ENABLE, &result);

    result = 0;

    if (smtkAudioMgrGetInterruptOverwriteStatus() == 1)
        smtkAudioMgrSetInterruptStatus(0);
    smtkAudioMgrSetInterruptOverwriteStatus(0);
    gnAudioMgrStop = 0;

#if defined(AUDIO_MGR_DUMP)
    if (fout)
    {
        DEBUG_PRINT("[Audio mgr] fclose \n");
        fclose(fout);
        fout = NULL;
    }
#endif
    return result;
}

MMP_INT
smtkAudioMgrPause(
    void)
{
    MMP_INT result = 0;

    result          = iteAudioPause(MMP_TRUE);
    audioMgr.pause  = MMP_TRUE;
    if (audioMgr.ptNetwork.nARMDecode == 1)
    {
        i2s_pause_DAC(1);
    }

    return result;
}

MMP_INT
smtkAudioMgrContinue(
    void)
{
    MMP_INT result = 0;

    audioMgr.pause      = MMP_FALSE;
    audioMgr.playing    = MMP_TRUE;
    result              = iteAudioPause(MMP_FALSE);
    if (audioMgr.ptNetwork.nARMDecode == 1)
    {
        i2s_pause_DAC(0);
    }

    sem_post(gpAudioMgrSemaphore);

    PalAssert(result == 0);

    return result;
}

MMP_INT
smtkAudioMgrSetVolume(
    MMP_INT nVolume)
{
    MMP_INT result = 0;
    i2s_set_direct_volperc(nVolume);
    return result;
}

MMP_INT
smtkAudioMgrGetVolume(
    void)
{
    unsigned int cVolume;
    cVolume = i2s_get_current_volperc();
    return cVolume;
}

MMP_INT
smtkAudioMgrMuteOn(
    void)
{
    MMP_INT result = 0;

    audioMgr.mute = MMP_TRUE;
    i2s_mute_DAC(audioMgr.mute);
    
    return result;
}

MMP_INT
smtkAudioMgrMuteOff(
    void)
{
    MMP_INT result = 0;

    audioMgr.mute = MMP_FALSE;
    i2s_mute_DAC(audioMgr.mute);

    return result;
}

MMP_BOOL
smtkAudioMgrIsMuteOn(
    void)
{
    return audioMgr.mute;
}


SMTK_AUDIO_STATE
smtkAudioMgrGetState(
    void)
{
    if (audioMgr.playing == MMP_TRUE)
    {
        return SMTK_AUDIO_PLAY;
    }
    else if (audioMgr.pause == MMP_TRUE)
    {
        return SMTK_AUDIO_PAUSE;
    }
    else if (audioMgr.stop == MMP_TRUE)
    {
        return SMTK_AUDIO_STOP;
    }
    else
    {
        return SMTK_AUDIO_NONE;
    }
}

// get the spectrum information with UINT8[20*5], get 5 frame's spectrum
MMP_UINT8 *
smtkAudioMgrGetSpectrum(
    )
{
    if (audioMgr.ptNetwork.pSpectrum)
    {
        return audioMgr.ptNetwork.pSpectrum;
    }
    else
    {
        printf("smtkAudioMgrGetSpectrum buff null \n");
        return 0;
    }
}

// set spectrum pointer,if null return -1
MMP_INT
smtkAudioMgrSetSpectrum(
    MMP_UINT8 *buffer)
{
    if (buffer)
    {
        audioMgr.ptNetwork.pSpectrum = buffer;
        return 0;
    }
    else
    {
        printf("smtkAudioMgrSetSpectrum buff null \n");
        return -1;
    }
}

// 0:re-start spectrum, 1:pause/stop spectrum
MMP_INT
smtkAudioMgrPauseSpectrum(
    MMP_INT nPause)
{
#if defined(__OPENRTOS__)
    // disable show spectrum
    if (nPause == 1)
    {
        iteAudioSetMusicShowSpectrum(0);
    }
    else
    {
        iteAudioSetMusicShowSpectrum(1);
    }
#endif
}


void
smtkAudioMgrSetPcmIdx(
    int index)
{
    audioMgr.ptNetwork.pcmidx = index;
}

int
smtkAudioMgrGetPcmIdx(
    void)
{
    return audioMgr.ptNetwork.pcmidx;
}

MMP_UINT
smtkAudioMgrGetTime(
    void)
{
    // unit : ms
    MMP_UINT time;
    iteAudioGetDecodeTime((uint32_t *)&time);
    return time;
}

void
smtkAudioMgrSetMode(
    SMTK_AUDIO_MODE mode)
{
    audioMgr.mode = mode;
}

MMP_INT
smtkAudioMgrSetTime(MMP_INT nTime)
{
    MMP_INT nResult;

    return SMTK_AUDIO_SEEK_OK;
}

MMP_UINT
smtkAudioMgrSetTotalTime(
    int nTime)
{
    // uint : milliseconds
    MMP_INT nResult = 0;
    //if(nTime>0) {
    DEBUG_PRINT("[Audio mgr]smtkAudioMgrSetTotalTime == %d  #line %d    \n", nTime, __LINE__);
    audioSetTotoalTime((MMP_UINT32)nTime);

    return nResult;
}

MMP_UINT
smtkAudioMgrGetTotalTime(
    MMP_UINT *time)
{
    // uint : milliseconds
    MMP_INT nResult = 0;
    if (audioGetTotoalTime() > 0)
    {
        *time = audioGetTotoalTime();
    }
    else
    {
        *time = 0;
        /*LOG_ERROR
        "Audio total time == 0 \n"
        LOG_END*/
    }
    return nResult;
}

int smtkAudioMgrSetDuration(int nDuration)
{
    //unit : ms
    DEBUG_PRINT("smtkAudioMgrSetDuration %d \n", nDuration);
    audioMgr.nDuration = nDuration;

    return 0;
}

int smtkAudioMgrGetDuration()
{
    return audioMgr.nDuration;
}

void
smtkAudioMgrSetParsing(
    MMP_BOOL status)
{
    audioMgr.parsing = status;
}

MMP_INT
smtkAudioMgrShowSpectrum(
    MMP_INT nMode)
{
    MMP_INT nResult = 0;
    return nResult;
}
void
smtkAudioSetEqualizer(SMTK_AUDIO_EQTYPE eqType)
{}

MMP_INT
smtkAudioMgrOpenEngine(
    MMP_INT nEngineType)
{
    MMP_INT     nResult     = 0;
    MMP_UINT8   *pAddress   = MMP_NULL;
    MMP_UINT32  length;
    nResult = iteAudioOpenEngine(nEngineType);

    return nResult;
}

void smtkAudioMgrGetNetworkState(int *pType, int *pErr)
{
    if (audioMgr.ptNetwork.nError)
    {
        *pType  = SMTK_AUDIO_NETWORK_STATE_ERROR;
        *pErr   = audioMgr.ptNetwork.nError;
    }
    else if (audioMgr.ptNetwork.bEOF)
    {
        *pType = SMTK_AUDIO_NETWORK_STATE_EOF;
    }
    else if (audioMgr.ptNetwork.bEOP)
    {
        *pType = SMTK_AUDIO_NETWORK_STATE_EOP;
    }
    else
    {
        *pType = SMTK_AUDIO_NETWORK_STATE_NOT_EOF_ERROR;
    }
}

void smtkAudioMgrSetNetworkError(int nErr)
{
    DEBUG_PRINT("[Audio mgr] set NetworkError %d #line %d \n", nErr, __LINE__);
    audioMgr.ptNetwork.nError = nErr;
}

unsigned int
smtkAudioMgrGetNetworkMusicTagSize(void)
{
    return audioMgr.nNetwrokTagSize;
}

void
smtkAudioMgrSetNetworkMusicTagSize(unsigned int nSize)
{
    audioMgr.nNetwrokTagSize = nSize;
}

int
smtkAudioMgrGetI2sPostpone()
{
    return audioMgr.ptNetwork.nPostpone;
}

int
smtkAudioMgrSetI2sPostpone(int nValue)
{
    audioMgr.ptNetwork.nPostpone = nValue;
}

int smtkAudioMgrSampleRate()
{
    int nTemp;
    iteAudioGetAttrib(ITE_AUDIO_CODEC_SET_SAMPLE_RATE, &nTemp);

    return nTemp;
}

int smtkAudioMgrGetType()
{
    return audioMgr.ptNetwork.nType;
}

// for convert gb2312, not a really solution

//=============================================================================
/**
 * Convert Big5 or GB2312 to UTF-16.
 *
 * @param nEncodingType  The encoding type of source string
 * @param ptDestText     The destination UTF-16 string
 * @param ptSrcText      The source Big5/GB2312 string
 * @param nSrcLength     The length of the source Big5/GB2312 string.
 * @return      0 if they succeed, and a failure code otherwise.
 */
//=============================================================================
/*int
smtkFontMgrEncodingConverter(
    MMP_WCHAR   *ptDestText,
    MMP_UINT8   *ptSrcText,
    MMP_UINT32  nSrcLength,
    int         nEncodeType)
{
    MMP_UINT32  nResult = 0;
    MMP_UINT32  i;
    MMP_UINT32  nTemp, nTemp1;

    nEncodeType = 1;

    if (!ptDestText
        || !ptSrcText
        || nSrcLength <= 0)
        return nResult;

    switch (nEncodeType)
    {
    case 0:
    
        break;

    case 1:
        nTemp   = 0;
        nTemp1  = 0;
        for (i = 0; i < (nSrcLength); i++, nTemp1++)
        {
            if ((ptSrcText[i] >= 0x20) && (ptSrcText[i] <= 0x80))
            {
                ptDestText[nTemp1] = ptSrcText[i];
            }
            else if (gb2312_mbtowc(0, &ptDestText[nTemp1], &ptSrcText[i], 2) == 2)
            {
                i++;
                nTemp++;
            }
        }
        ptDestText[(nSrcLength) - nTemp]    = 0;
        nResult                             = 0;
        break;

    default:
        break;
    }

    return nResult;
}*/

int (*smtkAudioMgrGetCallbackFunction())(int)
{
    return audioMgr.ptNetwork.audioMgrCallback;
}

void smtkAudioMgrSetCallbackFunction(int (*pApi)(int))
{
    audioMgr.ptNetwork.audioMgrCallback = pApi;
}

char *smtkAudioMgrGetFinishName()
{
    return audioMgr.ptNetwork.pFilename;
}

void smtkAudioMgrReloadSBC(void *func)
{
#if defined(__OPENRTOS__)
    if (func)
    {
        func;
    }
#endif
}

#ifdef SMTK_AUDIO_READ_BUFFER_THREAD

static void init_audio_buffer(void)
{
    int i;
    for (i = 0; i < SMTK_AUDIO_READ_BUFFER_FRAMES; i++)
    {
        audio_buffer[i].ready   = 0;
        audio_buffer[i].data    = malloc(SMTK_AUDIO_READ_BUFFER_SIZE);
    }
    DEBUG_PRINT("[Audio mgr] init_audio_buffer #line %d \n", __LINE__);
}

static void free_audio_buffer(void)
{
    int i;

    for (i = 0; i < SMTK_AUDIO_READ_BUFFER_FRAMES; i++)
    {
        if (audio_buffer[i].data)
        {
            free(audio_buffer[i].data);
            audio_buffer[i].data = NULL;
        }
    }
    DEBUG_PRINT("[Audio mgr] free_audio_buffer #line %d \n", __LINE__);
}

static void audio_buffer_reset(void)
{
    int i;
    for (i = 0; i < SMTK_AUDIO_READ_BUFFER_FRAMES; i++)
    {
        audio_buffer[i].ready   = 0;
        audio_buffer[i].size    = 0;
        memset(audio_buffer[i].data, 0, SMTK_AUDIO_READ_BUFFER_SIZE);
    }

    audioMgr.nReadBufferStop        = 0;
    audioMgr.nCurrentReadBuffer     = 0;
    audioMgr.nCurrentWriteBuffer    = 0;

    gOutReadPointer     = 0;
    gOutWritePointer    = 0;
}

static int audio_buffer_write_complete(void)
{
    int i = 0;
    for (i = 0; i < SMTK_AUDIO_READ_BUFFER_FRAMES; i++)
        if (audio_buffer[i].ready == 1)
            return 0;

    return 1;
}

// fix Sony TX read slow problem 0 : stop to read, 1: can read
static int audio_buffer_flow_control(void)
{
    int i;
    int nFullBuffer;
    if (audioMgr.nCurrentReadBuffer >= audioMgr.nCurrentWriteBuffer)
    {
        nFullBuffer = audioMgr.nCurrentReadBuffer - audioMgr.nCurrentWriteBuffer;
    }
    else
    {
        nFullBuffer = SMTK_AUDIO_READ_BUFFER_FRAMES - (audioMgr.nCurrentWriteBuffer - audioMgr.nCurrentReadBuffer);
    }

    if (nFullBuffer >= (SMTK_AUDIO_READ_BUFFER_FRAMES / 2))
    {
        return 0;
    }
    else
    {
        if (nFullBuffer == 2)
        {
            //usleep(150*1000);
        }
        else if (nFullBuffer == 3)
        {
            //usleep(350*1000);
        }
        return 1;
    }
}

/*static void audio_buffer_read_data_from_network(int nNext)
{
    int readSize, nResult;
 //   #ifdef READ_HTTP
    if (audioMgr.bCheckMusicComplete == MMP_FALSE)
    {
        DEBUG_PRINT("audio_buffer_read_data_from_network #line %d \n", __LINE__);

        checkMusic(nNext);

        DEBUG_PRINT("audio_buffer_read_data_from_network #line %d \n", __LINE__);
    }
    else if (!audioMgr.ptNetwork.bEOF && !audioMgr.ptNetwork.nError)
    {
        // read data flow contorl
        if (audioMgr.ptNetwork.bFlowConrol == 1)
        {
            nResult = audio_buffer_flow_control();
            if (nResult == 0)
                return;
        }

        readSize                    = audioMgr.ptNetwork.nReadLength;//SMTK_AUDIO_READ_BUFFER_SIZE;
        DEBUG_PRINT("[Audio mgr] NetworkRead read %d write %d #line %d \n", audioMgr.nCurrentReadBuffer, audioMgr.nCurrentWriteBuffer);
        nResult                     = audioMgr.ptNetwork.NetworkRead(audioMgr.ptNetwork.pHandle, audio_buffer[nNext].data,  &readSize, 1800);
        audio_buffer[nNext].ready   = 1;
        audio_buffer[nNext].size    = readSize;
        DEBUG_PRINT("[Audio mgr] Network play rds=%ld, read buffer %d [%d] ,rlt=%ld\r\n", readSize, audioMgr.nCurrentReadBuffer, nNext, nResult);
        if (nResult != 0)
        {
            if (nResult != -119)
            {
                // when read data error, should stop playing.
                DEBUG_PRINT("[Audio mgr] Network play Error nResult=%ld\r\n", nResult);
                audioMgr.nReadBufferStop    = 1;
                audio_buffer[nNext].ready   = 0;
                audioMgr.ptNetwork.nError   = nResult;
                return;
            }
        }
        if (readSize == 0)
        {
            // when EOF, cannot set audioMgr.playing to FALSE, so set bEOF to TRUE to stop reading data.
            DEBUG_PRINT("[Audio mgr] Network play readSize == 0, EOF\r\n");
            audioMgr.ptNetwork.bEOF     = MMP_TRUE;
            audio_buffer[nNext].ready   = 0;
            audio_buffer[nNext].size    = readSize;
            audioMgr.nReadBufferStop    = 1;

            return;
        }
    }
    else
    {
        //DEBUG_PRINT("[Audio mgr] Network play EOF %ld\r\n", audioMgr.ptNetwork.nEOFWaitCount);
        usleep(AUDIOMGR_NETWORKPLAY_EOF_SLEEP_US);
        audioMgr.ptNetwork.nEOFWaitCount--;
        if (audioMgr.ptNetwork.nEOFWaitCount <= 0)
        {
            DEBUG_PRINT("[Audio mgr] Network play, ControlPoint seems disapper, so set the player to stop\r\n");
            audioMgr.nReadBufferStop = 1;
        }
    }
}*/

static void audio_buffer_read_data_from_local(int nNext)
{
    int readSize, nResult;

    int i = 0;
    if (audioMgr.bCheckMusicComplete == MMP_FALSE)
    {
        DEBUG_PRINT("audio_buffer_read_data_from_local #line %d \n", __LINE__);

        checkMusic_Local(nNext);

    }
    else if (!audioMgr.ptNetwork.bEOF && !audioMgr.ptNetwork.nError)
    {
        readSize                    = audioMgr.ptNetwork.nReadLength;//SMTK_AUDIO_READ_BUFFER_SIZE;
        nResult                     = audioMgr.ptNetwork.LocalRead(audio_buffer[nNext].data, 1, SMTK_AUDIO_READ_BUFFER_SIZE, audioMgr.ptNetwork.pHandle);
        readSize                    = nResult;
        audio_buffer[nNext].ready   = 1;
        audio_buffer[nNext].size    = nResult;
        if ((MMP_UINT) nResult != SMTK_AUDIO_READ_BUFFER_SIZE)
        {
            if (smtkAudioMgrInterruptSoundGetRemoveCard() == 1)
            {
                i = 0;
                do
                {
                    usleep(200000);
                    i++;
                } while (smtkAudioMgrInterruptSoundGetRemoveCard() == 1 && i <= 40);
                DEBUG_PRINT("[Audio mgr] local eof remove card %d %d #line %d \n", i, nResult, __LINE__);
            }

            audioMgr.nReadBufferStop = 1;
        }
        #if defined(AUDIO_MGR_DUMP)

        if (SMTK_AUDIO_READ_BUFFER_SIZE != fwrite(audio_buffer[nNext].data, sizeof(char), SMTK_AUDIO_READ_BUFFER_SIZE, fout))
        {
            DEBUG_PRINT("[Audio mgr]Can not write audio output file \n");
        }

        #endif
    }
    else
    {
        //DEBUG_PRINT("[Audio mgr] Network play EOF %ld\r\n", audioMgr.ptNetwork.nEOFWaitCount);
        usleep(AUDIOMGR_NETWORKPLAY_EOF_SLEEP_US);
        audioMgr.ptNetwork.nEOFWaitCount--;
        if (audioMgr.ptNetwork.nEOFWaitCount <= 0)
        {
            DEBUG_PRINT("[Audio mgr] Network play, ControlPoint seems disapper, so set the player to stop\r\n");
            audioMgr.nReadBufferStop = 1;
        }
    }
}

/*static int checkNetworkRead(int nReadSize, int nResult)
{
    if (nResult != 0)
    {
        // when read data error, should stop playing.
        DEBUG_PRINT("[Audio mgr] Network play Error nResult=%ld, #line %d   \r\n", nResult, __LINE__);
        audioMgr.nReadBufferStop    = 1;
        audioMgr.ptNetwork.nError   = nResult;
        return 1;
    }
    if (nReadSize == 0)
    {
        // EOF
        DEBUG_PRINT("[Audio mgr] Network play readSize == 0, #line %d, EOF \r\n", __LINE__);
        audioMgr.ptNetwork.bEOF     = MMP_TRUE;
        //audio_buffer[nNext].ready = 0;
        //audio_buffer[nNext].size = readSize;
        audioMgr.nReadBufferStop    = 1;
        return 1;
    }
    return 0;
}*/
/*
static void checkMusic(int nNext)
{
    if (audioMgr.ptNetwork.nType == SMTK_AUDIO_MP3)
    {
        checkMusic_MP3(nNext);
        audioMgr.bCheckMusicComplete = MMP_TRUE;
    }
    else if (audioMgr.ptNetwork.nType == SMTK_AUDIO_WAV)
    {
        checkMusic_WAV(nNext);
        audioMgr.bCheckMusicComplete = MMP_TRUE;
    }
    else
    {
        audioMgr.bCheckMusicComplete = MMP_TRUE;
    }
}*/

static void checkMusic_Local(int nNext)
{
    //DEBUG_PRINT("[Audio mgr] checkMusic_Local #line %d \n",__LINE__);
    if (audioMgr.ptNetwork.nType == SMTK_AUDIO_MP3)
    {
        checkMusic_MP3_Local(nNext);
        audioMgr.bCheckMusicComplete = MMP_TRUE;
    }
    else if (audioMgr.ptNetwork.nType == SMTK_AUDIO_WAV)
    {
        checkMusic_WAV_Local(nNext);
        audioMgr.bCheckMusicComplete = MMP_TRUE;
    }
    else
    {
        audioMgr.bCheckMusicComplete = MMP_TRUE;
    }
}

/*static void checkMusic_MP3(int nNext)
{
    int readSize, nResult;
    int nTagData;
    readSize    = SMTK_AUDIO_READ_BUFFER_SIZE;
    memset(audioMgr.sampleBuf, 0, SMTK_AUDIO_READ_BUFFER_SIZE);
    nResult     = audioMgr.ptNetwork.NetworkRead(audioMgr.ptNetwork.pHandle, audioMgr.sampleBuf,  &readSize, 1800);
    if (checkNetworkRead(readSize, nResult))
        return;

    if (audioIsId3V2Tag_Buffer(audioMgr.sampleBuf, &nTagData))
    {
        // pass id3 tag
        if (nTagData <= SMTK_AUDIO_READ_BUFFER_SIZE)
        {
            memcpy(audio_buffer[nNext].data, audioMgr.sampleBuf + nTagData, SMTK_AUDIO_READ_BUFFER_SIZE - nTagData);
            audio_buffer[nNext].ready   = 1;
            audio_buffer[nNext].size    = SMTK_AUDIO_READ_BUFFER_SIZE - nTagData;
            DEBUG_PRINT("[Audio mgr]checkMusic_MP3 0x%X 0x%x %d %d #line %d \n", audio_buffer[nNext].data[0], audio_buffer[nNext].data[1], nNext, SMTK_AUDIO_READ_BUFFER_SIZE - nTagData, __LINE__);
        }
        else
        {
            do
            {
                nTagData    -= SMTK_AUDIO_READ_BUFFER_SIZE;
                DEBUG_PRINT("[Audio mgr] nTagData %d #line %d\n", nTagData, __LINE__);
                readSize    = SMTK_AUDIO_READ_BUFFER_SIZE;
                nResult     = audioMgr.ptNetwork.NetworkRead(audioMgr.ptNetwork.pHandle, audioMgr.sampleBuf,  &readSize, 1800);
                if (checkNetworkRead(readSize, nResult))
                    return;

                if (audioMgr.playing == MMP_FALSE)
                {
                    DEBUG_PRINT("[Audio mgr] checkMusic_MP3 stop #line %d\n", __LINE__);
                    return;
                }
                usleep(5000);
            } while (nTagData > SMTK_AUDIO_READ_BUFFER_SIZE);
            DEBUG_PRINT("[Audio mgr] nTagData %d #line %d\n", nTagData, __LINE__);

            memcpy(audio_buffer[nNext].data, audioMgr.sampleBuf + nTagData, SMTK_AUDIO_READ_BUFFER_SIZE - nTagData);
            audio_buffer[nNext].ready   = 1;
            audio_buffer[nNext].size    = SMTK_AUDIO_READ_BUFFER_SIZE - nTagData;
            DEBUG_PRINT("[Audio mgr]checkMusic_MP3 0x%X 0x%x %d %d #line %d \n", audio_buffer[nNext].data[0], audio_buffer[nNext].data[1], nNext, SMTK_AUDIO_READ_BUFFER_SIZE - nTagData, __LINE__);
        }
    }
    else
    {
        DEBUG_PRINT("[Audio mgr] checkMusic_MP3 has no id3 v2 size \n");
        audio_buffer[nNext].ready   = 1;
        audio_buffer[nNext].size    = SMTK_AUDIO_READ_BUFFER_SIZE;
        memcpy(audio_buffer[nNext].data, audioMgr.sampleBuf, SMTK_AUDIO_READ_BUFFER_SIZE);
    }
}*/

static void checkMusic_MP3_Local(int nNext)
{
    int readSize, nResult;
    int nTagData;
    readSize    = SMTK_AUDIO_READ_BUFFER_SIZE;
    memset(audioMgr.sampleBuf, 0, SMTK_AUDIO_READ_BUFFER_SIZE);
    nResult     = audioMgr.ptNetwork.LocalRead(audioMgr.sampleBuf, 1, SMTK_AUDIO_READ_BUFFER_SIZE, audioMgr.ptNetwork.pHandle);

    if (audioIsId3V2Tag_Buffer(audioMgr.sampleBuf, &nTagData))
    {
        // pass id3 tag
        if (nTagData <= SMTK_AUDIO_READ_BUFFER_SIZE)
        {
            memcpy(audio_buffer[nNext].data, audioMgr.sampleBuf + nTagData, SMTK_AUDIO_READ_BUFFER_SIZE - nTagData);
            audio_buffer[nNext].ready   = 1;
            audio_buffer[nNext].size    = SMTK_AUDIO_READ_BUFFER_SIZE - nTagData;
            DEBUG_PRINT("[Audio mgr]checkMusic_MP3 0x%X 0x%x %d %d #line %d \n", audio_buffer[nNext].data[0], audio_buffer[nNext].data[1], nNext, SMTK_AUDIO_READ_BUFFER_SIZE - nTagData, __LINE__);
        }
        else
        {
            do
            {
                nTagData    -= SMTK_AUDIO_READ_BUFFER_SIZE;
                DEBUG_PRINT("[Audio mgr] nTagData %d #line %d\n", nTagData, __LINE__);
                readSize    = SMTK_AUDIO_READ_BUFFER_SIZE;
                nResult     = audioMgr.ptNetwork.LocalRead(audioMgr.sampleBuf, 1, SMTK_AUDIO_READ_BUFFER_SIZE, audioMgr.ptNetwork.pHandle);

                if (audioMgr.playing == MMP_FALSE)
                {
                    DEBUG_PRINT("[Audio mgr] checkMusic_MP3 stop #line %d\n", __LINE__);
                    return;
                }
                usleep(5000);
            } while (nTagData > SMTK_AUDIO_READ_BUFFER_SIZE);
            DEBUG_PRINT("[Audio mgr] nTagData %d #line %d\n", nTagData, __LINE__);

            memcpy(audio_buffer[nNext].data, audioMgr.sampleBuf + nTagData, SMTK_AUDIO_READ_BUFFER_SIZE - nTagData);
            audio_buffer[nNext].ready   = 1;
            audio_buffer[nNext].size    = SMTK_AUDIO_READ_BUFFER_SIZE - nTagData;
            DEBUG_PRINT("[Audio mgr]checkMusic_MP3 0x%X 0x%x %d %d #line %d \n", audio_buffer[nNext].data[0], audio_buffer[nNext].data[1], nNext, SMTK_AUDIO_READ_BUFFER_SIZE - nTagData, __LINE__);
        }
    }
    else
    {
        DEBUG_PRINT("[Audio mgr] checkMusic_MP3 has no id3 v2 size \n");
        audio_buffer[nNext].ready   = 1;
        audio_buffer[nNext].size    = SMTK_AUDIO_READ_BUFFER_SIZE;
        memcpy(audio_buffer[nNext].data, audioMgr.sampleBuf, SMTK_AUDIO_READ_BUFFER_SIZE);
    }
}


/*static void checkMusic_WAV(int nNext)
{
    int             readSize, nResult, nOffset;
    ITE_WaveInfo    tWavInfo;
    int             nDropSize   = 0;
    int             nDrops      = 0;

    DEBUG_PRINT("[Audio mgr] checkMusic_WAV #line %d \n", __LINE__);

 //   #ifdef READ_HTTP
    // read music
    readSize                    = SMTK_AUDIO_READ_BUFFER_SIZE;
    nResult                     = audioMgr.ptNetwork.NetworkRead(audioMgr.ptNetwork.pHandle, audio_buffer[nNext].data, &readSize, 1800);
    audio_buffer[nNext].ready   = 1;
    audio_buffer[nNext].size    = nResult;
    if (checkNetworkRead(readSize, nResult))
        return;

    // check wav header
    memcpy(ptWaveHeader, audio_buffer[nNext].data, 48);
    if (!(ptWaveHeader[0] == 'R' && ptWaveHeader[1] == 'I' && ptWaveHeader[2] == 'F' && ptWaveHeader[3] == 'F' &&
          ptWaveHeader[8] == 'W' && ptWaveHeader[9] == 'A' && ptWaveHeader[10] == 'V' && ptWaveHeader[11] == 'E'))
    {
        DEBUG_PRINT("[Audio mgr]not a wav file %c%c%c%c , %c%c%c%c  , %d \n", ptWaveHeader[0], ptWaveHeader[1], ptWaveHeader[2], ptWaveHeader[3], ptWaveHeader[8], ptWaveHeader[9], ptWaveHeader[10], ptWaveHeader[11], __LINE__);
        audioMgr.nReadBufferStop = 1;
        return;
    }
    if (ptWaveHeader[12] == 'L' && ptWaveHeader[13] == 'I' && ptWaveHeader[14] == 'S' && ptWaveHeader[15] == 'T')
    {
        nOffset = 16;
        memcpy(ptWaveHeader, &audio_buffer[nNext].data[nOffset], 4);
        nOffset = (unsigned int)ENDIAN_LE32(&ptWaveHeader[0]);
        DEBUG_PRINT("[audio mgr] check wav list %d \n", nOffset);
        if (nOffset > SMTK_AUDIO_READ_BUFFER_SIZE)
            return;
        nOffset                     += 20;
        memcpy(ptWaveHeader, &audio_buffer[nNext].data[nOffset], 48);
        DEBUG_PRINT("[audio mgr] check list %d %d %d \n",ptWaveHeader[0],ptWaveHeader[1],ptWaveHeader[2]);
        gtWaveInfo.format           = (SMTK_AUIDO_RECORD_WAVE_FORMAT)ENDIAN_LE16(&ptWaveHeader[8]);
        gtWaveInfo.nChans           = (unsigned int)ptWaveHeader[10];
        gtWaveInfo.sampRate         = (unsigned int)ENDIAN_LE32(&ptWaveHeader[12]);
        gtWaveInfo.avgBytesPerSec   = (unsigned int)ENDIAN_LE32(&ptWaveHeader[16]);
        gtWaveInfo.blockAlign       = (unsigned int)ENDIAN_LE32(&ptWaveHeader[20]);
        gtWaveInfo.bitsPerSample    = (unsigned int)ptWaveHeader[22];
        tWavInfo.nChans             = gtWaveInfo.nChans;
        tWavInfo.sampRate           = gtWaveInfo.sampRate;
        tWavInfo.bitsPerSample      = gtWaveInfo.bitsPerSample;
    }
    else if (!(ptWaveHeader[12] == 'f' && ptWaveHeader[13] == 'm' && ptWaveHeader[14] == 't'))
    {
        DEBUG_PRINT("[Audio mgr] not a wav file %c%c%c ,#line %d \n", ptWaveHeader[12], ptWaveHeader[13], ptWaveHeader[14], __LINE__);
        audioMgr.nReadBufferStop = 1;
        return;
    }
    else
    {
        gtWaveInfo.format           = (SMTK_AUIDO_RECORD_WAVE_FORMAT)ENDIAN_LE16(&ptWaveHeader[20]);
        gtWaveInfo.nChans           = (unsigned int)ptWaveHeader[22];
        gtWaveInfo.sampRate         = (unsigned int)ENDIAN_LE32(&ptWaveHeader[24]);
        gtWaveInfo.avgBytesPerSec   = (unsigned int)ENDIAN_LE32(&ptWaveHeader[28]);
        gtWaveInfo.blockAlign       = (unsigned int)ENDIAN_LE32(&ptWaveHeader[32]);
        gtWaveInfo.bitsPerSample    = (unsigned int)ptWaveHeader[34];
    }
    DEBUG_PRINT("[Audio mgr] waveInfo: format %d (%s) \n", gtWaveInfo.format,
                gtWaveInfo.format == SMTK_AUIDO_RECORD_WAVE_FORMAT_ALAW  ? "aLaw"  :
                gtWaveInfo.format == SMTK_AUIDO_RECORD_WAVE_FORMAT_MULAW ? "uLaw"  :
                gtWaveInfo.format == SMTK_AUIDO_RECORD_WAVE_FORMAT_PCM ? "PCM"   :
                gtWaveInfo.format == SMTK_AUIDO_RECORD_WAVE_FORMAT_DVI_ADPCM ? "ADPCM" : "Unknown");

    if (!(gtWaveInfo.format == SMTK_AUIDO_RECORD_WAVE_FORMAT_ALAW || gtWaveInfo.format == SMTK_AUIDO_RECORD_WAVE_FORMAT_MULAW
          || gtWaveInfo.format == SMTK_AUIDO_RECORD_WAVE_FORMAT_PCM || gtWaveInfo.format == SMTK_AUIDO_RECORD_WAVE_FORMAT_DVI_ADPCM))
    {
        DEBUG_PRINT("[Audio mgr] wav unknown format %d #line %d \n", gtWaveInfo.format, __LINE__);
        audioMgr.nReadBufferStop = 1;
        return;
    }

    DEBUG_PRINT("[Audio mgr]format %d , channels %d , sampling rate %d , avgBytesPerSec %d , blockAlign %d , bitsPerSample %d \n", gtWaveInfo.format
                , gtWaveInfo.nChans, gtWaveInfo.sampRate, gtWaveInfo.avgBytesPerSec, gtWaveInfo.blockAlign, gtWaveInfo.bitsPerSample);

    if (gtWaveInfo.format == SMTK_AUIDO_RECORD_WAVE_FORMAT_DVI_ADPCM)
    {
        if (gtWaveInfo.bitsPerSample != 4)
        {
            DEBUG_PRINT("[Audio mgr] wav unsupport adpcm bitsPerSample %d #line %d \n", gtWaveInfo.bitsPerSample, __LINE__);
            audioMgr.nReadBufferStop = 1;
            return;
        }
        gtWaveInfo.samplesPerBlock  = (unsigned int)ENDIAN_LE16(&ptWaveHeader[38]);
        gnWavByte                   = ((gtWaveInfo.samplesPerBlock - 1) / 2 + 4) * gtWaveInfo.nChans;
        DEBUG_PRINT("[Audio mgr] samplesPerBlock %d  wavByte %d \n", gtWaveInfo.samplesPerBlock, gnWavByte);
    }

    if (gtWaveInfo.nChans > 2 || gtWaveInfo.nChans == 0)
    {
        DEBUG_PRINT("[Audio Mgr] wav unsupported channels %d #line %d \n", gtWaveInfo.nChans, __LINE__);
        audioMgr.nReadBufferStop = 1;
        return;
    }
    switch (gtWaveInfo.format)
    {
    case SMTK_AUIDO_RECORD_WAVE_FORMAT_PCM:
        tWavInfo.format = ITE_WAVE_FORMAT_PCM;
        break;

    case SMTK_AUIDO_RECORD_WAVE_FORMAT_ALAW:
        tWavInfo.format = ITE_WAVE_FORMAT_ALAW;
        break;

    case SMTK_AUIDO_RECORD_WAVE_FORMAT_MULAW:
        tWavInfo.format = ITE_WAVE_FORMAT_MULAW;
        break;

    case SMTK_AUIDO_RECORD_WAVE_FORMAT_ADPCM:
        tWavInfo.format = ITE_WAVE_FORMAT_DVI_ADPCM;
        break;

    default:
        tWavInfo.format = ITE_WAVE_FORMAT_PCM;
        break;
    }

    tWavInfo.bitsPerSample  = gtWaveInfo.bitsPerSample;
    tWavInfo.nChans         = gtWaveInfo.nChans;
    tWavInfo.sampRate       = gtWaveInfo.sampRate;
    // save data
    iteAudioSetWaveDecodeHeader(tWavInfo);
}*/

// return 1:reset hw, 0 : same special case,not reset hw
static int audio_special_case_check_state(int ch, int sampleRate, int length)
{
    int nI2S                = 0;
    int nSampleRate;
    int nCh                 = 0;
    int nI2SBufferLength    = 0;
    int nResult             = 1;

    nI2S = i2s_get_DA_running();

    if (nI2S)
    {
        iteAudioGetAttrib(  ITE_AUDIO_CODEC_SET_CHANNEL,        &nCh);
        iteAudioGetAttrib(  ITE_AUDIO_CODEC_SET_SAMPLE_RATE,    &nSampleRate);
        iteAudioGetAttrib(  ITE_AUDIO_CODEC_SET_BUFFER_LENGTH,  &nI2SBufferLength);

        //DEBUG_PRINT(" ch %d  %d %d 0x%x\n ",nCh,nSampleRate,nI2SBufferLength);
    }

    if (nCh == ch && nSampleRate == sampleRate && nI2SBufferLength == length)
    {
        DEBUG_PRINT("[Audio mgr] special case same condition , not reset #line %d \n", __LINE__);
        nResult = 0;
    }

    return nResult;
}

static int audio_special_case_play()
{
    #if defined(__OPENRTOS__)
    unsigned char   *I2SBuf;
    int             nChannels;
    int             nSampeRate;
    int             nBufferLength;
    int             nTemp = 1;
    unsigned int    *pBuf;
    unsigned int    nLength;
    STRC_I2S_SPEC   spec;
    int             i2sNewWP    = 0;
    int             nOffset     = 0;
    // 128k reference from mediastream2.c castor3snd_init()
    //gOutBuffer = memalign(64,128*1024);

    I2SBuf          = gOutBuffer;
    nChannels       = gtWaveInfo.nChans;
    nSampeRate      = gtWaveInfo.sampRate;
    nBufferLength   = SMTK_AUDIO_READ_BUFFER_SIZE;
    if (audioMgr.nInit == 0)
    {
        DEBUG_PRINT("[Audio mgr] audio mgr has terminated , return #line %d \n", __LINE__);
        return 0;
    }
    DEBUG_PRINT("[Audio Mgr]  audio_special_case_play 0x%x  %d %d %d %d #line %d \n", I2SBuf, nChannels, nSampeRate, nBufferLength, __LINE__);
    if (nChannels > 2 || nChannels <= 0)
    {
        DEBUG_PRINT("[Audio Mgr]  Error I2SInitDac 0x%x  %d %d %d \n", I2SBuf, nChannels, nSampeRate, nBufferLength);
        smtkAudioMgrSetNetworkError(1);
        return 0;
    }
    if (nSampeRate > 192000 || nSampeRate < 8000)
    {
        DEBUG_PRINT("[Audio Mgr]  Error I2SInitDac 0x%x  %d %d %d \n", I2SBuf, nChannels, nSampeRate, nBufferLength);
        smtkAudioMgrSetNetworkError(1);
        return 0;
    }

    nTemp = audio_special_case_check_state(nChannels, nSampeRate, nBufferLength);
    if (nTemp == 1 || !i2s_get_DA_running())
    {
        /* init I2S */
        //if (i2s_get_DA_running()) i2s_deinit_DAC();
        
        memset( I2SBuf, 0,  nBufferLength);
        memset( &spec,  0,  sizeof(spec));
        spec.channels                   = nChannels;
        spec.sample_rate                = nSampeRate;
        spec.buffer_size                = nBufferLength;
        spec.is_big_endian              = 0;
        spec.base_i2s                   = I2SBuf;

        spec.sample_size                = gtWaveInfo.bitsPerSample;
        spec.num_hdmi_audio_buffer      = 1;
        spec.is_dac_spdif_same_buffer   = 1;
        spec.enable_HeadPhone           = 1;
        spec.enable_Speaker             = 1;

        spec.base_hdmi[0]               = I2SBuf;
        spec.base_hdmi[1]               = I2SBuf;
        spec.base_hdmi[2]               = I2SBuf;
        spec.base_hdmi[3]               = I2SBuf;

        spec.base_spdif                 = I2SBuf;

        i2s_init_DAC(&spec);

        iteAudioSetAttrib(  ITE_AUDIO_CODEC_SET_SAMPLE_RATE,    &nSampeRate);
        iteAudioSetAttrib(  ITE_AUDIO_CODEC_SET_CHANNEL,        &nChannels);
        iteAudioSetAttrib(  ITE_AUDIO_CODEC_SET_BUFFER_LENGTH,  &nBufferLength);
        iteAudioSetAttrib(  ITE_AUDIO_I2S_PTR,                  I2SBuf);
    }
    else
    {
        //already init i2s, just open i2s
        i2s_pause_DAC(0);
    }
    nOffset = 0;
    if (i2s_get_DA_running() && audioMgr.ptNetwork.nSpecialCase == 1)
    {
        unsigned char   *dstBuffer  = NULL;
        int             copySize    = 0;
        //int dummysize = 256;

        I2S_DA32_SET_WP(I2S_DA32_GET_RP());
        gOutReadPointer     = I2S_DA32_GET_RP();
        gOutWritePointer    = I2S_DA32_GET_WP();
        dstBuffer           = gOutBuffer + gOutReadPointer;
        i2sNewWP            = gOutReadPointer;

        if (gOutWritePointer /*+ dummysize */ + audioMgr.ptNetwork.nLocalFileSize > nBufferLength)
            memset(I2SBuf, 0, nBufferLength); //clear I2S buffer data

        I2S_DA32_SET_WP(i2sNewWP);
        DEBUG_PRINT("[Audio mgr] audio_special_case_play da %d ,read pointer %d write pointer %d nLocalFileSize=%d\n", 128 * 1024, gOutReadPointer, gOutWritePointer, audioMgr.ptNetwork.nLocalFileSize);

        if (i2sNewWP + audioMgr.ptNetwork.nLocalFileSize <= nBufferLength)
        {
            dstBuffer   = gOutBuffer + i2sNewWP;
            copySize    = audioMgr.ptNetwork.nLocalFileSize;
            i2sNewWP    = i2sNewWP + audioMgr.ptNetwork.nLocalFileSize - SMTK_AUDIO_WAV_HEADER;
        }
        else if (i2sNewWP + audioMgr.ptNetwork.nLocalFileSize > nBufferLength)
        {
            dstBuffer   = gOutBuffer + i2sNewWP;
            copySize    = nBufferLength - i2sNewWP;
            memcpy(dstBuffer, audioMgr.sampleBuf + SMTK_AUDIO_WAV_HEADER, copySize);

            nOffset     = copySize;

            dstBuffer   = gOutBuffer;

            copySize    = audioMgr.ptNetwork.nLocalFileSize - (nBufferLength - i2sNewWP);
            i2sNewWP    = copySize;
        }
        else
        {
            DEBUG_PRINT("[Audio mgr] special case error %d \n", __LINE__);
        }

        if (copySize > 44)
        {
            memcpy(dstBuffer, audioMgr.sampleBuf + SMTK_AUDIO_WAV_HEADER + nOffset, copySize - SMTK_AUDIO_WAV_HEADER);
        #if CFG_CPU_WB
            ithFlushDCacheRange((void*)dstBuffer, copySize);
            ithFlushMemBuffer();
        #endif
            I2S_DA32_SET_WP(i2sNewWP);
        }

    }
    #endif

    audioMgr.bCheckMusicComplete = MMP_TRUE;//
    return 0;
}

static void checkMusic_WAV_Local(int nNext)
{
    int             readSize, nResult, nOffset;
    ITE_WaveInfo    tWavInfo;

    DEBUG_PRINT("[Audio mgr] checkMusic_WAV_Local specail case %d #line %d \n", audioMgr.ptNetwork.nSpecialCase, __LINE__);

    //#ifdef READ_HTTP
    // read music
    readSize                    = SMTK_AUDIO_READ_BUFFER_SIZE;
    nResult                     = audioMgr.ptNetwork.LocalRead(audioMgr.sampleBuf, 1, SMTK_AUDIO_READ_BUFFER_SIZE, audioMgr.ptNetwork.pHandle);
    memcpy(audio_buffer[nNext].data, audioMgr.sampleBuf, SMTK_AUDIO_READ_BUFFER_SIZE);
    audio_buffer[nNext].ready   = 1;
    audio_buffer[nNext].size    = nResult;


    // check wav header
    memcpy(ptWaveHeader, audio_buffer[nNext].data, 48);
    if (!(ptWaveHeader[0] == 'R' && ptWaveHeader[1] == 'I' && ptWaveHeader[2] == 'F' && ptWaveHeader[3] == 'F' &&
          ptWaveHeader[8] == 'W' && ptWaveHeader[9] == 'A' && ptWaveHeader[10] == 'V' && ptWaveHeader[11] == 'E'))
    {
        DEBUG_PRINT("[Audio mgr]not a wav file %c%c%c%c , %c%c%c%c  , %d \n", ptWaveHeader[0], ptWaveHeader[1], ptWaveHeader[2], ptWaveHeader[3], ptWaveHeader[8], ptWaveHeader[9], ptWaveHeader[10], ptWaveHeader[11], __LINE__);
        audioMgr.nReadBufferStop = 1;
        return;
    }
    if (ptWaveHeader[12] == 'L' && ptWaveHeader[13] == 'I' && ptWaveHeader[14] == 'S' && ptWaveHeader[15] == 'T')
    {
        nOffset = 16;
        memcpy(ptWaveHeader, &audio_buffer[nNext].data[nOffset], 4);
        nOffset = (unsigned int)ENDIAN_LE32(&ptWaveHeader[0]);
        DEBUG_PRINT("[audio mgr] check wav list %d \n", nOffset);
        if (nOffset > SMTK_AUDIO_READ_BUFFER_SIZE)
            return;
        nOffset                     += 20;
        memcpy(ptWaveHeader, &audio_buffer[nNext].data[nOffset], 48);
        DEBUG_PRINT("[audio mgr] check list %d %d %d \n",ptWaveHeader[0],ptWaveHeader[1],ptWaveHeader[2]);
        gtWaveInfo.format           = (SMTK_AUIDO_RECORD_WAVE_FORMAT)ENDIAN_LE16(&ptWaveHeader[8]);
        gtWaveInfo.nChans           = (unsigned int)ptWaveHeader[10];
        gtWaveInfo.sampRate         = (unsigned int)ENDIAN_LE32(&ptWaveHeader[12]);
        gtWaveInfo.avgBytesPerSec   = (unsigned int)ENDIAN_LE32(&ptWaveHeader[16]);
        gtWaveInfo.blockAlign       = (unsigned int)ENDIAN_LE32(&ptWaveHeader[20]);
        gtWaveInfo.bitsPerSample    = (unsigned int)ptWaveHeader[22];
        tWavInfo.nChans             = gtWaveInfo.nChans;
        tWavInfo.sampRate           = gtWaveInfo.sampRate;
        tWavInfo.bitsPerSample      = gtWaveInfo.bitsPerSample;
    }
    else if (!(ptWaveHeader[12] == 'f' && ptWaveHeader[13] == 'm' && ptWaveHeader[14] == 't'))
    {
        DEBUG_PRINT("[Audio mgr] not a wav file %c%c%c ,#line %d \n", ptWaveHeader[12], ptWaveHeader[13], ptWaveHeader[14], __LINE__);
        audioMgr.nReadBufferStop = 1;
        return;
    }
    else
    {
        gtWaveInfo.format           = (SMTK_AUIDO_RECORD_WAVE_FORMAT)ENDIAN_LE16(&ptWaveHeader[20]);
        gtWaveInfo.nChans           = (unsigned int)ptWaveHeader[22];
        gtWaveInfo.sampRate         = (unsigned int)ENDIAN_LE32(&ptWaveHeader[24]);
        gtWaveInfo.avgBytesPerSec   = (unsigned int)ENDIAN_LE32(&ptWaveHeader[28]);
        gtWaveInfo.blockAlign       = (unsigned int)ENDIAN_LE32(&ptWaveHeader[32]);
        gtWaveInfo.bitsPerSample    = (unsigned int)ptWaveHeader[34];
    }
    DEBUG_PRINT("[Audio mgr] waveInfo: format %d (%s) \n", gtWaveInfo.format,
                gtWaveInfo.format == SMTK_AUIDO_RECORD_WAVE_FORMAT_ALAW  ? "aLaw"  :
                gtWaveInfo.format == SMTK_AUIDO_RECORD_WAVE_FORMAT_MULAW ? "uLaw"  :
                gtWaveInfo.format == SMTK_AUIDO_RECORD_WAVE_FORMAT_PCM ? "PCM"   :
                gtWaveInfo.format == SMTK_AUIDO_RECORD_WAVE_FORMAT_DVI_ADPCM ? "ADPCM" : "Unknown");

    if (!(gtWaveInfo.format == SMTK_AUIDO_RECORD_WAVE_FORMAT_ALAW || gtWaveInfo.format == SMTK_AUIDO_RECORD_WAVE_FORMAT_MULAW
          || gtWaveInfo.format == SMTK_AUIDO_RECORD_WAVE_FORMAT_PCM || gtWaveInfo.format == SMTK_AUIDO_RECORD_WAVE_FORMAT_DVI_ADPCM))
    {
        DEBUG_PRINT("[Audio mgr] wav unknown format %d #line %d \n", gtWaveInfo.format, __LINE__);
        audioMgr.nReadBufferStop = 1;
        return;
    }

    DEBUG_PRINT("[Audio mgr]format %d , channels %d , sampling rate %d , avgBytesPerSec %d , blockAlign %d , bitsPerSample %d \n", gtWaveInfo.format
                , gtWaveInfo.nChans, gtWaveInfo.sampRate, gtWaveInfo.avgBytesPerSec, gtWaveInfo.blockAlign, gtWaveInfo.bitsPerSample);

    if (gtWaveInfo.format == SMTK_AUIDO_RECORD_WAVE_FORMAT_DVI_ADPCM)
    {
        if (gtWaveInfo.bitsPerSample != 4)
        {
            DEBUG_PRINT("[Audio mgr] wav unsupport adpcm bitsPerSample %d #line %d \n", gtWaveInfo.bitsPerSample, __LINE__);
            audioMgr.nReadBufferStop = 1;
            return;
        }
        gtWaveInfo.samplesPerBlock  = (unsigned int)ENDIAN_LE16(&ptWaveHeader[38]);
        gnWavByte                   = ((gtWaveInfo.samplesPerBlock - 1) / 2 + 4) * gtWaveInfo.nChans;
        DEBUG_PRINT("[Audio mgr] samplesPerBlock %d  wavByte %d \n", gtWaveInfo.samplesPerBlock, gnWavByte);
    }

    if (gtWaveInfo.nChans > 2 || gtWaveInfo.nChans == 0)
    {
        DEBUG_PRINT("[Audio Mgr] wav unsupported channels %d #line %d \n", gtWaveInfo.nChans, __LINE__);
        audioMgr.nReadBufferStop = 1;
        return;
    }
    switch (gtWaveInfo.format)
    {
    case SMTK_AUIDO_RECORD_WAVE_FORMAT_PCM:
        tWavInfo.format = ITE_WAVE_FORMAT_PCM;
        break;

    case SMTK_AUIDO_RECORD_WAVE_FORMAT_ALAW:
        tWavInfo.format = ITE_WAVE_FORMAT_ALAW;
        break;

    case SMTK_AUIDO_RECORD_WAVE_FORMAT_MULAW:
        tWavInfo.format = ITE_WAVE_FORMAT_MULAW;
        break;

    case SMTK_AUIDO_RECORD_WAVE_FORMAT_ADPCM:
        tWavInfo.format = ITE_WAVE_FORMAT_DVI_ADPCM;
        break;

    default:
        tWavInfo.format = ITE_WAVE_FORMAT_PCM;
        break;
    }

    tWavInfo.bitsPerSample  = gtWaveInfo.bitsPerSample;
    tWavInfo.nChans         = gtWaveInfo.nChans;
    tWavInfo.sampRate       = gtWaveInfo.sampRate;
    // save data
    iteAudioSetWaveDecodeHeader(tWavInfo);

    if (audioMgr.ptNetwork.nSpecialCase == 1)
    {
        audio_special_case_play();
    }

}

static void audio_buffer_read_data(void)
{
    int i;
    int nNext;
    int nRead = -1;
    int nResult;
    // check next empty buffer
    if (audio_buffer[audioMgr.nCurrentReadBuffer].ready == 0)
    {
        nNext = audioMgr.nCurrentReadBuffer;
    }
    else
    {
        nNext = audioMgr.nCurrentReadBuffer + 1;
    }
    if (nNext >= SMTK_AUDIO_READ_BUFFER_FRAMES)
        nNext = 0;
    if (nNext < 0)
    {
        DEBUG_PRINT("[Audio mgr] audio_buffer_read_data nNext %d <0 #line %d \n", nNext, __LINE__);
        nNext = 0;
    }
    if (audio_buffer[nNext].ready == 0)
    {
        nRead = nNext;
    }
    else
    {
        // no empty buffer
        DEBUG_PRINT("[Audio mgr] audio_buffer_read_data no empty buffer %d #line %d \n",nNext,__LINE__);
        usleep(20000);
        return;
    }

    //read data
    pthread_mutex_lock(&audio_read_buffer_mutex);

    if (audioMgr.filePlay == MMP_TRUE && audioMgr.nReadBufferStop == 0)
    {
        memset(audio_buffer[nNext].data, 0, SMTK_AUDIO_READ_BUFFER_SIZE);
        nResult                     = fread(audio_buffer[nNext].data, 1, SMTK_AUDIO_READ_BUFFER_SIZE, currFile);
        audio_buffer[nNext].ready   = 1;
        audio_buffer[nNext].size    = nResult;
        if ((MMP_UINT) nResult != SMTK_AUDIO_READ_BUFFER_SIZE)
        {
            audioMgr.nReadBufferStop = 1;
        }
    }
    /*else if (audioMgr.bNetworkPlay == MMP_TRUE && audioMgr.nReadBufferStop == 0 && audioMgr.ptNetwork.bLocalPlay == MMP_FALSE)
    {
        audio_buffer_read_data_from_network(nNext);
    }*/
    else if (audioMgr.bNetworkPlay == MMP_TRUE && audioMgr.nReadBufferStop == 0 && audioMgr.ptNetwork.bLocalPlay == MMP_TRUE)
    {
        audio_buffer_read_data_from_local(nNext);
    }
    else if (audioMgr.nReadBufferStop == 1)
    {
        pthread_mutex_unlock(&audio_read_buffer_mutex);
        return;
    }
    audioMgr.nCurrentReadBuffer = nNext;
    pthread_mutex_unlock(&audio_read_buffer_mutex);
}

static void *audio_read_thread_func(void *arg)
{
    int nTemp = 0;
    for (;;)
    {
        if (audioMgr.ptNetwork.nARMDecode == 0)
        {
            usleep(15 * 1000);
    #ifdef DUMP_MUSIC
            // get I2S data and Write into wav
            if (fI2SOut && audioMgr.ptNetwork.nType == SMTK_AUDIO_MP3)
                audio_dump_I2S();
    #endif
        }
        else
        {
            // sleep a little time
            usleep(20);
        }
        if (audioMgr.Nfilequeque)   //init i2s need some time that may cause video lag,
        {
            audioMgr.Nfilequeque--; //we init i2s in a thread
            if (audioMgr.Nfilequeque == 0)
            {
                pthread_mutex_lock(&audio_read_buffer_mutex);
                smtkAudioMgrQuickStop();
                gpNetwork.pHandle = fopen(gpNetwork.pFilename, "rb");
                smtkAudioMgrPlayNetwork(&gpNetwork);
                pthread_mutex_unlock(&audio_read_buffer_mutex);
            }

        }
        if (audioMgr.playing == MMP_TRUE)
        {
            audioMgr.nReading   = 1;
            audio_buffer_read_data();
            audioMgr.nReading   = 0;
        }
    }
}

#endif

static int audio_get_available_buffer_size(unsigned long *bufSize)
{
    int nResult = 0;
    int i       = 0;
    if (audioMgr.ptNetwork.nARMDecode == 0)
    {
        if (audioMgr.ptNetwork.nM4A == 0)
        {
            nResult = iteAudioGetAvailableBufferLength(ITE_AUDIO_OUTPUT_BUFFER, bufSize);
        }
    }
    return nResult;
}

static int audio_write_stream(char *pBuf, unsigned long bufSize)
{
    int nResult = 0;
    if (audioMgr.ptNetwork.nARMDecode == 0)
    {
        if (audioMgr.ptNetwork.nM4A == 0)
        {
            gnAudioWrite    += bufSize;
            nResult         = iteAudioWriteStream((MMP_UINT8 *) pBuf, bufSize);
        }
    }
    return nResult;
}