/*
  Simple DirectMedia Layer
  Copyright (C) 1997-2011 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/
#include "SDL_config.h"

#include "SDL_events.h"
#include "../../events/SDL_touch_c.h"

#include "SDL_castor3touch.h"
#include "tslib.h"
#include <stdbool.h>
#include <string.h>
#include "ite/ith.h"
#include "ite/itp.h"
#include "tslib-private.h"

#include <pthread.h>
#include <sys/time.h>

#define	MAX_FINGER_NUM	10

#ifdef  CFG_LCD_WIDTH
    #if (CFG_LCD_WIDTH > 0)
    #define	TOUCH_LCD_WIDTH	(CFG_LCD_WIDTH)
    #else
    #define	TOUCH_LCD_WIDTH	(CFG_TOUCH_X_MAX_VALUE + 1)
    #endif
#else
#define	TOUCH_LCD_WIDTH	(CFG_TOUCH_X_MAX_VALUE + 1)
#endif

#if defined(CFG_LCD_HEIGHT) 
    #if (CFG_LCD_HEIGHT > 0)
    #define	TOUCH_LCD_HEIGHT	(CFG_LCD_HEIGHT)
    #else
    #define	TOUCH_LCD_HEIGHT	(CFG_TOUCH_Y_MAX_VALUE + 1)
    #endif
#else
#define	TOUCH_LCD_HEIGHT	(CFG_TOUCH_Y_MAX_VALUE + 1)
#endif

typedef struct 
{
	int p;
	int x;
	int y;
	int fn;
}ts_last_samp;

static struct tsdev *ts;
static const SDL_TouchID touchId = 0;
static SDL_FingerID fingerId;
static bool down[MAX_FINGER_NUM];
static bool mDown[MAX_FINGER_NUM];
static bool rawMode;
static float rawX, rawY;
static struct ts_sample c3samp[MAX_FINGER_NUM];
static ts_last_samp lstSmp[MAX_FINGER_NUM];
static int	gLastId[10] = {0};
static int	gLastFingerNum = 0;
static int	gSdlFingerNum = 0;
static struct ts_sample OriSmp[1] = {0};
static uint32_t tc=0;
static uint8_t gTpSmpIsDiffCnt[10] = {0};

static void lp_tp_module_identify_callback_default(char **module)
{
	*module = "NO_CALLBACK";
}
void (*lp_tp_module_identify_callback)(char **module) = lp_tp_module_identify_callback_default;

static void update_last_sample(int tId, struct ts_sample* smp)
{
	lstSmp[tId].p = (int)smp->pressure;
	lstSmp[tId].x = (int)smp->x;
	lstSmp[tId].y = (int)smp->y;	
	lstSmp[tId].fn = (int)smp->finger;
}

/*
input: id the finger ID; smp the current sample structure pointer
return: true is the same, false is different
*/
static bool check_last_sample(int tId, struct ts_sample* smp)
{
	int tmpP;
	int tmpX;
	int tmpY;
	int tmpFN;
	bool tRst = true;	
	
	tmpP = (int)smp->pressure;
	tmpX = (int)smp->x;
	tmpY = (int)smp->y;
	tmpFN = (int)smp->finger;	

	if( lstSmp[tId].fn != tmpFN )	tRst = false;	
	if( lstSmp[tId].p != tmpP )	tRst = false;		
	if( lstSmp[tId].x != tmpX )	tRst = false;
	if( lstSmp[tId].y != tmpY )	tRst = false;

	//add this code to avoid from too many TP motion events to be filled up with
	if( (tmpFN > 1) && (tRst == false) )
	{
	    if(!gTpSmpIsDiffCnt[tId])	tc = itpGetTickCount();
	        
	    if(itpGetTickDuration(tc) < 16)
	    {
	        if(gTpSmpIsDiffCnt[tId]++)
	        {
	            //return false(need send move event) if gTpSmpIsDiffCnt==0
	            //return true(skip move event during 33ms) if gTpSmpIsDiffCnt!=0
	            tRst = true;
	        }
	    }
	    else
	    {
	        gTpSmpIsDiffCnt[tId] = 0;
	    }
	}

#ifdef	ENABLE_CHECK_LAST_SAMPLE_LOG
	if( (tId==2) && (tRst == false) )
{
		printf("	chkSmp: id=%d, fpxy=%d,%d,%d,%d\n", tId, tmpFN, tmpP, tmpX, tmpY);
		printf("	lastsp: id=%d, fpxy=%d,%d,%d,%d\n", tId, lstSmp[tId].fn, lstSmp[tId].p, lstSmp[tId].x, lstSmp[tId].y);
	}
#endif
	
	return tRst;
}

void Castor3_InitTouch(void)
{
	SDL_Touch touch;
	SDL_Touch* ptr;
	int i;

#if defined(CFG_TOUCH_I2C0)
    ts = ts_open(":i2c0",0);
#elif defined(CFG_TOUCH_I2C1)
    ts = ts_open(":i2c1",0);
#elif defined(CFG_TOUCH_I2C2)
    ts = ts_open(":i2c2",0);
#elif defined(CFG_TOUCH_I2C3)
    ts = ts_open(":i2c3",0);
#elif defined(CFG_TOUCH_SPI)
    ts = ts_open(":spi",0);
#endif // CFG_TOUCH_I2C

	if (!ts) 
    {
		perror("ts_open");
	}

	if(CFG_TOUCH_MODULE_COUNT > 1)
    {
    	char *m;

    	//To indentify TP module
    	lp_tp_module_identify_callback(&m);
    	
    	if(strcmp(m,"NO_CALLBACK") == 0)
    	{
    		//NO callback function, please implement the lp_tp_module_identify_callback() in AP layer
    		//if CFG_TOUCH_MODULE_COUNT > 1, you have to implement the callback function for a correct TP module.
    		//OR set "CFG_TOUCH_MODULE" as only one module, if you didn't implement the callback function
    		printf("[ERROR] NO callback function for identifying the correct module\n");
    	}
    	else if(m == NULL)
    	{
    		printf("Can not identify the TP module. Please check the callback function.\n");
    	}
    	else
    	{
    		//Load TP module by module name
    		if(ts_load_module_raw(ts, m, NULL))
    		{
    			//incorrect TP module, please check if the module is matched.
    			perror("ts_load_module_raw");  
    		}
    		else
    		{
    			printf("Load TP module finished, module: %s, count=%d\n", m, CFG_TOUCH_MODULE_COUNT);
    		}
    	}
    }

	if (ts_config(ts)) 
    {
		perror("ts_config");
	}

	touch.id = touchId;
	touch.x_min = 0;
	touch.x_max = CFG_TOUCH_X_MAX_VALUE;
	touch.native_xres = touch.x_max - touch.x_min;
	touch.y_min = 0;
	touch.y_max = CFG_TOUCH_Y_MAX_VALUE;
	touch.native_yres = touch.y_max - touch.y_min;
	touch.pressure_min = 0;
	touch.pressure_max = 1;
	touch.native_pressureres = touch.pressure_max - touch.pressure_min;

	for(i=0; i<MAX_FINGER_NUM-1; i++)	c3samp[i].next = (struct ts_sample*)&c3samp[i+1];
    c3samp[MAX_FINGER_NUM-1].next = NULL;	
	//printf("initial fingerId: %lld, c3samp = %x, %x, %x\n", fingerId, &c3samp[0], &c3samp[1], sizeof(struct ts_sample));
	printf("initial fingerId: %lld, c3samp = [0]=%x, [1]=%x, [2]=%x, [3]=%x, [4]=%x, sz=%x\n", fingerId, &c3samp[0], &c3samp[1], &c3samp[2], &c3samp[3], &c3samp[4], sizeof(struct ts_sample));
    
	memset(down, 0, sizeof(down));

	if (SDL_AddTouch((SDL_Touch*)&touch, "") < 0) 
    {
	    perror("SDL_AddTouch");
	}
	ptr = SDL_GetTouch(touchId);
    ptr->xres = (Uint16)TOUCH_LCD_WIDTH;   //ithLcdGetWidth();
    ptr->yres = (Uint16)TOUCH_LCD_HEIGHT;  //ithLcdGetHeight(); 
    
    rawMode = false;
    rawX = rawY = 0.0f;
}

void Castor3_PumpTouchEvent(void)
{
    int ret;
    int k=0;
    int curFgr = 0;
    bool cDn[10] = {false};
    int i;
    
    /* init samples */
    for(i=0; i<MAX_FINGER_NUM; i++)
    {
        c3samp[i].finger = 0;
        c3samp[i].pressure = 0;
    }    

    if (rawMode)
	        ret = ts_read_raw(ts, &c3samp[0], 1);
    else
	        ret = ts_read(ts, &c3samp[0], 1);

	if (ret < 0) 
    {
		perror("ts_read");
	}

	if (ret != 1)
	{
		if(gLastFingerNum)	ret = gLastFingerNum;
			
		c3samp[0].finger = 0;
	}	

#ifdef	CFG_TOUCH_MULTI_FINGER
	curFgr = c3samp[0].finger;
	
	if(gLastFingerNum!=curFgr)
	{
		int m;

		if(gLastFingerNum>curFgr)
		{
			for(m=curFgr; m<gLastFingerNum; m++)
			{
				if(down[c3samp[m].id]==false)
				{
				    //clear the pressure if last down[id] has been false
				    if(c3samp[m].pressure)
				    {
				        printf("    got ERROR state, m=%d, fn=(%d,%d,%d), id=%d, p=%x\n",m, curFgr, c3samp[m].finger, gLastFingerNum, c3samp[m].id, c3samp[m].pressure);
				        c3samp[m].pressure = 0;	
				    }
				}
			}
		}
		
		if(!curFgr)
		{
			for(m=0; m<gLastFingerNum; m++)	
			{
				c3samp[m].id = gLastId[m];
				gLastId[m] = 0;
			}
		}
	}
	
	if(gLastFingerNum>curFgr)	curFgr = gLastFingerNum;
#else
	curFgr = 1;
	
	if(c3samp[0].id != 0)
	{
		//workaround the S/W structure issue(force to keep id = 0, if only one finger)
		//printf("	id(%x) != 0:%x,%x\n", c3samp[k].id, k, down[c3samp[k].id]  );
		c3samp[0].id = 0;
	}
#endif
	
    //pre-init all cDn[k] = false
    for(k=0; k<MAX_FINGER_NUM; k++) cDn[k] = false;
    
	k = 0;
	while(curFgr)
	{
		if (c3samp[k].pressure) 
	    {
	        if (down[c3samp[k].id] == false)
	        {
	        	//printf("	sdl_sfd[%d]:fn=%d, id=%d, pxy=%d,%d,%d\n",k,c3samp[k].finger,c3samp[k].id,c3samp[k].pressure,c3samp[k].x,c3samp[k].y);
	        	//printf("	sdlDn[%d,%d] %d,%d,%d\n",k,c3samp[k].id,c3samp[k].pressure,c3samp[k].x,c3samp[k].y);
	        	gSdlFingerNum++;
			    SDL_SendFingerDown(touchId, (SDL_FingerID)c3samp[k].id, SDL_TRUE, (float)c3samp[k].x, (float)c3samp[k].y, 1);
			    if(k==0)
			    {
	            	rawX = (float)c3samp[k].x;
	            	rawY = (float)c3samp[k].y;
			    }

	            down[c3samp[k].id] = true;
	            
	            update_last_sample((int)c3samp[k].id, &c3samp[k]);
	        }
	        else
	        {
	        	if( check_last_sample((int)c3samp[k].id, &c3samp[k]) == false )	//motion has changed
	        	{
	        	    struct timeval tv;
                    gettimeofday(&tv,NULL);
	        		//printf("	sdlMv[%06d](%d,%d)(%d,%d,%d)\n",tv.tv_usec%1000000, k,c3samp[k].id,c3samp[k].pressure,c3samp[k].x,c3samp[k].y);
	        		SDL_SendTouchMotion(touchId, (SDL_FingerID)c3samp[k].id, SDL_FALSE, (float)c3samp[k].x, (float)c3samp[k].y, 1);
	        		update_last_sample((int)c3samp[k].id, &c3samp[k]);
	        	}
		    }
		    cDn[c3samp[k].id] = true;
	    }
	    else 
	    {
	    	if(down[c3samp[k].id]==true)
	    	{
	    		gSdlFingerNum--;
	    		//printf("	sdl_sfu[%d]:fid=%d, cf=%d, fn=%d, pxy=%d,%d,%d\n", k, c3samp[k].id, curFgr, gSdlFingerNum, c3samp[k].pressure, c3samp[k].x, c3samp[k].y);
	    		//printf("	sdlUp[%d,%d] %d,%d,%d\n",k,c3samp[k].id,c3samp[k].pressure,c3samp[k].x,c3samp[k].y);
				SDL_SendFingerDown(touchId, (SDL_FingerID)c3samp[k].id, SDL_FALSE, (float)c3samp[k].x, (float)c3samp[k].y, 1);	
				down[c3samp[k].id] = false;
				
				if(!c3samp[k].pressure)
				{
					c3samp[k].x = 0;
					c3samp[k].y = 0;
					c3samp[k].finger = 0;
				}

				update_last_sample((int)c3samp[k].id, &OriSmp[0]);
	    	}
		}
		
		gLastId[k] = c3samp[k].id;
		
		if(++k >= curFgr)	break;
		
		if( (k>MAX_FINGER_NUM) || (curFgr>MAX_FINGER_NUM) )	break;
	}

#ifdef	CFG_TOUCH_MULTI_FINGER

	//force to send touch-up if the ID has been not used & if (lastFN > currFN)
	if(gLastFingerNum > c3samp[0].finger)
	{
	    k=0;
	    for(k=0; k<MAX_FINGER_NUM; k++)
	    {
	        if( (down[k] == true) && (cDn[k] == false) ) 
	        {
	    		gSdlFingerNum--;
	    		//printf("	sdl_up: id=%d, lf=%d, cf=%d, dn=%d, cdn=%d\n", k, gLastFingerNum, c3samp[0].finger, down[k], cDn[k]);
				SDL_SendFingerDown(touchId, (SDL_FingerID)k, SDL_FALSE, (float)0, (float)0, 1);	
				down[k] = false;
				update_last_sample(k, &OriSmp[0]); 
	        }
	    }
	}
	
	if(gLastFingerNum != c3samp[0].finger)	gLastFingerNum = c3samp[0].finger;
#endif
}

int Castor3_CalibrateTouch(float screenX[], float screenY[], float touchX[], float touchY[])
{
    int j;
	float n, x, y, x2, y2, xy, z, zx, zy;
	float det, a, b, c, e, f, i;
	float scaling = 65536.0f;
    int result[7];
    FILE* file;

    // Get sums for matrix
	n = x = y = x2 = y2 = xy = 0;
	for(j=0;j<5;j++) 
    {
		n += 1.0;
		x += touchX[j];
		y += touchY[j];
		x2 += (touchX[j]*touchX[j]);
		y2 += (touchY[j]*touchY[j]);
		xy += (touchX[j]*touchY[j]);
	}

    // Get determinant of matrix -- check if determinant is too small
	det = n*(x2*y2 - xy*xy) + x*(xy*y - x*y2) + y*(x*xy - y*x2);
	if(det < 0.1 && det > -0.1) 
    {
		printf("ts_calibrate: determinant is too small -- %f\n",det);
		return -1;
	}

    // Get elements of inverse matrix
	a = (x2*y2 - xy*xy)/det;
	b = (xy*y - x*y2)/det;
	c = (x*xy - y*x2)/det;
	e = (n*y2 - y*y)/det;
	f = (x*y - n*xy)/det;
	i = (n*x2 - x*x)/det;

    // Get sums for x calibration
	z = zx = zy = 0;
	for(j=0;j<5;j++) 
    {
		z += screenX[j];
		zx += screenX[j]*touchX[j];
		zy += screenX[j]*touchY[j];
	}

    // Now multiply out to get the calibration for framebuffer x coord
	result[0] = (int)((a*z + b*zx + c*zy)*(scaling));
	result[1] = (int)((b*z + e*zx + f*zy)*(scaling));
	result[2] = (int)((c*z + f*zx + i*zy)*(scaling));

	printf("%f %f %f\n",(a*z + b*zx + c*zy),
				(b*z + e*zx + f*zy),
				(c*z + f*zx + i*zy));

    // Get sums for y calibration
	z = zx = zy = 0;
	for(j=0;j<5;j++) 
    {
		z += screenY[j];
		zx += screenY[j]*touchX[j];
		zy += screenY[j]*touchY[j];
	}

    // Now multiply out to get the calibration for framebuffer y coord
	result[3] = (int)((a*z + b*zx + c*zy)*(scaling));
	result[4] = (int)((b*z + e*zx + f*zy)*(scaling));
	result[5] = (int)((c*z + f*zx + i*zy)*(scaling));

	printf("%f %f %f\n",(a*z + b*zx + c*zy),
				(b*z + e*zx + f*zy),
				(c*z + f*zx + i*zy));

    // If we got here, we're OK, so assign scaling to a[6] and return
	result[6] = (int)scaling;

	printf ("Calibration constants: ");
	for (j = 0; j < 7; j++)
        printf("%d ", result[j]);
	printf("\n");

    file = fopen(CFG_PUBLIC_DRIVE ":/pointercal", "w");
    if (file == NULL)
        return -2;

	j = fprintf(file,"%d %d %d %d %d %d %d %d %d",
	              result[1], result[2], result[0],
	              result[4], result[5], result[3], result[6],
                  TOUCH_LCD_WIDTH, TOUCH_LCD_HEIGHT);
	fclose(file);

    if (j == 0)
        return -3;

	if (ts_config(ts)) 
    {
		perror("ts_config");
		return -4;
	}

    return 0;
}

void Castor3_ChangeTouchMode(int raw)
{
    rawMode = raw ? true : false;
}

void Castor3_ReadTouchRawPosition(float* x, float* y)
{
    if (x)
        *x = rawX;

    if (y)
        *y = rawY;
}

void Castor3_QuitTouch(void)
{
    ts_close(ts);
}

/* vi: set ts=4 sw=4 expandtab: */
