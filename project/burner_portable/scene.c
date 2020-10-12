#include <sys/time.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "SDL/SDL.h"
#include "ite/itu.h"
#include "scene.h"
#include "project.h"

#define FPS_ENABLE
#define GESTURE_THRESHOLD 40
//#define DOUBLE_KEY_ENABLE

extern ITUActionFunction actionFunctions[];

// scene
ITUScene theScene;
static SDL_Window *window;
static ITUSurface* screenSurf;
static int periodPerFrame;




//=================
static unsigned int start_tick;


unsigned int get_hour_passed(unsigned int seconds)
{
	unsigned int tmp;
	tmp = seconds / 3660;
	return tmp;

}

int get_min_passed(unsigned int seconds)
{
	unsigned int tmp;
	tmp = seconds % 3600;//erased hour
	tmp=tmp/60; //get minutes 
	return tmp;

}
unsigned int get_sec_passed(unsigned int seconds)
{
	unsigned int tmp;
	tmp = seconds % 60;
	return tmp;
}

unsigned int get_time_format_string(unsigned int timetick, char* buf)
{
	#define LOG_TIMESTAMP_LEN		16 //10//[12:34:56] //10 content with /0
	char tmp[64] = { 0 };
	unsigned int min, hour, sec;

	min = get_min_passed(timetick);
	sec = get_sec_passed(timetick);

	snprintf(buf, LOG_TIMESTAMP_LEN, "%0.2d:%0.2d", min, sec);
	//printf("%s\n", buf);
	return 0;
}





//elapsed_ticker

unsigned int get_elapsed_total_seconds()
{
	return 	(SDL_GetTicks()-start_tick)/1000;		

}

void set_timecounter_start()
{
	start_tick=SDL_GetTicks();
	return;
}
//=================
unsigned int flash_lever=8;
void led_flash_set_level(int level)
{
	//flash_lever= level;

}




int SceneRun(void)
{
    SDL_Event ev, lastev;
    int done, delay, frames, lastx, lasty;
    uint32_t tick, dblclk, lasttick;

    /* Watch keystrokes */
    done = dblclk = frames = lasttick = lastx = lasty = 0;

    while (!done)
    {
        bool result = false;

        tick = SDL_GetTicks();

        frames++;


#ifdef FPS_ENABLE
        if (tick - lasttick >= 1000)
        {
           // printf("fps: %d\n", frames);
            frames = 0;
            lasttick = tick;
			//Test_timeformat_out();
        }
#endif

        while (SDL_PollEvent(&ev))
        {
            switch (ev.type)
            {
            case SDL_KEYDOWN:		
				
				printf("SDL_KEYDOWN: down  0%x \n",ev.key.keysym.scancode);
				break;
            case SDL_KEYUP:
				printf("SDL_KEYUP: UP 0x%x \n",ev.key.keysym.scancode);


					 switch (ev.key.keysym.sym)
	                {


						case SDLK_LEFT:
							printf("SDLK_LEFT \n");

							break;
						case SDLK_DOWN:
							printf("SDLK_DOWN \n");
						
							break;

						case SDLK_RIGHT:
							printf("SDLK_RIGHT \n");

						    break;


						case SDLK_UP:
							printf("SDLK_UP \n");
							break;
							
						case SDLK_INSERT:
							printf("SDLK_INSERT \n");

							break;
						
						default:
							break;
					 }
				break;

            case SDL_QUIT:
                done = 1;
                break;
            }
            lastev = ev;
        }


        delay = periodPerFrame - (SDL_GetTicks() - tick);
        //printf("scene loop delay=%d\n", delay);
        if (delay > 0)
        {
            SDL_Delay(delay);
        }
        else
            sched_yield();
    }

    return 0;
}
