#include <assert.h>
#include <sys/time.h>
#include "scene.h"
#include "audio_mgr.h"
#include "SDL/SDL.h"
#include "leaf/leaf_mediastream.h"

#define speedGap 2
#define tachoGap 1

struct timeval tv;
struct tm* tm;


static int first_boot = true;
static int  tacho_m = 0;
static bool tacho_reverse = false;
static int  speed_m = 0;
static int  radiator_m = 0;
static int  fuel_m = 0;
static int  autodemo_count = 0;

static int rgbIndex = 0, demoIndex = 0, curType = 0, stallIndex = 1 ,mixIndex=0;
static bool testing = false, errorShow = false, switchType = false, stallsDemo = false;
static char widgetName[30], *stallName[11];

static uint32_t lastTick = 0;
static ITUText *mainSpeedText[3] = { 0 };
static ITUText *mainTotalOdoText[3] = { 0 };
static ITUText *mainTripAText[3] = { 0 };
static ITUText *mainTripBText[3] = { 0 };
static ITUText *mainTypeAMusicNameText = 0;
static ITUText *mainStallsText[2][16] = { 0 };
static ITUMeter *mainTypeATachoMeter = 0;
static ITUMeter *mainTypeDTachoMeter = 0;
static ITUMeter *mainTypeDSpeedMeter = 0;
static ITUMeter *mainTypeCMeter[3] = { 0 };
static ITUSprite *mainTypeAFuelSprite = 0;
static ITUSprite *mainAMPMSprite[3] = { 0 };
static ITUSprite *mainTypeABatterySprite = 0;
static ITUSprite *mainTypeAStallsSprite = 0;
static ITUSprite *mainBackgroundSprite[3] = { 0 };
static ITUAnimation *mainTurnLRAnimation[3][2] = { 0 };
static ITUAnimation *mainTypeAStallsAnimation = 0;
static ITUAnimation *mainMusicAnimation[3][4] = { 0 };
static ITUAnimation *mainBeamAnimation[3][3] = { 0 };
//static ITUAnimation *mainReadyAnimation = 0;
static ITUAnimation *mainErrorAnimation = 0;
static ITUAnimation *mainStallsAnimation[2][5] = { 0 };
static ITUContainer *mainBeamContainer[3] = { 0 };
static ITUTrackBar *mainTypeARadiatorTrackBar = 0;
static ITUCheckBox *mainStatusCheckBox[3][7] = { 0 };
static ITURadioBox *mainTypeALowBeamRadioBox = 0;
static ITURadioBox *mainTypeAHighBeamRadioBox = 0;
static ITUBackground *mainErrorBackground = 0;
static ITUButton *mainReTestButton = 0;
static ITUButton *mainNextBGButton = 0;
static ITUProgressBar *mainTypeDFuelProgressBar = 0;
static ITUIcon *mainLRIcon[3][2] = { 0 };

static ITUBackground *mainBackground[3][5] = { 0 };

static ITULayer* digitalTypeLayer;
static ITUBackground * mainTypeCBackground;
static ITUBackground* digitalTypeOpeningBg;

static void MainSwitchType(int, bool);
static void MainTestingDemo(int, uint32_t);
static void MainTurnLRDemo(int);
static void MainMusicDemo(int);
static void MainBeamDemo(int);
static void MainSignLightDemo(int, int);
static void MainTachoDemo(int, int);
static void MainRadiatorDemo(int, int);
static void MainSpeedDemo(int, int);
static void MainStallsDemo(int, int);
static void MainFuelDemo(int, int);
static void MainOdoDemo(int, int);
static void MainTestingReset(int);
static void MainStallsSetString(int);
static void MainValueReset(void);

static int AudioPlayerPlayCallback(int state)
{
	switch (state)
	{
	case AUDIOMGR_STATE_CALLBACK_PLAYING_FINISH:
		AudioResumeKeySound();
		break;
	}
	return 0;
}

bool MainOnEnter(ITUWidget* widget, char* param)
{
	int i;
	gettimeofday(&tv, NULL);
	tm = localtime(&tv.tv_sec);
	if (!mainSpeedText[0])
	{
		mainSpeedText[0] = ituSceneFindWidget(&theScene, "mainTypeASpeedText");
		assert(mainSpeedText[0]);

		mainSpeedText[2] = ituSceneFindWidget(&theScene, "mainTypeCSpeedText");
		assert(mainSpeedText[2]);

		mainSpeedText[1] = ituSceneFindWidget(&theScene, "mainTypeDSpeedText");
		assert(mainSpeedText[1]);

		mainTotalOdoText[0] = ituSceneFindWidget(&theScene, "mainTypeATotalOdoText");
		assert(mainTotalOdoText[0]);

		mainTotalOdoText[2] = ituSceneFindWidget(&theScene, "mainTypeCTotalOdoText");
		assert(mainTotalOdoText[2]);

		mainTotalOdoText[1] = ituSceneFindWidget(&theScene, "mainTypeDTotalOdoText");
		assert(mainTotalOdoText[1]);

		mainTripAText[0] = ituSceneFindWidget(&theScene, "mainTypeATripAText");
		assert(mainTripAText[0]);

		mainTripAText[2] = ituSceneFindWidget(&theScene, "mainTypeCTripAText");
		assert(mainTripAText[2]);

		mainTripAText[1] = ituSceneFindWidget(&theScene, "mainTypeDTripAText");
		assert(mainTripAText[1]);

		mainTripBText[0] = ituSceneFindWidget(&theScene, "mainTypeATripBText");
		assert(mainTripBText[0]);

		mainTripBText[2] = ituSceneFindWidget(&theScene, "mainTypeCTripBText");
		assert(mainTripBText[2]);

		mainTripBText[1] = ituSceneFindWidget(&theScene, "mainTypeDTripBText");
		assert(mainTripBText[1]);

		mainTypeAMusicNameText = ituSceneFindWidget(&theScene, "mainTypeAMusicNameText");
		assert(mainTypeAMusicNameText);

		digitalTypeLayer = ituSceneFindWidget(&theScene, "digitalTypeLayer");
		assert(digitalTypeLayer);

		digitalTypeOpeningBg = ituSceneFindWidget(&theScene, "digitalTypeOpeningBackground");
		assert(digitalTypeOpeningBg);

		mainTypeCBackground = ituSceneFindWidget(&theScene, "mainTypeCBackground");
		assert(mainTypeCBackground);

		for (i = 0; i < 16; i++){
			snprintf(widgetName, sizeof(widgetName), "mainTypeCStallsText%d", i);
			mainStallsText[1][i] = ituSceneFindWidget(&theScene, widgetName);
			assert(mainStallsText[1][i]);

			snprintf(widgetName, sizeof(widgetName), "mainTypeDStallsText%d", i);
			mainStallsText[0][i] = ituSceneFindWidget(&theScene, widgetName);
			assert(mainStallsText[0][i]);
		}
		
		mainTypeATachoMeter = ituSceneFindWidget(&theScene, "mainTypeAMeter");
		assert(mainTypeATachoMeter);

		mainTypeDTachoMeter = ituSceneFindWidget(&theScene, "mainTypeDMeter0");
		assert(mainTypeDTachoMeter);

		mainTypeDSpeedMeter = ituSceneFindWidget(&theScene, "mainTypeDMeter1");
		assert(mainTypeDSpeedMeter);

		for (i = 0; i < 3; i++){
			snprintf(widgetName, sizeof(widgetName), "mainTypeCMeter%d", i);
			mainTypeCMeter[i] = ituSceneFindWidget(&theScene, widgetName);
			assert(mainTypeCMeter[i]);
		}

		mainTypeAFuelSprite = ituSceneFindWidget(&theScene, "mainTypeAFuelSprite");
		assert(mainTypeAFuelSprite);

		mainAMPMSprite[0] = ituSceneFindWidget(&theScene, "mainTypeAAMPMSprite");
		assert(mainAMPMSprite[0]);

		mainAMPMSprite[2] = ituSceneFindWidget(&theScene, "mainTypeCAMPMSprite");
		assert(mainAMPMSprite[2]);

		mainAMPMSprite[1] = ituSceneFindWidget(&theScene, "mainTypeDAMPMSprite");
		assert(mainAMPMSprite[1]);

		mainTypeABatterySprite = ituSceneFindWidget(&theScene, "mainTypeABatterySprite");
		assert(mainTypeABatterySprite);

		mainTypeAStallsSprite = ituSceneFindWidget(&theScene, "mainTypeAStallsSprite");
		assert(mainTypeAStallsSprite);

		mainBackgroundSprite[0] = ituSceneFindWidget(&theScene, "mainTypeABackgroundSprite");
		assert(mainBackgroundSprite[0]);

		mainBackgroundSprite[2] = ituSceneFindWidget(&theScene, "mainTypeCBackgroundSprite");
		assert(mainBackgroundSprite[2]);

		mainBackgroundSprite[1] = ituSceneFindWidget(&theScene, "mainTypeDBackgroundSprite");
		assert(mainBackgroundSprite[1]);

		mainTurnLRAnimation[0][0] = ituSceneFindWidget(&theScene, "mainTypeARightAnimation");
		assert(mainTurnLRAnimation[0][0]);

		mainTurnLRAnimation[0][1] = ituSceneFindWidget(&theScene, "mainTypeALeftAnimation");
		assert(mainTurnLRAnimation[0][1]);

		mainTurnLRAnimation[2][0] = ituSceneFindWidget(&theScene, "mainTypeCRightAnimation");
		assert(mainTurnLRAnimation[2][0]);

		mainTurnLRAnimation[2][1] = ituSceneFindWidget(&theScene, "mainTypeCLeftAnimation");
		assert(mainTurnLRAnimation[2][1]);

		mainTurnLRAnimation[1][0] = ituSceneFindWidget(&theScene, "mainTypeDRightAnimation");
		assert(mainTurnLRAnimation[1][0]);

		mainTurnLRAnimation[1][1] = ituSceneFindWidget(&theScene, "mainTypeDLeftAnimation");
		assert(mainTurnLRAnimation[1][1]);

		mainTypeAStallsAnimation = ituSceneFindWidget(&theScene, "mainTypeAStallsAnimation");
		assert(mainTypeAStallsAnimation);

		for (i = 0; i < 4; i++){
			snprintf(widgetName, sizeof(widgetName), "mainTypeAMusicAnimation%d", i);
			mainMusicAnimation[0][i] = ituSceneFindWidget(&theScene, widgetName);
			assert(mainMusicAnimation[0][i]);

			snprintf(widgetName, sizeof(widgetName), "mainTypeCMusicAnimation%d", i);
			mainMusicAnimation[2][i] = ituSceneFindWidget(&theScene, widgetName);
			assert(mainMusicAnimation[2][i]);

			snprintf(widgetName, sizeof(widgetName), "mainTypeDMusicAnimation%d", i);
			mainMusicAnimation[1][i] = ituSceneFindWidget(&theScene, widgetName);
			assert(mainMusicAnimation[1][i]);
		}

		mainBeamAnimation[0][0] = ituSceneFindWidget(&theScene, "mainTypeATempAnimation");
		assert(mainBeamAnimation[0][0]);

		mainBeamAnimation[2][0] = ituSceneFindWidget(&theScene, "mainTypeCTempAnimation");
		assert(mainBeamAnimation[2][0]);

		mainBeamAnimation[1][0] = ituSceneFindWidget(&theScene, "mainTypeDTempAnimation");
		assert(mainBeamAnimation[1][0]);

		for (i = 1; i < 3; i++){
			snprintf(widgetName, sizeof(widgetName), "mainTypeABeamAnimation%d", i-1);
			mainBeamAnimation[0][i] = ituSceneFindWidget(&theScene, widgetName);
			assert(mainBeamAnimation[0][i]);

			snprintf(widgetName, sizeof(widgetName), "mainTypeCBeamAnimation%d", i-1);
			mainBeamAnimation[2][i] = ituSceneFindWidget(&theScene, widgetName);
			assert(mainBeamAnimation[2][i]);

			snprintf(widgetName, sizeof(widgetName), "mainTypeDBeamAnimation%d", i - 1);
			mainBeamAnimation[1][i] = ituSceneFindWidget(&theScene, widgetName);
			assert(mainBeamAnimation[1][i]);
		}

		//mainReadyAnimation = ituSceneFindWidget(&theScene, "mainReadyAnimation");
		//assert(mainReadyAnimation);

		mainErrorAnimation = ituSceneFindWidget(&theScene, "mainErrorAnimation");
		assert(mainErrorAnimation);

		for (i = 0; i < 5; i++){
			snprintf(widgetName, sizeof(widgetName), "mainTypeCStallsAnimation%d", i);
			mainStallsAnimation[1][i] = ituSceneFindWidget(&theScene, widgetName);
			assert(mainStallsAnimation[1][i]);

			snprintf(widgetName, sizeof(widgetName), "mainTypeDStallsAnimation%d", i);
			mainStallsAnimation[0][i] = ituSceneFindWidget(&theScene, widgetName);
			assert(mainStallsAnimation[0][i]);
		}
		
		mainBeamContainer[0] = ituSceneFindWidget(&theScene, "mainTypeABeamContainer");
		assert(mainBeamContainer[0]);

		mainBeamContainer[2] = ituSceneFindWidget(&theScene, "mainTypeCBeamContainer");
		assert(mainBeamContainer[2]);

		mainBeamContainer[1] = ituSceneFindWidget(&theScene, "mainTypeDBeamContainer");
		assert(mainBeamContainer[1]);

		mainTypeARadiatorTrackBar = ituSceneFindWidget(&theScene, "mainTypeARadiatorTrackBar");
		assert(mainTypeARadiatorTrackBar);

		for (i = 0; i < 7; i++){
			snprintf(widgetName, sizeof(widgetName), "mainTypeAStatusCheckBox%d", i);
			mainStatusCheckBox[0][i] = ituSceneFindWidget(&theScene, widgetName);
			assert(mainStatusCheckBox[0][i]);

			snprintf(widgetName, sizeof(widgetName), "mainTypeCStatusCheckBox%d", i);
			mainStatusCheckBox[2][i] = ituSceneFindWidget(&theScene, widgetName);
			assert(mainStatusCheckBox[2][i]);

			snprintf(widgetName, sizeof(widgetName), "mainTypeDStatusCheckBox%d", i);
			mainStatusCheckBox[1][i] = ituSceneFindWidget(&theScene, widgetName);
			assert(mainStatusCheckBox[1][i]);
		}

		mainTypeALowBeamRadioBox = ituSceneFindWidget(&theScene, "mainTypeALowBeamRadioBox");
		assert(mainTypeALowBeamRadioBox);

		mainTypeAHighBeamRadioBox = ituSceneFindWidget(&theScene, "mainTypeAHighBeamRadioBox");
		assert(mainTypeAHighBeamRadioBox);

		mainErrorBackground = ituSceneFindWidget(&theScene, "mainErrorBackground");
		assert(mainErrorBackground);

		mainReTestButton = ituSceneFindWidget(&theScene, "mainReTestButton");
		assert(mainReTestButton);

		mainNextBGButton = ituSceneFindWidget(&theScene, "mainNextBGButton");
		assert(mainNextBGButton);

		mainBackground[0][0] = ituSceneFindWidget(&theScene, "mainTypeAMidBackground");
		assert(mainBackground[0][0]);

		mainBackground[0][1] = ituSceneFindWidget(&theScene, "mainTypeARtBackground");
		assert(mainBackground[0][1]);

		mainBackground[0][2] = ituSceneFindWidget(&theScene, "mainTypeALfBackground");
		assert(mainBackground[0][2]);

		mainBackground[0][3] = ituSceneFindWidget(&theScene, "mainTypeATopBackground");
		assert(mainBackground[0][3]);

		mainBackground[0][4] = ituSceneFindWidget(&theScene, "mainTypeABotBackground");
		assert(mainBackground[0][4]);

		mainBackground[2][0] = ituSceneFindWidget(&theScene, "mainTypeCMidBackground");
		assert(mainBackground[2][0]);

		mainBackground[2][1] = ituSceneFindWidget(&theScene, "mainTypeCRtBackground");
		assert(mainBackground[2][1]);

		mainBackground[2][2] = ituSceneFindWidget(&theScene, "mainTypeCLfBackground");
		assert(mainBackground[2][2]);

		mainBackground[2][3] = ituSceneFindWidget(&theScene, "mainTypeCTopBackground");
		assert(mainBackground[2][3]);

		mainBackground[2][4] = ituSceneFindWidget(&theScene, "mainTypeCBotBackground");
		assert(mainBackground[2][4]);

		mainBackground[1][0] = ituSceneFindWidget(&theScene, "mainTypeDTopBackground");
		assert(mainBackground[1][0]);

		mainBackground[1][1] = ituSceneFindWidget(&theScene, "mainTypeDMidBackground");
		assert(mainBackground[1][1]);

		mainBackground[1][2] = ituSceneFindWidget(&theScene, "mainTypeDBotBackground");
		assert(mainBackground[1][2]);

		mainTypeDFuelProgressBar = ituSceneFindWidget(&theScene, "mainTypeDFuelProgressBar");
		assert(mainTypeDFuelProgressBar);

		mainLRIcon[0][0] = ituSceneFindWidget(&theScene, "mainTypeALeftIcon");
		assert(mainLRIcon[0][0]);

		mainLRIcon[0][1] = ituSceneFindWidget(&theScene, "mainTypeARightIcon");
		assert(mainLRIcon[0][1]);

		mainLRIcon[2][0] = ituSceneFindWidget(&theScene, "mainTypeCLeftIcon");
		assert(mainLRIcon[2][0]);

		mainLRIcon[2][1] = ituSceneFindWidget(&theScene, "mainTypeCRightIcon");
		assert(mainLRIcon[2][1]);

		mainLRIcon[1][0] = ituSceneFindWidget(&theScene, "mainTypeDLeftIcon");
		assert(mainLRIcon[1][0]);

		mainLRIcon[1][1] = ituSceneFindWidget(&theScene, "mainTypeDRightIcon");
		assert(mainLRIcon[1][1]);

		stallName[0] = "P";
		stallName[1] = "R";
		stallName[2] = "N";
		stallName[3] = "D";
		stallName[4] = "1";
		stallName[5] = "2";
		stallName[6] = "3";
		stallName[7] = "4";
		stallName[8] = "5";
		stallName[9] = "6";
		stallName[10] = "7";
	}
	mixIndex = -1;
	if (first_boot == true)
		first_boot = false;
	else
	{
		testing = false;
		curType = 0;
		rgbIndex = 0;
		demoIndex = 0;
		stallIndex = 1;
		//mixIndex = 0;
		errorShow = false;
		switchType = false;
		stallsDemo = false;
		MainSwitchType(curType, true);
		MainTestingReset(0);
		
	}
	
	return true;
}

bool MainOnTimer(ITUWidget* widget, char* param)
{
	uint32_t diff, tick = SDL_GetTicks();
	bool updated = false;

	if (tick >= lastTick)
		diff = tick - lastTick;
	else
		diff = 0xFFFFFFFF - lastTick + tick;

	if (switchType){
		if (!ituWidgetIsVisible(mainBackground[curType][0]))
		{
			if (curType == 2)
			{
				ituLayerGoto(digitalTypeLayer);
				ituWidgetSetVisible(digitalTypeOpeningBg, true);
			}

			curType = (curType + 1) % 3;
			MainSwitchType(curType, true);
			switchType = false;
			updated = true;	
		}
	}else if (testing) {
		MainTestingDemo(curType, diff);
		updated = true;
	}
//	else if (diff > 6500 && ituWidgetIsVisible(mainReadyAnimation)){
	else if (diff > 6500){
		//ituWidgetSetVisible(mainReadyAnimation, false);
		ituWidgetSetVisible(mainReTestButton, true);
#ifdef CFG_AUTO_RUN
        if(autodemo_count++ >= 2)
        {
            autodemo_count = 0;
            #ifdef CFG_AUTO_CHANGE_TYPE
    		if(switchType == false)
    		{
    			switchType = true;
    			MainSwitchType(curType, false);
    		}
            #endif
            MainValueReset();
            lastTick = SDL_GetTicks();
        }
        else if(testing == false)
		{
            MainValueReset();
			lastTick = SDL_GetTicks();
		}
#endif
        
		updated = true;
	}

	if (tm->tm_hour > 12)
		ituSpriteGoto(mainAMPMSprite[curType], 0);
	else
		ituSpriteGoto(mainAMPMSprite[curType], 1);

	return updated;
}

bool MainReTestButtonOnPress(ITUWidget* widget, char* param)
{
    #ifndef CFG_AUTO_RUN
    //mixIndex++;
	mixIndex = 0;
	if (mixIndex==0){
		if(testing == false && switchType == false)
		{
			ituWidgetSetVisible(mainReTestButton, false);
            MainValueReset();
			lastTick = SDL_GetTicks();
		}
	}
	else{
        char filepath[PATH_MAX];
        if(mixIndex==1){
            AudioPauseKeySound();
            AudioDeinit();
        }else if(mixIndex==2){
            smtkAudioMgrSetVolume(theConfig.keylevel);
            strcpy(filepath, CFG_PRIVATE_DRIVE ":/sounds/Music02.wav");
            LeafAudioPlay(filepath,RepeatPlay,NULL);
        }else if(mixIndex==3){
            strcpy(filepath,CFG_PRIVATE_DRIVE":/sounds/mix.wav");
            LeafMixSoundStop();
            LeafMixSoundStart(filepath,0.7,1);
        }else if(mixIndex==4){
            strcpy(filepath,CFG_PRIVATE_DRIVE ":/sounds/turn.wav");
            LeafMixSoundStop();
            LeafMixSoundStart(filepath,0.5,-1);
        }else if(mixIndex==5){
            LeafMixSoundStop();
        }else{
            LeafAudioStop();
            mixIndex=-1;
        }
	}
    printf("mixIndex=%d\n",mixIndex);
    #endif
	return true;
}

bool MainMusicAnimationOnStop(ITUWidget* widget, char* param)
{
	if (testing){
		int index = (atoi(param) + 1) % 4;
		ituWidgetSetVisible(mainMusicAnimation[curType][atoi(param)], false);
		ituWidgetSetVisible(mainMusicAnimation[curType][index], true);
		ituAnimationPlay(mainMusicAnimation[curType][index], 0);
	}
	return true;
}

bool MainBeamAnimationOnStop(ITUWidget* widget, char* param)
{
	if (testing){
		int index = (atoi(param) + 1) % 3;
		ituAnimationPlay(mainBeamAnimation[curType][index], 0);
	}
	return true;
}

bool MainNextBGButtonOnPress(ITUWidget* widget, char* param)
{
    #ifndef CFG_AUTO_RUN
	if(switchType == false)
		ituSpriteGoto(mainBackgroundSprite[curType], ++rgbIndex % 3);
	
	if (++demoIndex > 2) {
		if(switchType == false && testing == false)
		{
			switchType = true;
			MainSwitchType(curType, false);
		}
	}
    #endif
	return true;
}

bool mainStallsAnimationOnStop(ITUWidget* widget, char* param)
{
	int i;
	stallIndex = (stallIndex+1)%11;
	MainStallsSetString(stallIndex);
	if (stallIndex == 1){
		for (i = 0; i < 5; i++)
			ituAnimationStop(mainStallsAnimation[curType - 1][i]);
		stallsDemo = false;
	}
	
	return true;
}

void MainSwitchType(int type, bool show)
{
	int i;
	if (type!=1)
		for (i = 0; i < 5; i++)
			ituWidgetSetVisible(mainBackground[type][i], show);
	else{
		for (i = 0; i < 3; i++)
			ituWidgetSetVisible(mainBackground[type][i], show);

            LeafAudioStop();
            AudioResumeKeySound();
            mixIndex=-1;
	}
	
	demoIndex = 0;
	rgbIndex = 0;
}

void MainValueReset(void)
{
    tacho_m = 0;
    tacho_reverse = false;
    speed_m = 0;
    radiator_m = 0;
    fuel_m = 0;
    testing = true;
    stallsDemo = true;        
}

void MainTestingDemo(int type, uint32_t diff)
{
	if (diff < 5000)
	{
		int signInterval = diff % 1500, tacho = diff % 2000;
		MainTurnLRDemo(type);
		MainMusicDemo(type);
		MainBeamDemo(type);
		MainSignLightDemo(type, signInterval);
		MainTachoDemo(type, tacho);
		MainRadiatorDemo(type, tacho);
		MainSpeedDemo(type, diff);
		MainStallsDemo(type, diff);
		MainFuelDemo(type, diff);
		MainOdoDemo(type, diff);
	}
	else
	{   
		MainTestingReset(type);
		testing = false;
	}
}

void MainTurnLRDemo(int type)
{
	int i;
	if (!ituWidgetIsVisible(mainTurnLRAnimation[curType][0])) {
        AudioPlayMusic("A:/sounds/sound.wav", AudioPlayerPlayCallback);
		for (i = 0; i < 2; i++){
			ituWidgetSetVisible(mainLRIcon[curType][i], false);
			ituWidgetSetVisible(mainTurnLRAnimation[curType][i], true);
			ituAnimationPlay(mainTurnLRAnimation[curType][i], 0);
		}
	}
}

void MainMusicDemo(int type)
{
	if (!ituWidgetIsVisible(mainMusicAnimation[curType][0]) 
		&& !ituWidgetIsVisible(mainMusicAnimation[curType][1]) 
		&& !ituWidgetIsVisible(mainMusicAnimation[curType][2]) 
		&& !ituWidgetIsVisible(mainMusicAnimation[curType][3])) {
		ituWidgetSetVisible(mainMusicAnimation[curType][0], true);
		ituAnimationPlay(mainMusicAnimation[curType][0], 0);
	}
}

void MainBeamDemo(int type)
{
	int i;
	if (!ituWidgetIsVisible(mainBeamAnimation[type][1])) {
		for (i = 0; i < 3; i++){
			ituWidgetSetVisible(mainBeamAnimation[type][i], true);
		}
		ituAnimationPlay(mainBeamAnimation[type][0], 0);
	}
	ituWidgetSetVisible(mainBeamContainer[type], false);
}

void MainSignLightDemo(int type, int signInterval)
{
	int i;
	if (signInterval < 300){
		if (!ituCheckBoxIsChecked(mainStatusCheckBox[type][0])){
			ituCheckBoxSetChecked(mainStatusCheckBox[type][0], true);
			ituCheckBoxSetChecked(mainStatusCheckBox[type][1], true);
		}
	}
	else if (signInterval < 600)
	{
		if (!ituCheckBoxIsChecked(mainStatusCheckBox[type][2])){
			ituCheckBoxSetChecked(mainStatusCheckBox[type][2], true);
			ituCheckBoxSetChecked(mainStatusCheckBox[type][3], true);
		}
	}
	else if (signInterval < 900)
	{
		if (!ituCheckBoxIsChecked(mainStatusCheckBox[type][4])){
			ituCheckBoxSetChecked(mainStatusCheckBox[type][4], true);
			ituCheckBoxSetChecked(mainStatusCheckBox[type][5], true);
		}
	}
	else if (signInterval < 1200)
	{
		if (!ituCheckBoxIsChecked(mainStatusCheckBox[type][6]))
			ituCheckBoxSetChecked(mainStatusCheckBox[type][6], true);
	}
	else
	{
		if (ituCheckBoxIsChecked(mainStatusCheckBox[type][6]))
		for (i = 0; i < 7; i++)
			ituCheckBoxSetChecked(mainStatusCheckBox[type][i], false);
	}
}

void MainRadiatorDemo(int type, int tacho)
{
    radiator_m += tachoGap;
    if(radiator_m > 100)
        radiator_m = 100;
    
	switch (type)
	{
	case 2:
		ituMeterSetValue(mainTypeCMeter[1], radiator_m);
		break;
	default:
		break;
	}
}

void MainTachoDemo(int type, int tacho)
{
	if (!tacho_reverse){
		switch (type)
		{
		case 0:
			tacho_m += tachoGap;
			ituMeterSetValue(mainTypeATachoMeter, tacho_m);
			if (tacho_m == 100)
				tacho_reverse = true;
			break;
		case 1:
			tacho_m += tachoGap;
			ituMeterSetValue(mainTypeDTachoMeter, tacho_m);
			if (tacho_m == 100)
				tacho_reverse = true;
			break;
		default:
			break;
		}
	}
	else
	{
		switch (type)
		{
		case 0:
			tacho_m -= tachoGap;
			ituMeterSetValue(mainTypeATachoMeter, tacho_m);
			if (tacho_m == 0)
				tacho_reverse = false;
			break;
		case 1:
			tacho_m -= tachoGap;
			ituMeterSetValue(mainTypeDTachoMeter, tacho_m);
			if (tacho_m == 0)
				tacho_reverse = false;
			break;
		default:
			break;
		}
	}

}

void MainSpeedDemo(int type, int diff)
{
	char speed[10];
	speed_m += speedGap;
	
	switch (type)
	{
	case 0:
        if (speed_m < 240)
        {
            sprintf(speed, "%d", speed_m);
		    ituTextSetString(mainSpeedText[type], speed);
        }
        break;
	case 1:
		if (speed_m < 260)
		{
			ituMeterSetValue(mainTypeDSpeedMeter, speed_m);
			sprintf(speed, "%d", speed_m);
			ituTextSetString(mainSpeedText[type], speed);
		}
		else
		{
			ituMeterSetValue(mainTypeDSpeedMeter, 260);
			sprintf(speed, "%d", 260);
			ituTextSetString(mainSpeedText[type], speed);
		}
		break;
	case 2:
		if (speed_m < 240)
        {      
			ituMeterSetValue(mainTypeCMeter[0], speed_m);
            sprintf(speed, "%d", speed_m);
		    ituTextSetString(mainSpeedText[type], speed);
        }
		else
        {      
			ituMeterSetValue(mainTypeCMeter[0], 240);
            sprintf(speed, "%d", 240);
		    ituTextSetString(mainSpeedText[type], speed);
        }
		break;
	default:
		break;
	}
}

void MainStallsDemo(int type, int diff)
{
	int i;
	if (type == 0){
		if (mainTypeAStallsAnimation->playing == 0)
			ituAnimationPlay(mainTypeAStallsAnimation, 0);
		if (diff / 450 < 11)
			ituSpriteGoto(mainTypeAStallsSprite, diff / 450);
		else
			ituSpriteGoto(mainTypeAStallsSprite, 10);
	}
	else {
		if (mainStallsAnimation[type - 1][0]->playing == 0 && stallsDemo)
		for (i = 0; i < 5; i++)
			ituAnimationPlay(mainStallsAnimation[type - 1][i], 0);
	}
}

void MainFuelDemo(int type, int diff)
{
    
	if (diff < 2000 && type != 2){
		switch (type)
		{
		case 0:
			if (diff / 100 < 17)
				ituSpriteGoto(mainTypeAFuelSprite, 17 - (diff / 100));
			else
				ituSpriteGoto(mainTypeAFuelSprite, 0);
			break;
		case 1:
			mainTypeDFuelProgressBar->value = 100 - diff / 20;
			break;
		default:
			break;
		}
	}
	else
	{
	    fuel_m += tachoGap;
        if(fuel_m > 100)
            fuel_m = 100;
		switch (type)
		{
		case 0:
			if ((diff - 2000) / 100 < 17)
				ituSpriteGoto(mainTypeAFuelSprite, (diff - 2000) / 100);
			else
				ituSpriteGoto(mainTypeAFuelSprite, 16);
			break;
		case 1:
			mainTypeDFuelProgressBar->value = (diff-2000) / 20;
			break;
		case 2:
			ituMeterSetValue(mainTypeCMeter[2], fuel_m);
			break;
		default:
			break;
		}
	}
}

void MainOdoDemo(int type, int diff)
{
	int i;
	char odo[8];
	i = diff / 480;
	if (i < 10){
		sprintf(odo, "%d%d%d%d%d.%d",
			i, i, i, i, i, i);
		ituTextSetString(mainTotalOdoText[type], odo);
		ituTextSetString(mainTripAText[type], odo);
		ituTextSetString(mainTripBText[type], odo);
	}
	else{
		ituTextSetString(mainTotalOdoText[type], "99999.9");
		ituTextSetString(mainTripAText[type], "99999.9");
		ituTextSetString(mainTripBText[type], "99999.9");
	}
}

void MainTestingReset(int type)
{
	int i;
	if (ituWidgetIsVisible(mainTurnLRAnimation[type][0])){
		for (i = 0; i < 2; i++){
			ituWidgetSetVisible(mainLRIcon[type][i], true);
			ituWidgetSetVisible(mainTurnLRAnimation[type][i], false);
			ituAnimationStop(mainTurnLRAnimation[type][i]);
		}
		for (i = 0; i < 4; i++){
			ituWidgetSetVisible(mainMusicAnimation[type][i], false);
			ituAnimationStop(mainMusicAnimation[type][i]);
		}

		for (i = 1; i < 3; i++){
			ituWidgetSetVisible(mainBeamAnimation[type][i], false);
			ituAnimationStop(mainBeamAnimation[type][i]);
		}

		ituWidgetSetVisible(mainBeamContainer[type], true);

		for (i = 0; i < 7; i++)
			ituCheckBoxSetChecked(mainStatusCheckBox[type][i], false);

		switch (type)
		{
		case 0:
			ituMeterSetValue(mainTypeATachoMeter, 0);
			break;
		case 1:
			ituMeterSetValue(mainTypeDTachoMeter, 0);
			ituMeterSetValue(mainTypeDSpeedMeter, 0);
			break;
		case 2:
			for (i = 0; i < 3; i++)
				ituMeterSetValue(mainTypeCMeter[i], 0);
			break;
		}

		ituTextSetString(mainSpeedText[type], "0");

		ituSpriteGoto(mainTypeAStallsSprite, 0);

		ituAnimationStop(mainTypeAStallsAnimation);

		//ituWidgetSetVisible(mainReadyAnimation, true);
		//ituAnimationPlay(mainReadyAnimation, 0);
		ituTextSetString(mainTotalOdoText[type], "14852.2");
		ituTextSetString(mainTripAText[type], "862.2");
		ituTextSetString(mainTripBText[type], "1436.9");
	}
}

void MainStallsSetString(int stallIndex)
{
	int i;
	for (i = 0; i < 4; i++)
		ituTextSetString(mainStallsText[curType - 1][i], stallName[stallIndex]);
	for (i = 4; i < 8; i++)
		ituTextSetString(mainStallsText[curType - 1][i], stallName[(stallIndex + 10) % 11]);
	for (i = 8; i < 10; i++)
		ituTextSetString(mainStallsText[curType - 1][i], stallName[(stallIndex + 8) % 11]);
	for (i = 10; i < 12; i++)
		ituTextSetString(mainStallsText[curType - 1][i], stallName[(stallIndex + 9) % 11]);
	for (i = 12; i < 14; i++)
		ituTextSetString(mainStallsText[curType - 1][i], stallName[(stallIndex + 1) % 11]);
	for (i = 14; i < 16; i++)
		ituTextSetString(mainStallsText[curType - 1][i], stallName[(stallIndex + 2) % 11]);
}

bool MainOnLeave(ITUWidget* widget, char* param)
{
	//MainSwitchType(2, false);
	
	return true;
}