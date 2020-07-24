/*
 you have to sure 3 things before porting TP driver
 1).INT is work normal
 2).I2C BUS can read data from TP chip
 3).Parse the X,Y coordination correctly

 These function are customized.
 You Have to modify these function with "(*)" mark.
 These functions(3&4) are almost without modification
 Function(5~7) will be modified deponding on chip's feature.
  0._tpInitSpec_vendor()           //set control config(*)
  1._tpReadPointBuffer_vendor()    //read point buffer(*)
  2._tpParseRawPxy_vendor()        //parse the touch point(*)
  3._tpIntActiveRule_vendor()      //touch-down RULE
  4._tpIntNotActiveRule_vendor()   //touch-up RULE
5._tpParseKey_vendor()           //depend on TP with key
6._tpDoPowerOnSeq_vendor();      //depend on TP with power-on sequence
7._tpDoInitProgram_vendor();         //depend on TP with initial programming
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <alloca.h>
#include <pthread.h>	
#include "openrtos/FreeRTOS.h"
#include "openrtos/queue.h"
#include "ite/ith.h" 
#include "ite/itp.h"
#include "config.h"
#include "tslib-private.h"

#include "api-raw.h"

#ifdef	CFG_TOUCH_MULTI_FINGER
    #define TP_MULTI_FINGER_ENABLE
#endif

#define USE_RAW_API
#define TP_USE_XQUEUE
/****************************************************************************
 * initial Kconfig setting
 ****************************************************************************/

#if	defined(CFG_TOUCH_I2C0) || defined(CFG_TOUCH_I2C1) || defined(CFG_TOUCH_I2C2) || defined(CFG_TOUCH_I2C3)
#define TP_INTERFACE_I2C   (0)
#endif

#if	defined(CFG_TOUCH_SPI) || defined(CFG_TOUCH_SPI0) || defined(CFG_TOUCH_SPI1)
#define TP_INTERFACE_SPI   (1)
#endif

#define TP_INT_PIN	    CFG_GPIO_TOUCH_INT
#define TP_GPIO_MASK    (1<<(TP_INT_PIN%32))

#ifdef	CFG_GPIO_TOUCH_WAKE
#if (CFG_GPIO_TOUCH_WAKE<128)
#define TP_GPIO_WAKE_PIN	CFG_GPIO_TOUCH_WAKE
#endif 
#endif 

#ifdef	CFG_GPIO_TOUCH_RESET
#if (CFG_GPIO_TOUCH_RESET<128)
#define TP_GPIO_RESET_PIN	CFG_GPIO_TOUCH_RESET
#else
#define TP_GPIO_RESET_PIN	(-1)
#endif 
#else
#define TP_GPIO_RESET_PIN	(-1)
#endif 

#ifdef	CFG_TOUCH_ADVANCE_CONFIG

#ifdef	CFG_TOUCH_SWAP_XY
#define	TP_SWAP_XY		(1)
#else
#define	TP_SWAP_XY		(0)
#endif

#ifdef	CFG_TOUCH_REVERSE_X
#define	TP_REVERSE_X	(1)
#else
#define	TP_REVERSE_X	(0)
#endif

#ifdef	CFG_TOUCH_REVERSE_Y
#define	TP_REVERSE_Y	(1)
#else
#define	TP_REVERSE_Y	(0)
#endif

#else

#define	TP_SWAP_XY		(0)
#define	TP_REVERSE_X	(1)
#define	TP_REVERSE_Y	(0)

#endif

#define	TOUCH_NO_CONTACT		(0)
#define	TOUCH_DOWN				(1)
#define	TOUCH_UP				(2)

#define	TP_ACTIVE_LOW           (0)
#define	TP_ACTIVE_HIGH          (1)

#ifdef	CFG_GPIO_TOUCH_INT_ACTIVE_HIGH
#define	TP_INT_ACTIVE_STATE     TP_ACTIVE_HIGH
#else
#define	TP_INT_ACTIVE_STATE     TP_ACTIVE_LOW
#endif

#define	TP_INT_LEVLE_TRIGGER    (1)
#define	TP_INT_EDGE_TRIGGER     (0)

#define	TP_INT_TYPE_KEEP_STATE  (0)
#define	TP_INT_TYPE_ZT2083      (0)
#define	TP_INT_TYPE_FT5XXX      (1)
#define	TP_INT_TYPE_IT7260      (2)

#define	TP_WITHOUT_KEY          (0)
#define	TP_HAS_TOUCH_KEY        (1)
#define	TP_GPIO_PIN_NO_DEF      (-1)

#ifdef	CFG_TOUCH_BUTTON
#define	TP_TOUCH_BUTTON		TP_HAS_TOUCH_KEY
#else
#define	TP_TOUCH_BUTTON		TP_WITHOUT_KEY
#endif

#ifdef CFG_TOUCH_INTR
#define	TP_ENABLE_INTERRUPT     (1)
#else
#define	TP_ENABLE_INTERRUPT     (0)
#endif

#ifdef TP_MULTI_FINGER_ENABLE
#define	MAX_FINGER_NUM	(2)		//depend on TP Native Max Finger Numbers  
#else
#define	MAX_FINGER_NUM	(1)
#endif

#ifdef TP_USE_XQUEUE
#define	TP_QUEUE_LEN	(64)
#endif
/****************************************************************************
 * touch cofig setting
 ****************************************************************************/
#define TP_IDLE_TIME                (2000)
#define TP_IDLE_TIME_NO_INITIAL     (100000)

/****************************************************************************
 * ENABLE_TOUCH_POSITION_MSG :: just print X,Y coordination & 
 * 								touch-down/touch-up
 * ENABLE_TOUCH_IIC_DBG_MSG  :: show the IIC command 
 * ENABLE_TOUCH_PANEL_DBG_MSG:: show send-queue recieve-queue, 
 *                              and the xy value of each INTr
 ****************************************************************************/
//#define ENABLE_TOUCH_POSITION_MSG
//#define ENABLE_TOUCH_RAW_POINT_MSG
//#define ENABLE_TOUCH_PANEL_DBG_MSG
//#define ENABLE_TOUCH_IIC_DBG_MSG
//#define ENABLE_SEND_FAKE_SAMPLE

/****************************************************************************
 * MACRO define of cytma568
 ****************************************************************************/
#define TP_I2C_DEVICE_ID       (0x48>>1)
#define TP_SAMPLE_RATE	       (33)

#ifdef	CFG_LCD_ENABLE
#define	TP_SCREEN_WIDTH	    ithLcdGetWidth()
#define	TP_SCREEN_HEIGHT	ithLcdGetHeight()
#else
#define	TP_SCREEN_WIDTH	    (400)
#define	TP_SCREEN_HEIGHT	(1280)
#endif

#ifdef	CFG_TOUCH_BUTTON
//#define TP_CYTMA448_BUTTON_REPORT_BY_COORDINATION    //cytma568 has specific registers for button report
#endif
/****************************************
 *
 ***************************************/
typedef struct 
{
	char tpCurrINT;
	char tpStatus;
	char tpNeedToGetSample;
	char tpNeedUpdateSample;
	char tpFirstSampHasSend;
	char tpIntr4Probe;
	char tpIsInitFinished;
	int  tpDevFd;
	int  tpIntrCnt;
}tp_info_tag;//tp_gv_tag

typedef struct tp_spec_tag
{
    //TP H/W setting
    char tpIntPin;		    //INT signal GPIO pin number
    char tpIntActiveState;	//High=1, Low=0
    char tpIntTriggerType;  //interrupt trigger type. 0:edge trigger, 1:level trigger
    char tpWakeUpPin;		//Wake-Up pin GPIO pin number, -1: means NO Wake-Up pin.
    char tpResetPin;		//Reset pin GPIO pin number, -1: means NO reset pin.
    char tpIntrType;		//0:keep state when touch down(like ZT2083), 1:like FT5XXX type 2:like IT7260, 3:others....  
    char tpInterface; 		//0:I2C, 1:SPI, 2:other...
    char tpI2cDeviceId; 	//I2C device ID(slave address) if TP has I2C interface
    char tpHasTouchKey;		//0: NO touch key, 1:touch key type I, 2:touch key type II, ...
    char tpIntUseIsr;	    //0:polling INT siganl, 1:INT use interrupt, 
    char tpMaxFingerNum;	//The TP native maximun of finger numbers
    char tpIntActiveMaxIdleTime;    //default: 33ms, cytma568: 100ms
        
    //TP resolution
    int  tpMaxRawX;
    int  tpMaxRawY;
    int  tpScreenX;
    int  tpScreenY;
    
    //TP convert function
    char tpCvtSwapXY;		//0:Disable, 1:Enable
    char tpCvtReverseX;     //0:Disable, 1:Enable
    char tpCvtReverseY;     //0:Disable, 1:Enable 
    char tpCvtScaleX;		//0:Disable, 1:Enable
    char tpCvtScaleY;		//0:Disable, 1:Enable
    
    //TP sample specification
    char tpEnTchPressure;	//0:disable pressure info, 1:enable pressure info
    char tpSampleNum;		//0:NO scense, 1: single touch 2~10:multi-touch("tpSampleNum" must be <= "tpMaxFingerNum") 
    char tpSampleRate;		//UNIT: mill-second, range 8~16 ms(60~120 samples/per second)  
    
    //TP idle time
    int  tpIdleTime;		//sleep time for polling INT signal(even if interrupt mode).    
    int  tpIdleTimeB4Init;	//sleep time if TP not initial yet.       
    int  tpReadChipRegCnt;	//read register count for getting touch xy coordination
    
    //TP specific function
    char tpHasPowerOnSeq;	//0:NO power-on sequence, 1:TP has power-on sequence
    char tpNeedProgB4Init;	//0:TP IC works well without programe flow, 1:TP IC need program before operation.
    char tpNeedAutoTouchUp;
    char tpIntPullEnable;	//use internal pull up/down function    
} TP_SPEC;

/***************************
 * global variable
 **************************/
static struct ts_sample g_sample[MAX_FINGER_NUM];
static struct ts_sample gTmpSmp[10];

static char g_TouchDownIntr = false;
static char  g_IsTpInitialized = false;
static pthread_mutex_t 	gTpMutex;

#ifdef USE_RAW_API
static RA_TP_SPEC  gTpSpec;
static RA_GV       gTpInfo = { 0,RA_TOUCH_NO_CONTACT,1,0,0,0,0,0,0};
#else
static TP_SPEC     gTpSpec;
static tp_info_tag gTpInfo = { 0,TOUCH_NO_CONTACT,1,0,0,0,0,0,0};
#endif

static unsigned int dur=0;
static unsigned int iDur=0;
static unsigned int lowDur=0;

struct timeval T1, T2;
static int g_tpCntr = 0;
static unsigned int gLastNumFinger = 0;

//for the function "_tpFixIntHasNoResponseIssue()"
static int  g_IntrLowCnt = 0;
static int  g_IntrAtvCnt = 0;

struct timeval tv1, tv2;
static int  gNoEvtCnt = 0;

#ifdef TP_USE_XQUEUE
static QueueHandle_t tpQueue;
static int  SendQueCnt = 0;
#endif

/*************************************************
 global variable: gTpKeypadValue
 key0 is pressed if gTpKeypadValue's bit 0 is 1
 key1 is pressed if gTpKeypadValue's bit 1 is 1
   ...and so on

 NO key event if gTpKeypadValue = 0 
 MAX key number: 32 keys
***************************************************/
static uint32_t	gTpKeypadValue;

/*##################################################################################
 *                         the protocol of private function
 ###################################################################################*/
static void _tpInitSpec_vendor(void);
static int  _tpReadPointBuffer_vendor(unsigned char *buf, int cnt);
static int  _tpParseRawPxy_vendor(struct ts_sample *s, unsigned char *buf);
static void _tpParseKey_vendor(struct ts_sample *s, unsigned char *buf);

static void _tpIntActiveRule_vendor(struct ts_sample *tpSmp);
static void _tpIntNotActiveRule_vendor(struct ts_sample *tpSmp);

static void _tpDoPowerOnSeq_vendor(void);
static int _tpDoInitProgram_vendor(void);

/* *************************************************************** */

/* *************************************************************** */
#ifdef	CFG_TOUCH_BUTTON
extern unsigned int (*ts_read_kp_callback)(void);
#endif
/*##################################################################################
 *                        the private function implementation
 ###################################################################################*/
static int _tpCyRdSysInfo(void)
{
	unsigned char buf[512];
	ITPI2cInfo *evt;
    unsigned char	I2cCmd[4];
    int i2cret;
    uint8_t datalen;

    printf("Get System Information: 02h\n");

    evt = alloca(sizeof(ITPI2cInfo));

   	buf[0] = 0x04;
   	buf[1] = 0x00;
   	buf[2] = 0x05;
   	buf[3] = 0x00;
   	buf[4] = 0x2F;
   	buf[5] = 0x00;
   	buf[6] = 0x02;
   	
   	evt->slaveAddress   = TP_I2C_DEVICE_ID; 
   	evt->cmdBuffer      = &buf;
   	evt->cmdBufferSize  = 7;
   	evt->dataBuffer     = buf;
   	evt->dataBufferSize = 0;
   	i2cret = write(gTpInfo.tpDevFd, evt, 1);
   	if(i2cret<0)	return -1;
    	    
    while(ithGpioGet(TP_INT_PIN))
    {
       usleep(1000);
    }

    evt->slaveAddress   = TP_I2C_DEVICE_ID;
    evt->cmdBuffer      = &I2cCmd;
    evt->cmdBufferSize  = 0;
    evt->dataBuffer     = buf;
    evt->dataBufferSize = 0x33;
    i2cret = read(gTpInfo.tpDevFd, evt, 1);
    if(i2cret<0)	return -1;
    	    
	{
		int i,c;
			
		c = evt->dataBufferSize;
		printf("\n	Program buf:");
		for(i=0; i<c; i++)
		{
			printf("%02x ",buf[i]);	
			if( (i&0xF) == 0xF )    printf("\n                    ");
		}

		printf("\n");
	}
}

static void _tpInitSpec_vendor(void)
{
    gTpSpec.tpIntPin          	= (char)TP_INT_PIN;           //from Kconfig setting
    gTpSpec.tpWakeUpPin         = (char)TP_GPIO_PIN_NO_DEF;   //from Kconfig setting
    gTpSpec.tpResetPin          = (char)TP_GPIO_PIN_NO_DEF;   //from Kconfig setting
    gTpSpec.tpIntUseIsr         = (char)TP_ENABLE_INTERRUPT;  //from Kconfig setting
    gTpSpec.tpIntActiveState    = (char)TP_INT_ACTIVE_STATE;        //from Kconfig setting    
    gTpSpec.tpIntTriggerType    = (char)TP_INT_EDGE_TRIGGER; //from Kconfig setting   level/edge
    
    gTpSpec.tpInterface         = (char)TP_INTERFACE_I2C;	  //from Kconfig setting
    gTpSpec.tpIntActiveMaxIdleTime = (char)100;	              //from Kconfig setting
        
    gTpSpec.tpMaxRawX           = (int)CFG_TOUCH_X_MAX_VALUE; //from Kconfig setting
    gTpSpec.tpMaxRawY           = (int)CFG_TOUCH_Y_MAX_VALUE; //from Kconfig setting
    gTpSpec.tpScreenX           = (int)TP_SCREEN_WIDTH;       //from Kconfig setting
    gTpSpec.tpScreenY           = (int)TP_SCREEN_HEIGHT;      //from Kconfig setting
    
    gTpSpec.tpCvtSwapXY        = (char)TP_SWAP_XY;            //from Kconfig setting
    gTpSpec.tpCvtReverseX      = (char)TP_REVERSE_X;          //from Kconfig setting
    gTpSpec.tpCvtReverseY      = (char)TP_REVERSE_Y;          //from Kconfig setting
    gTpSpec.tpCvtScaleX        = (char)0;                    //from Kconfig setting
    gTpSpec.tpCvtScaleY        = (char)0;                    //from Kconfig setting
    
    gTpSpec.tpI2cDeviceId       = (char)TP_I2C_DEVICE_ID;	  //from this driver setting
    gTpSpec.tpEnTchPressure     = (char)0;                    //from this driver setting
    gTpSpec.tpSampleNum         = (char)MAX_FINGER_NUM;       //from this driver setting
    gTpSpec.tpSampleRate        = (char)TP_SAMPLE_RATE;       //from this driver setting
    gTpSpec.tpIntrType          = (char)TP_INT_TYPE_IT7260;	  //from this driver setting
    gTpSpec.tpHasTouchKey       = (char)TP_TOUCH_BUTTON;      //from this driver setting                                                               
    gTpSpec.tpIdleTime          = (int)TP_IDLE_TIME;          //from this driver setting
    gTpSpec.tpIdleTimeB4Init    = (int)TP_IDLE_TIME_NO_INITIAL;//from this driver setting    
    //gTpSpec.tpMoveDetectUnit    = (int)1;
    gTpSpec.tpReadChipRegCnt    = (int)32;

    //special initial flow
    gTpSpec.tpHasPowerOnSeq     = (char)0;
    gTpSpec.tpNeedProgB4Init    = (char)1;    
    gTpSpec.tpNeedAutoTouchUp	= (char)0; 

/*
    printf("gTpSpec.tpIntPin         = %d\n",gTpSpec.tpIntPin);
    printf("gTpSpec.tpIntActiveState = %x\n",gTpSpec.tpIntActiveState);
    printf("gTpSpec.tpWakeUpPin      = %d\n",gTpSpec.tpWakeUpPin);
    printf("gTpSpec.tpResetPin       = %d\n",gTpSpec.tpResetPin);
    printf("gTpSpec.tpIntrType       = %x\n",gTpSpec.tpIntrType);
    printf("gTpSpec.tpInterface      = %x\n",gTpSpec.tpInterface);
    printf("gTpSpec.tpI2cDeviceId    = %x\n",gTpSpec.tpI2cDeviceId);
    printf("gTpSpec.tpHasTouchKey    = %x\n",gTpSpec.tpHasTouchKey);
    printf("gTpSpec.tpIntUseIsr      = %x\n",gTpSpec.tpIntUseIsr);
    printf("gTpSpec.tpMaxRawX        = %d\n",gTpSpec.tpMaxRawX);
    printf("gTpSpec.tpMaxRawY        = %d\n",gTpSpec.tpMaxRawY);
    printf("gTpSpec.tpScreenX        = %d\n",gTpSpec.tpScreenX);
    printf("gTpSpec.tpScreenY        = %d\n",gTpSpec.tpScreenY);
    printf("gTpSpec.tpCvtSwapXY     = %x\n",gTpSpec.tpCvtSwapXY);
    printf("gTpSpec.tpCvtReverseX   = %x\n",gTpSpec.tpCvtReverseX);
    printf("gTpSpec.tpCvtReverseY   = %x\n",gTpSpec.tpCvtReverseY);
    printf("gTpSpec.tpCvtScaleX     = %x\n",gTpSpec.tpCvtScaleX);
    printf("gTpSpec.tpCvtScaleY     = %x\n",gTpSpec.tpCvtScaleY);
    printf("gTpSpec.tpEnTchPressure  = %x\n",gTpSpec.tpEnTchPressure);
    printf("gTpSpec.tpSampleNum      = %x\n",gTpSpec.tpSampleNum);
    printf("gTpSpec.tpSampleRate     = %x\n",gTpSpec.tpSampleRate);
    printf("gTpSpec.tpIdleTime       = %d\n",gTpSpec.tpIdleTime);
    printf("gTpSpec.tpIdleTimeB4Init = %d\n",gTpSpec.tpIdleTimeB4Init);
    printf("gTpSpec.tpHasPowerOnSeq  = %x\n",gTpSpec.tpHasPowerOnSeq);
    printf("gTpSpec.tpNeedProgB4Init = %x\n",gTpSpec.tpNeedProgB4Init);
	printf("gTpSpec.tpNeedAutoTouchUp= %x\n",gTpSpec.tpNeedAutoTouchUp);
*/
    //initial global variable "gTpInfo"
/*    
    printf("gTpInfo.tpCurrINT              = %x\n",gTpInfo.tpCurrINT);
    printf("gTpInfo.tpStatus               = %x\n",gTpInfo.tpStatus);
    printf("gTpInfo.tpNeedToGetSample      = %x\n",gTpInfo.tpNeedToGetSample);
    printf("gTpInfo.tpNeedUpdateSample     = %x\n",gTpInfo.tpNeedUpdateSample);
    printf("gTpInfo.tpFirstSampHasSend     = %x\n",gTpInfo.tpFirstSampHasSend);
    printf("gTpInfo.tpFirstSampHasSend     = %x\n",gTpInfo.tpIsInitFinished);
    printf("gTpInfo.tpIntr4Probe           = %x\n",gTpInfo.tpIntr4Probe);
    printf("gTpInfo.tpDevFd                = %x\n",gTpInfo.tpDevFd);    
    printf("gTpInfo.tpIntrCnt              = %x\n",gTpInfo.tpIntrCnt);
*/
}    

static void _tpDoPowerOnSeq_vendor(void)
{
	//for cytma448 power-on sequence	
	
	//0.set "GPIO26 pin" as pull-up	
	ithGpioSetMode( gTpSpec.tpResetPin, ITH_GPIO_MODE0);
	ithGpioClear(gTpSpec.tpResetPin);
	ithGpioSetOut(gTpSpec.tpResetPin);
	ithGpioEnable(gTpSpec.tpResetPin);	

    printf("DO POWER-ON sequence, reset pin:%d\n",gTpSpec.tpResetPin);	
    if(gTpSpec.tpResetPin == (char)-1) return;
    
	//1.set "Reset pin" output as low for 1ms	
	ithGpioSetMode( gTpSpec.tpResetPin, ITH_GPIO_MODE0);
	ithGpioClear(gTpSpec.tpResetPin);
	ithGpioSetOut(gTpSpec.tpResetPin);
	ithGpioEnable(gTpSpec.tpResetPin);	
	usleep(1*1000);

    //2.set "Reset pin" output HIGH for 12ms
	ithGpioSet(gTpSpec.tpResetPin);
	usleep(12*1000);
}

static int _tpDoInitProgram_vendor(void)
{
	unsigned char buf[64];
    
	{
        ITPI2cInfo *evt;
    	unsigned char	I2cCmd;
        int i2cret;
        uint8_t datalen;
    
        evt = alloca(sizeof(ITPI2cInfo));
    	
    	I2cCmd = 0xFE;
    	evt->slaveAddress   = TP_I2C_DEVICE_ID;
    	evt->cmdBuffer      = &I2cCmd;
    	evt->cmdBufferSize  = 1;
    	evt->dataBuffer     = buf;
    	evt->dataBufferSize = 7;
    	i2cret = read(gTpInfo.tpDevFd, evt, 1);

    	usleep(10*1000);
	}
	
	_tpCyRdSysInfo();
	
	//if You don't do this, INT will not be pulled HIGH(I don't know why?)	 
	if(_tpReadPointBuffer_vendor(buf, 7)<0)    return -1;
    else    return 0;
}

/****************************************************************
input: 
    buf: the buffer base, 
    cnt: the buffer size in bytes
output: 
    0: pass(got valid data)
    1: skip sample this time 
    -1: i2c error (upper-layer will send touch-up event)
*****************************************************************/
static int _tpReadPointBuffer_vendor(unsigned char *buf, int cnt)
{
    ITPI2cInfo *evt;
	unsigned char	I2cCmd;
    int i2cret;
    uint8_t datalen;

    evt = alloca(sizeof(ITPI2cInfo));
	
	I2cCmd = 0xFE;		//1100 0010
	evt->slaveAddress   = gTpSpec.tpI2cDeviceId;
	evt->cmdBuffer      = &I2cCmd;
	evt->cmdBufferSize  = 1;
	evt->dataBuffer     = buf;
	evt->dataBufferSize = 7;
	i2cret = read(gTpInfo.tpDevFd, evt, 1);
	if(i2cret<0)	return -1;
		
	if( buf[0] && (buf[0]!=0x02) )	
	{
		datalen = buf[0];
		unsigned char *tmpBuf = NULL;		
		
		//printf("tpLen:%x\n",buf[0]);

		if(datalen > gTpSpec.tpReadChipRegCnt)
		{
			tmpBuf = (unsigned char *)malloc(datalen+16);
			if(tmpBuf==NULL)
			{
				printf("Read Pointer buffer fail, out of mem!!\n");
				return -1;
			}
			//printf("tpLen > 32 :%x\n",buf[0]);
		}
		
		evt->slaveAddress   = gTpSpec.tpI2cDeviceId;
		evt->cmdBuffer      = &I2cCmd;
		evt->cmdBufferSize  = 1;
		evt->dataBufferSize =datalen;		
		
		if(datalen > gTpSpec.tpReadChipRegCnt)	evt->dataBuffer     = tmpBuf;
		else			evt->dataBuffer     = buf;			

		i2cret = read(gTpInfo.tpDevFd, evt, 1);

		if(datalen > gTpSpec.tpReadChipRegCnt)
		{
			memcpy(buf, tmpBuf, gTpSpec.tpReadChipRegCnt);
			if(tmpBuf!=NULL)	free(tmpBuf);
		}

		#ifdef	ENABLE_TOUCH_PANEL_DBG_MSG
		if( buf[0] && (buf[0]!=0x02) )	
		{
			int i,c;
			
			c = (buf[5]&0x1F)*10+8;
			printf("\n	raw_buf:");
			for(i=0; i<7; i++)	printf("%02x ",buf[i]);
			printf("\n		");
			if(c>7)	for(i=7; i<17; i++)	printf("%02x ",buf[i]);
			printf("\n		");
			if(c>17)	for(i=17; i<27; i++)	printf("%02x ",buf[i]);
			printf("\n");		
		}
		if(i2cret<0)	printf("	~~ RdPbfErr: ~~\n");
		#endif
	
		if(i2cret<0)	return -1;
	}
	else
	{
	    //printf("No TP event, Ori-Buf: %x,%x, %x,%x, %x,%x\n",buf[0],buf[1],buf[2],buf[3],buf[4],buf[5]);	    
        if(buf[0] == 2)
        {
            //printf("skip TP event\n");
            return 1;
        }
	}
	
	return 0;
}                               

/*
ref: CYTMA568 Technical Reference Manual (TRM) "touch report" at page:142
 In multi-finger case, parse the registers of TP controller, event if fingers became zero.
 Because it still has its touch-up event need to be reported when finger is 0.
*/
static int _tpParseRawPxy_vendor(struct ts_sample *s, unsigned char *buf)
{
#ifdef TP_MULTI_FINGER_ENABLE
	int i=0;
	char device_mode = 0;
	char numOfTchPnt = 0;
	unsigned int lfnum = gLastNumFinger;
	char loopNum = 0;
	
	device_mode = buf[0];
	numOfTchPnt = buf[5]&0x1F;
	
	/* Device Mode[2:0] == 0 :Normal operating Mode*/
	if(device_mode <= 2) return (-1); 
		
	/* buf[2]!=0x01: NOT a touch report*/
	if(buf[2] != 0x01) return (-1); 

	loopNum = numOfTchPnt;
	
	if(lfnum < numOfTchPnt)
	{
		//printf("fgr_chg++: %d, %d\n", lfnum, numOfTchPnt);
	}
	
	if(lfnum > numOfTchPnt)
	{
		for(i = numOfTchPnt; i < lfnum; i++)
		{
			if( !(buf[8 + 10 * i] & 0x80) && ((buf[8 + 10 * i] & 0x60) == 0x60) )	loopNum++;
		}
	}	

	if(loopNum)
	{		
		struct ts_sample *smp = (struct ts_sample*)s;		
		#ifdef	TP_CYTMA448_BUTTON_REPORT_BY_COORDINATION
		uint32_t tmpKey = 0;	
		#endif

		for(i = 0; i < loopNum; i++)
		{
			if( (buf[8 + 10 * i] & 0x80) && ((buf[8 + 10 * i] & 0x60) != 0x60) )	smp->pressure = 1;
			else	smp->pressure = 0;
				
			smp->id = (unsigned int)(buf[8+10*i]&0x1F); //touch id
			smp->finger = loopNum;
			smp->x = (int)((((unsigned int)buf[10+10*i]<<8)&0x0F00) | ((unsigned int)buf[9+10*i]));
			smp->y = (int)((((unsigned int)buf[12+10*i]<<8)&0x0F00) | ((unsigned int)buf[11+10*i]));
			//printf("	RAW->[%d][%x, %d, %d]--> %d %d %d\n", i, smp, smp->id, smp->finger, smp->pressure, smp->x, smp->y);

			#ifdef	TP_CYTMA448_BUTTON_REPORT_BY_COORDINATION
			//parse buttons by coordination
			//button1:(880,0)->(980,100)
			//button2:(880,180)->(980,280)
			//button3:(880,340)->(980,460)
			if(smp->x > 800)
			{
			    if( (smp->x > 880) && (smp->x < 980) )
			    {
			        unsigned char status = (unsigned char)(buf[8+10*i]&0x80);
			        
			        if( (smp->y > 0) && (smp->y < 100) && (status) )    tmpKey |= 0x01;	
			        if( (smp->y > 180) && (smp->y < 280) && (status) )    tmpKey |= 0x02;
			        if( (smp->y > 340) && (smp->y < 460) && (status) )    tmpKey |= 0x04;
			            
			        buf[2] = 0x03  ;    //mark as key report(follow cytma448's datasheet P144)
			    }
			    
			    //clear this sample & continue if x > 800
			    smp->finger = 0;			    
			    smp->pressure = 0;
			    smp->x = 0;
			    smp->y = 0;
			    continue;
			}
			#endif

            //skip the sample that finger id > MAX_FINGER_NUM
			if( (smp->id >= gTpSpec.tpSampleNum) && (gTpSpec.tpSampleNum > 1) )
			{
			    //clear this sample & continue
			    smp->finger = 0;			    
			    smp->pressure = 0;
			    smp->x = 0;
			    smp->y = 0;
			    continue;
			}
			   
			if(gTpSpec.tpSampleNum > 1)   smp++;
		}
		
		pthread_mutex_lock(&gTpMutex);
		gLastNumFinger = numOfTchPnt;
		pthread_mutex_unlock(&gTpMutex);
			
#ifdef	TP_CYTMA448_BUTTON_REPORT_BY_COORDINATION
		buf[5] = tmpKey;
		if( !tmpKey && gTpKeypadValue)  buf[2] = 0x03;//set key report ID if gTpKeypadValue != 0 to clean this variable
		//printf("GotKeyEv:%x\n",tmpKey);
#endif

		return 0;
	}

	return (-1);

#else
	if(buf[2] != 0x01)	return;	//if NOT touch report, then return directly
		
	if( (buf[0]>2) && buf[5] && (buf[6+2]&0x80) && ((buf[6+2]&0x60)!=0x60) )
	{
		int tmpX;						
		int tmpY;	
		uint8_t tmpP;
		
		tmpX=(buf[10]<<8)+buf[9];
		tmpY=(buf[12]<<8)+buf[11];	
		
		tmpP = buf[13];
		if(tmpP>1)	tmpP = 1;
			
		s->finger = 1;
		s->pressure = tmpP;
		s->x = tmpX;
		s->y = tmpY;

		//printf("x=%d,y=%d,p=%d\n", tmpX,tmpY,tmpP);			
	}
	else
	{
		//printf("	Parse:tch-Up\n",buf[0]);
		s->pressure = 0;
		s->x = 0;
		s->y = 0;		
	}
#endif
}

static void _tpParseKey_vendor(struct ts_sample *s, unsigned char *buf)
{
	if(buf[0] <= 2) return (-1); 
		
	/* buf[2]!=0x03: NOT a button report*/
	if(buf[2] != 0x03) return (-1); 

    pthread_mutex_lock(&gTpMutex);	
    gTpKeypadValue = buf[5]&0x0F;
    pthread_mutex_unlock(&gTpMutex);	
	
	#ifdef	ENABLE_TOUCH_PANEL_DBG_MSG
	{
		int i,c;
		
		c = buf[0];
		printf("\n	kp-raw-buf:");
		for(i=0; i<7; i++)	printf("%02x ",buf[i]);
		printf("\n		");
		if(datalen>7)	for(i=7; i<11; i++)	printf("%02x ",buf[i]);
		printf("\n		");
	}
	#endif
}

static void _tpIntActiveRule_vendor(struct ts_sample *tpSmp)
{
    //IGNORE: default use _raIntActiveRule_vendor() in api-raw.c
}

static void _tpIntNotActiveRule_vendor(struct ts_sample *tpSmp)
{
    //IGNORE: default use _raIntActiveRule_vendor() in api-raw.c
}

#ifdef USE_RAW_API
static void _tpRegRawFunc(int fd)
{
	//initial raw api function
	RA_FUNC *ra = (RA_FUNC *)&gTpSpec.rawApi;
	
   	ra->raInitSpec 			= _tpInitSpec_vendor;
   	ra->raParseRawPxy 		= _tpParseRawPxy_vendor;
   	ra->raReadPointBuffer 	= _tpReadPointBuffer_vendor;
   	ra->raParseKey 			= _tpParseKey_vendor;
   	ra->raDoInitProgram 	= _tpDoInitProgram_vendor;
   	ra->raDoPowerOnSeq 		= _tpDoPowerOnSeq_vendor;
   	ra->raIntActiveRule 	= NULL;
   	ra->raIntNotActiveRule 	= NULL;
   	//ra->raIntActiveRule 	= _tpIntActiveRule_vendor;
   	//ra->raIntNotActiveRule = _tpIntNotActiveRule_vendor;   	

   	gTpInfo.tpDevFd = fd;
   	gTpSpec.gTpSmpBase = (struct ts_sample *)&g_sample[0];
   	
   	gTpSpec.raInfoBase = &gTpInfo;
   	gTpSpec.raMutex = &gTpMutex;
   	gTpSpec.pTouchDownIntr = &g_TouchDownIntr;
   	gTpSpec.pTpInitialized = &g_IsTpInitialized;   	

	#ifdef	CFG_TOUCH_BUTTON
	gTpSpec.pTpKeypadValue = &gTpKeypadValue;
	#endif
   	
   	_raSetSpecBase(&gTpSpec);
}
#endif

#ifdef	ENABLE_SEND_FAKE_SAMPLE
int _getFakeSample(struct ts_sample *samp, int nr)
{
	printf("tp_getXY::cnt=%x\n",g_tpCntr);
	
	if(g_tpCntr++>0x100)
	{
		if( !(g_tpCntr&0x07) )
		{
			unsigned int i;
			i = (g_tpCntr>>3)&0x1F;
			if(i<MAX_FAKE_NUM)
			{
				samp->pressure = 1;
				samp->x = gFakeTableX[i];
				samp->y = gFakeTableY[i];
				printf("sendXY.=%d,%d\n",samp->x,samp->y);	
			}
		}
	}

	return nr;
}
#endif
/*##################################################################################
 *                           private function above
 ###################################################################################*/

/*##################################################################################
 #                       public function implementation
 ###################################################################################*/

/**
 * Send touch sample(samp->pressure, samp->x, samp->y, and samp->tv)
 *
 * @param inf: the module information of tslibo(just need to care "inf->dev")
 * @param samp: the touch samples
 * @param nr: the sample count that upper layer wanna get.
 * @return: the total touch sample count
 *
 * [HINT 1]:this function will be called by SDL every 33 ms. 
 * [HINT 2]:Upper layer(SDL) will judge finger-down(contact on TP) if samp->pressure>0, 
 *          finger-up(no touch) if samp->pressure=0.
 * [HINT 3]:please return either 0 or 1 (don't return other number for tslib rule, even if sample number is > 1) 
 */ 
static int cytma568_read(struct tslib_module_info *inf, struct ts_sample *samp, int nr)
{
	struct tsdev *ts = inf->dev;
	unsigned int regValue;
	int ret;
	int total = 0;
	int tchdev = ts->fd;
	struct ts_sample *s=samp;
	
	#ifdef	ENABLE_SEND_FAKE_SAMPLE
	return _getFakeSample(samp,nr);
	#endif	
	
	if(g_IsTpInitialized==false)
	{
#ifdef	USE_RAW_API
    	_tpRegRawFunc(tchdev);

		if(!_raDoInitial())	return -1;
		else                return 0;
#else
		printf("TP first init(INT is GPIO %d)\n",TP_INT_PIN);
        gTpInfo.tpDevFd = tchdev;	
		if(!_tpDoInitial())	return 0;
		else                return -1;
#endif
	}
	
	//to probe touch sample 
#ifdef	USE_RAW_API
	ret = _raProbeSample(samp, nr);
#else
	ret = _tpProbeSample(samp, nr);
#endif

	#ifdef	ENABLE_TOUCH_PANEL_DBG_MSG
	if(ret)	printf("    deQue-O:fn=%d (%d,%d,%d)r=%d\n", samp->finger, samp->pressure, samp->x, samp->y, ret );
	#endif
	
	#ifdef	ENABLE_TOUCH_PANEL_DBG_MSG
	if(samp->pressure)	gNoEvtCnt = 3;	
	if( gNoEvtCnt )	
	{
		printf("    deQue-O1:[%x] fn=%d, id=%d (%d,%d,%d)r=%d\n", samp, samp->finger, samp->id, samp->pressure, samp->x, samp->y, ret );
		if(samp->finger>1)
		{
			struct ts_sample *tsp = (struct ts_sample *)samp->next;
			printf("    deQue-Q2:[%x] fn=%d, id=%d (%d,%d,%d)r=%d\n", tsp, tsp->finger, tsp->id, tsp->pressure, tsp->x, tsp->y, ret );
		}
		if( !samp->pressure )	gNoEvtCnt--;
	}
	#endif
	
	return ret;
}

static const struct tslib_ops cytma568_ops =
{
	cytma568_read,
};

TSAPI struct tslib_module_info *cytma568_mod_init(struct tsdev *dev, const char *params)
{
	struct tslib_module_info *m;

	m = malloc(sizeof(struct tslib_module_info));
	if (m == NULL)
		return NULL;

	m->ops = &cytma568_ops;
	return m;
}

#ifndef TSLIB_STATIC_CASTOR3_MODULE
	TSLIB_MODULE_INIT(cytma568_mod_init);
#endif
