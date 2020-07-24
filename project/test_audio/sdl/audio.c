#include <assert.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include "ite/audio.h"
#include "audio_mgr.h"

typedef int (*AudioPlayCallback)(int state);
static bool                       audioInited;
static AudioPlayCallback          audioPlayCallback;
static pthread_mutex_t            audioPlayMutex;

extern MMP_INT smtkAudioMgrPlayNetwork(SMTK_AUDIO_PARAM_NETWORK* pNetwork);

static int AudioPlayCallbackFunc(int state);

static void *               AL_Local_player_http_handle = NULL;

#define AUDIOLINK_LOCAL_PLAYER_BUFFER_LEN            (64 * 1024)
#define AUDIO_WAIT_INIT_I2S


void AudioInit(void)
{
    smtkAudioMgrInitialize();
    //smtkAudioMgrSetCallbackFunction((int*)AudioPlayCallbackFunc);

    audioInited         = false;

    pthread_mutex_init(&audioPlayMutex, NULL);

    audioInited = true;
    //smtkAudioMgrOpenEngine(SMTK_AUDIO_WAV);
}

void AudioExit(void)
{
    if (!audioInited)
    {
        return;
    }

    smtkAudioMgrTerminate();
    pthread_mutex_destroy(&audioPlayMutex);
}

static int AudioPlayCallbackFunc(int state)
{
    if (audioPlayCallback)
    {
        return audioPlayCallback(state);
    }
    return 0;
}

static int AudioPlayCallbackFuncEmpty(int state)
{
    // DO NOTHING
    return 0;
}

int AudioPlay(char* filename, AudioPlayCallback func)
{
    void *                   local_player_http_handle = NULL;
    SMTK_AUDIO_PARAM_NETWORK audiomgr_local;
    int                      nResult                  = 0;
    char*                    ext;

    ext = strrchr(filename, '.');
    if (!ext)
    {
        printf("Invalid file name: %s\n", filename);
        return -1;
    }
    ext++;


    pthread_mutex_lock(&audioPlayMutex);

#ifdef AUDIO_WAIT_INIT_I2S

    #ifdef __OPENRTOS__
    smtkAudioMgrQuickStop();
    #endif

    // close handler (if any)
    if (AL_Local_player_http_handle)
    {
        fclose(AL_Local_player_http_handle);
        AL_Local_player_http_handle = NULL;
    }
    
#endif
    
    
    audiomgr_local.audioMgrCallback = AudioPlayCallbackFunc;

    if (stricmp(ext, "wav") == 0)
    {
        audiomgr_local.nType            = SMTK_AUDIO_WAV;
        audioPlayCallback  = func;
        if(func == NULL)
            audiomgr_local.audioMgrCallback = AudioPlayCallbackFuncEmpty;        
    }
    else if (stricmp(ext, "mp3") == 0)
    {
        audiomgr_local.nType = SMTK_AUDIO_MP3;
        audioPlayCallback  = func;
    }
    else if (stricmp(ext, "wma") == 0)
    {
        audiomgr_local.nType = SMTK_AUDIO_WMA;
        audioPlayCallback  = func;
    }
    else if (stricmp(ext, "aac") == 0)
    {
        audiomgr_local.nType = SMTK_AUDIO_AAC;
        audioPlayCallback  = func;
    }
    else
    {
        printf("Unsupport file format: %s\n", ext);
        pthread_mutex_unlock(&audioPlayMutex);
        return -1;
    }
#ifdef AUDIO_WAIT_INIT_I2S
    if (filename)
    {
        AL_Local_player_http_handle = fopen(filename, "rb");
    }
    if (!AL_Local_player_http_handle)
    {
        printf("[Main]%s() L#%ld: fopen error \r\n", __FUNCTION__, __LINE__);
        pthread_mutex_unlock(&audioPlayMutex);
        return -1;
    }
#endif
    audiomgr_local.pHandle     = AL_Local_player_http_handle;
    audiomgr_local.LocalRead   = fread;
    audiomgr_local.nReadLength = AUDIOLINK_LOCAL_PLAYER_BUFFER_LEN;
    audiomgr_local.bSeek       = 0;
    audiomgr_local.nM4A        = 0;
    audiomgr_local.bLocalPlay  = 1;
    audiomgr_local.pFilename   = filename;
    
    printf("[Main]%s() L#%ld:  %s success \r\n", __FUNCTION__, __LINE__, filename);
#ifdef AUDIO_WAIT_INIT_I2S
    nResult                    = smtkAudioMgrPlayNetwork(&audiomgr_local);
#else
    nResult                    = smtkSetFileQueue(audiomgr_local);
#endif
    
    pthread_mutex_unlock(&audioPlayMutex);
    return 0;
}

void AudioTimeParsing(char* filename){
    SMTK_AUDIO_PARAM_NETWORK audiomgr_local;
    char*                    ext;
    
    ext = strrchr(filename, '.');
    ext++;
    
    if (stricmp(ext, "wav") == 0)
    {
        audiomgr_local.nType = SMTK_AUDIO_WAV;       
    }
    else if (stricmp(ext, "mp3") == 0)
    {
        audiomgr_local.nType = SMTK_AUDIO_MP3;
    }
    else
    {
        printf("Unsupport file format: %s\n", ext);
        return -1;
    }
    audiomgr_local.pFilename   = filename;
    smtkAudioParsingTime(&audiomgr_local);
}

void AudioStop(void)
{
    audioPlayCallback = NULL;
    smtkAudioMgrQuickStop();
}

int AudioPlayMusic(char* filename, AudioPlayCallback func)
{
    return AudioPlay(filename, func);
}
