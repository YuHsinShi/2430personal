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



static int check_codec_normal(int *error)
{
    static unsigned int I2sWP         = 0;
    static unsigned int I2sLastWriteP = 0;
    int I2SBuf;
    int nBufferLength;
    static int errorDEC=0;
    static int count=0;

    // dump data
    if (i2s_get_DA_running())
    {
        iteAudioGetAttrib(  ITE_AUDIO_CODEC_SET_BUFFER_LENGTH,  &nBufferLength);
        iteAudioGetAttrib(  ITE_AUDIO_I2S_PTR,                  &I2SBuf);

        I2sWP = I2S_DA32_GET_WP();
        // first time
        if ((I2sWP > I2sLastWriteP) && I2sLastWriteP == 0 && I2sWP == 0)
        {
            I2sLastWriteP  = I2sWP;
        }
        else if ((I2sWP > I2sLastWriteP) && (I2sWP != I2sLastWriteP))
        {
            unsigned int idx=0;
            unsigned int len=I2sWP - I2sLastWriteP;

            for(idx=0;idx<len;idx+=2)
            {
                int data           =(int)*(int16_t*)(I2SBuf+I2sLastWriteP+idx);
                if(data>3500||data<-3500){
                    errorDEC++;
                    //printf("%d \n",data);
                }
            }
            I2sLastWriteP  = I2sWP;
        }
        else if (I2sWP < I2sLastWriteP && I2sWP != I2sLastWriteP)
        {
            I2sLastWriteP  = I2sWP;
        }
        else
        {
            if(errorDEC!=0) *error=1;
            //sleep(5); //debug
            AudioStop();
            printf("[Audio mgr]dump end %d %d  #line %d \n", (I2sWP), I2sLastWriteP,__LINE__);
            return 0;
        }
    }
    if(count>50) {
        *error=2;
        return 0; //time out;
    }
    count++;
    return 1;
}


void* TestFunc(void* arg)
{
    pthread_t       task;

    int error = 0;
    itpInit();
    AudioInit();

    pthread_create(&task, NULL, event_loop, NULL);
    AudioSetVolume(20);

    AudioPlay_hex(NULL);
    
    while(check_codec_normal(&error)){
        usleep(50000);
    }

    if(error == 0) printf("**mp3 decoder PASS**\n");
    if(error == 1) printf("**mp3 codec error**\n");
    if(error == 2) printf("**codec may not work**\n");
    printf("end of test\n");

    return NULL;
}
