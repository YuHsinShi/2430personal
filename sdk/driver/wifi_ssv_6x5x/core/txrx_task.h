/*
*  (C) Copyright 2014-2016 Shenzhen South Silicon Valley microelectronics co.,limited
*
*  All Rights Reserved
*/


#ifndef _TXRX_TASK_H
#define _TXRX_TASK_H

#define INT_RX              0x00000001  //1<<0
#define INT_TX              0x00000002  //1<<1
#define INT_SOC             0x00000004  //1<<2
#define INT_LOW_EDCA_0      0x00000008  //1<<3
#define INT_LOW_EDCA_1      0x00000010  //1<<4
#define INT_LOW_EDCA_2      0x00000020  //1<<5
#define INT_LOW_EDCA_3      0x00000040  //1<<6
#define INT_RESOURCE_LOW    0x00000080  //1<<7

ssv_type_s32 TXRXTask_Init(void);
ssv_type_s32 TXRXTask_FrameEnqueue(void* frame, ssv_type_u32 priority);
void TXRXTask_Isr(ssv_type_u32 signo,ssv_type_bool isfromIsr);

void TXRXTask_ShowSt(void);
void TXRXTask_TxLock (ssv_type_bool lock);

#endif //_TXRX_TASK_H
