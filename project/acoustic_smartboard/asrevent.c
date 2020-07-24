#include <sys/ioctl.h>
#include <assert.h>
#include <mqueue.h>
#include <string.h>
#include <unistd.h>
#include "ite/itp.h"
#include "project.h"
#include "scene.h"
#include "leaf/leaf_mediastream.h"
#include "audio_mgr.h"
#include "asrtable.h"

#define ASR_TIME 30.0

extern LeafCall *call;

static float AsrTriggerCountDown;
static uint32_t AsrTriggerLastTick;
static bool AsrTrigger;
static bool Isevent=false;
static char event[256];

typedef struct _asrStruct{
    int index;
    int rs;
    char* text;
    float score;
}asrStruct;

void fAsrCallback(int state,void *arg)
{
    asrStruct data =*(asrStruct*)arg;
     switch (state)
    {
        case ASR_SUCCESS_ARG:
            strcpy(event,data.text);
            Isevent=true;
            printf("%s %d\n",__FUNCTION__,__LINE__);
        break;
        case ASR_FAIL:
            printf("ASR FAIL\n");
        break;
        default:
        break;
    }
}

static int AsrTipsPlayCallback(int state)
{
	switch (state)
	{
	case AUDIOMGR_STATE_CALLBACK_PLAYING_FINISH:
		AsrStartOnBackGround(1);
        AudioResumeKeySound();
		break;
	}
	return 0;
}

void triggerAsrEvent(void){
    Isevent = true;
}

void BroadcastTip(char *filepath,bool trigger){
    AsrStartOnBackGround(0);
    AudioPauseKeySound();
    AudioPlayMusic(filepath, AsrTipsPlayCallback);
    
    AsrTriggerCountDown = (float)ASR_TIME;
    AsrTriggerLastTick = SDL_GetTicks();
    AsrTrigger = trigger;
    if(trigger) ScreenSaverRefresh();//do not enter power saving mode
    printf("***%f %d %d***\n",AsrTriggerCountDown,AsrTriggerLastTick,AsrTrigger); 
}

void countdowntick(void){
    
    if (AsrTriggerCountDown > 0.0f && AsrTrigger)
	{
        uint32_t diff, tick = SDL_GetTicks();

        if (tick >= AsrTriggerLastTick)
            diff = tick - AsrTriggerLastTick;
        else
            diff = 0xFFFFFFFF - AsrTriggerLastTick + tick;   

        if (diff >= 1000)
        {
            AsrTriggerCountDown -= (float)diff / 1000.0f;
            AsrTriggerLastTick = tick;

            if (AsrTriggerCountDown <= 0.0f)
            {//leave asr
                AsrTrigger = false;
                BroadcastTip("B:/media/tips/Bye.mp3",false);
                if(!ScreenIsOff() || !ScreenClockIsOff()) ituLayerGoto(mainLayer);
                //ConfigSave();
            }
        }        
	}    
}

void CheckAsrEvent(void){
    
    if(Isevent){
        if (stricmp(event, result0) == 0){
            if(ScreenIsOff() || ScreenClockIsOff()) ituLayerGoto(mainLayer);
            BroadcastTip("B:/media/tips/Hello.mp3",true);   
            printf("event0\n");
        }
    }
      
    if(Isevent && AsrTrigger){
        if (stricmp(event, result1) == 0){
            BroadcastTip("B:/media/tips/MovieMode.mp3",true);
            theConfig.scene =  2; //scene 0: 工作模式 1: 在家  2:電影 3:離家
            ituLayerGoto(scenesLayer);
            printf("event1\n");
        }else if (stricmp(event, result2) == 0){
            BroadcastTip("B:/media/tips/InMode.mp3",true);
            theConfig.scene =  1; //scene
            ituLayerGoto(scenesLayer);
            printf("event2\n");
        }else if (stricmp(event, result3) == 0){
            BroadcastTip("B:/media/tips/WorkMode.mp3",true);
            theConfig.scene =  0; //scene
            ituLayerGoto(scenesLayer);
            printf("event3\n");
        }else if (stricmp(event, result4) == 0){
            BroadcastTip("B:/media/tips/OutMode.mp3",true);
            theConfig.scene =  3; //scene
            ituLayerGoto(scenesLayer);
            printf("event4\n");
        }else if (stricmp(event, result5) == 0){
            BroadcastTip("B:/media/tips/OpneAir.mp3",true);
            theConfig.air_mode =  1; //wind 0: off, 3:風量, 2:風量2, 1:風量1, 4:風量Auto
            ituLayerGoto(mainLayer);
            printf("event5\n");
        }else if (stricmp(event, result6) == 0){
            BroadcastTip("B:/media/tips/CloseAir.mp3",true);
            theConfig.air_mode =  0; //wind
            ituLayerGoto(mainLayer);
            printf("event6\n");
        }else if (stricmp(event, result7) == 0){
            BroadcastTip("B:/media/tips/MinWind.mp3",true);
            theConfig.air_mode =  1; //wind
            ituLayerGoto(mainLayer);
            printf("event7\n");
        }else if (stricmp(event, result8) == 0){
            BroadcastTip("B:/media/tips/MedWind.mp3",true);
            theConfig.air_mode =  2; //wind
            ituLayerGoto(mainLayer);
            printf("event8\n");
        }else if (stricmp(event, result9) == 0){
            BroadcastTip("B:/media/tips/MaxWind.mp3",true);
            theConfig.air_mode =  3; //wind
            ituLayerGoto(mainLayer);
            printf("event9\n");
        }else if (stricmp(event, resulta) == 0){
            BroadcastTip("B:/media/tips/AutoWind.mp3",true);
            theConfig.air_mode =  4; //wind
            ituLayerGoto(mainLayer);
            printf("eventa\n");
        }else if (stricmp(event, resultb) == 0){ 
            if(theConfig.air_mode >= 1 && theConfig.air_mode < 3){
                BroadcastTip("B:/media/tips/WindHigh.mp3",true);
                theConfig.air_mode++; //wind
                ituLayerGoto(mainLayer);
            }else if(theConfig.air_mode==4){
                BroadcastTip("B:/media/tips/WindHigh.mp3",true);
                theConfig.air_mode=3;
                ituLayerGoto(mainLayer);
            }else{
                BroadcastTip("B:/media/tips/LimitValue.mp3",true);
            }
            printf("eventb\n");            
        }else if (stricmp(event, resultc) == 0){
            if(theConfig.air_mode > 1 && theConfig.air_mode <= 3){
                BroadcastTip("B:/media/tips/WindLow.mp3",true);
                theConfig.air_mode--; //wind
                ituLayerGoto(mainLayer);
            }else if(theConfig.air_mode==4){
                BroadcastTip("B:/media/tips/WindLow.mp3",true);
                theConfig.air_mode=1;
                ituLayerGoto(mainLayer);                
            }else{
                BroadcastTip("B:/media/tips/LimitValue.mp3",true);
            }
            printf("eventc\n");
        }else if (stricmp(event, resultd) == 0){
            if(theConfig.set_temperature < 32){
                BroadcastTip("B:/media/tips/TempHigh.mp3",true);
                theConfig.set_temperature++;
                ituLayerGoto(mainLayer);
            }else{
                BroadcastTip("B:/media/tips/LimitValue.mp3",true);
            }
            printf("eventd\n");
        }else if (stricmp(event, resulte) == 0){
            if(theConfig.set_temperature > 16){
                BroadcastTip("B:/media/tips/TempLow.mp3",true);
                theConfig.set_temperature--;
                ituLayerGoto(mainLayer);
            }else{
                BroadcastTip("B:/media/tips/LimitValue.mp3",true);
            }
            printf("evente\n");
        }else if (stricmp(event, resultf) == 0){
            if(ScreenIsOff()){
                ituLayerGoto(mainLayer);
                BroadcastTip("B:/media/tips/OpenLight.mp3",true);
                ScreenOn();
            }
            printf("eventf\n");
        }else if (stricmp(event, result10) == 0){
            if(!ScreenIsOff()){
                ituLayerGoto(mainLayer);
                BroadcastTip("B:/media/tips/CloseLight.mp3",true);
                ScreenOff();
            }
            printf("event10\n");
        }else if (stricmp(event, result11) == 0){
            int value = theConfig.brightness;
            value+=2;
            if (value > 10) value = 10;
            if (!ScreenIsOff() && (theConfig.brightness!=value)){
                ituLayerGoto(mainLayer);
                theConfig.brightness = value;
                ScreenSetBrightness(theConfig.brightness - 1);
                BroadcastTip("B:/media/tips/Brighten.mp3",true);
            }else{
                if(!ScreenIsOff())
                    BroadcastTip("B:/media/tips/LimitValue.mp3",true);
                else{
                    BroadcastTip("B:/media/tips/Brighten.mp3",true);
                    ituLayerGoto(mainLayer);
                    ScreenOn();
                }
            }
            printf("event11\n");
        }else if (stricmp(event, result12) == 0){
            int value = theConfig.brightness;
            value-=2;
            if (value < 1) value = 1;
            if (!ScreenIsOff() && (theConfig.brightness!=value)){
                ituLayerGoto(mainLayer);
                theConfig.brightness = value;
                ScreenSetBrightness(theConfig.brightness - 1);
                BroadcastTip("B:/media/tips/Dim.mp3",true);
            }else{
                if(!ScreenIsOff())
                    BroadcastTip("B:/media/tips/LimitValue.mp3",true);       
            }
            printf("event12\n");
        }else if (stricmp(event, result13) == 0){       
            BroadcastTip("B:/media/tips/OpenCurtain.mp3",true);
            ituLayerGoto(curtainLayer); 
            ControlCurtain(1);//curtain 0: close, 1 : open, 2 : stop
            printf("event13\n");
        }else if (stricmp(event, result14) == 0){        
            BroadcastTip("B:/media/tips/CloseCurtain.mp3",true);
            ituLayerGoto(curtainLayer); 
            ControlCurtain(0);//curtain
            printf("event14\n");
        }else if (stricmp(event, result15) == 0){
            BroadcastTip("B:/media/tips/StopCurtain.mp3",true);
            ituLayerGoto(curtainLayer); 
            ControlCurtain(2);//curtain
            printf("event15\n");
        }else if (stricmp(event, result16) == 0){
            BroadcastTip("B:/media/tips/OpenNetwork.mp3",true);
            theConfig.wifi_on_off =  1;//network 0:off, 1:on 
            ituSpriteGoto(settingPageSprite,2); 
            ituCoverFlowGoto(settingCoverFlow,2); 
            ituLayerGoto(settingLayer);
            printf("event16\n");
        }else if (stricmp(event, result17) == 0){
            BroadcastTip("B:/media/tips/CloseNetwork.mp3",true);
            theConfig.wifi_on_off =  0;//network 0:off, 1:on 
            ituSpriteGoto(settingPageSprite,2); 
            ituCoverFlowGoto(settingCoverFlow,2); 
            ituLayerGoto(settingLayer);
            printf("event17\n");
        }else{
            printf("ASR fcallback SUCCESS ARG %s\n",event);
        }        
        
        Isevent=false;
    }
    
    countdowntick();
    
}