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
#define	TP_REVERSE_X	(0)
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
 * MACRO define of GT911
 ****************************************************************************/
#define TOUCH_DEVICE_ID1      (0xBA >> 1)
#define TOUCH_DEVICE_ID2      (0x28 >> 1)

#define TP_I2C_DEVICE_ID       TOUCH_DEVICE_ID1

#define TP_SAMPLE_RATE	(33)

#ifdef	CFG_LCD_ENABLE
#define	TP_SCREEN_WIDTH	    ithLcdGetWidth()
#define	TP_SCREEN_HEIGHT    ithLcdGetHeight()
#else
#define	TP_SCREEN_WIDTH	    800
#define	TP_SCREEN_HEIGHT	480
#endif

#define GTP_POLL_TIME         10
#define GTP_ADDR_LENGTH       2
#define GTP_CONFIG_MAX_LENGTH 186
#define FAIL                  0
#define SUCCESS               1

//Register define
#define GTP_READ_COOR_ADDR    0x814E
#define GTP_REG_SLEEP         0x8040
#define GTP_REG_SENSOR_ID     0x814A
#define GTP_REG_CONFIG_DATA   0x8047
#define GTP_REG_VERSION       0x8140
#define GTP_REG_READ_POINT    0x8150
#define GTP_REG_MODULE_SWITCH 0x804D
#define GTP_REG_CONFIG_CHKSUM 0x80FF
#define GTP_REG_CONFIG_FRESH  0x8100

#define X2Y_Enable		 0x08
#define RESOLUTION_LOC   3
#define TRIGGER_LOC      8

#define GTP_MAX_HEIGHT   600
#define GTP_MAX_WIDTH    1024
#define GTP_INT_TRIGGER  1    //0:Rising 1:Falling
#define GTP_MAX_TOUCH         6
#define GTP_ESD_CHECK_CIRCLE  2000
#define GTP_ADDR_LENGTH       2


//#define	ENABLE_GT911_FW_UPGRADE
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

static int g_tchCoorX2Y=0;
/*##################################################################################
 *                         the protocol of private function
 ###################################################################################*/
#ifdef CFG_TOUCH_INTR
static void _tp_isr(void* data);
static void _initTouchIntr(void);
#endif

static bool _tpChkIntActive(void);
static int _tpCheckMultiPressure(struct ts_sample *theSmp);
static int _checkIfSmpChg(struct ts_sample *s1, struct ts_sample *s2);
/* *************************************************************** */
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
static int  _tpDoInitial(void);
static void _tpInitWakeUpPin(void);
static void _tpInitResetPin(void);
static void _tpInitIntPin(void);
static void _tpInitTouchGpioPin(void);

static void _initSample(struct ts_sample *s, int nr);
static char _tpGetIntr(void);
/* *************************************************************** */

/* *************************************************************** */
static void _tpGetRawPoint(struct ts_sample *samp, int nr);
static void _tpConvertRawPoint(struct ts_sample *samp, int nr);
static void _tpUpdateLastXY(struct ts_sample *smp);

static bool _tpNeedToGetSample(void);
static void _tpGetSample(struct ts_sample *samp, int nr);
static void _tpUpdate(struct ts_sample *tpSmp);
/* *************************************************************** */

static void* _tpProbeHandler(void* arg);
static int  _tpProbeSample(struct ts_sample *samp, int nr);

//*************************** PART2:TODO define **********************************
// STEP_1(REQUIRED): Define Configuration Information Group(s)
// Sensor_ID Map:
/* sensor_opt1 sensor_opt2 Sensor_ID
    GND           GND          0 
    VDDIO         GND          1 
    NC            GND          2 
    GND           NC/300K      3 
    VDDIO         NC/300K      4 
    NC            NC/300K      5 
*/

/*##################################################################################
 *                        the private function implementation
 ###################################################################################*/

/******************************************************************************
 * the read flow for reading the FT5316's register by using iic repead start
 ******************************************************************************/
static bool gtp_i2c_read(uint8_t* cmd, uint8_t *buf, uint32_t len)
{
	int i2cret;
	ITPI2cInfo *gt911_evt;

	gt911_evt = alloca(sizeof(*gt911_evt));
	gt911_evt->slaveAddress   = TP_I2C_DEVICE_ID;
	gt911_evt->cmdBuffer      = cmd;
	gt911_evt->cmdBufferSize  = 2;
	gt911_evt->dataBuffer     = buf;
	gt911_evt->dataBufferSize = len;	
	i2cret = read(gTpInfo.tpDevFd, gt911_evt, 1);	
	return i2cret;
}

static bool gtp_i2c_write(uint8_t* cmd, uint32_t cmdlen, uint8_t *buf, uint32_t len)
{
	int i2cret;
	ITPI2cInfo *gt911_evt;

	gt911_evt = alloca(sizeof(*gt911_evt));
	gt911_evt->slaveAddress   = TP_I2C_DEVICE_ID;
	gt911_evt->cmdBuffer      = cmd;
	gt911_evt->cmdBufferSize  = cmdlen;
	gt911_evt->dataBuffer     = buf;
	gt911_evt->dataBufferSize = len;		
	i2cret = write(gTpInfo.tpDevFd, gt911_evt, 1);	

	return i2cret;
}

static unsigned char gtp_rev8(unsigned char *SrcData, unsigned char length)
{
	unsigned int value = 0;
	unsigned char rev = 0;
	unsigned char i;
	
	for (i = 0; i < length; i++)
	{
		value += SrcData[i];
		//printf("i=%d, sum=%02x, v=%02x\n",i,value,SrcData[i]);
	}
	rev = (unsigned char)(value&0xFF);
	rev = (~rev) + 1;
	//printf("sum = %02x, rev=%02x\n",value,rev);
	return rev;
}

/*##################################################################################
 *                        the private function implementation
 ###################################################################################*/
static void _tpInitSpec_vendor(void)
{
    gTpSpec.tpIntPin          	= (char)TP_INT_PIN;           //from Kconfig setting
    gTpSpec.tpWakeUpPin         = (char)TP_GPIO_PIN_NO_DEF;   //from Kconfig setting
    gTpSpec.tpResetPin          = (char)TP_GPIO_RESET_PIN;   //from Kconfig setting
    gTpSpec.tpIntUseIsr         = (char)TP_ENABLE_INTERRUPT;  //from Kconfig setting
    gTpSpec.tpIntActiveState    = (char)TP_ACTIVE_LOW;        //from Kconfig setting    
    gTpSpec.tpIntTriggerType    = (char)TP_INT_EDGE_TRIGGER; //from Kconfig setting   level/edge
    
    gTpSpec.tpInterface         = (char)TP_INTERFACE_I2C;	  //from Kconfig setting
    gTpSpec.tpIntActiveMaxIdleTime = (char)TP_SAMPLE_RATE;	  //from Kconfig setting
        
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
    gTpSpec.tpIntrType          = (char)TP_INT_TYPE_FT5XXX;	  //from this driver setting
    gTpSpec.tpHasTouchKey       = (char)TP_WITHOUT_KEY;       //from this driver setting                                                               
    gTpSpec.tpIdleTime          = (int)TP_IDLE_TIME;          //from this driver setting
    gTpSpec.tpIdleTimeB4Init    = (int)TP_IDLE_TIME_NO_INITIAL;//from this driver setting    
#ifdef  TP_MULTI_FINGER_ENABLE
    gTpSpec.tpReadChipRegCnt    = (int)48;
#else
    gTpSpec.tpReadChipRegCnt    = (int)6;
#endif
    
    //special initial flow
    gTpSpec.tpHasPowerOnSeq     = (char)1;
    gTpSpec.tpNeedProgB4Init    = (char)1;    
    gTpSpec.tpNeedAutoTouchUp	= (char)1; 
    gTpSpec.tpIntPullEnable     = (char)0;
    
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
	//for Gt911 Linux  power on methods.	

    printf("DO POWER-ON sequence, reset pin:%d\n",gTpSpec.tpResetPin);	
    if(gTpSpec.tpResetPin == (char)-1) return;
    
	//1.set "Reset pin" & "INT pin" are output-low for 1ms	
	ithGpioSetMode( gTpSpec.tpResetPin, ITH_GPIO_MODE0);
	ithGpioClear(gTpSpec.tpResetPin);
	ithGpioSetOut(gTpSpec.tpResetPin);
	ithGpioEnable(gTpSpec.tpResetPin);	
	
	ithGpioSetMode( gTpSpec.tpIntPin, ITH_GPIO_MODE0);
	ithGpioClear(gTpSpec.tpIntPin);
	ithGpioSetOut(gTpSpec.tpIntPin);
	ithGpioEnable(gTpSpec.tpIntPin);
	
	usleep(1*1000);

    //2.set "Reset pin" output HIGH for 55ms
	ithGpioSet(gTpSpec.tpResetPin);
	usleep(55*1000);

    //3.set "INT pin" output HIGH for 50 ms
	ithGpioSet(gTpSpec.tpIntPin);
	usleep(50*1000);

}

static int _tpDoInitProgram_vendor(void)
{
	uint8_t buf = 0x4;
	uint8_t writeCmd[2] = { 0x80, 0x40 };	
	uint8_t write_cfg_config[GTP_CONFIG_MAX_LENGTH];	
	uint8_t rd_cfg_config[GTP_CONFIG_MAX_LENGTH];	
	uint8_t rd_cfg_buf[3];	
	int i,ret;

	#ifdef ENABLE_GT911_FW_UPGRADE
	ret = gtp_i2c_write(writeCmd, 2, &buf, 1);	
	#endif
	rd_cfg_buf[0] = GTP_REG_CONFIG_DATA >> 8;								 ///read config information
	rd_cfg_buf[1] = GTP_REG_CONFIG_DATA & 0xff; 	
	
	memset(write_cfg_config, 0, GTP_CONFIG_MAX_LENGTH);
	ret = gtp_i2c_read(rd_cfg_buf, write_cfg_config, GTP_CONFIG_MAX_LENGTH);

	#ifdef	ENABLE_TOUCH_PANEL_DBG_MSG
	for(i=0;i<GTP_CONFIG_MAX_LENGTH;i++)
	{
		printf("0x%04X: 0x%02X\n", GTP_REG_CONFIG_DATA + i, write_cfg_config[i]);
	}
	#endif

	gTpSpec.tpMaxRawX = (write_cfg_config[2]<<8 | write_cfg_config[1]) - 1;
	gTpSpec.tpMaxRawY = (write_cfg_config[4]<<8 | write_cfg_config[3]) - 1;
	
	printf("CFG_RawX=%d, CFG_RawY=%d\n",gTpSpec.tpMaxRawX,gTpSpec.tpMaxRawY);
	
	#ifdef	ENABLE_GT911_FW_UPGRADE
	{
		//force to set resolution
		uint8_t wxbuf[2] = {0x20, 0x03};
		uint8_t wybuf[2] = {0xE0, 0x01};
		uint8_t wCmd[4];	
		uint16_t crc16;
		uint8_t  rev8;
		
		//modified the register to change x/y resolution 
		wCmd[0] = 0x80;	
		wCmd[1] = 0x48;	
		wCmd[2] = 0x20;	
		wCmd[3] = 0x03;	
		ret = gtp_i2c_write(&wCmd[0], 4, &wxbuf[0], 0);
		
		wCmd[0] = 0x80;	
		wCmd[1] = 0x4A;	
		wCmd[2] = 0xE0;	
		wCmd[3] = 0x01;	
		ret = gtp_i2c_write(&wCmd[0], 4, &wybuf[0], 0);

		wCmd[0] = 0x80;	
		wCmd[1] = 0x4D;	
		wCmd[2] = 0x8C;	
		ret = gtp_i2c_write(&wCmd[0], 3, &wybuf[0], 0);

		//calculate the sum of reg 0x8047~0x80FE
		write_cfg_config[1]	= 0x20;
		write_cfg_config[2]	= 0x03;
		write_cfg_config[3]	= 0xE0;
		write_cfg_config[4]	= 0x01;
		write_cfg_config[6]	= 0x8C;
		
		rev8 = gtp_rev8(&write_cfg_config[0], GTP_CONFIG_MAX_LENGTH-2);
		
		wCmd[0] = 0x80;	
		wCmd[1] = 0xFF;	
		wCmd[2] = rev8;	
		ret = gtp_i2c_write(&wCmd[0], 3, &wybuf[0], 0);	
		
		wCmd[0] = 0x81;	
		wCmd[1] = 0x00;	
		wCmd[2] = 0x01;	
		ret = gtp_i2c_write(&wCmd[0], 3, &wybuf[0], 0);	
		//ret = gtp_i2c_write(end_cmd,3, tmpBuf, 1);
		printf("write rev8=%02x\n",rev8);
	}
	#endif

	memset(write_cfg_config, 0, GTP_CONFIG_MAX_LENGTH);
	ret = gtp_i2c_read(rd_cfg_buf, write_cfg_config, GTP_CONFIG_MAX_LENGTH);

	#ifdef	ENABLE_TOUCH_PANEL_DBG_MSG
	for(i=0;i<GTP_CONFIG_MAX_LENGTH;i++)	
	{
		printf("0x%04X: 0x%02X\n", GTP_REG_CONFIG_DATA + i, write_cfg_config[i]);
	}
	gTpSpec.tpMaxRawX = (write_cfg_config[2]<<8 | write_cfg_config[1]) - 1;
	gTpSpec.tpMaxRawY = (write_cfg_config[4]<<8 | write_cfg_config[3]) - 1;
	printf("CFG2_RawX=%d, CFG_RawY=%d\n",gTpSpec.tpMaxRawX,gTpSpec.tpMaxRawY);
	#endif	
	
	if( (write_cfg_config[6]&0x03) == 0x00)
	{
		printf("ACTIVE_HIGH:1\n");
		gTpSpec.tpIntActiveState = TP_ACTIVE_HIGH;
	}
		
	if( (write_cfg_config[6]&0x03) == 0x02)
	{
		printf("ACTIVE_HIGH:2\n");
		gTpSpec.tpIntActiveState = TP_ACTIVE_HIGH;
	}

	//for workaround the GT911 version A F/W issue
	#ifndef	CFG_TOUCH_ADVANCE_CONFIG
	if( (write_cfg_config[0x8E] == 0x28) && (write_cfg_config[0x8F] == 0x26) && (write_cfg_config[0x90] == 0x24) && (write_cfg_config[0x91] == 0x22) )
	{
		printf("\nX reverse:0x804D=%x\n",write_cfg_config[6]);
		gTpSpec.tpCvtReverseX = 1;
		for(i=0x8E; i<0x92; i++)	printf("0x%04X: 0x%02X\n", GTP_REG_CONFIG_DATA + i, write_cfg_config[i]);		
	}
	#endif

	if( write_cfg_config[6] & X2Y_Enable) //write_cfg_config[6]  = 0x804D
		g_tchCoorX2Y = 1;
		
	#ifdef ENABLE_GT911_FW_UPGRADE
    buf = 0x0;
	ret = gtp_i2c_write(writeCmd, 2, &buf, 1);	
	#endif

    printf("_tpDoInitProgram:2, g_tchCoorX2Y=%x, IntActiveState=%x\n",g_tchCoorX2Y,gTpSpec.tpIntActiveState);

    return 0;
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
    bool recv = true;
    int ret;
    uint8_t touch_num;
    uint8_t tmpBuf[1]={0};
    uint8_t rd_cfg_buf[2]={GTP_READ_COOR_ADDR >> 8, GTP_READ_COOR_ADDR & 0xFF};	
    uint8_t  end_cmd[3] = {GTP_READ_COOR_ADDR >> 8, GTP_READ_COOR_ADDR & 0xFF , 0};
    uint8_t rd_point_buf[2]={GTP_REG_READ_POINT >> 8, GTP_REG_READ_POINT & 0xFF};	
    static uint8_t lastTmp = 0;

    ret = gtp_i2c_read(rd_cfg_buf, tmpBuf, 1);
    if (ret < 0)
    {
        printf("I2C transfer error. errno:%d\n ", ret);
        recv = false;
        goto end;
    }

#ifdef	ENABLE_TOUCH_PANEL_DBG_MSG
    printf("	------tmpBuf=%02x-------\r\n",tmpBuf[0]);
#endif

    if((tmpBuf[0] == 0x0) && (lastTmp >= 0x81) )
    {
        //skip it at this time, but need restore the last value for next layer
        //but it must be not to send exit-work command
        printf("skip TP this time:%x,%x\n",tmpBuf[0],lastTmp);     
        return 1;
    }
    lastTmp = tmpBuf[0];
    buf[0] = lastTmp;

    if((tmpBuf[0] & 0x80) == 0)
    {
        recv = false;
        goto end;
    }

    touch_num = tmpBuf[0] & 0x0f;
    if (touch_num > GTP_MAX_TOUCH || tmpBuf[0] == 0x80)
    {
        recv = false;
        goto exit_work_func;
    }
    
    if(touch_num==0)
    {
        recv = false;
        printf("GOT fn = 0!!\n");
        goto exit_work_func;
    }
    
    #ifdef  TP_MULTI_FINGER_ENABLE
    ret = gtp_i2c_read(rd_point_buf, &buf[1], touch_num*8-1);
    #else
    ret = gtp_i2c_read(rd_point_buf, &buf[1], cnt-1);
    #endif
    
    if (ret < 0)
    {
        printf("I2C transfer error. errno:%d\n ", ret);
        recv = false;
        goto end;
    }

exit_work_func: 
	tmpBuf[0] = 0;
    ret = gtp_i2c_write(end_cmd,3, tmpBuf, 1);
    if (ret < 0)
    {
        printf("I2C write end_cmd  error!"); 
        recv = false;
    } 
    
end:

    if(recv==true)  return 0;
	else            return -1;
}                               

static int _tpParseRawPxy_vendor(struct ts_sample *s, unsigned char *buf)
{
	int i=0;
	char device_mode = 0;
	char numOfTchPnt = 0;
	unsigned int lfnum=gLastNumFinger;
	char loopNum = 0;
	
	if(buf[0]==0)   return (-1);
	
	numOfTchPnt = buf[0]&0xF;

	if( gTpSpec.tpSampleNum == 1)    numOfTchPnt = 1;

	loopNum = numOfTchPnt;
	
	if(lfnum < numOfTchPnt)
	{
		//printf("fgr_chg++: %d, %d\n", lfnum, numOfTchPnt);
	}
	
	if(loopNum)
	{		
		struct ts_sample *smp = (struct ts_sample*)s;		

		for(i = 0; i < loopNum; i++)
		{
			smp->pressure = 1;
				
		    if(i)   smp->id = (unsigned int)(buf[8*i]&0xF); //touch id
		    else    smp->id = (unsigned int)0;

			smp->finger = loopNum;

		    smp->x = (int)((((unsigned int)buf[2+8*i]<<8)&0x0F00) | ((unsigned int)buf[1+8*i]));
		    smp->y = (int)((((unsigned int)buf[4+8*i]<<8)&0x0F00) | ((unsigned int)buf[3+8*i]));        	    

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

			//printf("	RAW->[%d][%x, %d, %d]--> %d %d %d\n", i, smp, smp->id, smp->finger, smp->pressure, smp->x, smp->y);
			if(gTpSpec.tpSampleNum > 1)   smp++;
		}
		
		pthread_mutex_lock(&gTpMutex);
		gLastNumFinger = s->finger;
		pthread_mutex_unlock(&gTpMutex);	
			
		return 0;
	}
	
	return (-1);
}

static void _tpParseKey_vendor(struct ts_sample *s, unsigned char *buf)
{
    //TODO: get key information and input to xy sample...? as a special xy?
    //maybe define a special area for key
    //(like touch is 800x480, for example, y>500 for key, x=0~100 for keyA, x=100~200 for keyB... )
    //SDL layer could parse this special defination xy into key event(but this layer is not ready yet).
    
    //example::
    //pthread_mutex_lock(&gTpMutex);	
    //gTpKeypadValue = buf[5]&0x0F;
    //pthread_mutex_unlock(&gTpMutex);	
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
static int gt911_read(struct tslib_module_info *inf, struct ts_sample *samp, int nr)
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

static const struct tslib_ops gt911_ops =
{
	gt911_read,
};

TSAPI struct tslib_module_info *gt911_mod_init(struct tsdev *dev, const char *params)
{
	struct tslib_module_info *m;

	m = malloc(sizeof(struct tslib_module_info));
	if (m == NULL)
		return NULL;

	m->ops = &gt911_ops;
	return m;
}

#ifndef TSLIB_STATIC_CASTOR3_MODULE
	TSLIB_MODULE_INIT(gt911_mod_init);
#endif
