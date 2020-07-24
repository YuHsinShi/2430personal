#ifndef __CAN_API_H__
#define __CAN_API_H__

#include "ite/itp.h"
#include "can_hw.h"

#ifdef __cplusplus
extern "C" {
#endif

#define Min(a , b) ( (a) < (b)? a : b)
#define Alignment4BYTE(a) (a >> 2) + ((a % 4) ? 1: 0)
#define _1M              1000000u
#define NPRESCALER_MIN   1
#define NPRESCALER_MAX   255
#define TIME_QUANTA_MIN  9
#define TIME_QUANTA_MAX  40
#define SLOWBTMAX        255              
#define FASTBTMAX        31
//TBPTR pointer slot (0 - 63)
#define TBSLOT_BUFFER_MAX 63
#ifndef NON_FD
#define DLC_MAX           64
#else
#define DLC_MAX           8
#endif

//=============================================================================
//                Structure Definition
//=============================================================================

typedef enum {
    CAN_DLC_0,
    CAN_DLC_1,
    CAN_DLC_2,
    CAN_DLC_3,
    CAN_DLC_4,
    CAN_DLC_5,
    CAN_DLC_6,
    CAN_DLC_7,
    CAN_DLC_8,
#ifndef NON_FD
    CAN_DLC_12,
    CAN_DLC_16,
    CAN_DLC_20,
    CAN_DLC_24,
    CAN_DLC_32,
    CAN_DLC_48,
    CAN_DLC_64
#endif
} CAN_DLC;

typedef struct{
    uint8_t   TX;
    uint32_t  KOER;
}CAN_RBUF_STATUS;

typedef struct{
    uint32_t  DLC; //Data len code
    uint8_t   BRS; //Bit Rate Switch 0 - nominal / slow bit rate for the complete frame ,1 - switch to data / fast bit rate for the data payload and the CRC (only FD use)
    uint8_t   EDL; //0 - CAN 2.0B or 1 - FD
    uint8_t   RTR; //0 - data frame , 1 - remote frame  Remote Transmission Request(only CAN 2.0B use)
    uint8_t   IDE; //0 - STD , 1 - EXT identifier format 
}CAN_BUF_CONTROL;

//Receive Buffer Registers RBUF
typedef struct{
    uint32_t          Identifier;//Id
    CAN_BUF_CONTROL   Control;
    CAN_RBUF_STATUS   Status;
    uint8_t           RXData[DLC_MAX];
    uint32_t          RXRTS[2];
    #ifndef NON_FD
    uint8_t           ESI;   //Error State Indicator, Only FD
    #endif
}CAN_RXOBJ;

//Transmit Buffer User Sel
typedef struct{
    uint32_t Identifier;//Id
    CAN_BUF_CONTROL Control;
    bool TBSEL;         //0 - PTB, 1 - STB choose tx buffer type
    bool SingleShot;    //0 - off,  1 - on single shot mode
    bool TTSENSEL;      //0 - disable , 1 - enable CiA 603 time stamping      
}CAN_TXOBJ;


//=============================================================================
//                Private Function Definition
//=============================================================================

//=============================================================================
//                Public Function Definition
//=============================================================================
void ithCANOpen(CAN_HANDLE* base, void* rx_callback, void* tx_callback);
void ithCANClose(CAN_HANDLE* base);
int ithCANRead(CAN_HANDLE* base, CAN_RXOBJ* info);
int  ithCANWrite(CAN_HANDLE* base,  CAN_TXOBJ* info, uint8_t* dataptr);
uint32_t ithCANDlcToBytes(CAN_DLC dlc);
void ithCANSetGPIO(uint32_t Instance, uint32_t rxpin, uint32_t txpin);
int ithCANGetTTS(CAN_HANDLE *base);




#ifdef __cplusplus
}
#endif

#endif //CAN_API_H

