#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include "ite/itp.h"
#include "i2s/i2s.h" 
#include "audio_mgr.h" 
#include "sdl/main_processor_message_queue.h"

//#define Volume_test
/**********************************************************/

int audioCallbackFucntion(int nState)
{
    switch (nState)
    {
        case AUDIOMGR_STATE_CALLBACK_PLAYING_FINISH:
        printf("file play EOF");
        AudioPlayMusic("a:/test.mp3",NULL);
        break;

        case AUDIOMGR_STATE_CALLBACK_GET_FINISH_NAME:
        printf("file : %s\n",smtkAudioMgrGetFinishName());
        break;
        
        default:
        break;
        
    }
}


static void *event_loop(void *arg)
{
    while(1)
    {
        uint16_t nAudioPluginRegister = 0;
        nAudioPluginRegister = getAudioPluginMessageStatus();
        if (((nAudioPluginRegister & 0xc000)>>14) == SMTK_AUDIO_PROCESSOR_ID)
        {
            // do audio api
            //printf("nAudioPluginRegister 0x%x \n",nAudioPluginRegister);
            smtkMainProcessorExecuteAudioPluginCmd(nAudioPluginRegister);
        }
        
        usleep(500000);
    } 
}

void* TestFunc(void* arg)
{
    pthread_t       task;    
    char *filename = "a:/test.mp3";
    uint32_t musictime;
    itpInit();
    AudioInit();
    sleep(2);
    pthread_create(&task, NULL, event_loop, NULL);
    AudioSetVolume(20);
    AudioPlayMusic(filename,NULL);
    //AudioPlayMusic(filename,audioCallbackFucntion);
    //AudioTimeParsing(filename);
    //smtkAudioMgrGetTotalTime(&musictime)
    
    while(1){
        sleep(1);
    };
    
    
    return NULL;
}
