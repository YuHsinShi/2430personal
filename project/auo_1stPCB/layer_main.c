#include <assert.h>
#include "scene.h"

/* widgets:
LayerMain
Background3
IconTurnRight
IconTurnLeft
Container54
SpriteDigits
Icon39
Icon40
Icon41
Icon42
Icon43
Icon44
Icon45
Icon46
Icon47
Icon48
SpriteDecade
IconN0
IconN1
IconN2
IconN3
IconN4
IconN5
IconN6
IconN7
IconN8
IconN9
Background50
*/

//flash
//find the widgets of and 


static ITUSprite* mainSprite_TenDigit;
static ITUSprite* mainSprite_Digit;
static ITUSprite* mainSpriteSportMode;
static ITUIcon* mainIconLeft;
static ITUIcon* mainIconRight;

//leo
static ITUSprite* mainSpriteSpeedBar;
static ITUIcon* mainSemicolon;

static int speed;
static unsigned int flush_counter = 0;

static ITUContainer* ContainerMainMeter;

static int winker_status;

int auto_run=1;	
void ui_set_meter_speed_value(int value)
{	
if( (value < 99) && (value > 0) )
		speed = value;

//ithPrintf(" speed =%d \n",speed);
}

void ui_set_winker_left()
{	
	winker_status=0x01;

}
void ui_set_winker_right()
{	
	winker_status=0x02; 

}

void ui_set_sport_mode_on()
{	
	
	ituWidgetSetVisible(mainSpriteSportMode,1);
	ituSpritePlay(mainSpriteSportMode, 0);

}


void ui_engine_start()
{	
	ituWidgetSetVisible(ContainerMainMeter,1);


}
void ui_set_unlock_mode()
{	

	ituWidgetSetVisible(ContainerMainMeter,0);
}




void SpeedMeterUpdate()
{
	int show_speed;

	int ten;
	int digit;
/*
	if (speed > 99)
		speed = 0;
	else
		speed++;
*/

	show_speed=speed; //to avoid context switch when showing
	ten = show_speed / 10;
	digit = show_speed % 10;

	ituSpriteGoto(mainSprite_TenDigit, ten); // show ten number 
	ituSpriteGoto(mainSprite_Digit, digit); // show digit number 
	
	
	//leo
	if(speed < 71)
	{
		ituSpriteGoto(mainSpriteSpeedBar, speed); // show speed bar
	}

}


//leo
void SemicolonFlash()
{
	static int counter = 0;
	static bool isflash = 0;
	
	if(counter > 3)
	{
		counter = 0;
	}
	else
	{
		counter++;
		return ;
	}
	
	ituWidgetSetVisible(mainSemicolon,isflash);
	isflash = !isflash;
	
}


int TurnLeftRightWinker(ITUIcon* flash_icon)
{
	static int flush_interal = 3;
	static int flash_counter = 10;

	if (flush_interal > 2)
		flush_interal = 0;
	else{ //to reduce the update rate to 1/3
		flush_interal++;
		return 1;
	}

	if (flash_counter % 2)
		ituWidgetSetVisible(flash_icon, 1);
	else
		ituWidgetSetVisible(flash_icon, 0);

	flash_counter--;
	if (0 == flash_counter)
	{
		flash_counter = 10;
		ituWidgetSetVisible(flash_icon, 0);
		return 0;
	}
return 1;
}


void WinkerLRStatusUpdate()
{
	if(0==winker_status){	
		ituWidgetSetVisible(mainIconLeft, 0);	
		ituWidgetSetVisible(mainIconRight, 0);
	}
	else if (0x01==winker_status){
		if(0 == TurnLeftRightWinker(mainIconLeft) )
		{
			winker_status&=~0x01;
		}
	}
	else if (0x02==winker_status){
		if(0 == TurnLeftRightWinker(mainIconRight) ){
				winker_status&=~0x02;
		
		}
	}
	else
	{

	}

}
void ui_set_sport_mode_auto_run()
{

}

bool MainLayerOnSimulateControl(ITUWidget* widget, char* param)
{
	return false;
}

bool MainLayerOnTimer(ITUWidget* widget, char* param)
{
	if (NULL == mainSprite_TenDigit)
		return false;

	flush_counter++;
	if (0 != (flush_counter % 5) ) // to control the simulation play rate to 17msx5 per frame
	{ 	
		return false;
	}


	SpeedMeterUpdate();
if(auto_run)
{

	if (295 == flush_counter % 300)
	{
		static bool tmp_LR=true;
		if (tmp_LR)
			ui_set_winker_left();
		else 	
			ui_set_winker_right();

		tmp_LR = !tmp_LR;
	}


	if (995 == flush_counter % 1000)
	{
		ui_set_sport_mode_on();
	}


}

	WinkerLRStatusUpdate();

	//leo
	SemicolonFlash();

	return true;
}


bool MainLayerOnEnter(ITUWidget* widget, char* param)
{

	ContainerMainMeter = ituSceneFindWidget(&theScene, "ContainerMainMeter");
	assert(ContainerMainMeter);

	mainSprite_TenDigit = ituSceneFindWidget(&theScene, "Sprite_TenDigit");
	assert(mainSprite_TenDigit);

	mainSprite_Digit = ituSceneFindWidget(&theScene, "Sprite_Digit");
	assert(mainSprite_Digit);

	mainSpriteSportMode = ituSceneFindWidget(&theScene, "SpriteSportMode");
	assert(mainSpriteSportMode);

	mainIconLeft = ituSceneFindWidget(&theScene, "IconTurnLeft");
	assert(mainIconLeft);

	mainIconRight = ituSceneFindWidget(&theScene, "IconTurnRight");
	assert(mainIconRight);
	
	//leo
	mainSpriteSpeedBar = ituSceneFindWidget(&theScene, "SpriteSpeedBar");
	assert(mainSpriteSpeedBar);
	
	mainSemicolon = ituSceneFindWidget(&theScene, "Semicolon");
	assert(mainSemicolon);
	
	return true;
}

