/*
 * Copyright (c) 2018 ITE Tech. Inc. All Rights Reserved.
 *
 * @brief Can ctrl test code.
 * @author
 * @version 1.0.0
 *
 */
#ifdef WIN32

#else
#include <stdio.h>
#include <malloc.h>
#include "ite/itp.h"    //for all ith driver
#include "ite/ith.h"
#include "can_bus/it9860/can_api.h"

CAN_HANDLE      *can0;
CAN_HANDLE      *can1;
pthread_t       can2_task;
pthread_mutex_t rx_mutex = PTHREAD_MUTEX_INITIALIZER;        //for gCapinitNum mutex protect

CAN_FILTEROBJ FilterTable[16]= { {true,  0x0, 0x1FFFFFFF, false, false},//FILTER0
                                 {false, 0x0, 0x1FFFFFFF, false, false},//FILTER1
                                 {false, 0x0, 0x1FFFFFFF, false, false},//FILTER2
                                 {false, 0x0, 0x1FFFFFFF, false, false},//FILTER3
                                 {false, 0x0, 0x1FFFFFFF, false, false},//FILTER4
                                 {false, 0x0, 0x1FFFFFFF, false, false},//FILTER5
                                 {false, 0x0, 0x1FFFFFFF, false, false},//FILTER6
                                 {false, 0x0, 0x1FFFFFFF, false, false},//FILTER7
                                 {false, 0x0, 0x1FFFFFFF, false, false},//FILTER8
                                 {false, 0x0, 0x1FFFFFFF, false, false},//FILTER9
                                 {false, 0x0, 0x1FFFFFFF, false, false},//FILTER10
                                 {false, 0x0, 0x1FFFFFFF, false, false},//FILTER11 
                                 {false, 0x0, 0x1FFFFFFF, false, false},//FILTER12
                                 {false, 0x0, 0x1FFFFFFF, false, false},//FILTER13
                                 {false, 0x0, 0x1FFFFFFF, false, false},//FILTER14
                                 {false, 0x0, 0x1FFFFFFF, false, false},//FILTER15                                 
};

static void CANRXCallback(void *arg1, uint32_t arg2)
{
    pthread_mutex_unlock(&rx_mutex);
}

static void *_CAN2Thread()
{
    uint8_t  txbuffer2[8] = {0XFE,0X05,0X34,0X00,0X00};
    uint32_t i;

    //define can1
    can1                         = (CAN_HANDLE *)malloc(sizeof(CAN_HANDLE));
    can1->Instance               = 1;
    can1->ADDR                   = CAN1_BASE_ADDRESS;
    can1->BaudRate               = CAN_1000K_4M;//CAN_500K_1M;
    can1->SourceClock            = CAN_SRCCLK_40M;//CAN_SRCCLK_160M;
    can1->ProtocolType           = protocol_CAN_2_0B;
    can1->ExternalLoopBackMode   = false;
    can1->InternalLoopBackMode   = false;
    can1->ListenOnlyMode         = false;
    can1->TPtr                   = FilterTable;



    ithCANOpen(can1, (void *)NULL, (void *)NULL);
	unsigned char curr_speed=0;
	int tmp=0;
	unsigned char crc=0;
	int speed_up;
    while (1)
    {
        //tx information
        CAN_TXOBJ _txObj2;
        _txObj2.Identifier  = 0x0;
        _txObj2.Control.DLC = CAN_DLC_5;
        _txObj2.Control.RTR = 0x0;
        _txObj2.Control.IDE = 0x0;
        _txObj2.TBSEL       = 0x1;
        _txObj2.SingleShot  = 0x0;
        _txObj2.TTSENSEL    = 0x0;

		txbuffer2[3]= curr_speed;

		tmp =0 - (txbuffer2[0]+txbuffer2[1]+txbuffer2[2]+txbuffer2[3]);
		crc =(unsigned char)(tmp);

		//printf("tmp=0x%x\n",tmp);
		//printf("crc=0x%x\n",crc);

		txbuffer2[4]= crc;



		
        uint32_t r = ithCANWrite(can1, &_txObj2, txbuffer2);
        //check error count
        //printf("send error count = %d\n", ithCANGetTransmitErrorCouNT(can1));
        //printf("kind of error = %x\n", ithCANGetKindOfError(can1));
        usleep(50*1000);
		if(curr_speed==100)
		{
			speed_up=0;
		}
		else if(curr_speed==1)
		{	
			speed_up=1;
		}
		
		if(speed_up)
 			curr_speed++;
		else
			curr_speed--;	
    }
}

static void *_CAN1RECVThread()
{
	CAN_RXOBJ _rxObj;
    uint8_t  txbuffer[8] = {0x0,0x1,0x2,0x3,0x4,0x5,0x6,0x7};
    //define can0
    can0                         = (CAN_HANDLE *)malloc(sizeof(CAN_HANDLE));
    can0->Instance               = 0;
    can0->ADDR                   = CAN0_BASE_ADDRESS;
    can0->BaudRate               = CAN_1000K_4M;//CAN_500K_1M;
    can0->SourceClock            = CAN_SRCCLK_40M;//CAN_SRCCLK_160M;
    can0->ProtocolType           = protocol_CAN_2_0B;
    can0->ExternalLoopBackMode   = false;
    can0->InternalLoopBackMode   = false;
    can0->ListenOnlyMode         = false;
    can0->TPtr                   = FilterTable;
    //try open & init can ctrl
    ithCANOpen(can0, (void *)CANRXCallback, (void *)NULL);
		//main loop
		while (1)
		{
        #if 0 //Tx test.
			if(ithCANWrite(can0, &_txObj, txbuffer))
			{
				printf("write fail\n");
			}
			sleep(1);
        #else // Rx test
			pthread_mutex_lock(&rx_mutex); 
			
			if (ithCANRead(can0, &_rxObj) == 0)
			{
			/*
				printf("ID[%x]: IDE = %x,DLC = %x\n"
					   , _rxObj.Identifier, _rxObj.Control.IDE, _rxObj.Control.DLC);
				printf("data[0-5]=%x %x %x %x %x \n", _rxObj.RXData[0], _rxObj.RXData[1], _rxObj.RXData[2], _rxObj.RXData[3]
					   , _rxObj.RXData[4]);
					   */
				//printf("can0 error count = %d\n", ithCANGetReceiveErrorCouNT(can0));
				//printf("kind of error = %x\n", ithCANGetKindOfError(can0));

//				
			//UNLOCK				{0xFE,0x05,0x39,0x00,0xC4},
			//ENGINE START			{0xFE,0x05,0x39,0x02,0xC2},
			//WINKER LEFT			{0xFE,0x05,0x39,0x49,0x7B},
			//WINKER RIGHT			{0xFE,0x05,0x39,0x49,0x7A},
			//SPORT  MODE			{0xFE,0x05,0x39,0x09,0xBB},
			//SPEED VALUE			{0XFE,0X05,0X34,0X00,0X00};

						if(0x39 ==_rxObj.RXData[2])  )
						{
							if(0x00 ==_rxObj.RXData[3]) {								
									ui_set_unlock_mode();
							}
							else if (0x02 ==_rxObj.RXData[2]) { 
								
									ui_engine_start();
							}
							else if (0x49 ==_rxObj.RXData[2]) {
									if(0x7B ==_rxObj.RXData[3])
										ui_set_winker_left();						
									else
										ui_set_winker_right();
							}						
							else if (0x09 ==_rxObj.RXData[2]) { 
									ui_set_sport_mode_on();	
							}
							else{
								
							}
						}
						else if(0x34 ==_rxObj.RXData[2])  ){
							ui_set_meter_speed_value(_rxObj.RXData[3]);

						}
						else{

						}

			}
			
        #endif
		}

}

void Can_init(void *arg)
{


    printf("Can_init\n");

    //set GPIO by target board. <can0 rx pin 42 tx pin 43>
//    ithCANSetGPIO(0, 27, 28);
    ithCANSetGPIO(0, 17, 18);
    //<can1 rx pin 46 tx pin 45>
  //  ithCANSetGPIO(1, 29, 30);

//	ithGpioSetOut(31);
//	ithGpioSetMode(31, ITH_GPIO_MODE0);
//	ithGpioClear(31);



    


    pthread_create(&can2_task, NULL, _CAN1RECVThread, NULL);
   // pthread_create(&can2_task, NULL, _CAN2Thread, NULL);



    return NULL;
}
#endif
