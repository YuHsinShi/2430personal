/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * Castor3 keypad module.
 *
 * @author Joseph Chang
 * @version 1.0
 */
#include "../itp_cfg.h"
#include "iic/mmp_iic.h"
#include <errno.h>
#include <pthread.h>

#include "openrtos/FreeRTOS.h"		//must "include FreeRTOS.h" before "include queue.h"
#include "openrtos/queue.h"			//for using xQueue function

/**************************************************************************
** MACRO defination                                                      **
***************************************************************************/
#ifndef	CFG_TOUCH_KEY_NUM
#define CFG_TOUCH_KEY_NUM	(16)
#endif

#if (CFG_TOUCH_KEY_NUM==5)
#define IT7236_IIC_ADDR		(0x6A>>1)
#else
#define IT7236_IIC_ADDR		(0x8C>>1)
#endif

//#define ENABLE_KEYPAD_DBG_MODE
#define ENABLE_KEYPAD_INT
//#define ENABLE_SHARE_GPIOEXTENDER_INTR
//#define ENABLE_CUTOFF_ALL_INTR
#define ENABLE_CLEAR_TP_INT
#define ENABLE_SKIP_I2C_SLAVE_NACK

#ifdef	ENABLE_KEYPAD_INT
#define TK_GPIO_PIN	    CFG_GPIO_KEYPAD
#endif

#define QUEUE_LEN 256
#define EN_NEW_I2C_CTRL

#ifdef  CFG_TOUCH_KEYPAD_I2C0
    #define TK_IIC_PORT    ":i2c0"
#endif
#ifdef  CFG_TOUCH_KEYPAD_I2C1
    #define TK_IIC_PORT    ":i2c1"
#endif
#ifdef  CFG_TOUCH_KEYPAD_I2C2
    #define TK_IIC_PORT    ":i2c2"
#endif
#ifdef  CFG_TOUCH_KEYPAD_I2C3
    #define TK_IIC_PORT    ":i2c3"
#endif
/**************************************************************************
** global variable                                                      **
***************************************************************************/
//IT7235BR touch 16-key pad mapping table
//  x   80   40   02   01
//y	 ----------------------
//04 | [1]  [2]  [3]  [A] |
//08 | [4]  [5]  [6]  [B] |
//10 | [7]  [8]  [9]  [C] |
//20 | [*]  [0]  [#] [Cen]|
//	 ----------------------
//kay:: 	[0]  [1]  [2]  [3]  [4]  [5]  [6]  [7]  [8]  [9]  [A]  [B]  [C]  [Cen] [*]  [#]
//code::   	0x60,0x84,0x44,0x02,0x88,0x48,0x0a,0x90,0x50,0x12,0x05,0x09,0x11,0x21,0xa0,0x22
//SDL code: 13  ,0,  ,1   ,2   ,4   ,5   ,6   ,8   ,9   ,10  ,3   ,7   ,11  ,15   ,12, ,14 
//x:0x80,0x40,0x02,0x01
//y:0x04,0x08,0x10,0x20

typedef struct it7236_fw_tag { /* Used in the IBM Arctic II */
	uint8_t fwCR;
	uint8_t fwStatus;
	uint8_t fwDevName[6];
	uint8_t fwVer[4];
	uint8_t fwOtpVer[2];
	uint16_t fwSramSize;
	uint16_t fwMtpSize;
	uint16_t fwCodeSize;
	uint8_t fwDummy[3];
}FW_Info;

static	pthread_mutex_t     keypad_mutex = PTHREAD_MUTEX_INITIALIZER;

static	const uint8_t gTotalTouchKeyNum = (uint8_t)(CFG_TOUCH_KEY_NUM);

static	uint8_t		gRegPage=0xFE;
static	uint8_t		gLastIicStatus=0;

#ifdef	ENABLE_SHARE_GPIOEXTENDER_INTR
static QueueHandle_t tkQueue;
static uint8_t		gLastKey = 0;
#endif

static	uint8_t		gTocuhDown = 0;
static	uint8_t		gLastIndex = 0xFF;
static	uint8_t		g_1stTch = 1;
static	FW_Info		gFwInfo;
static	const uint8_t		gFwVerCycle = 23;

static	uint8_t		g_kpI2cPort = IIC_PORT_0;
static	int		    gKpFd = -1;

static	uint8_t		gKpInitFinished = false;
static	uint8_t		gKeyPadIsExist = 1;
/**************************************************************************
** private function                                                      **
***************************************************************************/
static uint32_t _i2c_read(uint8_t *b1, uint32_t len1, uint8_t *b2, uint32_t len2)
{
#ifdef EN_NEW_I2C_CTRL
    ITPI2cInfo evt;
    uint8_t buf[1] = {*b1};
    uint32_t i2cRes = 0;
    
    evt.slaveAddress   = IT7236_IIC_ADDR;       //device slave address
    evt.cmdBuffer      = &buf[0];                 //send slave device data buffer(command)
    evt.cmdBufferSize  = len1;                    //data size,UNIT:byte
    evt.dataBuffer     = b2;                //slave data buffer
    evt.dataBufferSize = len2;                    // data size,UNIT:byte
    
    i2cRes = read(gKpFd, &evt, 1);
    
    return i2cRes;
#else
    uint8_t buf[1] = {*b1};
    return mmpIicReceiveData(g_kpI2cPort, IIC_MASTER_MODE, IT7236_IIC_ADDR, &buf[0], len1, b2, len2);
#endif
}

static uint32_t _i2c_write(uint8_t cmd, uint8_t *buf, uint32_t cnt)
{
#ifdef EN_NEW_I2C_CTRL
    ITPI2cInfo evt;
    uint32_t i2cRes = 0;
    uint8_t cBuf[4] = {0};
    
    if(cnt > 2)
    {
        //it's only for this module
        printf("incorrect value:: cnt(%d) > 2\n",cnt);
    }
    else
    {
        cBuf[0] = cmd;
        cBuf[1] = buf[0];
        if(cnt==2) cBuf[2] = buf[1];
    
        evt.slaveAddress   = IT7236_IIC_ADDR;       //device slave address
        evt.cmdBuffer      = &cBuf[0];                 //send slave device data buffer(command)
        evt.cmdBufferSize  = cnt + 1;                    //data size,UNIT:byte

        i2cRes = write(gKpFd, &evt, 1);
    }
    return i2cRes;
#else
    return mmpIicSendData(g_kpI2cPort, IIC_MASTER_MODE, IT7236_IIC_ADDR, cmd, buf, cnt);
#endif
}

static uint8_t _checkSum(uint16_t sum)
{
	uint8_t	i;
	uint8_t	cnt=0;
	
	for(i=0; i<gTotalTouchKeyNum; i++)
	{
		if( (sum>>i)&0x01 )	cnt++;
	}
	return cnt;
}

#ifdef	ENABLE_KEYPAD_INT
static void _initTkGpioPin(void)
{
	ithGpioSetMode(TK_GPIO_PIN, ITH_GPIO_MODE0);
	ithGpioSetIn(TK_GPIO_PIN);
	ithGpioCtrlEnable(TK_GPIO_PIN, ITH_GPIO_PULL_ENABLE);
	ithGpioCtrlEnable(TK_GPIO_PIN, ITH_GPIO_PULL_UP);
	ithGpioEnable(TK_GPIO_PIN);	     
}
#endif

#ifdef	ENABLE_SKIP_I2C_SLAVE_NACK
static uint8_t _checkKeyPadExist(void)
{
	unsigned int result = 1;
	uint32_t	 reg32, errCnt=0;
	uint8_t      pg=0;
	
	if(!gKeyPadIsExist)	return false;
	
	while(result)
	{
		result = _i2c_write(0xF0, &pg, 1); 

		if(result)    printf("[KEYPAD] error: _checkKeyPadExist fail, result = %x.\n",result);

        if(errCnt++ > 5)
        {
        	//printf("~~~ KEY PAD IS NOT EXIST ~~~\n");
        	gKeyPadIsExist = 0;
        	return false;
        }
    }
    
    gRegPage = 0;
    
    return true;
}
#endif

static void _checkPage(uint8_t page)
{
	unsigned int result = 0;
	uint8_t		buf[2];
	uint32_t	reg32, cnt=0;
	
	if(gRegPage!=page)
	{
		result = _i2c_write(0xF0, &page, 1); 
		if(result)
	    {
	        printf("[KEYPAD] error: _checkPage fail, result = %x.\n",result);
		    return;
        }

		#ifdef	ENABLE_KEYPAD_DBG_MODE
		printf("write page=0x%X to regAddr 0x%X\n", page, 0xF0);
		#endif
		
		gRegPage = page;
	}
}

static uint16_t _getTouchKey(void)
{
	uint8_t		page = 0x60;
	uint16_t	KeyValue;
	unsigned int result = 0;
	uint8_t		buf[2];
    uint32_t	regData1,regData2;
    uint32_t	reg32, cnt=0;

	_checkPage(page);
	
	buf[0] = 0xFE; 
	buf[1] = 0xFE; 

	result = _i2c_read(buf, 1, buf, 2);
    if(result)
    {
        printf("[KEYPAD] error: _getTouchKey fail, result = %x.\n",result);
        return 0;
    }
		
    #ifdef	ENABLE_CLEAR_TP_INT
    {
    	uint8_t		cbuf[2];
    	
    	cbuf[0]=0x00;
    	
    	result = _i2c_write(0xF3, cbuf, 1);

    }
    #endif
    
    #ifdef	ENABLE_KEYPAD_DBG_MODE
    //if( buf[0] || buf[1])
		//printf("buf=[0x%X,0x%X], R=%x\n", buf[0], buf[1], result);
	#endif

	KeyValue = (uint16_t)buf[1] & 0x00FF;

	return KeyValue;
}

static void _getFirmwareVersion(FW_Info* fwInfo)
{
	uint8_t		page = 0x00;
	uint16_t	KeyValue;
	unsigned int result = 0;
	uint8_t		buf[24];
	uint8_t		cbuf[4];
	uint8_t     regAddr;  
    uint32_t	regData1,regData2;
    uint32_t	reg32, cnt=0;
    uint32_t	FwVer=0;
    
    printf("get FW version:fwInfo=%x, size=%d:\n",fwInfo, gFwVerCycle);

	_checkPage(page);
	
	//write 0xF1 0x80
	cbuf[0]=0x80;
	result = _i2c_write(0xF1, cbuf, 1);
	if(result)	printf("Err1\n");
	usleep(1000);
	
	//read 0xFA bit 0
	buf[0]=0;
	while( !(buf[0]&0x1))
	{
		regAddr = 0xFA;
		result = _i2c_read(&regAddr, 1, buf, 1);
	}
	if(result)	printf("Err2\n");
	usleep(1000);
	
	//write 0x40 0x01 0x01
	cbuf[0]=0x01;	cbuf[1]=0x01;
	result = _i2c_write(0x40, cbuf, 2);
	if(result)	printf("Err3\n");	
	usleep(1000);
	
	//write 0xF1 0x40
	cbuf[0]=0x40;
	result = _i2c_write(0xF1, cbuf, 1);
	if(result)	printf("Err4\n");
	usleep(1000);
	
	
	//wait INTr pull-down	
    //printf("Before INTr check\n");
	//while(ithGpioGet(TK_GPIO_PIN));
	printf("after INTr check\n");
	
	//write 0xF1 0x80
	cbuf[0]=0x80;
	result = _i2c_write(0xF1, cbuf, 1);
	if(result)	printf("Err5\n");
	usleep(1000);
	
	//read 0xFA bit 0
	buf[0]=0;
	while( !(buf[0]&0x1))
	{
		regAddr = 0xFA;
		result = _i2c_read(&regAddr, 1, buf, 1);
	}
	if(result)	printf("Err6\n");
	usleep(1000);
	
	//read 0x40 16BYTES
	regAddr = 0x40;
	result = _i2c_read(&regAddr, 1, buf, gFwVerCycle);
	if(result)	printf("Err7\n");
	else		memcpy((uint8_t*)fwInfo, (uint8_t*)buf, gFwVerCycle );
	usleep(1000);
	
	//write 0xF3 bit6 as 0
	cbuf[0]=0x00;
	result = _i2c_write(0xF3, cbuf, 1);
	if(result)	printf("Err8\n");
	usleep(1000);
	
	//write 0xF1 bit6 as 1
	cbuf[0]=0x40;
	result = _i2c_write(0xF1, cbuf, 1);
	if(result)	printf("Err9\n");
	usleep(1000);
}

static uint8_t _tanslateTouchValue(uint16_t value, uint8_t totalKeyNum)
{
	uint8_t ChkSum = _checkSum(value);
	uint8_t	i=0;
	uint16_t	flag;
	
	#ifdef	ENABLE_KEYPAD_DBG_MODE
	printf("parseTchKey(%x,%x), chkSum=%x\n",value,totalKeyNum,ChkSum);
	#endif
	
	if(ChkSum==0)	return -1;
	
	switch(totalKeyNum)
	{
	case 5:
		if(ChkSum>1)	return -1;
		while(1)
		{
			flag = 0x01<<i;
			if(value & flag)	return i;//index_table[i];
			if(i++>totalKeyNum)	return -1;
		}
	case 16:
		{
			uint8_t	BtnX,BtnY;
			
		    if(ChkSum==1)	return -1;
		    
		    if(ChkSum>2)
		    {
		    	printf("[KEYPAD]warning:: multi-key, skip it(value=%x, chk=%d)\n",value, ChkSum);
		    	return -1;
		    }
		    switch(value&0xC3)
		    {
		    	case 0x80:	BtnX = 0;	break;
		    	case 0x40:	BtnX = 1;	break;
		    	case 0x02:	BtnX = 2;	break;
		    	case 0x01:  BtnX = 3;	break;
		    	default:
		    		printf("[KEYPAD]warning:: incorrect X, skip it(value=%x, chk=%d)\n",value, value&0xC3);
		    		return -1;
		    }

		    switch(value&0x3C)
		    {
		    	case 0x04:	BtnY = 0;	break;
		    	case 0x08:	BtnY = 1;	break;
		    	case 0x10:	BtnY = 2;	break;
		    	case 0x20:  BtnY = 3;	break;
		    	default:
		    		printf("[KEYPAD]warning:: incorrect Y, skip it(value=%x, chk=%d)\n",value, value&0x3C);
		    		return -1;
		    }
		    return (BtnY*4 + BtnX);	
	    }
	default:
		printf("touch keypad error: totalKeyNum incorrect, keyNum=%d\n",totalKeyNum);
		break;
	}
	return (uint8_t)-1;
}

/**************************************************************************
** public function(keypad API)                                           **
***************************************************************************/
#ifdef	ENABLE_SHARE_GPIOEXTENDER_INTR
void tkGpioExtHandler()
{
	uint8_t	tempKey=0;
	uint8_t	value;
	
	//1.i2c read key status
	#ifdef	ENABLE_CUTOFF_ALL_INTR
	portSAVEDISABLE_INTERRUPTS();
	#else
    pthread_mutex_lock(&keypad_mutex);
    #endif
    
    if(g_1stTch)
    {
    	_getFirmwareVersion(&gFwInfo);
    	printf("The %d-key FW version of IT7236:(%02x.%02x.%02x.%02x)\n", gTotalTouchKeyNum, gFwInfo.fwVer[0], gFwInfo.fwVer[1], gFwInfo.fwVer[2], gFwInfo.fwVer[3] );
    	g_1stTch = 0;
    }
    
    value = _getTouchKey();
    
    #ifdef	ENABLE_CUTOFF_ALL_INTR
    portRESTORE_INTERRUPTS();
    #else    
    pthread_mutex_unlock(&keypad_mutex);
    #endif
    
	//send xqueue    
    tempKey = value&0xFF;  
    xQueueSend(tkQueue, &tempKey, 0);

}
#endif

int itpKeypadProbe(void)
{
    uint8_t  index = 0;
    uint16_t value=0;
    uint32_t	kpIntr = 0;
    
    #ifdef ENABLE_SKIP_I2C_SLAVE_NACK
    if(gKpInitFinished!=true)	return (int)-1;
    #endif
    
    #ifdef	ENABLE_KEYPAD_INT
    kpIntr = ithGpioGet(TK_GPIO_PIN);
    #endif
    
    #ifdef	ENABLE_KEYPAD_DBG_MODE
    printf("_kpPrb:%x\n",kpIntr);
    #endif
    
    if(g_1stTch)
    {
    	_getFirmwareVersion(&gFwInfo);
    	printf("The %d-key FW version of IT7236:(%02x.%02x.%02x.%02x)\n", gTotalTouchKeyNum, gFwInfo.fwVer[0], gFwInfo.fwVer[1], gFwInfo.fwVer[2], gFwInfo.fwVer[3] );
    	g_1stTch = 0;
    }
    
    if(!kpIntr)
    {
    	value = _getTouchKey();

    	#ifdef	ENABLE_KEYPAD_DBG_MODE
    	printf("getTK, v=%x\n",value);
    	#endif
    	
    	if(!value)
    	{
    		gTocuhDown = 0;
    		gLastIndex = 0xFF;
    		index = 0xFF;
    	}
    	else
    	{
    	    index = _tanslateTouchValue(value, gTotalTouchKeyNum);
            if(gTocuhDown && gLastIndex==index)
            {
            	#ifdef	ENABLE_KEYPAD_DBG_MODE
            	printf("[KEYPAD] key repead, ignore this event(%x,%x)(%x,%x)\n",kpIntr,gTocuhDown,index,gLastIndex);
            	#endif
            	return index;
            }
    	    gTocuhDown = 1;
    	    gLastIndex = index;
    	}
    }
    else
    {
        //keep last key event for long-press function(even if INT is not active)
        index = gLastIndex;
    }
    
    #ifdef	ENABLE_KEYPAD_DBG_MODE
    printf("itp_kp=%x\n\n",index);
    #endif

    if(gTocuhDown)	return index;
    else            return -1;
}

void itpKeypadInit(void)
{
	//TODO::
	//skip i2c init flow
	#ifdef	CFG_TOUCH_KEYPAD_I2C1
	g_kpI2cPort = IIC_PORT_1;
	#endif

	#ifdef EN_NEW_I2C_CTRL
	gKpFd = open(TK_IIC_PORT, 0); 
	if(gKpFd != -1)     printf("open keypad file device OK, gKpFd = %d!!\n",gKpFd);
	else                printf("open keypad fd FAIL, gKpFd = %d!!\n",gKpFd);
	#endif

	#ifdef	ENABLE_KEYPAD_INT
	_initTkGpioPin();
	#endif
	
	#ifdef	ENABLE_SKIP_I2C_SLAVE_NACK
	if(_checkKeyPadExist()==false)
	{
	    printf("[KeyPad] ERROR: KEYPAD CANNOT be recognized\n");
	    return;
	}
	#endif
	
	#ifdef	ENABLE_SHARE_GPIOEXTENDER_INTR
	tkQueue = xQueueCreate(QUEUE_LEN, (unsigned portBASE_TYPE) sizeof(unsigned char));
	#endif
	
	#ifdef	ENABLE_SHARE_GPIOEXTENDER_INTR
	itpRegisterGpioExtenderIntrHandler(1, tkGpioExtHandler);
	#endif
	
	#ifdef	ENABLE_SKIP_I2C_SLAVE_NACK
	gKpInitFinished = true;	
	#endif
}

int itpKeypadGetMaxLevel(void)
{
    return gTotalTouchKeyNum;
}
