/*
 * Copyright (c) 2017 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * common APIs in raw driver
 *
 * @author Joseph
 * @version 1.0
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <alloca.h>
#include <pthread.h>	

#ifdef __OPENRTOS__
#include "openrtos/FreeRTOS.h"
#include "openrtos/queue.h"
#endif

#include "ite/ith.h" 
#include "ite/itp.h"
#include "config.h"
#include "tslib-private.h"

#include "api-raw.h"

/****************************************************************************
 * SPECIFIC FUNFUION
 ****************************************************************************/

/****************************************************************************
 * global variable
 ****************************************************************************/
static struct ts_sample *gp_Sample;

static RA_TP_SPEC   *gpRaSpec = NULL;
static RA_GV 		*gpRaInfo = NULL;

static unsigned int raDur=0;
static unsigned int raiDur=0;
static unsigned int raLowDur=0;
struct timeval raT1, raT2;
struct timeval raTv1, raTv2;

static int  g_raIntrLowCnt = 0;
static int  g_raIntrAtvCnt = 0;

#ifdef RA_USE_XQUEUE
static QueueHandle_t raQueue;
#endif

/*##################################################################################
 *                         the protocol of private function
 ###################################################################################*/
static bool _raGetIntr(void);
static bool _raChkIntActive(void);
static void _raUpdateLastXY(struct ts_sample *smp);
/* *************************************************************** */

/* *************************************************************** */
#ifdef	CFG_TOUCH_BUTTON
extern unsigned int (*ts_read_kp_callback)(void);
#endif
/*##################################################################################
 *                        the private function implementation
 ###################################################################################*/

/******************************************************************************
 * the read flow for reading the FT5316's register by using iic repead start
 ******************************************************************************/
 
/*##################################################################################
 *                        the private function implementation
 ###################################################################################*/
static int _raCheckMultiPressure(struct ts_sample *theSmp)
{
	struct ts_sample *s = theSmp;
	int cfgr = theSmp->finger;
	int tpPressure = 0;
	int i;
		
	if(cfgr)
	{
		for(i=0; i<cfgr; i++)
		{
			if(s->pressure)	tpPressure++;
			s++;	
		}
	}
	
	if(tpPressure)	return 1;
	else			return 0;
}

/*
return 0:no change, 1:sample has changed
*/
static int _raCheckIfSmpChg(struct ts_sample *s1, struct ts_sample *s2)
{
    int doChkDistance = 1;
    int deltaDist = 1;
    //int deltaDist = gpRaSpec->tpMoveDetectUnit;

    if( (s1==NULL) || (s2==NULL) )	return 1;
        
	if(s1->finger != s2->finger)	return 1;
		
	{
		int i;
		int fn = s2->finger;
		
		for(i=0; i<fn; i++)
		{
			if(s1->pressure != s2->pressure)	return 1;

			if(doChkDistance)
			{
			    if(s1->x >= s2->x)
			    {
			        if((s1->x - s2->x) >= deltaDist)    return 1;
			    }
			    else
			    {
			        if((s2->x - s1->x) >= deltaDist)    return 1;
			    }
			        
			    if(s1->y >= s2->y)
			    {
			        if((s1->y - s2->y) >= deltaDist)    return 1;
			    }
			    else
			    {
			        if((s2->y - s1->y) >= deltaDist)    return 1;
			    }
			}
			else
			{
			    if(s1->x != s2->x)	return 1;
			
			    if(s1->y != s2->y)	return 1;
			}
			
			if(s1->id != s2->id)	return 1;
				
			s1++; s2++;
		}
	}
	
	return 0;
}

static void _raIntActiveRule_vendor(struct ts_sample *tpSmp)
{
    gpRaInfo->tpIntrCnt = 0;
    gpRaInfo->tpNeedUpdateSample = 0;
    
    if(!gpRaSpec->tpIntUseIsr)
    {
        //for prevent from the issue that polling INT signal will get the same sample.
        if(!gpRaInfo->tpNeedToGetSample)	return;
        else    gpRaInfo->tpNeedToGetSample = 0;
    }
    
    //status rule for RA_TOUCH_DOWN/RA_TOUCH_UP/RA_TOUCH_NO_CONTACT
	switch(gpRaInfo->tpStatus)
	{
		case RA_TOUCH_NO_CONTACT:
			if (_raCheckMultiPressure(tpSmp) )
			{
				gpRaInfo->tpStatus = RA_TOUCH_DOWN;
				gpRaInfo->tpIntr4Probe = 1;
				gpRaInfo->tpNeedUpdateSample = 1;
				gpRaInfo->tpFirstSampHasSend = 0;
			}
			break;
		
		case RA_TOUCH_DOWN:
			if ( !_raCheckMultiPressure(tpSmp) )
			{
				gpRaInfo->tpStatus = RA_TOUCH_UP;
			}				
			if(gpRaInfo->tpFirstSampHasSend)	gpRaInfo->tpNeedUpdateSample = 1;
			break;
			
		case RA_TOUCH_UP:
			if ( !_raCheckMultiPressure(tpSmp) )
			{
				gpRaInfo->tpStatus = RA_TOUCH_NO_CONTACT;
				gpRaInfo->tpIntr4Probe = 0;
			}
			else
			{
				gpRaInfo->tpStatus = RA_TOUCH_DOWN;
				gpRaInfo->tpIntr4Probe = 1;
				gpRaInfo->tpNeedUpdateSample = 1;
			}
			break;
			
		default:
			printf("ERROR touch STATUS, need to check it!!\n");
			break;				
	}

	#ifdef	ENABLE_TOUCH_PANEL_DBG_MSG
	printf("	tpStatus=%x, NSQ=%x, cINT=%x, send=%x\n", gpRaInfo->tpStatus, gpRaInfo->tpNeedUpdateSample, gpRaInfo->tpIntr4Probe, gpRaInfo->tpFirstSampHasSend);
	#endif		
	
	//use this flag to judge if update the touch sample
	//1.have to update the first RA_TOUCH_DOWN event
	//2.don't update the touch event if UI does not get the first event
	//3.real-time update the X,Y point after send the 1st event
	//4.must send the touch event if last status is touch-up, and INT active again in this time.
	//  to handle the quickly touch case.
	//5.others...
	if(gpRaInfo->tpNeedUpdateSample)
	{
		_raUpdateLastXY(tpSmp);
	}		
	
	if(gpRaSpec->tpIntUseIsr)
	{
	    //clear INT flag and enable interrupt if use ISR to handle INT signal
	    *gpRaSpec->pTouchDownIntr = 0;
	    ithGpioEnableIntr(RA_INT_PIN); 
	}
}

static void _raIntNotActiveRule_vendor(struct ts_sample *tpSmp)
{
    if(!gpRaSpec->tpIntUseIsr)
    {
        //if INT not active, then set this flag to call _raGetSample() if next INT active
	    gpRaInfo->tpNeedToGetSample = 1;
	}
	
	#ifdef	ENABLE_TOUCH_PANEL_DBG_MSG
	if( (gpRaInfo->tpStatus != RA_TOUCH_NO_CONTACT) )
		printf("	UpdateSmp0:INT=%x, ss=(%d,%d)\n",gpRaInfo->tpCurrINT, gpRaInfo->tpStatus, gpRaInfo->tpFirstSampHasSend);
	#endif
	
	//In order to prevent from loss of the first touch event
	//Need To set "status=RA_TOUCH_NO_CONTACT" if "last status=RA_TOUCH_UP" + "first sample has send"
	if( (gpRaInfo->tpStatus == RA_TOUCH_UP) && (gpRaInfo->tpFirstSampHasSend) )
	{
        _raUpdateLastXY(NULL);
	    gpRaInfo->tpStatus = RA_TOUCH_NO_CONTACT;
	    gpRaInfo->tpIntr4Probe = 0;

		#ifdef	ENABLE_TOUCH_PANEL_DBG_MSG
		printf("INT=0, force to set status=0!!\n");
		#endif
	}
	
	//For prevent from lossing the touch-up event
	//sometimes, S/W can not get TOUCH-UP event when INT is actived
	//So, this code will force to set touch-status as RA_TOUCH_UP after INT is not actived for a specific time(16ms)
	if( gpRaSpec->tpNeedAutoTouchUp && (gpRaInfo->tpStatus == RA_TOUCH_DOWN) && (_raGetIntr()==false) )
	{
		static uint32_t tc1=0;
		
	    //printf("	UdSmp:s=%d, int=%x, ic=%d\n",gpRaInfo->tpStatus,gpRaInfo->tpCurrINT,gpRaInfo->tpIntrCnt);
	    
	    if(!gpRaInfo->tpIntrCnt)	tc1 =itpGetTickCount();
	    raDur = itpGetTickDuration(tc1);

		if( gpRaInfo->tpFirstSampHasSend && (gpRaInfo->tpIntrCnt > 3) )
		{
			//when first smaple has send, or main-loop idle over 33 ms.
			//for fixing the FT5XXX's issue that sometimes it cannot get the RA_TOUCH_UP EVENT
			//and need "gpRaInfo->tpIntrCnt" > 3 times to prevent from main task idle issue
			if( (gpRaSpec->tpIntrType == RA_INT_TYPE_ZT2083) || (raDur > gpRaSpec->tpSampleRate) )
			{
				//FORCE RA_TOUCH_UP if RA_INT_TYPE_ZT2083 or raDur > one-sample-rate-time
				//printf("	__/TchUp:2\n");
				gpRaInfo->tpStatus = RA_TOUCH_UP;
				gpRaInfo->tpIntr4Probe = 0;
				_raUpdateLastXY(NULL);					
				
				#ifdef	ENABLE_TOUCH_PANEL_DBG_MSG
				printf("INT=0, and raDur>%dms, so force to set status=2!!\n",gpRaSpec->tpSampleRate);
				#endif
				//printf("INT=0, and raDur>%dms, force RA_TOUCH_UP!!\n",gpRaSpec->tpSampleRate);
			}
		}
		
		gpRaInfo->tpIntrCnt++;
	}

	//to handle the INT actived, but *gpRaSpec->pTouchDownIntr doesn't become true.
	//need send a i2c read command to clear INT for IT7260.
	//If INT will keep active state until I2C send command to TP IC for clearing INT active state(like IT7260).
	//Then this workaround will be necessary for fixing the issue 
	//which TP's INT signal has NO response after suspend mode
    if(gpRaSpec->tpIntrType == RA_INT_TYPE_IT7260)
    {
        //_raFixIntHasNoResponseIssue();
    	if( gpRaSpec->tpIntUseIsr && (_raChkIntActive()==true) )
    	{
    		static uint32_t tc2 = 0;
    		if(!g_raIntrAtvCnt++)	tc2 =itpGetTickCount();
	    	raiDur = itpGetTickDuration(tc2);
    	    
    	    if(raiDur > gpRaSpec->tpIntActiveMaxIdleTime)
    	    {
     			unsigned char *buf = (unsigned char *)malloc(gpRaSpec->tpReadChipRegCnt);
     			memset(buf, 0, gpRaSpec->tpReadChipRegCnt);
     			if(gpRaSpec->rawApi.raReadPointBuffer(buf, gpRaSpec->tpReadChipRegCnt) == 0)
     			{
     			    if(gpRaSpec->tpHasTouchKey)
     			    {
     			    	struct ts_sample s1;
     			    	gpRaSpec->rawApi.raParseKey(&s1, buf);
     			    }
     			}
     			g_raIntrAtvCnt = 0;
     			if(buf!=NULL)	free(buf);
                *gpRaSpec->pTouchDownIntr = true;
     			printf("read Sample while INT is active\n");
    		}
    	}
    	else
    	{
    	    g_raIntrAtvCnt = 0;
    	}
    }
}

/*##################################################################################
 *                middle APIs for handling raw x,y data
 ###################################################################################*/
#ifdef CFG_TOUCH_INTR
static void _ra_isr(void* data)
{	
	unsigned int regValue;
	
	#ifdef	ENABLE_TOUCH_PANEL_DBG_MSG
	//ithPrintf("$in\n");
	#endif

	*gpRaSpec->pTouchDownIntr = true;

    ithGpioClearIntr(RA_INT_PIN);
    if(gpRaSpec->tpIntrType == RA_INT_TYPE_KEEP_STATE)
    {
        ithGpioDisableIntr(RA_INT_PIN); 
    }
	
	#ifdef	ENABLE_TOUCH_PANEL_DBG_MSG
	//ithPrintf("$out(%x)\n",*gpRaSpec->pTouchDownIntr);
	#endif
}

static void _raInitTouchIntr(void)
{
	#ifdef	ENABLE_TOUCH_PANEL_DBG_MSG
    printf("TP init in\n");	
    #endif
    
    ithEnterCritical();
    
    ithGpioClearIntr(RA_INT_PIN);
    ithGpioRegisterIntrHandler(RA_INT_PIN, (ITHGpioIntrHandler)_ra_isr, NULL);
        
    if(gpRaSpec->tpIntTriggerType==RA_INT_LEVLE_TRIGGER)     
        ithGpioCtrlEnable(RA_INT_PIN, ITH_GPIO_INTR_LEVELTRIGGER);
    else
        ithGpioCtrlDisable(RA_INT_PIN, ITH_GPIO_INTR_LEVELTRIGGER);
        
    if(gpRaSpec->tpIntTriggerType==RA_INT_EDGE_TRIGGER)  //if edge trigger
        ithGpioCtrlDisable(RA_INT_PIN, ITH_GPIO_INTR_BOTHEDGE);		//set as single edge
//    else
//        ithGpioCtrlEnable(RA_INT_PIN, ITH_GPIO_INTR_BOTHEDGE);		//set as single edge
    
    if(gpRaSpec->tpIntActiveState==RA_ACTIVE_HIGH)    
        ithGpioCtrlDisable(RA_INT_PIN, ITH_GPIO_INTR_TRIGGERFALLING);	//set as rising edge
    else
        ithGpioCtrlEnable(RA_INT_PIN, ITH_GPIO_INTR_TRIGGERFALLING);	//set as falling edge

    ithIntrEnableIrq(ITH_INTR_GPIO);
    ithGpioEnableIntr(RA_INT_PIN);
        
    ithExitCritical();
    
    #ifdef	ENABLE_TOUCH_PANEL_DBG_MSG
    printf("TP init out\n");	
    #endif
}
#endif

static bool _raChkIntActive(void)
{
	unsigned int regValue = ithGpioGet(RA_INT_PIN);
	
    if(gpRaSpec->tpIntActiveState)
    {
    	if ( regValue & RA_GPIO_MASK )	return true;
    	else	return false;	    
    }
    else
    {
    	if ( !(regValue & RA_GPIO_MASK) )	return true;
    	else	return false;	
    }
}

static void _raInitWakeUpPin(void)
{
	if( (gpRaSpec->tpWakeUpPin>0) && (gpRaSpec->tpWakeUpPin<128) )
	{
		ithGpioSetMode(gpRaSpec->tpWakeUpPin,ITH_GPIO_MODE0);
   		ithGpioSetOut(gpRaSpec->tpWakeUpPin);
   		ithGpioSet(gpRaSpec->tpWakeUpPin);    	
   		ithGpioEnable(gpRaSpec->tpWakeUpPin);
   	}
	else
	{
		printf("NOT initial TOUCH_GPIO_WAKE_PIN\n");
	}
}

static void _raInitResetPin(void)
{
	if( (gpRaSpec->tpResetPin>0) && (gpRaSpec->tpResetPin<128) )
	{
		ithGpioSetMode(gpRaSpec->tpResetPin,ITH_GPIO_MODE0);
   		ithGpioSetOut(gpRaSpec->tpResetPin);
   		ithGpioSet(gpRaSpec->tpResetPin);    	
   		ithGpioEnable(gpRaSpec->tpResetPin);
   	}
	else
	{
		printf("NOT initial TOUCH_RESET_PIN(%d)\n",gpRaSpec->tpResetPin);
	}
}

static void _raInitIntPin(void)
{
	ithGpioSetMode(RA_INT_PIN, ITH_GPIO_MODE0);
	ithGpioSetIn(RA_INT_PIN);

	if(gpRaSpec->tpIntPullEnable)
	{
	    char s = gpRaSpec->tpIntActiveState;
	    
		if(s)   ithGpioCtrlDisable(RA_INT_PIN, ITH_GPIO_PULL_UP);
		else    ithGpioCtrlEnable(RA_INT_PIN, ITH_GPIO_PULL_UP);
			
		ithGpioCtrlEnable(RA_INT_PIN, ITH_GPIO_PULL_ENABLE);		
	}
    
	ithGpioEnable(RA_INT_PIN);	
}

static void _raInitTouchGpioPin(void)
{
	_raInitWakeUpPin();
	
	_raInitResetPin();
	
	_raInitIntPin();
	
	#ifdef CFG_TOUCH_INTR
	_raInitTouchIntr();
	#endif
}

static bool _raGetIntr(void)
{
    if(gpRaSpec->tpIntUseIsr)
    {
        //printf("GetInt1:%x\n",*gpRaSpec->pTouchDownIntr);
        return *gpRaSpec->pTouchDownIntr;
    }
    else
    {
    	return _raChkIntActive();
    }
}

static void _raInitSample(struct ts_sample *s, int nr)
{
	int i;
	struct ts_sample *samp=s;	
	struct ts_sample *ns = (struct ts_sample*)s->next;	
	
	for(i = 0; i < nr; i++)
	{
		samp->finger = 0;
		samp->id = 0;
		samp->x = 0;
		samp->y = 0;
		samp->pressure = 0;
		gettimeofday(&(samp->tv),NULL);	
			
		if(i)   samp++;
		else    samp = (struct ts_sample*)ns;
	}
}

static void _raGetRawPoint(struct ts_sample *samp, int nr)
{
	int real_nr=0;
	struct ts_sample *s=samp;
	unsigned char *buf = (unsigned char *)malloc(gpRaSpec->tpReadChipRegCnt);
	static unsigned char *gLastSamp = NULL;
	int k;
	
	//printf("GetRawPoint...\n");
	
	_raInitSample(s, nr);	
	memset(buf, 0, gpRaSpec->tpReadChipRegCnt);
	
	if(gLastSamp == NULL)
	{
	     gLastSamp = (unsigned char *)malloc(gpRaSpec->tpReadChipRegCnt);
	     if(gLastSamp == NULL)  printf("TP ERROR: gLastSamp out of memory!!\n");
	}
	
	while(real_nr++<nr) 
	{
		//if(gpRaSpec->rawApi.raReadPointBuffer(buf, gpRaSpec->tpReadChipRegCnt)<0)	break;
		int res = gpRaSpec->rawApi.raReadPointBuffer(buf, gpRaSpec->tpReadChipRegCnt);
	    
	    switch(res)
	    {
	        case 0: //got sample & pass(backup buffer)
                for(k=0; k<gpRaSpec->tpReadChipRegCnt; k++)    gLastSamp[k] = buf[k];  
	            break;
	            
	        case 1: //got skip event, restore buffer
	            printf("ra:skip smp!!\n");
                for(k=0; k<gpRaSpec->tpReadChipRegCnt; k++)    buf[k] = gLastSamp[k];
	            break;
	            
	        default:    //error, send touch-up
	            goto errEnd;    //smp will be {0};
	            break;
	    }
		
		if(gpRaSpec->rawApi.raParseRawPxy(s, buf)<0)	break;
		
		if(gpRaSpec->tpHasTouchKey)	gpRaSpec->rawApi.raParseKey(s, buf);

		break;
	}
	
errEnd:
	if(buf!=NULL)	free(buf);		
}

static void _raConvertRawPoint(struct ts_sample *samp, int nr)
{
	int real_nr=0;
	int tmpMaxRawX = gpRaSpec->tpMaxRawX;
	int tmpMaxRawY = gpRaSpec->tpMaxRawY;
	struct ts_sample *s=samp;
	int fgr_nr=0;
	
	#ifdef ENABLE_TOUCH_RAW_POINT_MSG
	printf("	CvtRawPnt:%x,%d,%d, [%x,%x,%x]\n",s->pressure,s->x,s->y, gpRaSpec->tpCvtSwapXY,gpRaSpec->tpCvtReverseX,gpRaSpec->tpCvtReverseY);	
	#endif
	
#ifdef RA_MULTI_FINGER_ENABLE
	fgr_nr = s->finger;
#else
	fgr_nr = 1;
#endif

    while(real_nr++<fgr_nr)
    {
        if(!s->pressure)
        {
		    s++;
		    real_nr++;
            continue;
        }

        if(gpRaSpec->tpCvtSwapXY)
        {
            int tmp = s->x;
            s->x = s->y;
            s->y = tmp;
            tmpMaxRawX = gpRaSpec->tpMaxRawY;
            tmpMaxRawY = gpRaSpec->tpMaxRawX;
        }

        if( (gpRaSpec->tpHasTouchKey) && (s->y > tmpMaxRawX) )	
        {
            s++;
		    real_nr++;
            continue;//NEED TO VERIFY THIS CODE
        }
        
        if(gpRaSpec->tpCvtReverseX)
        {
            if(s->x>=tmpMaxRawX)	s->x = 0;
            else					s->x = tmpMaxRawX - s->x;
        }

        if(gpRaSpec->tpCvtReverseY)
        {
            if(s->y>=tmpMaxRawY)	s->y = 0;
            else					s->y = tmpMaxRawY - s->y;
        }

        if(gpRaSpec->tpCvtScaleX)
        {
            s->x = (short)(((uint32_t)s->x*gpRaSpec->tpScreenX)/tmpMaxRawX);
        }
        
        if(gpRaSpec->tpCvtScaleY)
        {
            s->y = (short)(((uint32_t)s->y*gpRaSpec->tpScreenY)/tmpMaxRawY);
        }
    			
/*
    	if( (s->x>=gpRaSpec->tpScreenX) || (s->y>=gpRaSpec->tpScreenY) || (s->x<0) || (s->y<0) )
    		printf("[TP warning] XY are abnormal, x=%d,%d y=%d,%d\n",s->x,gpRaSpec->tpScreenX,s->y,gpRaSpec->tpScreenY);
    			
    	if(s->x>=gpRaSpec->tpScreenX)	s->x = gpRaSpec->tpScreenX - 1;
    	if(s->y>=gpRaSpec->tpScreenY)	s->y = gpRaSpec->tpScreenY - 1;
    			
    	if(s->x<0)	s->x = 0;
    	if(s->y<0)	s->y = 0;
    				
    	//printf("modify x,y = %d, %d -##\n",s->x,s->y);		
*/ 
		
		#ifdef	ENABLE_TOUCH_PANEL_DBG_MSG
       	printf("	cvtPXY--> %d %d %d\n", s->pressure, s->x, s->y);
		#endif
        
		s++;
    }
}

static void _raGetSample(struct ts_sample *samp, int nr)
{
	int real_nr=0;
	struct ts_sample *s=samp;
	
	_raGetRawPoint(s, 1);		
		
	_raConvertRawPoint(s, 1);
}

#if 0
/*	
only report finger id 0 ~ gpRaSpec->tpSampleNum
*/
static void _raRefineSamples(struct ts_sample *smp)
{
	struct ts_sample *ts = (struct ts_sample *)smp;
	struct ts_sample *tsBase;
	struct ts_sample *tmpS;
	int currFgrNum = (int)smp->finger;
	int maxFgrNum = (int)gpRaSpec->tpSampleNum;
	uint8_t valid_i[10] = {0xFF};
	uint8_t cId = 0;
	uint8_t got0thPressureIsZero = 0;
	uint8_t swapIndex = 0;
	int totalFgrNum = 0;//really report Finger Number to up-layer
	int i;
	int k=0;
	int need_log = 0;//memcpy(smp, tsBase, sizeof(struct ts_sample)*totalFgrNum);
	struct ts_sample *bakSmpBs = (struct ts_sample *)smp;
	struct ts_sample *bakBase;
	int smpSz = sizeof(struct ts_sample);
	
	//if(currFgrNum <= gpRaSpec->tpSampleNum)	return;
	
	if(currFgrNum<1 || currFgrNum>5)   
	{
	    printf("TP Error!! currFgrNum(%d) is abnormal!!\n",currFgrNum);
	    if(currFgrNum)  while(1);
	}
	
	tsBase = (struct ts_sample *)( malloc(sizeof(struct ts_sample)*maxFgrNum) );
	if(tsBase==NULL)
	{
		printf("TP Error!! tsBase is out of memory!!\n");
		return;
	}
	
	tmpS = (struct ts_sample *)tsBase;
	bakBase = (struct ts_sample *)tsBase;
/*
	{
		struct ts_sample *tSp = (struct ts_sample *)smp;
		
		printf("origin Smp:\n");
		for(i=0; i<currFgrNum; i++)	
		{
			printf("	oriSp[%d] fn=%d, id=%d, pxy=%d,%d,%d\n", i,tSp->finger,tSp->id,tSp->pressure,tSp->x, tSp->y);
			tSp++;
		}
	}
*/
	//find out 0 ~ maxFgrNum id
	for(k=0; k<maxFgrNum; k++)
	{
		ts = (struct ts_sample *)smp;
		for(i=0; i<currFgrNum; i++)	
		{
			cId = (uint8_t)ts->id;
			
			if(cId == k)
			{
				valid_i[k] = i;	//IDth in ith sample
				totalFgrNum++;
				
				if( (k==0) && !ts->pressure )	got0thPressureIsZero = 1;
				if(got0thPressureIsZero && k && ts->pressure)	swapIndex = k;
					
				break;
			}
			ts++;
		}
	}
	
	if(got0thPressureIsZero && swapIndex)
	{
		uint8_t tmpIdx = valid_i[0];
		
		printf("Need Swap: %x, %x, %x\n",swapIndex,valid_i[0],valid_i[swapIndex]);
		
		valid_i[0] = valid_i[swapIndex];
		valid_i[swapIndex] = tmpIdx;
	}

	//ts = (struct ts_sample *)smp;
	
	//if the pressureof 0th sample  is 0, then swap sample sequence with the sampe(with p=1)

/*
	printf("valid ID index:[");
	for(i=0; i<gpRaSpec->tpSampleNum; i++)	printf("%x, ", valid_i[i]);
	printf("]\n");
*/
	for(i = 0; i < totalFgrNum; i++)	
	{
		char ci = valid_i[i];		
		
		if( ci != 0xFF )
		{
			ts = (struct ts_sample *)(&smp[ci]);	
			//printf("ci=%x, smp=%x, ts=%x\n",ci,smp,ts);	
			memcpy(tmpS, ts, sizeof(struct ts_sample));
			
			if(tmpS->id>2)	need_log = 1;
			if( tmpS->pressure!=0 && tmpS->pressure!=1 )	need_log = 1;
			if(tmpS->x>800)	need_log = 1;
			if(tmpS->y>480)	need_log = 1;	
				
			if(need_log)
			{
				printf("	tmpS[%d,%d] bs=%x fn=%d, id=%d, pxy=%d,%d,%d\n", i,ci, tmpS,tmpS->finger,tmpS->id,tmpS->pressure,tmpS->x, tmpS->y);
				printf("	ts[%d,%d] bs=%x fn=%d, id=%d, pxy=%d,%d,%d\n", i,ci, ts, ts,ts->finger,ts->id,ts->pressure,ts->x, ts->y);
			}
			
			tmpS->finger = (unsigned int)totalFgrNum;
			tmpS++;
		}
	}
	
	if(0)//need_log)
	{
		struct ts_sample *tSp = (struct ts_sample *)tsBase;
		
		printf("newSmp: smp=%x\n",tSp);
		for(i=0; i<totalFgrNum; i++)	
		{
			printf("	smp[%d] bs=%x fn=%d, id=%d, pxy=%d,%d,%d\n", i,tSp,tSp->finger,tSp->id,tSp->pressure,tSp->x, tSp->y);
			tSp++;
		}
	}
	
	if( (bakSmpBs!=smp) || (tsBase!=bakBase) || (smp->finger<0) || (smp->finger>5) )
	{
	    printf("TP Error!! smp!=bakSmpBs or tsBase!=bakBase:(%x,%x)(%x,%x)\n",smp, bakSmpBs, tsBase, bakBase);
	    while(1);
	}

	memcpy(smp, tsBase, sizeof(struct ts_sample)*totalFgrNum);

	{
		struct ts_sample *tSp = (struct ts_sample *)smp;
		int ShowLog = 0;
		
			if(tmpS->id>2)	ShowLog = 1;
			if( tmpS->pressure!=0 && tmpS->pressure!=1 )	ShowLog = 1;
			if(tmpS->x>800)	ShowLog = 1;
			if(tmpS->y>480)	ShowLog = 1;	
		
		if(ShowLog)
		{
			printf("<newSmp>: smp=%x, oriSp=%x\n",tSp,bakSmpBs,tsBase);
			for(i=0; i<totalFgrNum; i++)	
			{
				printf("	smpA[%d] bs=%x fn=%d, id=%d, pxy=%d,%d,%d\n", i,tSp,tSp->finger,tSp->id,tSp->pressure,tSp->x, tSp->y);
				tSp++;
			}
			
			tSp = (struct ts_sample *)tsBase;
			printf("<tsBase>: smp=%x, oriSp=%x\n",tSp,bakSmpBs,tsBase);
			for(i=0; i<totalFgrNum; i++)	
			{
				printf("	smpA[%d] bs=%x fn=%d, id=%d, pxy=%d,%d,%d\n", i,tSp,tSp->finger,tSp->id,tSp->pressure,tSp->x, tSp->y);
				tSp++;
			}
		}
	}

	{
		char need_show_log = 0;
		struct ts_sample *tSp = (struct ts_sample *)smp;
		
		//printf("newSmp:\n");
		for(i=0; i<totalFgrNum; i++)	
		{
			//printf("	smp[%d] fn=%d, id=%d, pxy=%d,%d,%d\n", i,tSp->finger,tSp->id,tSp->pressure,tSp->x, tSp->y);
			if(tSp->id >= 2)
			{
				need_show_log = 1;
				break;
			}
			tSp++;
		}
		
		
		if(need_show_log)
		{
			tSp = (struct ts_sample *)smp;
			printf("newSmp2: maxFN=%d, ttlFN=%d\n",maxFgrNum,totalFgrNum);
			for(i=0; i<maxFgrNum; i++)	
			{
				printf("	smpB[%d] bs=%x, fn=%d, id=%d, pxy=%d,%d,%d\n", i,tSp,tSp->finger,tSp->id,tSp->pressure,tSp->x, tSp->y);
				tSp++;
			}
		}
	}
	
	if(tsBase!=NULL)    free(tsBase);
}
#else
/*
if finger number > gTpSpec.tpSampleNum(N), then report only N samples that has the first N minimal ID
EX1: finger number = 5, gTpSpec.tpSampleNum = 2, current id list is 1,3,0,5,4 by sequence
    then report 0th & 2nd samples(exchange the sequence of samples "smp")

EX2: finger number = 4, gTpSpec.tpSampleNum = 2, current id list is 2,3,0,5 by sequence
    then report 0th & 2nd sample(the first 2 minimal ID)
*/
static void _raRefineSamples(struct ts_sample *smp)
{
	struct ts_sample *ts = (struct ts_sample *)smp;
	struct ts_sample *tsBase;
	struct ts_sample *tmpS;
	int currFgrNum = (int)smp->finger;
	int maxFgrNum = (int)gpRaSpec->tpSampleNum;
	uint8_t all_id[10];
	uint8_t valid_i[10];
	uint8_t max_id = 0;
	uint8_t min_id = 255;
	uint8_t cId;
	int i;
	int k=0;
	
	//do swap first sample
	if ( !smp->pressure && _raCheckMultiPressure(smp) )
	{
	    //check if all p=1 but [0].p = 0
	    struct ts_sample tmpBkS;
	    int sz = (int)sizeof(struct ts_sample);

	    //search the sample with p=1
	    for(i=0; i<currFgrNum; i++)
	    {
	        if(ts->pressure)    break;
	        
	        ts++;
	    }
	    
	    if(!ts->pressure)   printf("TP ERROR: ts->p(%x) != 1, \n",ts->pressure);
        
        //swap data
	    memcpy( &tmpBkS, smp, sz);
	    memcpy( smp, ts, sz);
	    memcpy( ts, &tmpBkS, sz);
	    
	    //reset ts pointer
	    ts = (struct ts_sample *)smp;
	}

	if(currFgrNum <= gpRaSpec->tpSampleNum)	return;
	
	tsBase = (struct ts_sample *)( malloc(sizeof(struct ts_sample)*maxFgrNum) );
	if(tsBase==NULL)
	{
		printf("TP Error!! tsBase is out of memory!!\n");
		return;
	}
	
	tmpS = (struct ts_sample *)tsBase;
	
	//find out all finger id & Max/Min ID
	for(i=0; i<currFgrNum; i++)	
	{
		all_id[i] = (unsigned char)ts->id;
		
		if(all_id[i] > max_id) max_id = (unsigned char)ts->id;
			
		if(all_id[i] < min_id) min_id = (unsigned char)ts->id;
			
		ts++;
	}
	
/*
{
	printf("minID=%d, maxID=%d\n",min_id,max_id);
	printf("all ID:[");
	for(i=0; i<currFgrNum; i++)	printf("%d, ", all_id[i]);
	printf("]\n");
}
*/

	ts = (struct ts_sample *)smp;
	
	//find out the N index with MIN finger id 
	for(cId=min_id; cId<(max_id+1); cId++)
	{
		for(i=0; i<currFgrNum; i++)	
		{
			if(cId == all_id[i])
			{
			    //choose ID 0 ~ maxFgrNum by sequence
				valid_i[k++] = i;
				//printf("cid=%d, i=%d, FN=%d, id=%d, k=%d, vi=%d\n",cId, i, currFgrNum, all_id[i], k-1, valid_i[k-1]);
				break;
			}
		}	
		if(k>=gpRaSpec->tpSampleNum)	break;
	}
/*
	printf("valid ID index:[");
	for(i=0; i<gpRaSpec->tpSampleNum; i++)	printf("%d, ", valid_i[i]);
	printf("]\n");
*/
	for(i=0; i<maxFgrNum; i++)	
	{
		char ci = valid_i[i];
		ts = (struct ts_sample *)(&smp[ci]);		
		memcpy(tmpS, ts, sizeof(struct ts_sample));
		tmpS->finger = maxFgrNum;
		tmpS++;
	}

	memcpy(smp, tsBase, sizeof(struct ts_sample)*maxFgrNum);
/*
	{
		struct ts_sample *tSp = (struct ts_sample *)smp;
		
		printf("newSmp:\n");
		for(i=0; i<maxFgrNum; i++)	
		{
			printf("	smp[%d] fn=%d, id=%d, pxy=%d,%d,%d\n", i,tSp->finger,tSp->id,tSp->pressure,tSp->x, tSp->y);
			tSp++;
		}
	}
*/
    if(tsBase!=NULL)    free(tsBase);
}
#endif

static void _raUpdateLastXY(struct ts_sample *smp)
{
	int smpChgFlag = 0;

	pthread_mutex_lock(gpRaSpec->raMutex);
	if(smp!=NULL)
	{
#ifdef RA_MULTI_FINGER_ENABLE
		//if(smp->finger > gpRaSpec->tpSampleNum)
        {
			//to refine the sample numbers
			//reduce FN if FN > Max_FN
			//re-align: if the pressure of 0th sample is 0, then swap with the sampe with p=1
			_raRefineSamples(smp);
        }
        
        //to workaround the FT5316's issue that finger down == finger number
        if(smp->finger == 0)
        {
            //if( smp->pressure || smp->x || smp->y)
            if(gpRaInfo->tpStatus != RA_TOUCH_UP)
            {
        	    struct ts_sample *tSmp = (struct ts_sample *)smp;
        	    printf("TP err3A:fn=%d, pxy=%d,%d,%d\n",tSmp->finger, tSmp->pressure, tSmp->x,tSmp->y);
        	    tSmp++;
        	    printf("TP err3B:fn=%d, pxy=%d,%d,%d\n",tSmp->finger, tSmp->pressure, tSmp->x,tSmp->y);                
            }        	
        	smp->finger = 1;
        }
#else
        smp->finger = 1;
#endif

#ifdef RA_USE_XQUEUE
		smpChgFlag = _raCheckIfSmpChg(gp_Sample, smp);
		if( smpChgFlag )
		{
			memcpy((void*)gp_Sample ,(void*)smp, sizeof(struct ts_sample) * smp->finger);
			//SendQueCnt++;
			//printf("inQ1:%d\n",SendQueCnt);
        	if (xQueueSend(raQueue, gp_Sample, 0) != pdTRUE)
        	{
        		printf("	raQuSd1: send queue error (%x,%d,%d)\n",gp_Sample->pressure, gp_Sample->x, gp_Sample->y);
        	}
		}
        gpRaInfo->tpFirstSampHasSend = 1;
#else
		memcpy((void *)gp_Sample ,(void*)smp, sizeof(struct ts_sample) * smp->finger);
#endif
	}
	else
	{
#ifdef RA_USE_XQUEUE
		smpChgFlag = _raCheckIfSmpChg(gp_Sample, smp);
		
       	if(gpRaInfo->tpStatus != TOUCH_NO_CONTACT)
       	{
       		if(smpChgFlag)
       		{
       			memset((void*)gp_Sample , 0, sizeof(struct ts_sample));
       			//SendQueCnt++;
       			//printf("inQ2:%d\n",SendQueCnt);
       			if (xQueueSend(raQueue, gp_Sample, 0) != pdTRUE)
       			{
       				printf("	raQuSd0: send queue error (%x,%d,%d)\n",gp_Sample->pressure, gp_Sample->x, gp_Sample->y);
       			}
       		}
       	}
#else
       	memset((void*)gp_Sample , 0, sizeof(struct ts_sample));
#endif
	}
	
	#ifdef	ENABLE_TOUCH_PANEL_DBG_MSG
	if(smp!=NULL)	printf("	EnQue:p=%x,xy=%d,%d\n", smp->pressure, smp->x, smp->y);
	else            printf("	EnQue:p=%x,xy=%d,%d\n", 0, 0, 0);
	#endif

	pthread_mutex_unlock(gpRaSpec->raMutex);
}

static uint32_t _raSendKeypadValue(void)
{
	uint32_t var=0;
    pthread_mutex_lock(gpRaSpec->raMutex);
    var = *gpRaSpec->pTpKeypadValue;
    pthread_mutex_unlock(gpRaSpec->raMutex);
	return var;
}

/**
 * to judge if S/W needs to get touch point
 *
 * @return: true for YES, false for NO
 * 
 * [NOTE]:return true if need to get touch sample via i2c bus(ex: when TP's INT signal actived, or other special rules)
          return false if NO need to get sample(NO TP's INT actived, or other special rules)
          in usual case, return true for INT actived, false for INT not actived..
          special rule1: To prevent from getting the same sample, when use "pulling INT" without interrupt.
          special rule2: DO NOT wanna loss the first point when finger just touch down(or quickly touch)
 */
static bool _raNeedToGetSample(void)
{
    if(*gpRaSpec->pTpInitialized!=true)	return false;

    if(_raGetIntr()==true)
    {
        gpRaInfo->tpCurrINT = 1;
        
    	if(gpRaSpec->tpIntUseIsr)
    	{
    		if( (gpRaSpec->tpIntrType == RA_INT_TYPE_ZT2083) || (gpRaSpec->tpIntrType == RA_INT_TYPE_IT7260) )
    		{
    			static uint32_t tc3=0;
	    		if(!g_raIntrLowCnt++)	tc3 = itpGetTickCount();
	    		raLowDur = itpGetTickDuration(tc3);
		    	
		    	if(raLowDur>gpRaSpec->tpSampleRate)
		    	{
		    		//printf("	NGS:%x,%d,%d\n", gpRaInfo->tpNeedToGetSample, g_raIntrLowCnt, raLowDur);	
	    			//printf("	  -ztInt:%x,%x\n",regValue,regValue & RA_GPIO_MASK);
	    			g_raIntrLowCnt = 0;
		    		gpRaInfo->tpNeedToGetSample = 1;
		    		ithGpioEnableIntr(RA_INT_PIN); 
		    	}
		    	
	    		if ( gpRaInfo->tpNeedToGetSample )	return true;
	    		else	return false;
    		}
    		else
    		{
	    		ithGpioDisableIntr(RA_INT_PIN);
    	        return true;
    		}
    	}
    	else
    	{
        	if( (gpRaSpec->tpIntrType == RA_INT_TYPE_ZT2083) || (gpRaSpec->tpIntrType == RA_INT_TYPE_IT7260) )
        	{
				//printf("IAR2:%x,%x,%d,%d\n",gpRaInfo->tpNeedToGetSample,gpRaInfo->tpFirstSampHasSend,g_raIntrLowCnt,raLowDur);
				if(gpRaInfo->tpFirstSampHasSend)
				{
    			    static uint32_t tc3=0;
	    		    if(!g_raIntrLowCnt++)	tc3 = itpGetTickCount();
	    		    raLowDur = itpGetTickDuration(tc3);

			    	if(raLowDur>gpRaSpec->tpSampleRate)
			    	{
			    		gpRaInfo->tpNeedToGetSample = 1;
			    		g_raIntrLowCnt = 0;
			    	}
				}
				else
				{
					gpRaInfo->tpNeedToGetSample = 1;
				}
        	}
 
        	if( gpRaInfo->tpNeedToGetSample )    return true;	
        	else    return false;    	
    	}
    }
    else
    {
        g_raIntrLowCnt = 0;
        gpRaInfo->tpCurrINT = 0;
        return false;
    }
}

/**
 * to update the touch status
 * 
 * [HINT 1]: when use "pulling INT", remember to prevent from getting the same sample
 * [HINT 2]: DO NOT loss the first point(each point) in quickly clicking case.
 * [HINT 3]: if not initial yet, TP will sleep 100ms
 * [HINT 4]: Basically, INT is active, then report "TOUCH DOWN" event; INT is not active, then report "TOUCH UP" event
 * [HINT 5]: INT has 2 action type:
    1). pull low until finger is not touch(contact) on TP
    2). pull low as a pulse which width about 2~20 micro-second until finger is not touch(contact) on TP
*/
static void _raUpdate(struct ts_sample *tpSmp)
{
    if(*gpRaSpec->pTpInitialized!=true)
    {
        printf("WARNING:: TP has not initial, yet~~~\n");
        usleep(RA_IDLE_TIME_NO_INITIAL);
        return;
    }
    
    #ifdef	ENABLE_TOUCH_PANEL_DBG_MSG
    if(gpRaInfo->tpStatus != RA_TOUCH_NO_CONTACT)
    	printf("	UpdateSmp:INT=%x, s=%x pxy=(%d,%d,%d)\n",gpRaInfo->tpCurrINT, gpRaInfo->tpStatus, tpSmp->pressure,tpSmp->x,tpSmp->y);
    #endif
    
    if(gpRaInfo->tpCurrINT)
    {
        gpRaSpec->rawApi.raIntActiveRule(tpSmp);
    }
    else
    {
    	gpRaSpec->rawApi.raIntNotActiveRule(tpSmp);
    }
    
    usleep(RA_IDLE_TIME);	//sleep 2ms
}

static void _raSendQue2Sample(struct ts_sample *os, struct ts_sample *ts)
{
	int ret=0,i;
    int fn = (int)ts->finger;
	struct ts_sample *s = (struct ts_sample *)os;
	struct ts_sample *tts = (struct ts_sample *)ts;
	struct ts_sample *tmpNxt=(struct ts_sample *)os->next;
	int sz = (int)sizeof(struct ts_sample);
	int loop = fn;
	
	if( (os==NULL) || (ts==NULL) )
	{
		printf("NULL pointer, STOP!!\n");
		return; //while(1);
	}
	
	//printf("loop = %d, %d\n",loop,fn);

    for(i=0; i<loop; i++)
    {
    	//printf("prb12:[%d,%d]:%d,%d,%d\n",tts->finger, tts->id, tts->pressure, tts->x, tts->y);
    	//if(tts->finger==2)	printf("prb2:[%d,%d]:%d,%d,%d\n",tts[1]->finger, tts[1]->id, tts[1]->pressure, tts[1]->x, tts[1]->y);

    	if(i >= gpRaSpec->tpSampleNum)	break;

		if(!i)
		{
     	    memcpy((void *)s, (void *)ts, sz);
     	    s->next = (struct ts_sample *)tmpNxt;	
     	    gettimeofday(&s->tv,NULL);
     	    if(s->pressure)	gpRaInfo->tpFirstSampHasSend = 1; 
     	    
     	    //printf("prb0:[%d,%d]:%d,%d,%d\n",s->finger, s->id, s->pressure, s->x, s->y);
		}
		else
    	{
    	 	struct ts_sample *s1 = (struct ts_sample *)(&tmpNxt[i-1]);
    	  	struct ts_sample *s2 = (struct ts_sample *)(&ts[i]);

    	   	if(i >= fn)	break;
    	    		
    	   	if( (s1==NULL) || (s2==NULL) )
    	   	{
    	   		printf("	[TP ERROR]: incorrect pointer:s1=%x, s2=%x\n",s1,s2);
    	   		continue;
    	   	}
    	   	
    	   	s1->pressure = (unsigned int)s2->pressure;
    	   	s1->x = s2->x;
    	   	s1->y = s2->y;
    	   	s1->id = (unsigned int)s2->id;
    	   	s1->finger = (unsigned int)fn;
    	   	//printf("prb3:[%d,%d,%d]:%d,%d,%d\n",i, s1->finger, s1->id, s1->pressure, s1->x, s1->y);
    	   	//printf("prb13:[%d,%d]:%d,%d,%d\n",s1->finger, s1->id, s1->pressure, s1->x, s1->y);
    	}
    }
}

/**
 * the thread for handling real-time touch event(<2ms)
 * 
 * [HINT]: use gp_Sample for comunication with function "_raProbeSample()"
 */ 
static void* _raProbeHandler(void* arg)
{
    struct ts_sample tpSmp[10];
    
    while(1)
    {
    	//printf("probing...\n");
        if(_raNeedToGetSample())	_raGetSample(&tpSmp[0], 1);
        
        _raUpdate(&tpSmp[0]);           
    }
    return NULL;
}

/******************************************************************************
 * do initial flow
 ******************************************************************************/
/**
 * Set raw api spec base address
 *
 * @spec: the spec structure
 *
  */ 
void _raSetSpecBase(RA_TP_SPEC *spec)
{
	gpRaSpec = spec;	
	gpRaInfo = gpRaSpec->raInfoBase;
}


/**
 * TP initial flow
 *
 * @return: 0 for success, -1 for failure
 *
  */ 
int _raDoInitial(void)
{
	int ret;
    int res;
    pthread_t task;
    pthread_attr_t attr;
    
    gp_Sample = gpRaSpec->gTpSmpBase;  
    
    gpRaSpec->pTpKeypadValue			= NULL;							//default function

    if(gpRaSpec->rawApi.raIntActiveRule == NULL)
        gpRaSpec->rawApi.raIntActiveRule 	= _raIntActiveRule_vendor;		//default function
        
    if(gpRaSpec->rawApi.raIntNotActiveRule == NULL)
        gpRaSpec->rawApi.raIntNotActiveRule = _raIntNotActiveRule_vendor;	//default function
    
    //initialize the TP SPEC first.
    gpRaSpec->rawApi.raInitSpec();
    	
    if(gpRaSpec->tpHasPowerOnSeq)     gpRaSpec->rawApi.raDoPowerOnSeq();
    
    //do initial GPIO between raDoPowerOnSeq() & raDoInitProgram()
    printf("try to init GPIO pin\n");  
    _raInitTouchGpioPin();    

    if(gpRaSpec->tpNeedProgB4Init)
    {
    	if(gpRaSpec->rawApi.raDoInitProgram()<0)
    	{
    		printf("[TOUCH]warning:: touch panel do initial progeram chip fail\n");
    		return -1;
    	}
	}

    //create touch mutex	
    res = pthread_mutex_init(gpRaSpec->raMutex, NULL);
   	if(res)
   	{
   	    printf("[Touch Panel]%s() L#%ld: ERROR, init touch mutex fail! res=%ld\r\n", __FUNCTION__, __LINE__, res);
   	    return -1;
   	}

    printf("Create touch pthread~~\n");	
	//create thread	
    pthread_attr_init(&attr);
    res = pthread_create(&task, &attr, _raProbeHandler, NULL);        
    if(res)
    {
    	printf( "[TouchPanel]%s() L#%ld: ERROR, create _raProbeHandler() thread fail! res=%ld\n", res );
    	return -1;
    }

#ifdef RA_USE_XQUEUE
	raQueue = xQueueCreate(RA_QUEUE_LEN, (unsigned portBASE_TYPE) sizeof(struct ts_sample)*gpRaSpec->tpSampleNum);	
    if(raQueue == NULL)
    {
    	printf( "[TouchPanel]%s() L#%ld: ERROR, create xQueueCreate() fail! res=%ld\n", raQueue );
    	return -1;
    }
#endif

    *gpRaSpec->pTouchDownIntr = false;
	*gpRaSpec->pTpInitialized = true;
	
	
	#ifdef	CFG_TOUCH_BUTTON
	if( gpRaSpec->tpHasTouchKey && (gpRaSpec->pTpKeypadValue != NULL) )
	{
		ts_read_kp_callback = _raSendKeypadValue;
	}
	#endif

	
	printf("TP initial has finished(%d)\n",*gpRaSpec->pTpInitialized);  
	
	return 0;
}

/**
 * Send touch sample(samp->pressure, samp->x, samp->y, and samp->tv)
 *
 * @param samp: the touch samples
 * @param nr: the sample count that upper layer wanna get.
 * @return: the really total touch sample count
 *
 * [HINT 1]:this function will be called by it7260_read(). 
 * [HINT 2]:get the samples from the global variable(g_sample).
 */ 
int _raProbeSample(struct ts_sample *samp, int nr)
{
	struct ts_sample *s=samp;
	struct ts_sample *tSmpBase = (struct ts_sample *)malloc(sizeof(struct ts_sample)*gpRaSpec->tpSampleNum);

	pthread_mutex_lock(gpRaSpec->raMutex);
	
	_raInitSample(samp, nr);	
	
#ifdef RA_MULTI_FINGER_ENABLE
	if(samp->next == NULL)	printf("	$$$$$$ WARNING!! samp->next = NULL $$$$$$\n");
#endif
	
	if(gpRaInfo->tpIntr4Probe)
	{
        struct ts_sample *tSmp = (struct ts_sample *)gp_Sample;

#ifdef RA_USE_XQUEUE
		if(tSmpBase==NULL)	printf("memory locate error\n");
        else    tSmp = (struct ts_sample *)tSmpBase;

        if ( xQueueReceive(raQueue, tSmp, 0) != pdTRUE )    tSmp = (struct ts_sample *)gp_Sample;
#endif
        _raSendQue2Sample(s, tSmp);
	}
	else
	{
#ifdef RA_USE_XQUEUE
        struct ts_sample *ts = (struct ts_sample *)tSmpBase;
        	
        if (xQueueReceive(raQueue, ts, 0) == pdTRUE)
        {
        	_raSendQue2Sample(s, ts);
        }
        else
        {
			if(s->finger)
			{
				printf("	###>>> s->fgr != 0, fn=%d\n",s->finger);
				s->finger = 0;
			}
        }
#else
		if(s->finger)
		{
			printf("	##>>> s->fgr != 0, fn=%d\n",s->finger);
			s->finger = 0;
		}
#endif
	}
	
	
#ifdef RA_MULTI_FINGER_ENABLE
    if( (s->finger>1) && !(s->pressure) )
    {
    	struct ts_sample *ns = (struct ts_sample *)(samp->next);
    	struct ts_sample swpSmp;
    	int tmpLoop, foundFlag=0;
    	int i,fn = 0;

    	fn = s->finger;
    	for(i=1; i<fn; i++)
    	{
    	    if(ns->pressure)    foundFlag = 1;
    	    ns++;
    	}
    	
    	if(foundFlag)
    	{
            printf("do swap sample0 with other sample, os=%x, ns=%x\n",s,ns);
            printf("s1: fn=%x, id=%x, pxy=(%d,%d,%d)\n", s->finger, s->id, s->pressure, s->x, s->y);
            printf("s2: fn=%x, id=%x, pxy=(%d,%d,%d)\n", ns->finger, ns->id, ns->pressure, ns->x, ns->y);    	    
    	}
    }
#endif
	
	#ifdef	ENABLE_TOUCH_PANEL_DBG_MSG
	if(s->pressure)	gNoEvtCnt = 3;	
	if( gNoEvtCnt )	
	{
		printf("gfQ, INT=%x, s=%x, fn=%x, id=%d, pxy=(%d,%d,%d)\n", gpRaInfo->tpIntr4Probe, gpRaInfo->tpStatus, s->finger, s->id, s->pressure, s->x, s->y);
		if( !s->pressure )	gNoEvtCnt--;
	}
	#endif

#ifdef RA_USE_XQUEUE
	if(tSmpBase!=NULL)  free(tSmpBase);
#endif

	pthread_mutex_unlock(gpRaSpec->raMutex);

	if(s->finger)   return 1;
	else            return 0;
}