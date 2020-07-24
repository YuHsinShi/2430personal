#ifndef	API_RAW_H
#define API_RAW_H

/****************************************************************************
 * initial Kconfig setting
 ****************************************************************************/

#if	defined(CFG_TOUCH_I2C0) || defined(CFG_TOUCH_I2C1) || defined(CFG_TOUCH_I2C2) || defined(CFG_TOUCH_I2C3)
#define RA_INTERFACE_I2C   (0)
#endif

#if	defined(CFG_TOUCH_SPI) || defined(CFG_TOUCH_SPI0) || defined(CFG_TOUCH_SPI1)
#define RA_INTERFACE_SPI   (1)
#endif

#define RA_INT_PIN	    CFG_GPIO_TOUCH_INT
#define RA_GPIO_MASK    (1<<(RA_INT_PIN%32))

#ifdef	CFG_GPIO_TOUCH_WAKE
#if (CFG_GPIO_TOUCH_WAKE<128)
#define RA_GPIO_WAKE_PIN	CFG_GPIO_TOUCH_WAKE
#endif 
#endif 

#ifdef	CFG_GPIO_TOUCH_RESET
#if (CFG_GPIO_TOUCH_RESET<128)
#define RA_GPIO_RESET_PIN	CFG_GPIO_TOUCH_RESET
#else
#define RA_GPIO_RESET_PIN	(-1)
#endif 
#endif 

#ifdef	CFG_TOUCH_ADVANCE_CONFIG

#ifdef	CFG_TOUCH_SWAP_XY
#define	RA_SWAP_XY		(1)
#else
#define	RA_SWAP_XY		(0)
#endif

#ifdef	CFG_TOUCH_REVERSE_X
#define	RA_REVERSE_X	(1)
#else
#define	RA_REVERSE_X	(0)
#endif

#ifdef	CFG_TOUCH_REVERSE_Y
#define	RA_REVERSE_Y	(1)
#else
#define	RA_REVERSE_Y	(0)
#endif

#else

#define	RA_SWAP_XY		(0)
#define	RA_REVERSE_X	(0)
#define	RA_REVERSE_Y	(0)

#endif

#define	TOUCH_NO_CONTACT		(0)
#define	TOUCH_DOWN				(1)
#define	TOUCH_UP				(2)

#define	RA_TOUCH_NO_CONTACT		(0)
#define	RA_TOUCH_DOWN			(1)
#define	RA_TOUCH_UP				(2)

#define	RA_ACTIVE_LOW           (0)
#define	RA_ACTIVE_HIGH          (1)

#define	RA_INT_LEVLE_TRIGGER    (1)
#define	RA_INT_EDGE_TRIGGER     (0)

#define	RA_INT_TYPE_ZT2083                  (0)
#define	RA_INT_TYPE_KEEP_STATE              (0)
#define	RA_INT_TYPE_FT5XXX                  (1)
#define	RA_INT_TYPE_ACTIVE_FOR_A_WHILE      (1)
#define	RA_INT_TYPE_IT7260                  (2)
#define	RA_INT_TYPE_RELEASE_AFTER_READ      (2)

#define	RA_WITHOUT_KEY          (0)
#define	RA_HAS_TOUCH_KEY        (1)
#define	RA_GPIO_PIN_NO_DEF      (-1)

#ifdef CFG_TOUCH_INTR
#define	RA_ENABLE_INTERRUPT     (1)
#else
#define	RA_ENABLE_INTERRUPT     (0)
#endif

#ifdef	CFG_TOUCH_BUTTON
#define	RA_TOUCH_BUTTON		RA_HAS_TOUCH_KEY
#else
#define	RA_TOUCH_BUTTON		RA_WITHOUT_KEY
#endif

#ifdef	CFG_TOUCH_MULTI_FINGER
    #define RA_MULTI_FINGER_ENABLE
#endif

#ifdef __OPENRTOS__
#define RA_USE_XQUEUE
#endif

#ifdef RA_USE_XQUEUE
#define	RA_QUEUE_LEN	(32)
#endif
/****************************************************************************
 * touch cofig setting
 ****************************************************************************/
#define RA_IDLE_TIME                (2000)
#define RA_IDLE_TIME_NO_INITIAL     (100000)


/****************************************************************************
 * MACRO define
 ****************************************************************************/


/****************************************
 *
 ***************************************/
typedef struct ra_info_tag
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
}RA_GV;//tp_gv_tag

typedef struct ra_func_tag
{
	void (*raInitSpec)(void);
	int (*raParseRawPxy)(struct ts_sample *s, unsigned char *buf);
	int (*raReadPointBuffer)(unsigned char *buf, int cnt);
	void (*raParseKey)(struct ts_sample *s, unsigned char *buf);
	int (*raDoInitProgram)(void);
	void (*raDoPowerOnSeq)(void);
	void (*raIntActiveRule)(struct ts_sample *tpSmp);
	void (*raIntNotActiveRule)(struct ts_sample *tpSmp);
}RA_FUNC;

typedef struct ra_spec_tag
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
    char tpMaxFinferNum;	//0: meaningless, 1: single touch, >=2: multi-touch(multi-fingers), 
    char tpIntActiveMaxIdleTime;
    
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
    char tpSampleNum;		//0:NO scense, 1: single touch 2~10:multi-touch
    char tpSampleRate;		//UNIT: mill-second, range 8~16 ms(60~120 samples/per second)  
    
    //TP idle time
    int  tpIdleTime;		//sleep time for polling INT signal(even if interrupt mode).    
    int  tpIdleTimeB4Init;	//sleep time if TP not initial yet.       
    int  tpReadChipRegCnt;	//read register count for getting touch xy coordination
    int  tpMoveDetectUnit;	//the minimun of movement
    
    //TP specific function
    char tpHasPowerOnSeq;	//0:NO power-on sequence, 1:TP has power-on sequence
    char tpNeedProgB4Init;	//0:TP IC works well without programe flow, 1:TP IC need program before operation.
    char tpNeedAutoTouchUp;
    char tpIntPullEnable;	//use internal pull up/down function
    
    //TP raw api function pointer
    char *pTouchDownIntr;
    char *pTpInitialized;
	uint32_t *pTpKeypadValue;	
    struct ts_sample *gTpSmpBase;
    RA_GV         *raInfoBase;
    pthread_mutex_t 	*raMutex;
    RA_FUNC	rawApi;
} RA_TP_SPEC;//put it in api-raw.h

/******************************************************************************
 * the read flow for reading the FT5316's register by using iic repead start
 ******************************************************************************/

/*##################################################################################
 *                        the private function implementation
 ###################################################################################*/
//RA_FUNC	rawApi;
/*##################################################################################
 *                        the public function protocol
 ###################################################################################*/
extern int _raDoInitial(void);
extern void _raSetSpecBase(RA_TP_SPEC *spec);
extern int _raProbeSample(struct ts_sample *samp, int nr);

#else
#endif	//#ifndef	API_RAW_H

