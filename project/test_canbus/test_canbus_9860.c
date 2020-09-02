/*
 * Copyright (c) 2018 ITE Tech. Inc. All Rights Reserved.
 *
 * @brief Can ctrl test code.
 * @author
 * @version 1.0.0
 *
 */
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

CAN_FILTEROBJ FilterTable2[16]= {{true,  0x0, 0x1FFFFFFF, false, false},//FILTER0
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
    uint8_t  txbuffer2[8] = {0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0};
    CAN_RXOBJ _rxObj2;
    //define can1
    can1                         = (CAN_HANDLE *)malloc(sizeof(CAN_HANDLE));
    can1->Instance               = 1;
    can1->ADDR                   = CAN1_BASE_ADDRESS;
    can1->BaudRate               = CAN_500K_1M;
    can1->SourceClock            = CAN_SRCCLK_40M;
    can1->ProtocolType           = protocol_CAN_2_0B;
    can1->ExternalLoopBackMode   = false;
    can1->InternalLoopBackMode   = false;
    can1->ListenOnlyMode         = false;
    can1->TPtr                   = FilterTable2;

    ithCANOpen(can1, (void *)NULL, (void *)NULL);

    while (1)
    {
        //tx information
        CAN_TXOBJ _txObj2;
        _txObj2.Identifier  = 0x2;
        _txObj2.Control.DLC = CAN_DLC_8;
        _txObj2.Control.RTR = 0x0;
        _txObj2.Control.IDE = 0x0;
        _txObj2.TBSEL       = 0x0;
        _txObj2.SingleShot  = 0x0;
        _txObj2.TTSENSEL    = 0x0;

        if(ithCANWrite(can1, &_txObj2, txbuffer2))
        {
            printf("can1 write fail, bus busy\n");
        }
        //printf("send done, tts = %x\n",ithCANGetTTS(can1));
        //check error count
        //printf("send error count = %d\n", ithCANGetTransmitErrorCouNT(can1));
        //printf("kind of error = %x\n", ithCANGetKindOfError(can1));
        usleep(100*1000);

    }
}

void *TestFunc1(void *arg)
{
    CAN_RXOBJ _rxObj;
    CAN_TXOBJ _txObj;
    uint8_t  txbuffer[8] = {0x0,0x1,0x2,0x3,0x4,0x5,0x6,0x7};
    
    itpInit();
    printf("TestFunc1 test 9860 canbus!\n");

    //set GPIO by target board. 
    //<can0 rx pin 44 tx pin 43>
//    ithCANSetGPIO(0, 44, 43);
    ithCANSetGPIO(0, 17, 18);
    //<can1 rx pin 46 tx pin 45>
    //ithCANSetGPIO(1, 46, 45);

    //define can0
    can0                         = (CAN_HANDLE *)malloc(sizeof(CAN_HANDLE));

	printf("TestFunc1 \d");

	
    can0->Instance               = 0;
    can0->ADDR                   = CAN0_BASE_ADDRESS;
    can0->BaudRate               = CAN_1000K_4M;//CAN_500K_1M;
    can0->SourceClock            = CAN_SRCCLK_40M;//CAN_SRCCLK_160M;
    can0->ProtocolType           = protocol_CAN_2_0B;
    can0->ExternalLoopBackMode   = false;
    can0->InternalLoopBackMode   = false;
    can0->ListenOnlyMode         = false;
    can0->TPtr                   = FilterTable;

    _txObj.Identifier  = 0x1;
    _txObj.Control.DLC = CAN_DLC_8;
    _txObj.Control.RTR = 0x0;
    _txObj.Control.IDE = 0x0;
    _txObj.TBSEL       = 0x0;
    _txObj.SingleShot  = 0x0;
    _txObj.TTSENSEL    = 0x0;
    
    //try open & init can ctrl
    ithCANOpen(can0, (void *)CANRXCallback, (void *)NULL);

    //pthread_create(&can2_task, NULL, _CAN2Thread, NULL);

    //main loop
    while (1)
    {
        #if 0 //Tx test.
        if(ithCANWrite(can0, &_txObj, txbuffer))
        {
            printf("can0 write fail, bus busy\n");
        }
        usleep(100*1000);
        //printf("send done, tts = %x\n",ithCANGetTTS(can0));
        #else // Rx test
        pthread_mutex_lock(&rx_mutex); 
        
        if (ithCANRead(can0, &_rxObj) == 0)
        {
            printf("can0 ID[%x]: IDE = %x,DLC = %x, RTR = %x,RTS[0] = %x\n"
                   , _rxObj.Identifier, _rxObj.Control.IDE, _rxObj.Control.DLC, _rxObj.Control.RTR, _rxObj.RXRTS[0]);
            printf("data[0-7]=%x %x %x %x %x %x %x %x\n", _rxObj.RXData[0], _rxObj.RXData[1], _rxObj.RXData[2], _rxObj.RXData[3]
                   , _rxObj.RXData[4], _rxObj.RXData[5], _rxObj.RXData[6], _rxObj.RXData[7]);
            //printf("can0 error count = %d\n", ithCANGetReceiveErrorCouNT(can0));
            //printf("kind of error = %x\n", ithCANGetKindOfError(can0));
        }
        #endif
    }

    return NULL;
}