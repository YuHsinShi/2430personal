#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "SDL/SDL.h"
#include "ite/itu.h"
#include <sys/time.h>
#include "ctrlboard.h"
#include "obd2_handle.h"


#define fordebug
#define GESTURE_THRESHOLD 40
#define MOUSEDOWN_LONGPRESS_DELAY   1000
#define DRAW_PER_GAP 4
static ITUScene theScene;
struct timeval tv;
struct tm* tm;

#ifdef fordebug
static int debug = 0;
static bool reverse = false;
#define debugSpeed 1
#endif
static int rgbIndex = 0, demoIndex = 0, curType = 1, stallIndex = 1;//curType = 0
static bool testing = true, errorShow = false, switchType = false, stallsDemo = false;
static char widgetName[30], *stallName[11];

//for CAR ECU status
static int gaslevel = 0;
static int kmh = 0;
static int temperature = 0;
static int RPM = 0;



static uint32_t lastTick = 0;
static ITUText *mainSpeedText[3] = { 0 };
static ITUText *mainTotalOdoText[3] = { 0 };
static ITUText *mainTripAText[3] = { 0 };
static ITUText *mainTripBText[3] = { 0 };
//static ITUText *mainTypeAMusicNameText = 0;
static ITUText *mainStallsText[2][16] = { 0 };
//static ITUMeter *mainTypeATachoMeter = 0;
static ITUMeter *mainTypeDTachoMeter = 0;
static ITUMeter *mainTypeDSpeedMeter = 0;
static ITUMeter *mainTypeCMeter[3] = { 0 };
//static ITUSprite *mainTypeAFuelSprite = 0;
static ITUSprite *mainAMPMSprite[3] = { 0 };
//static ITUSprite *mainTypeABatterySprite = 0;
//static ITUSprite *mainTypeAStallsSprite = 0;
static ITUSprite *mainBackgroundSprite[3] = { 0 };
static ITUAnimation *mainTurnLRAnimation[3][2] = { 0 };
//static ITUAnimation *mainTypeAStallsAnimation = 0;
static ITUAnimation *mainMusicAnimation[3][4] = { 0 };
static ITUAnimation *mainBeamAnimation[3][3] = { 0 };
static ITUAnimation *mainReadyAnimation = 0;
static ITUAnimation *mainErrorAnimation = 0;
static ITUAnimation *mainStallsAnimation[2][5] = { 0 };
static ITUContainer *mainBeamContainer[3] = { 0 };
//static ITUTrackBar *mainTypeARadiatorTrackBar = 0;
static ITUCheckBox *mainStatusCheckBox[3][7] = { 0 };
//static ITURadioBox *mainTypeALowBeamRadioBox = 0;
//static ITURadioBox *mainTypeAHighBeamRadioBox = 0;
static ITUBackground *mainErrorBackground = 0;
static ITUButton *mainReTestButton = 0;
static ITUProgressBar *mainTypeDFuelProgressBar = 0;
static ITUProgressBar *mainTypeDTempProgressBar = 0;

static ITUIcon *mainLRIcon[3][2] = { 0 };

static ITUBackground *mainBackground[3][5] = { 0 };

static void MainSwitchType(int, bool);
static bool MainTestingDemo(int);
static void MainTurnLRDemo(int);
static void MainMusicDemo(int);
static void MainBeamDemo(int);
static void MainSignLightDemo(int, int);
static bool MainTachoDemo(int, int);
static bool MainRadiatorDemo(int, int);
static bool MainSpeedDemo(int, int);
static void MainStallsDemo(int, int);
static bool MainFuelDemo(int, int);
static void MainOdoDemo(int, int);
static void MainTestingReset(int);
static void MainStallsSetString(int);

bool MainOnEnter(ITUWidget* widget, char* param)
{
    int i;
    gettimeofday(&tv, NULL);
    tm = localtime(&tv.tv_sec);
    if (!mainSpeedText[0])
    {
        //mainSpeedText[0] = ituSceneFindWidget(&theScene, "mainTypeASpeedText");
        //assert(mainSpeedText[0]);

        mainSpeedText[1] = ituSceneFindWidget(&theScene, "mainTypeCSpeedText");
        assert(mainSpeedText[1]);

        mainSpeedText[2] = ituSceneFindWidget(&theScene, "mainTypeDSpeedText");
        assert(mainSpeedText[2]);

        //mainTotalOdoText[0] = ituSceneFindWidget(&theScene, "mainTypeATotalOdoText");
        //assert(mainTotalOdoText[0]);

        mainTotalOdoText[1] = ituSceneFindWidget(&theScene, "mainTypeCTotalOdoText");
        assert(mainTotalOdoText[1]);

        mainTotalOdoText[2] = ituSceneFindWidget(&theScene, "mainTypeDTotalOdoText");
        assert(mainTotalOdoText[2]);

        //mainTripAText[0] = ituSceneFindWidget(&theScene, "mainTypeATripAText");
        //assert(mainTripAText[0]);

        mainTripAText[1] = ituSceneFindWidget(&theScene, "mainTypeCTripAText");
        assert(mainTripAText[1]);

        mainTripAText[2] = ituSceneFindWidget(&theScene, "mainTypeDTripAText");
        assert(mainTripAText[2]);

        //mainTripBText[0] = ituSceneFindWidget(&theScene, "mainTypeATripBText");
        //assert(mainTripBText[0]);

        mainTripBText[1] = ituSceneFindWidget(&theScene, "mainTypeCTripBText");
        assert(mainTripBText[1]);

        mainTripBText[2] = ituSceneFindWidget(&theScene, "mainTypeDTripBText");
        assert(mainTripBText[2]);

        //mainTypeAMusicNameText = ituSceneFindWidget(&theScene, "mainTypeAMusicNameText");
        //assert(mainTypeAMusicNameText);

        for (i = 0; i < 16; i++){
            snprintf(widgetName, sizeof(widgetName), "mainTypeCStallsText%d", i);
            mainStallsText[0][i] = ituSceneFindWidget(&theScene, widgetName);
            assert(mainStallsText[0][i]);

            snprintf(widgetName, sizeof(widgetName), "mainTypeDStallsText%d", i);
            mainStallsText[1][i] = ituSceneFindWidget(&theScene, widgetName);
            assert(mainStallsText[1][i]);
        }

        //mainTypeATachoMeter = ituSceneFindWidget(&theScene, "mainTypeAMeter");
        //assert(mainTypeATachoMeter);

        mainTypeDTachoMeter = ituSceneFindWidget(&theScene, "mainTypeDMeter0");
        assert(mainTypeDTachoMeter);

        mainTypeDSpeedMeter = ituSceneFindWidget(&theScene, "mainTypeDMeter1");
        assert(mainTypeDSpeedMeter);

        for (i = 0; i < 3; i++){
            snprintf(widgetName, sizeof(widgetName), "mainTypeCMeter%d", i);
            mainTypeCMeter[i] = ituSceneFindWidget(&theScene, widgetName);
            assert(mainTypeCMeter[i]);
        }

        //mainTypeAFuelSprite = ituSceneFindWidget(&theScene, "mainTypeAFuelSprite");
        //assert(mainTypeAFuelSprite);

        //mainAMPMSprite[0] = ituSceneFindWidget(&theScene, "mainTypeAAMPMSprite");
        //assert(mainAMPMSprite[0]);

        mainAMPMSprite[1] = ituSceneFindWidget(&theScene, "mainTypeCAMPMSprite");
        assert(mainAMPMSprite[1]);

        mainAMPMSprite[2] = ituSceneFindWidget(&theScene, "mainTypeDAMPMSprite");
        assert(mainAMPMSprite[2]);

        //mainTypeABatterySprite = ituSceneFindWidget(&theScene, "mainTypeABatterySprite");
        //assert(mainTypeABatterySprite);

        //mainTypeAStallsSprite = ituSceneFindWidget(&theScene, "mainTypeAStallsSprite");
        //assert(mainTypeAStallsSprite);

        //mainBackgroundSprite[0] = ituSceneFindWidget(&theScene, "mainTypeABackgroundSprite");
        //assert(mainBackgroundSprite[0]);

        mainBackgroundSprite[1] = ituSceneFindWidget(&theScene, "mainTypeCBackgroundSprite");
        assert(mainBackgroundSprite[1]);

        mainBackgroundSprite[2] = ituSceneFindWidget(&theScene, "mainTypeDBackgroundSprite");
        assert(mainBackgroundSprite[2]);

        //mainTurnLRAnimation[0][0] = ituSceneFindWidget(&theScene, "mainTypeARightAnimation");
        //assert(mainTurnLRAnimation[0][0]);

        //mainTurnLRAnimation[0][1] = ituSceneFindWidget(&theScene, "mainTypeALeftAnimation");
        //assert(mainTurnLRAnimation[0][1]);

        mainTurnLRAnimation[1][0] = ituSceneFindWidget(&theScene, "mainTypeCRightAnimation");
        assert(mainTurnLRAnimation[1][0]);

        mainTurnLRAnimation[1][1] = ituSceneFindWidget(&theScene, "mainTypeCLeftAnimation");
        assert(mainTurnLRAnimation[1][1]);

        mainTurnLRAnimation[2][0] = ituSceneFindWidget(&theScene, "mainTypeDRightAnimation");
        assert(mainTurnLRAnimation[2][0]);

        mainTurnLRAnimation[2][1] = ituSceneFindWidget(&theScene, "mainTypeDLeftAnimation");
        assert(mainTurnLRAnimation[2][1]);

        //mainTypeAStallsAnimation = ituSceneFindWidget(&theScene, "mainTypeAStallsAnimation");
        //assert(mainTypeAStallsAnimation);

        for (i = 0; i < 4; i++){
            //snprintf(widgetName, sizeof(widgetName), "mainTypeAMusicAnimation%d", i);
            //mainMusicAnimation[0][i] = ituSceneFindWidget(&theScene, widgetName);
            //assert(mainMusicAnimation[0][i]);

            snprintf(widgetName, sizeof(widgetName), "mainTypeCMusicAnimation%d", i);
            mainMusicAnimation[1][i] = ituSceneFindWidget(&theScene, widgetName);
            assert(mainMusicAnimation[1][i]);

            snprintf(widgetName, sizeof(widgetName), "mainTypeDMusicAnimation%d", i);
            mainMusicAnimation[2][i] = ituSceneFindWidget(&theScene, widgetName);
            assert(mainMusicAnimation[2][i]);
        }

        //mainBeamAnimation[0][0] = ituSceneFindWidget(&theScene, "mainTypeATempAnimation");
        //assert(mainBeamAnimation[0][0]);

        mainBeamAnimation[1][0] = ituSceneFindWidget(&theScene, "mainTypeCTempAnimation");
        assert(mainBeamAnimation[1][0]);

        mainBeamAnimation[2][0] = ituSceneFindWidget(&theScene, "mainTypeDTempAnimation");
        assert(mainBeamAnimation[2][0]);

        for (i = 1; i < 3; i++){
            //snprintf(widgetName, sizeof(widgetName), "mainTypeABeamAnimation%d", i-1);
            //mainBeamAnimation[0][i] = ituSceneFindWidget(&theScene, widgetName);
            //assert(mainBeamAnimation[0][i]);

            snprintf(widgetName, sizeof(widgetName), "mainTypeCBeamAnimation%d", i - 1);
            mainBeamAnimation[1][i] = ituSceneFindWidget(&theScene, widgetName);
            assert(mainBeamAnimation[1][i]);

            snprintf(widgetName, sizeof(widgetName), "mainTypeDBeamAnimation%d", i - 1);
            mainBeamAnimation[2][i] = ituSceneFindWidget(&theScene, widgetName);
            assert(mainBeamAnimation[2][i]);
        }

        mainReadyAnimation = ituSceneFindWidget(&theScene, "mainReadyAnimation");
        assert(mainReadyAnimation);

        mainErrorAnimation = ituSceneFindWidget(&theScene, "mainErrorAnimation");
        assert(mainErrorAnimation);

        for (i = 0; i < 5; i++){
            snprintf(widgetName, sizeof(widgetName), "mainTypeCStallsAnimation%d", i);
            mainStallsAnimation[0][i] = ituSceneFindWidget(&theScene, widgetName);
            assert(mainStallsAnimation[0][i]);

            snprintf(widgetName, sizeof(widgetName), "mainTypeDStallsAnimation%d", i);
            mainStallsAnimation[1][i] = ituSceneFindWidget(&theScene, widgetName);
            assert(mainStallsAnimation[1][i]);
        }

        //mainBeamContainer[0] = ituSceneFindWidget(&theScene, "mainTypeABeamContainer");
        //assert(mainBeamContainer[0]);

        mainBeamContainer[1] = ituSceneFindWidget(&theScene, "mainTypeCBeamContainer");
        assert(mainBeamContainer[1]);

        mainBeamContainer[2] = ituSceneFindWidget(&theScene, "mainTypeDBeamContainer");
        assert(mainBeamContainer[2]);

        //mainTypeARadiatorTrackBar = ituSceneFindWidget(&theScene, "mainTypeARadiatorTrackBar");
        //assert(mainTypeARadiatorTrackBar);

        for (i = 0; i < 7; i++){
            //snprintf(widgetName, sizeof(widgetName), "mainTypeAStatusCheckBox%d", i);
            //mainStatusCheckBox[0][i] = ituSceneFindWidget(&theScene, widgetName);
            //assert(mainStatusCheckBox[0][i]);

            snprintf(widgetName, sizeof(widgetName), "mainTypeCStatusCheckBox%d", i);
            mainStatusCheckBox[1][i] = ituSceneFindWidget(&theScene, widgetName);
            assert(mainStatusCheckBox[1][i]);

            snprintf(widgetName, sizeof(widgetName), "mainTypeDStatusCheckBox%d", i);
            mainStatusCheckBox[2][i] = ituSceneFindWidget(&theScene, widgetName);
            assert(mainStatusCheckBox[2][i]);
        }

        //mainTypeALowBeamRadioBox = ituSceneFindWidget(&theScene, "mainTypeALowBeamRadioBox");
        //assert(mainTypeALowBeamRadioBox);

        //mainTypeAHighBeamRadioBox = ituSceneFindWidget(&theScene, "mainTypeAHighBeamRadioBox");
        //assert(mainTypeAHighBeamRadioBox);

        mainErrorBackground = ituSceneFindWidget(&theScene, "mainErrorBackground");
        assert(mainErrorBackground);

        mainReTestButton = ituSceneFindWidget(&theScene, "mainReTestButton");
        assert(mainReTestButton);

        //mainBackground[0][0] = ituSceneFindWidget(&theScene, "mainTypeAMidBackground");
        //assert(mainBackground[0][0]);

        //mainBackground[0][1] = ituSceneFindWidget(&theScene, "mainTypeARtBackground");
        //assert(mainBackground[0][1]);

        //mainBackground[0][2] = ituSceneFindWidget(&theScene, "mainTypeALfBackground");
        //assert(mainBackground[0][2]);

        //mainBackground[0][3] = ituSceneFindWidget(&theScene, "mainTypeATopBackground");
        //assert(mainBackground[0][3]);

        //mainBackground[0][4] = ituSceneFindWidget(&theScene, "mainTypeABotBackground");
        //assert(mainBackground[0][4]);

        mainBackground[1][0] = ituSceneFindWidget(&theScene, "mainTypeCMidBackground");
        assert(mainBackground[1][0]);

        mainBackground[1][1] = ituSceneFindWidget(&theScene, "mainTypeCRtBackground");
        assert(mainBackground[1][1]);

        mainBackground[1][2] = ituSceneFindWidget(&theScene, "mainTypeCLfBackground");
        assert(mainBackground[1][2]);

        mainBackground[1][3] = ituSceneFindWidget(&theScene, "mainTypeCTopBackground");
        assert(mainBackground[1][3]);

        mainBackground[1][4] = ituSceneFindWidget(&theScene, "mainTypeCBotBackground");
        assert(mainBackground[1][4]);

        mainBackground[2][0] = ituSceneFindWidget(&theScene, "mainTypeDTopBackground");
        assert(mainBackground[2][0]);

        mainBackground[2][1] = ituSceneFindWidget(&theScene, "mainTypeDMidBackground");
        assert(mainBackground[2][1]);

        mainBackground[2][2] = ituSceneFindWidget(&theScene, "mainTypeDBotBackground");
        assert(mainBackground[2][2]);

        mainTypeDFuelProgressBar = ituSceneFindWidget(&theScene, "mainTypeDFuelProgressBar");
        assert(mainTypeDFuelProgressBar);

        mainTypeDTempProgressBar = ituSceneFindWidget(&theScene, "mainTypeDTempProgressBar");
        assert(mainTypeDTempProgressBar);        

        //mainLRIcon[0][0] = ituSceneFindWidget(&theScene, "mainTypeALeftIcon");
        //assert(mainLRIcon[0][0]);

        //mainLRIcon[0][1] = ituSceneFindWidget(&theScene, "mainTypeARightIcon");
        //assert(mainLRIcon[0][1]);

        mainLRIcon[1][0] = ituSceneFindWidget(&theScene, "mainTypeCLeftIcon");
        assert(mainLRIcon[1][0]);

        mainLRIcon[1][1] = ituSceneFindWidget(&theScene, "mainTypeCRightIcon");
        assert(mainLRIcon[1][1]);

        mainLRIcon[2][0] = ituSceneFindWidget(&theScene, "mainTypeDLeftIcon");
        assert(mainLRIcon[2][0]);

        mainLRIcon[2][1] = ituSceneFindWidget(&theScene, "mainTypeDRightIcon");
        assert(mainLRIcon[2][1]);

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

    AudioPlayMusic("A:/sounds/sound.mp3", NULL);
    
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
			//curType = (curType + 1) % 3;
			if (curType == 1)
				curType = 2;
			else
				curType = 1;
			//curType = (curType + 1) % 3;
			MainSwitchType(curType, true);
            AudioPlayMusic("A:/sounds/sound.mp3", NULL);
			switchType = false;
			updated = true;
		}
	}

    
    if(MainTestingDemo(curType))
        updated = true;
#if 0
	if (testing) {
		MainTestingDemo(curType);
		updated = true;
	}
	else if (diff > 6500 && ituWidgetIsVisible(mainReadyAnimation)){
		ituWidgetSetVisible(mainReadyAnimation, false);
		ituWidgetSetVisible(mainReTestButton, true);
		updated = true;
	}
#endif
	// auto demo
	if (diff > 15000)
	{
		switchType = true;
		MainSwitchType(curType, false);
		lastTick = SDL_GetTicks();
	}

	// auto demo
	if (tm->tm_hour > 12)
		ituSpriteGoto(mainAMPMSprite[curType], 0);
	else
		ituSpriteGoto(mainAMPMSprite[curType], 1);

	return updated;
}

bool MainReTestButtonOnPress(ITUWidget* widget, char* param)
{
    if (!errorShow){
        ituWidgetSetVisible(mainErrorBackground, false);
        ituAnimationStop(mainErrorAnimation);
        testing = true;
#ifdef fordebug
        debug = 0;
        reverse = false;
#endif
        stallsDemo = true;
        ituWidgetSetVisible(mainReTestButton, false);
        lastTick = SDL_GetTicks();
        errorShow = !errorShow;
    }
    else{
        testing = false;
        errorShow = !errorShow;
        ituWidgetSetVisible(mainErrorBackground, true);
        ituAnimationPlay(mainErrorAnimation, 0);
    }
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
    ituSpriteGoto(mainBackgroundSprite[curType], ++rgbIndex % 3);
    if (++demoIndex > 2) {
        switchType = true;
        MainSwitchType(curType, false);
    }
    return true;
}

bool mainStallsAnimationOnStop(ITUWidget* widget, char* param)
{
    int i;
    stallIndex = (stallIndex + 1) % 11;
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
    if (type != 2)
    for (i = 0; i < 5; i++)
        ituWidgetSetVisible(mainBackground[type][i], show);
    else
    for (i = 0; i < 3; i++)
        ituWidgetSetVisible(mainBackground[type][i], show);

    demoIndex = 0;
    rgbIndex = 0;
}

bool MainTestingDemo(int type)
{
    bool updateR = false;
	//MainTurnLRDemo(type);
	//MainMusicDemo(type);
	//MainBeamDemo(type);
	//MainSignLightDemo(type, signInterval);
	//MainTachoDemo(type, tacho);
	updateR |= MainRadiatorDemo(type, OBD2_GETCOOLANTTEMP());
	updateR |= MainSpeedDemo(type, OBD2_GETSPEED());
    updateR |= MainTachoDemo(type, OBD2_GETRPM());
	updateR |= MainFuelDemo(type, OBD2_GETFUEL());
    //MainStallsDemo(type, diff);
	//MainOdoDemo(type, diff);
	return updateR;

}

void MainTurnLRDemo(int type)
{
    int i;
    if (!ituWidgetIsVisible(mainTurnLRAnimation[curType][0])) {

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

bool MainRadiatorDemo(int type, int Value)
{
    bool updateR = false;
    
    if(Value > 100)
        Value = 100;
    
    if(Value <= 0)
    {
        temperature = 0;
        updateR = true;
    }
    else if(abs(temperature - Value) > (DRAW_PER_GAP))
    {
        if(Value > temperature)
        {
            temperature += DRAW_PER_GAP;
            updateR = true;
        }
        else if(Value < temperature)
        {
            temperature -= DRAW_PER_GAP;
            updateR = true;
        }

    }
    
	switch (type)
	{
	case 1:
		ituMeterSetValue(mainTypeCMeter[1], temperature);
		break;
    case 2:
		mainTypeDTempProgressBar->value = temperature;
		break;
	default:
		break;
	}
    
    return updateR;
}

bool MainTachoDemo(int type, int Value)
{
    bool updateR = false;
    int  Rpm_normalization = 0;

    if(Value == 0)
    {
        RPM = 0;
        updateR = true;
    }
    else
    {
        if(Value > 8000)
            Value = 8000;
        
        Rpm_normalization = Value / 80;

        if(abs(RPM - Rpm_normalization) > DRAW_PER_GAP)
        {
            if(Rpm_normalization > RPM)
            {
                RPM += DRAW_PER_GAP;
                updateR = true;
            }
            else if(Rpm_normalization < RPM)
            {
                RPM -= DRAW_PER_GAP;
                updateR = true;
            }
        }
    }
    
	switch (type)
	{
	case 1:
		break;
	case 2:
	    ituMeterSetValue(mainTypeDTachoMeter, RPM);
		break;
	default:
		break;
	}
    return updateR;
}

bool MainSpeedDemo(int type, int Value)
{
	char speed[10];
	
    bool updateR = true;

    if(abs(kmh - Value) > (DRAW_PER_GAP) || Value == 0)
    {
        if(Value > kmh)
        {
            kmh += DRAW_PER_GAP;
        }
        else if(Value < kmh)
        {
            kmh -= DRAW_PER_GAP;
        }

    }
    
	if (Value < 300){
		sprintf(speed, "%d", Value);
		ituTextSetString(mainSpeedText[type], speed);
	}
	else
		ituTextSetString(mainSpeedText[type], "299");

	switch (type)
	{
	case 1:
		if (kmh < 240)
			ituMeterSetValue(mainTypeCMeter[0], kmh);
		else
			ituMeterSetValue(mainTypeCMeter[0], 240);
		break;
	case 2:
		if (kmh < 260)
			ituMeterSetValue(mainTypeDSpeedMeter, kmh);
		else
			ituMeterSetValue(mainTypeDSpeedMeter, 260);
		break;
	default:
		break;
	}
    return updateR;

}

void MainStallsDemo(int type, int diff)
{
    int i;
    if (type == 0){
        //if (mainTypeAStallsAnimation->playing == 0)
        //    ituAnimationPlay(mainTypeAStallsAnimation, 0);
        //if (diff / 450 < 11)
        //    ituSpriteGoto(mainTypeAStallsSprite, diff / 450);
        //else
        //    ituSpriteGoto(mainTypeAStallsSprite, 10);
    }
    else {
        if (mainStallsAnimation[type - 1][0]->playing == 0 && stallsDemo)
        for (i = 0; i < 5; i++)
            ituAnimationPlay(mainStallsAnimation[type - 1][i], 0);
    }
}

bool MainFuelDemo(int type, int Value)
{
    bool updateR = false;
    
    if(abs(gaslevel - Value) > (DRAW_PER_GAP))
    {
        if(Value > gaslevel)
        {
            gaslevel += DRAW_PER_GAP;
            updateR = true;
        }
        else if(Value < gaslevel)
        {
            gaslevel -= DRAW_PER_GAP;
            updateR = true;
        }      
    }
	switch (type)
	{
	case 1:
		ituMeterSetValue(mainTypeCMeter[2], 100 - gaslevel);
		break;
	case 2:
		mainTypeDFuelProgressBar->value = gaslevel;
		break;
	default:
		break;
	}
    
    return updateR;

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
            //case 0:
            //    ituMeterSetValue(mainTypeATachoMeter, 0);
            //    break;
        case 1:
            for (i = 0; i < 3; i++)
                ituMeterSetValue(mainTypeCMeter[i], 0);
            break;
        case 2:
            ituMeterSetValue(mainTypeDTachoMeter, 0);
            ituMeterSetValue(mainTypeDSpeedMeter, 0);
            break;
        }

        ituTextSetString(mainSpeedText[type], "0");

        //ituSpriteGoto(mainTypeAStallsSprite, 0);

        //ituAnimationStop(mainTypeAStallsAnimation);

        ituWidgetSetVisible(mainReadyAnimation, true);
        ituAnimationPlay(mainReadyAnimation, 0);
        ituTextSetString(mainTotalOdoText[type], "14852.2");
        ituTextSetString(mainTripAText[type], "862.2");
        ituTextSetString(mainTripBText[type], "1436.9");
        testing = false;
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

ITUActionFunction actionFunctions[] =
{
    "MainOnEnter", MainOnEnter,
    "MainOnTimer", MainOnTimer,
    "MainReTestButtonOnPress", MainReTestButtonOnPress,
    "MainMusicAnimationOnStop", MainMusicAnimationOnStop,
    "MainBeamAnimationOnStop", MainBeamAnimationOnStop,
    "MainNextBGButtonOnPress", MainNextBGButtonOnPress,
    "mainStallsAnimationOnStop", mainStallsAnimationOnStop,
    NULL, NULL
};

int SDL_main(int argc, char *argv[])
{
    SDL_Window *window;
    SDL_Event ev;
    int done, delay, lastx, lasty;
    uint32_t tick, dblclk, lasttick, mouseDownTick;


    /* Initialize SDL */
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        printf("Couldn't initialize SDL: %s\n", SDL_GetError());
        return (1);
    }
    window = SDL_CreateWindow("test_dashboard",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        1280, 480, 0);
    if (!window)
    {
        printf("Couldn't create 1280x480 window: %s\n",
            SDL_GetError());
        SDL_Quit();
        exit(2);
    }

    // init itu
    ituLcdInit();

#ifdef CFG_M2D_ENABLE
    ituM2dInit();
#else
    ituSWInit();
#endif
    ConfigInit();
    StorageInit();
    AudioInit();
        
    ituFtInit();
    ituFtLoadFont(0, CFG_PRIVATE_DRIVE ":/font/wqyMicroHei.ttf", ITU_GLYPH_8BPP);
    //OBD2 task init
    OBD2TASKInit();
    // load itu file
    tick = SDL_GetTicks();

    ituSceneInit(&theScene, NULL);
    ituSceneSetFunctionTable(&theScene, actionFunctions);
    ituSceneLoadFile(&theScene, CFG_PRIVATE_DRIVE ":/ctrlboard.itu");

    printf("loading time: %dms\n", SDL_GetTicks() - tick);

    theScene.leftKey = SDLK_LEFT;
    theScene.upKey = SDLK_UP;
    theScene.rightKey = SDLK_RIGHT;
    theScene.downKey = SDLK_DOWN;

    /* Watch keystrokes */
    done = dblclk = lasttick = lastx = lasty = mouseDownTick = 0;
    while (!done)
    {
        bool result = false;

        tick = SDL_GetTicks();

        while (SDL_PollEvent(&ev))
        {
            switch (ev.type)
            {
            case SDL_MOUSEMOTION:
                result = ituSceneUpdate(&theScene, ITU_EVENT_MOUSEMOVE, ev.button.button, ev.button.x, ev.button.y);
                break;

            case SDL_MOUSEBUTTONDOWN:
                mouseDownTick = SDL_GetTicks();
                if (mouseDownTick - dblclk <= 300)
                {
                    result = ituSceneUpdate(&theScene, ITU_EVENT_MOUSEDOUBLECLICK, ev.button.button, ev.button.x, ev.button.y);
                    dblclk = mouseDownTick = 0;
                }
                else
                {
                    result = ituSceneUpdate(&theScene, ITU_EVENT_MOUSEDOWN, ev.button.button, ev.button.x, ev.button.y);
                    dblclk = mouseDownTick;
                    lastx = ev.button.x;
                    lasty = ev.button.y;
                }
                break;

            case SDL_MOUSEBUTTONUP:
                if (SDL_GetTicks() - dblclk <= 300)
                {
                    int xdiff = abs(ev.button.x - lastx);
                    int ydiff = abs(ev.button.y - lasty);

                    if (xdiff >= GESTURE_THRESHOLD)
                    {
                        if (ev.button.x > lastx)
                        {
                            printf("mouse: slide to right\n");
                            result |= ituSceneUpdate(&theScene, ITU_EVENT_TOUCHSLIDERIGHT, 1, ev.button.x, ev.button.y);
                        }
                        else
                        {
                            printf("mouse: slide to left\n");
                            result |= ituSceneUpdate(&theScene, ITU_EVENT_TOUCHSLIDELEFT, 1, ev.button.x, ev.button.y);
                        }
                    }
                    else if (ydiff >= GESTURE_THRESHOLD)
                    {
                        if (ev.button.y > lasty)
                        {
                            printf("mouse: slide to down\n");
                            result |= ituSceneUpdate(&theScene, ITU_EVENT_TOUCHSLIDEDOWN, 1, ev.button.x, ev.button.y);
                        }
                        else
                        {
                            printf("mouse: slide to up\n");
                            result |= ituSceneUpdate(&theScene, ITU_EVENT_TOUCHSLIDEUP, 1, ev.button.x, ev.button.y);
                        }
                    }
                }
                result |= ituSceneUpdate(&theScene, ITU_EVENT_MOUSEUP, ev.button.button, ev.button.x, ev.button.y);
                mouseDownTick = 0;
                break;

            case SDL_FINGERMOTION:
                printf("touch: move %d, %d\n", ev.tfinger.x, ev.tfinger.y);
                result = ituSceneUpdate(&theScene, ITU_EVENT_MOUSEMOVE, 1, ev.tfinger.x, ev.tfinger.y);
                break;

            case SDL_FINGERDOWN:
                printf("touch: down %d, %d\n", ev.tfinger.x, ev.tfinger.y);
                {
                    mouseDownTick = SDL_GetTicks();
                    if (mouseDownTick - dblclk <= 300)
                    {
                        result = ituSceneUpdate(&theScene, ITU_EVENT_MOUSEDOUBLECLICK, 1, ev.tfinger.x, ev.tfinger.y);
                        dblclk = mouseDownTick = 0;
                    }
                    else
                    {
                        result = ituSceneUpdate(&theScene, ITU_EVENT_MOUSEDOWN, 1, ev.tfinger.x, ev.tfinger.y);
                        dblclk = mouseDownTick;
                        lastx = ev.tfinger.x;
                        lasty = ev.tfinger.y;
                    }
                }
                break;

            case SDL_FINGERUP:
                printf("touch: up %d, %d\n", ev.tfinger.x, ev.tfinger.y);
                if (SDL_GetTicks() - dblclk <= 300)
                {
                    int xdiff = abs(ev.tfinger.x - lastx);
                    int ydiff = abs(ev.tfinger.y - lasty);

                    if (xdiff >= GESTURE_THRESHOLD)
                    {
                        if (ev.tfinger.x > lastx)
                        {
                            printf("touch: slide to right %d %d\n", ev.tfinger.x, ev.tfinger.y);
                            result |= ituSceneUpdate(&theScene, ITU_EVENT_TOUCHSLIDERIGHT, 1, ev.tfinger.x, ev.tfinger.y);
                        }
                        else
                        {
                            printf("touch: slide to left %d %d\n", ev.button.x, ev.button.y);
                            result |= ituSceneUpdate(&theScene, ITU_EVENT_TOUCHSLIDELEFT, 1, ev.tfinger.x, ev.tfinger.y);
                        }
                    }
                    else if (ydiff >= GESTURE_THRESHOLD)
                    {
                        if (ev.tfinger.y > lasty)
                        {
                            printf("touch: slide to down %d %d\n", ev.tfinger.x, ev.tfinger.y);
                            result |= ituSceneUpdate(&theScene, ITU_EVENT_TOUCHSLIDEDOWN, 1, ev.tfinger.x, ev.tfinger.y);
                        }
                        else
                        {
                            printf("touch: slide to up %d %d\n", ev.tfinger.x, ev.tfinger.y);
                            result |= ituSceneUpdate(&theScene, ITU_EVENT_TOUCHSLIDEUP, 1, ev.tfinger.x, ev.tfinger.y);
                        }
                    }
                }
                result |= ituSceneUpdate(&theScene, ITU_EVENT_MOUSEUP, 1, ev.tfinger.x, ev.tfinger.y);
                mouseDownTick = 0;
                break;

            case SDL_QUIT:
                done = 1;
                break;
            }
        }

        if (mouseDownTick > 0 && (SDL_GetTicks() - mouseDownTick >= MOUSEDOWN_LONGPRESS_DELAY))
        {
            printf("long press: %d %d\n", lastx, lasty);
            result |= ituSceneUpdate(&theScene, ITU_EVENT_MOUSELONGPRESS, 1, lastx, lasty);
            mouseDownTick = 0;
        }

        result |= ituSceneUpdate(&theScene, ITU_EVENT_TIMER, 0, 0, 0);
        if (result)
        {
            ituSceneDraw(&theScene, ituGetDisplaySurface());
            ituFlip(ituGetDisplaySurface());
        }

        delay = 17- (SDL_GetTicks() - tick);
        if (delay > 0)
        {
            SDL_Delay(delay);
        }
    }

    AudioExit();
    ConfigExit();
    SDL_Quit();
    return (0);
}
