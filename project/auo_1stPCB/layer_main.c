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
static int flush_counter = 0;
void SpeedMeterUpdate()
{
	int ten;
	int digit;

	if (speed > 99)
		speed = 0;
	else
		speed++;
	ten = speed / 10;
	digit = speed % 10;

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


void TurnLeftRightFlash()
{
	static int flush_interal = 3;
	static int flash_counter = 10;
	static bool Leftflag = 0;
	ITUIcon* turning;

	if (flush_interal > 2)
		flush_interal = 0;
	else{ //to reduce the update rate to 1/3
		flush_interal++;
		return ;
	}

	//for Left Right switching
	if (1== Leftflag)
		turning = mainIconLeft;
	else 
		turning = mainIconRight;

	if (flash_counter % 2)
		ituWidgetSetVisible(turning, 1);
	else
		ituWidgetSetVisible(turning, 0);

	flash_counter--;
	if (0 == flash_counter)
	{
		flash_counter = 10;
		Leftflag = !Leftflag;
		ituWidgetSetVisible(mainIconLeft, 0);
		ituWidgetSetVisible(mainIconRight, 0);
	}

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

	TurnLeftRightFlash();
	
	//leo
	SemicolonFlash();

	return true;
}


bool MainLayerOnEnter(ITUWidget* widget, char* param)
{
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

