#include <assert.h>
#include <sys/time.h>
#include "scene.h"
#include "audio_mgr.h"
#include "SDL/SDL.h"
#include "leaf/leaf_mediastream.h"

static ITUBackground* digitalTypeOpeningBg;
static ITUBackground* digitalTypeOpeningBackground[6];
static ITUBackground* digitalTypeCentrolBackground;
static ITUButton* digitalTypeBackButton;
static ITUButton* digitalTypePlayButton;
static ITUButton* digitalTypeChangeUIButton;
static ITUCircleProgressBar* digitalTypeOpenCircleProgressBar;
static ITUMeter* digitalTypeOpenMeter;
static ITUIcon* digitalTypeREARIcon;

static ITUAnimation*  digitalTypeDirectionAnimation[2];
static ITUAnimation*  digitalTypeBottomAnimation;
static ITUAnimation* digitalTypeFuelAnimation;
static ITUAnimation* digitalTypeTempAnimation;
static ITUAnimation* digitalTypeSpeedAnimation;

static ITUIcon* digitalTypeTempPointIcon[2];
static ITUIcon* digitalTypeFuelPointIcon[2];

static ITUSprite* digitalTypeModeSprite;
static ITUSprite* digitalTypeGearLeverSprite;
static ITUSprite* digitalTypeDirectionSprite[2][2];
static ITUSprite* digitalTypInfoUpSprite;
static ITUSprite* digitalTypInfoDownSprite;
static ITUBackground* digitalTypeInfoBackground;
static ITUAnimation* digitalTypeInfoAnimation;
static ITUIcon* digitalTypeInfoIcon[6];
static ITUIcon* digitalTypeInfoAnIcon[2][2];

static ITUProgressBar* digitalTypeFuelProgressBar[2];
static ITUProgressBar* digitalTypeTempProgressBar[2];
static ITUBackground* digitalTypeSpeedBackground1;
static ITUCircleProgressBar* digitalTypeSpeedCircleProgressBar[2];//260~0
static ITUText* digitalTypeSpeedText[2]; //0~260
static ITUText* digitalTypeODOText[2]; //00000000000~9999999999

static ITUBackground* digitalTypeInfoTimeBackground[2];
static ITUDigitalClock* digitalTypeInfoDigitalClock1;
static ITUSprite* digitalTypeInfoMonthSprite[2];
static ITUSprite* digitalTypeInfoAMPMSprite[2];

static int speedStep = 1;


static bool opne_progress_done = false;
static bool is_playing = false;
static uint32_t lastTick = 0;
static bool temp_is_add = false;
static bool fuel_is_add = true;
static bool speed_is_add = true;
static bool next_info = true;
static bool change_info = false;
static int direction_frame = 0;
static int temperature = 110;
static int fuel = 0;
static int speed = 0;
static int odo = 0;
static int is_central_pos = true;
static int info_index = 0;
static int demo_loop = 0;

struct timeval tv;
struct tm* tm;

int cur_month;//0~11
int pre_month;//0~11
int cur_is_pm = 0;
int pre_is_pm = 0;


//int TemperaturePointX[16] = {37,28,20,13,8,4,1,0,0,1,4,8,13,20,28,37};//temperature range: 15~30
//int TemperaturePointY[16] = { 315,294,273,252,231,210,189,168,147,126,105,84,63,42,21,0};//temperature range: 15~30
int TemperaturePointX[86] = {
	37,35,34,32,30,29,27,26,24,23,21,20,19,17,16,15,14,13,12,11,
	10,9,8,8,7,6,5,5,4,4,3,3,2,2,1,1,1,1,0,0,
	0,0,0,0,0,0,0,0,1,1,1,1,2,2,3,3,4,4,5,5,
	6,7,8,8,9,10,11,12,13,14,15,16,17,19,20,21,23,24,26,27,
	29,30,32,34,35,37
};//temperature range: 25~110
int TemperaturePointY[86] = {
	315, 311, 308, 304, 300, 296, 293, 289, 285, 282, 278, 274, 271, 267, 263, 259, 256, 252, 248, 245,
	241, 237, 233, 230, 226, 222, 219, 215, 211, 208, 204, 200, 196, 193, 189, 185, 182, 178, 174, 170,
	167, 163, 159, 156, 152, 148, 145, 141, 137, 133, 130, 126, 122, 119, 115, 111, 107, 104, 100, 96,
	93, 89, 85, 82, 78, 74, 70, 67, 63, 59, 56, 52, 48, 44, 41, 37, 33, 30, 26, 22,
	19, 15, 11, 7, 4, 0
};//temperature range: 25~110

int FuelPointX[101] = {
	18, 20, 21, 23, 24, 26, 27, 28, 30, 31, 32, 33, 34, 36, 37, 38, 39, 40, 41, 42,
	43, 43, 44, 45, 46, 47, 47, 48, 49, 49, 50, 50, 51, 51, 52, 52, 53, 53, 53, 54,
	54, 54, 55, 55, 55, 55, 55, 55, 55, 55, 56, 55, 55, 55, 55, 55, 55, 55, 55, 54,
	54, 54, 53, 53, 53, 52, 52, 51, 51, 50, 50, 49, 49, 48, 47, 47, 46, 45, 44, 43,
	43, 42, 41, 40, 39, 38, 37, 36, 34, 33, 32, 31, 30, 28, 27, 26, 24, 23, 21, 20, 18 };

int FuelPointY[101] = {
	315, 312, 309, 306, 302, 299, 296, 293, 290, 287, 284, 280, 277, 274, 271, 268, 265, 261, 258, 255,
	252, 249, 246, 243, 239, 236, 233, 230, 227, 224, 221, 217, 214, 211, 208, 205, 202, 198, 195, 192,
	189, 186, 183, 180, 176, 173, 170, 167, 164, 161, 158, 154, 151, 148, 145, 142, 139, 135, 132, 129,
	126, 123, 120, 117, 113, 110, 107, 104, 101, 98, 95, 91, 88, 85, 82, 79, 76, 72, 69, 66,
	63, 60, 57, 54, 50, 47, 44, 41, 38, 35, 32, 28, 25, 22, 19, 16, 13, 9, 6, 3, 0 };


bool ModeSpriteDemo(int time);
bool GearSpriteDemo(int time);
bool DirectionSpriteDemo(int time);
bool TempDemo(int loops);
bool FuelDemo(int loop);
bool SpeedDemo(int loop);
bool ODODemo(int loop);
bool InfoDemo(int loop);


bool ModeSpriteDemo(int time)// change in each 200ms 
{
	return true;
	static int frame = 0;
	static int pre_frame = 0;
	int ret = false;

	frame = (time / 200) % 4;
	if (pre_frame != frame)
	{
		ituSpriteGoto(digitalTypeModeSprite, frame);
		pre_frame = frame;
		ret = true;
	}
	return ret;
}

bool GearSpriteDemo(int time)// change in each 200ms 
{
	return true;
	static int frame = 0;
	static int pre_frame = 0;
	bool ret = false;

	frame = (time / 200) % 6;
	if (pre_frame != frame)
	{
		ituSpriteGoto(digitalTypeGearLeverSprite, frame);
		pre_frame = frame;
		ret = true;
	}
	return ret;
}

bool DirectionSpriteDemo(int time)//change in each 500ms
{
	return true;
	static int pre_frame = 0;
	bool ret = false;

	direction_frame = (time / 500) % 2;
	if (pre_frame != direction_frame)
	{
		if (is_central_pos == true)
		{
			ituSpriteGoto(digitalTypeDirectionSprite[0][0], direction_frame);
			ituSpriteGoto(digitalTypeDirectionSprite[1][0], direction_frame);
		}
		else
		{
			ituSpriteGoto(digitalTypeDirectionSprite[0][1], direction_frame);
			ituSpriteGoto(digitalTypeDirectionSprite[1][1], direction_frame);
		}

		pre_frame = direction_frame;
		ret = true;
	}
	return ret;
}

bool TempDemo(int loop)
{
	bool ret = false;

	if (temp_is_add == true)
	{
		temperature = temperature + loop;
		if (temperature > 110)
			temperature = 110;

		if (temperature == 110)
		{
			temp_is_add = false;
		}
	}
	else
	{
		temperature = temperature - loop;
		if (temperature < 25)
			temperature = 25;

		if (temperature == 25)
		{
			temp_is_add = true;
		}
	}
	if (is_central_pos == true)
	{
		ituProgressBarSetValue(digitalTypeTempProgressBar[0], temperature);
		ituWidgetSetPosition(digitalTypeTempPointIcon[0], TemperaturePointX[temperature - 25], TemperaturePointY[temperature - 25]);
	}		
	else
	{
		ituProgressBarSetValue(digitalTypeTempProgressBar[1], temperature);
		ituWidgetSetPosition(digitalTypeTempPointIcon[1], TemperaturePointX[temperature - 25], TemperaturePointY[temperature - 25]);
	}

	ret = true;
	

	return ret;
}

bool FuelDemo(int loop)
{
	bool ret = false;

	if (fuel_is_add == true)
	{
		fuel = fuel + loop;
		if (fuel > 100)
			fuel = 100;

		if (fuel == 100)
		{
			fuel_is_add = false;
		}
	}
	else
	{
		fuel = fuel - loop;
		if (fuel < 0)
			fuel = 0;

		if (fuel == 0)
		{
			fuel_is_add = true;
		}
	}

	if (is_central_pos == true)
	{
		ituProgressBarSetValue(digitalTypeFuelProgressBar[0], fuel);
		ituWidgetSetPosition(digitalTypeFuelPointIcon[0], FuelPointX[fuel], FuelPointY[fuel]);
	}
	else
	{
		ituProgressBarSetValue(digitalTypeFuelProgressBar[1], fuel);
		ituWidgetSetPosition(digitalTypeFuelPointIcon[1], FuelPointX[fuel], FuelPointY[fuel]);
	}
	ret = true;
	
	return ret;
}


bool SpeedDemo(int loop)
{
	bool ret = false;
	char tmp[32];

	if (speed_is_add == true)
	{
		speed = speed + speedStep;
		if (speed >= 260)
		{
			speed = 260;
			speed_is_add = false;
		}
	}
	else
	{
		speed = speed - speedStep;
		if (speed <= 0)
		{
			speed = 0;
			speed_is_add = true;
			demo_loop++;
			if (demo_loop == 2)
			{
				is_playing = false;
				demo_loop = 0;
			}
		}
	}

	sprintf(tmp, "%d", speed);
	if (is_central_pos == true)
	{
		ituCircleProgressBarSetValue(digitalTypeSpeedCircleProgressBar[0], 260 - speed);
		ituTextSetString(digitalTypeSpeedText[0], tmp);
	}
	else
	{
		ituCircleProgressBarSetValue(digitalTypeSpeedCircleProgressBar[1], 260 - speed);
		ituTextSetString(digitalTypeSpeedText[1], tmp);
	}
	ret = true;
	
	return ret;
}

bool ODODemo(int loop)
{
	bool ret = false;
	char tmp[16];

	if (loop % 5 == 0)
	{
		odo++;
		if (odo == 10)
		{
			odo = 0;
		}
		
		sprintf(tmp, "%d%d%d%d%d%d%d%d%d%d", odo, odo, odo, odo, odo, odo, odo, odo, odo, odo);

		if (is_central_pos == true)
			ituTextSetString(digitalTypeODOText[0], tmp);
		else
			ituTextSetString(digitalTypeODOText[1], tmp);
		ret = true;
	}


	return ret;
}

bool InfoDemo(int loop)
{
	bool ret = false;

	if (loop % 100 == 0 && change_info == false)
	{
		if (next_info == true)
		{
			if (info_index == 5)
			{
				printf("Up!\n");
				next_info = false;
				ituSpritePlay(digitalTypInfoUpSprite, 0);
				change_info = true;
			}
			else
			{
				printf("Down! \n");
				ituSpritePlay(digitalTypInfoDownSprite, 0);
				change_info = true;
			}
		}
		else
		{
			if (info_index == 0)
			{
				printf("Down! \n");
				next_info = true;
				ituSpritePlay(digitalTypInfoDownSprite, 0);
				change_info = true;
			}
			else
			{
				printf("Up!\n");
				ituSpritePlay(digitalTypInfoUpSprite, 0);
				change_info = true;
			}
		}

		ret = true;
	}
	return ret;
}

void PlayAnimation()
{
	demo_loop = 0;
	is_playing = true;
}


void ShowOpening(int index)
{
	if (index == 2)
	{
		ituWidgetShow(digitalTypeOpeningBackground[index], ITU_EFFECT_FADE, 30);
		ituWidgetShow(digitalTypeOpeningBackground[index + 1], ITU_EFFECT_SCROLL_UP_FADE, 30);
	}
	else if (index == 1)
	{
		ituWidgetSetVisible(digitalTypeOpeningBackground[index], true);
	}
	else if (index == 6)
	{
		ituWidgetSetVisible(digitalTypeOpeningBg, false);
		ituWidgetSetVisible(digitalTypeCentrolBackground, true);
		is_playing = true;
		lastTick = SDL_GetTicks();
	}
	else
		ituWidgetShow(digitalTypeOpeningBackground[index], ITU_EFFECT_FADE, 30);
}


bool DigitalTypeOnEnter(ITUWidget* widget, char* param)
{
	int i = 0;
	int j = 0;
	char tmp[64];

	gettimeofday(&tv, NULL);
	tm = localtime(&tv.tv_sec);

	if (!digitalTypeOpeningBg)
	{
		digitalTypeOpeningBg = ituSceneFindWidget(&theScene, "digitalTypeOpeningBackground");
		assert(digitalTypeOpeningBg);

		digitalTypeCentrolBackground = ituSceneFindWidget(&theScene, "digitalTypeCentrolBackground");
		assert(digitalTypeCentrolBackground);

		digitalTypeBackButton = ituSceneFindWidget(&theScene, "digitalTypeBackButton");
		assert(digitalTypeBackButton);

		digitalTypePlayButton = ituSceneFindWidget(&theScene, "digitalTypePlayButton");
		assert(digitalTypePlayButton);

		digitalTypeChangeUIButton = ituSceneFindWidget(&theScene, "digitalTypeChangeUIButton");
		assert(digitalTypeChangeUIButton);

		digitalTypeOpenCircleProgressBar = ituSceneFindWidget(&theScene, "digitalTypeOpenCircleProgressBar");
		assert(digitalTypeOpenCircleProgressBar);

		digitalTypeOpenMeter = ituSceneFindWidget(&theScene, "digitalTypeOpenMeter");
		assert(digitalTypeOpenMeter);

		digitalTypeREARIcon = ituSceneFindWidget(&theScene, "digitalTypeREARIcon");
		assert(digitalTypeREARIcon);

		digitalTypeModeSprite = ituSceneFindWidget(&theScene, "digitalTypeModeSprite");
		assert(digitalTypeModeSprite);

		digitalTypeGearLeverSprite = ituSceneFindWidget(&theScene, "digitalTypeGearLeverSprite");
		assert(digitalTypeGearLeverSprite);

		digitalTypeBottomAnimation = ituSceneFindWidget(&theScene, "digitalTypeBottomAnimation");
		assert(digitalTypeBottomAnimation);

		digitalTypeFuelAnimation = ituSceneFindWidget(&theScene, "digitalTypeFuelAnimation");
		assert(digitalTypeFuelAnimation);

		digitalTypeTempAnimation = ituSceneFindWidget(&theScene, "digitalTypeTempAnimation");
		assert(digitalTypeTempAnimation);

		digitalTypeSpeedAnimation = ituSceneFindWidget(&theScene, "digitalTypeSpeedAnimation");
		assert(digitalTypeSpeedAnimation);

		digitalTypeInfoBackground = ituSceneFindWidget(&theScene, "digitalTypeInfoBackground");
		assert(digitalTypeInfoBackground);

		digitalTypInfoUpSprite = ituSceneFindWidget(&theScene, "digitalTypInfoUpSprite");
		assert(digitalTypInfoUpSprite);

		digitalTypInfoDownSprite = ituSceneFindWidget(&theScene, "digitalTypInfoDownSprite");
		assert(digitalTypInfoDownSprite);

		digitalTypeInfoAnimation = ituSceneFindWidget(&theScene, "digitalTypeInfoAnimation");
		assert(digitalTypeInfoAnimation);

		digitalTypeInfoDigitalClock1 = ituSceneFindWidget(&theScene, "digitalTypeInfoDigitalClock1");
		assert(digitalTypeInfoDigitalClock1);

		digitalTypeSpeedBackground1 = ituSceneFindWidget(&theScene, "digitalTypeSpeedBackground1");
		assert(digitalTypeSpeedBackground1);

		for (i = 0; i < 6; i++)
		{
			sprintf(tmp, "digitalTypeOpeningBackground%d", i);
			digitalTypeOpeningBackground[i] = ituSceneFindWidget(&theScene, tmp);
			assert(digitalTypeOpeningBackground[i]);

			sprintf(tmp, "digitalTypeInfoIcon%d", i);
			digitalTypeInfoIcon[i] = ituSceneFindWidget(&theScene, tmp);
			assert(digitalTypeInfoIcon[i]);
		}

		for (i = 0; i < 2; i++)
		{
			sprintf(tmp, "digitalTypeDirectionAnimation%d", i);
			digitalTypeDirectionAnimation[i] = ituSceneFindWidget(&theScene, tmp);
			assert(digitalTypeDirectionAnimation[i]);

			sprintf(tmp, "digitalTypeTempPointIcon%d", i);
			digitalTypeTempPointIcon[i] = ituSceneFindWidget(&theScene, tmp);
			assert(digitalTypeTempPointIcon[i]);

			sprintf(tmp, "digitalTypeFuelPointIcon%d", i);
			digitalTypeFuelPointIcon[i] = ituSceneFindWidget(&theScene, tmp);
			assert(digitalTypeFuelPointIcon[i]);

			sprintf(tmp, "digitalTypeInfoTimeBackground%d", i);
			digitalTypeInfoTimeBackground[i] = ituSceneFindWidget(&theScene, tmp);
			assert(digitalTypeInfoTimeBackground[i]);

			sprintf(tmp, "digitalTypeInfoMonthSprite%d", i);
			digitalTypeInfoMonthSprite[i] = ituSceneFindWidget(&theScene, tmp);
			assert(digitalTypeInfoMonthSprite[i]);

			sprintf(tmp, "digitalTypeInfoAMPMSprite%d", i);
			digitalTypeInfoAMPMSprite[i] = ituSceneFindWidget(&theScene, tmp);
			assert(digitalTypeInfoAMPMSprite[i]);

			for (j = 0; j < 2; j++)
			{
				sprintf(tmp, "digitalTypeDirectionSprite%d%d", i,j);
				digitalTypeDirectionSprite[i][j] = ituSceneFindWidget(&theScene, tmp);
				assert(digitalTypeDirectionSprite[i][j]);

				sprintf(tmp, "digitalTypeInfoAnIcon%d%d", i, j);
				digitalTypeInfoAnIcon[i][j] = ituSceneFindWidget(&theScene, tmp);
				assert(digitalTypeInfoAnIcon[i][j]);
				
			}
			

			sprintf(tmp, "digitalTypeFuelProgressBar%d", i);
			digitalTypeFuelProgressBar[i] = ituSceneFindWidget(&theScene, tmp);
			assert(digitalTypeFuelProgressBar[i]);

			sprintf(tmp, "digitalTypeTempProgressBar%d", i);
			digitalTypeTempProgressBar[i] = ituSceneFindWidget(&theScene, tmp);
			assert(digitalTypeTempProgressBar[i]);

			sprintf(tmp, "digitalTypeSpeedCircleProgressBar%d", i);
			digitalTypeSpeedCircleProgressBar[i] = ituSceneFindWidget(&theScene, tmp);
			assert(digitalTypeSpeedCircleProgressBar[i]);

			sprintf(tmp, "digitalTypeSpeedText%d", i);
			digitalTypeSpeedText[i] = ituSceneFindWidget(&theScene, tmp);
			assert(digitalTypeSpeedText[i]);

			sprintf(tmp, "digitalTypeODOText%d", i);
			digitalTypeODOText[i] = ituSceneFindWidget(&theScene, tmp);
			assert(digitalTypeODOText[i]);
			
		}
	}

	opne_progress_done = false;
	is_central_pos = true;

	ituCircleProgressBarSetValue(digitalTypeOpenCircleProgressBar, 100);
	ituMeterSetValue(digitalTypeOpenMeter, 100);
	ituWidgetSetVisible(digitalTypeBackButton, false);
	ituWidgetSetVisible(digitalTypePlayButton, false);
	ituWidgetSetVisible(digitalTypeChangeUIButton, false);
	ituWidgetSetVisible(digitalTypeREARIcon, true);
	ituWidgetSetVisible(digitalTypeCentrolBackground, false);
	

	ituSpriteGoto(digitalTypeModeSprite, 0);
	ituSpriteGoto(digitalTypeGearLeverSprite, 0);
	ituSpriteGoto(digitalTypeDirectionSprite[0][0], 0);
	ituSpriteGoto(digitalTypeDirectionSprite[0][1], 0);
	ituSpriteGoto(digitalTypeDirectionSprite[1][0], 0);
	ituSpriteGoto(digitalTypeDirectionSprite[1][1], 0);

	temp_is_add = false;
	temperature = 110;
	fuel_is_add = true;
	fuel = 0;
	speed_is_add = true;
	speed = 0;
	odo = 0;
	next_info = true;
	info_index = 0;
	change_info = false;
	demo_loop = 0;
	direction_frame = 0;
	ituWidgetSetVisible(digitalTypeInfoBackground, false);

	for (i = 0; i < 2; i++)
	{
		ituProgressBarSetValue(digitalTypeTempProgressBar[i], 25);
		ituWidgetSetPosition(digitalTypeTempPointIcon[i], TemperaturePointX[0], TemperaturePointY[0]);

		ituProgressBarSetValue(digitalTypeFuelProgressBar[i], 0);
		ituWidgetSetPosition(digitalTypeFuelPointIcon[i], FuelPointX[0], FuelPointY[0]);

		ituCircleProgressBarSetValue(digitalTypeSpeedCircleProgressBar[i], 260 - speed);
	}
	
	ituTextSetString(digitalTypeSpeedText[0], "0");
	ituTextSetString(digitalTypeSpeedText[1], "0");

	ituTextSetString(digitalTypeODOText[0], "0000000000");
	ituTextSetString(digitalTypeODOText[1], "0000000000");
	


	ituWidgetShow(digitalTypeOpeningBackground[0], ITU_EFFECT_FADE, 30);
	ituSceneExecuteCommand(&theScene, 20, ShowOpening, 1);

	ituAnimationGoto(digitalTypeDirectionAnimation[0], 0);
	ituAnimationGoto(digitalTypeDirectionAnimation[1], 0);
	ituAnimationGoto(digitalTypeBottomAnimation, 0);
	ituAnimationGoto(digitalTypeFuelAnimation, 0);
	ituAnimationGoto(digitalTypeTempAnimation, 0);
	ituAnimationGoto(digitalTypeSpeedAnimation, 0);
	ituWidgetSetVisible(digitalTypeInfoBackground, false);

	cur_month = tm->tm_mon;
	pre_month = cur_month;

	printf("111111 cur_month %d \n", cur_month);

	ituSpriteGoto(digitalTypeInfoMonthSprite[0], cur_month);
	ituSpriteGoto(digitalTypeInfoMonthSprite[1], cur_month);

	if (tm->tm_hour > 12)
	{
		ituSpriteGoto(digitalTypeInfoAMPMSprite[0], 1);
		ituSpriteGoto(digitalTypeInfoAMPMSprite[1], 1);
		cur_is_pm = 1;
		pre_is_pm = cur_is_pm;
	}		
	else
	{
		ituSpriteGoto(digitalTypeInfoAMPMSprite[0], 0);
		ituSpriteGoto(digitalTypeInfoAMPMSprite[1], 0);
		cur_is_pm = 0;
		pre_is_pm = cur_is_pm;
	}

	ituWidgetSetVisible(digitalTypeDirectionSprite[0][1], false);
	ituWidgetSetVisible(digitalTypeDirectionSprite[1][1], false);
	ituWidgetSetVisible(digitalTypeSpeedBackground1, false);


    return true;
}

bool DigitalTypeOnTimer(ITUWidget* widget, char* param)
{
	bool ret = false;
	uint32_t diff, tick = SDL_GetTicks();
	static int loop_cnt = 0;
	static uint32_t pre_diff = 0;

	int i = 0;

	static int opne_progress_value = 100;

	if (tick >= lastTick)
		diff = tick - lastTick;
	else
		diff = 0xFFFFFFFF - lastTick + tick;

	if (!ituWidgetIsVisible(digitalTypeBackButton) && ituWidgetIsVisible(digitalTypeOpeningBackground[5]))
	{
		ituWidgetSetVisible(digitalTypeBackButton, true);
		ituWidgetSetVisible(digitalTypePlayButton, true);
		ituWidgetSetVisible(digitalTypeChangeUIButton, true);
	
		ret = true;
	}

	if (ituWidgetIsVisible(digitalTypeOpeningBackground[1]) && opne_progress_done == false)
	{
		opne_progress_value--;

		if (opne_progress_value < 0)
		{
			for (i = 2; i < 7; i++)
			{
				if (i != 3 && i!= 6)
					ituSceneExecuteCommand(&theScene, (i-1) * 20, ShowOpening, i);//i==6, ituWidgetSetVisible(digitalTypeOpeningBg, false);
				else if (i == 6)
					ituSceneExecuteCommand(&theScene,  150, ShowOpening, i);
				else
					continue;
			}
			opne_progress_value = 100;
			opne_progress_done = true;
		}
		else
		{
			ituCircleProgressBarSetValue(digitalTypeOpenCircleProgressBar, opne_progress_value);
			ituMeterSetValue(digitalTypeOpenMeter, opne_progress_value);
		}
			
		pre_diff = diff;
		ret = true;
	}

	if (is_playing == false)
		pre_diff = diff;
	else
	{
		
		ret = ret | ModeSpriteDemo(diff);
		ret = ret | GearSpriteDemo(diff);
		ret = ret | DirectionSpriteDemo(diff);
		ret = ret | SpeedDemo(loop_cnt);

		if ((diff - pre_diff) > 50) //50ms
		{
			int add = (diff - pre_diff) / 50;
			ret = ret | TempDemo(add);
			ret = ret | FuelDemo(add);
			pre_diff = diff;
		}

		ret = ret | ODODemo(loop_cnt);

		if (ituWidgetIsVisible(digitalTypeInfoBackground))
		{
			ret = ret | InfoDemo(loop_cnt);
		}		
	}

	cur_month = digitalTypeInfoDigitalClock1->month;
	if (digitalTypeInfoDigitalClock1->hour > 12)
	{
		cur_is_pm = 1;
	}
	else
		cur_is_pm = 0;

	if (cur_month != pre_month)
	{
		ituSpriteGoto(digitalTypeInfoMonthSprite[0], cur_month);
		ituSpriteGoto(digitalTypeInfoMonthSprite[1], cur_month);
		pre_month = cur_month;
		ret = ret | true;
	}

	if (cur_is_pm != pre_is_pm)
	{
		ituSpriteGoto(digitalTypeInfoAMPMSprite[0], cur_is_pm);
		ituSpriteGoto(digitalTypeInfoAMPMSprite[1], cur_is_pm);
		pre_is_pm = cur_is_pm;
	}

	loop_cnt++;
	if (loop_cnt > 1000)
		loop_cnt = 0;

	return ret;
}

bool DigitalTypeOnLeave(ITUWidget* widget, char* param)
{
	int i = 0;
	for (i = 0; i < 6; i++)
		ituWidgetSetVisible(digitalTypeOpeningBackground[i], false);
	ituWidgetSetVisible(digitalTypeOpeningBg, true);
	lastTick = 0;
	is_playing = false;
	return true;
}

bool DigitalTypeChangeUIBtOnMouseUp(ITUWidget* widget, char* param)
{
	char tmp[32];
	

	if (is_central_pos == true)
	{
		is_playing = false;
		demo_loop = 0;

		ituWidgetSetVisible(digitalTypeDirectionSprite[0][1], true);
		ituWidgetSetVisible(digitalTypeDirectionSprite[1][1], true);
		ituWidgetSetVisible(digitalTypeSpeedBackground1, true);
		ituSpriteGoto(digitalTypeDirectionSprite[0][1], direction_frame);
		ituSpriteGoto(digitalTypeDirectionSprite[1][1], direction_frame);

		ituProgressBarSetValue(digitalTypeTempProgressBar[1], temperature);
		ituWidgetSetPosition(digitalTypeTempPointIcon[1], TemperaturePointX[temperature - 25], TemperaturePointY[temperature - 25]);

		ituProgressBarSetValue(digitalTypeFuelProgressBar[1], fuel);
		ituWidgetSetPosition(digitalTypeFuelPointIcon[1], FuelPointX[fuel], FuelPointY[fuel]);


		sprintf(tmp, "%d", speed);
		ituCircleProgressBarSetValue(digitalTypeSpeedCircleProgressBar[1], 260 - speed);
		
		ituTextSetString(digitalTypeSpeedText[1], tmp);
		
		sprintf(tmp, "%d%d%d%d%d%d%d%d%d%d", odo, odo, odo, odo, odo, odo, odo, odo, odo, odo);
		ituTextSetString(digitalTypeODOText[1], tmp);
		
		ituAnimationPlay(digitalTypeFuelAnimation, 0);
		ituWidgetSetVisible(digitalTypeREARIcon, false);

		is_central_pos = false;

	}
	else
	{
		is_playing = false;

		ituSpriteGoto(digitalTypeDirectionSprite[0][0], direction_frame);
		ituSpriteGoto(digitalTypeDirectionSprite[1][0], direction_frame);

		ituProgressBarSetValue(digitalTypeTempProgressBar[0], temperature);
		ituWidgetSetPosition(digitalTypeTempPointIcon[0], TemperaturePointX[temperature - 25], TemperaturePointY[temperature - 25]);

		ituProgressBarSetValue(digitalTypeFuelProgressBar[0], fuel);
		ituWidgetSetPosition(digitalTypeFuelPointIcon[0], FuelPointX[fuel], FuelPointY[fuel]);

		sprintf(tmp, "%d", speed);
		ituCircleProgressBarSetValue(digitalTypeSpeedCircleProgressBar[0], 260 - speed);

		ituTextSetString(digitalTypeSpeedText[0], tmp);

		sprintf(tmp, "%d%d%d%d%d%d%d%d%d%d", odo, odo, odo, odo, odo, odo, odo, odo, odo, odo);
		ituTextSetString(digitalTypeODOText[0], tmp);

		ituAnimationGoto(digitalTypeDirectionAnimation[0], 0);
		ituAnimationGoto(digitalTypeDirectionAnimation[1], 0);
		ituAnimationGoto(digitalTypeBottomAnimation, 0);
		ituAnimationGoto(digitalTypeFuelAnimation, 0);
		ituAnimationGoto(digitalTypeTempAnimation, 0);
		ituAnimationGoto(digitalTypeSpeedAnimation, 0);

		ituSceneExecuteCommand(&theScene, 5, PlayAnimation, NULL);

		ituWidgetSetVisible(digitalTypeInfoBackground, false);
		ituWidgetSetVisible(digitalTypeREARIcon, true);


		ituWidgetSetVisible(digitalTypeDirectionSprite[0][1], false);
		ituWidgetSetVisible(digitalTypeDirectionSprite[1][1], false);
		ituWidgetSetVisible(digitalTypeSpeedBackground1, false);

		is_central_pos = true;
	}

	return true;
}

bool DigitalTypeTempAnOnStopped(ITUWidget* widget, char* param)
{
	change_info = false;
	ituIconLinkSurface(digitalTypeInfoAnIcon[0][1], digitalTypeInfoIcon[info_index]);
	ituAnimationGoto(digitalTypeInfoAnimation, 0);
	ituWidgetSetVisible(digitalTypeInfoBackground, true);
	
	
	is_playing = true;
	return true;
}

bool DigitalTypInfoUpSpriteOnStopped(ITUWidget* widget, char* param)
{
	if (info_index == 5)
	{
		ituWidgetSetVisible(digitalTypeInfoAnIcon[0][0], false);
		ituWidgetSetVisible(digitalTypeInfoAnIcon[1][0], false);
		ituWidgetSetVisible(digitalTypeInfoTimeBackground[0], true);
		ituWidgetSetVisible(digitalTypeInfoTimeBackground[1], true);
	}
	else
	{
		ituWidgetSetVisible(digitalTypeInfoAnIcon[0][0], true);
		ituWidgetSetVisible(digitalTypeInfoAnIcon[1][0], true);
		ituWidgetSetVisible(digitalTypeInfoTimeBackground[0], false);
		ituWidgetSetVisible(digitalTypeInfoTimeBackground[1], false);
		ituIconLinkSurface(digitalTypeInfoAnIcon[0][0], digitalTypeInfoIcon[info_index]);
		ituIconLinkSurface(digitalTypeInfoAnIcon[1][0], digitalTypeInfoIcon[info_index]);
	}
	ituIconLinkSurface(digitalTypeInfoAnIcon[0][1], digitalTypeInfoIcon[info_index - 1]);
	ituIconLinkSurface(digitalTypeInfoAnIcon[1][1], digitalTypeInfoIcon[info_index - 1]);
	

	ituAnimationGoto(digitalTypeInfoAnimation, 1);
	ituAnimationReversePlay(digitalTypeInfoAnimation, 1);

	info_index--;
	return true;
}

bool DigitalTypInfoDownSpriteOnStopped(ITUWidget* widget, char* param)
{
	if (info_index == 4)
	{
		ituWidgetSetVisible(digitalTypeInfoAnIcon[0][0], false);
		ituWidgetSetVisible(digitalTypeInfoAnIcon[1][0], false);
		ituWidgetSetVisible(digitalTypeInfoTimeBackground[0], true);
		ituWidgetSetVisible(digitalTypeInfoTimeBackground[1], true);
	}
	else
	{
		ituWidgetSetVisible(digitalTypeInfoAnIcon[0][0], true);
		ituWidgetSetVisible(digitalTypeInfoAnIcon[1][0], true);
		ituWidgetSetVisible(digitalTypeInfoTimeBackground[0], false);
		ituWidgetSetVisible(digitalTypeInfoTimeBackground[1], false);
		ituIconLinkSurface(digitalTypeInfoAnIcon[0][0], digitalTypeInfoIcon[info_index + 1]);
		ituIconLinkSurface(digitalTypeInfoAnIcon[1][0], digitalTypeInfoIcon[info_index + 1]);
	}
	ituIconLinkSurface(digitalTypeInfoAnIcon[0][1], digitalTypeInfoIcon[info_index]);
	ituIconLinkSurface(digitalTypeInfoAnIcon[1][1], digitalTypeInfoIcon[info_index]);


	ituAnimationGoto(digitalTypeInfoAnimation, 0);
	ituAnimationPlay(digitalTypeInfoAnimation, 0);

	info_index++;
	return true;
}

bool DigitalTypeInfoAnOnStopped(ITUWidget* widget, char* param)
{
	change_info = false;
	return true;
}

bool DigitalTypePlayBtOnMouseUp(ITUWidget* widget, char* param)
{
	if (is_playing == false)
	{
		demo_loop = 0;
		is_playing = true;
	}

	return true;
}

bool digitalSpeedButtonOnPress(ITUWidget* widget, char* param)
{
	if (atoi(param) == 1)
	{
		speedStep++;
	}
	else
	{
		speedStep--;
		if (speedStep < 1)
			speedStep = 1;
	}

	printf("speedStep = %d\n", speedStep);

	return true;
}

bool digitalDelayButtonOnPress(ITUWidget* widget, char* param)
{
	IsDelay = !IsDelay;
	if (IsDelay)
		printf("IsDelay true\n");
	else
		printf("IsDelay false\n");
	return true;
}