#ifndef __CAN_HW_H__
#define __CAN_HW_H__

#include <assert.h>
#include "ite/itp.h"
#include "can_reg.h"


#ifdef __cplusplus
extern "C" {
#endif

#define CHECK_STB_STATUS(base) while(ithCANGetTBStatus(base) == BUFF_FULL){usleep(100);}
#define CAN0_BASE_ADDRESS 0XDC100000
#define CAN1_BASE_ADDRESS 0XDC200000
#define IDFILTER_BUFFER   2
#define NON_FD            1 //if define this ,only support can 2.0b
//=============================================================================
//                Structure Definition
//=============================================================================

//enum
typedef enum{
    Error_Enable = 1,
    TS_Enable = 2,
    TP_Enable = 3,
    RB_Almost_Full_Enable = 4,
    RB_Full_Enable = 5,
    RB_Overrun_Enable = 6,
    Receive_Enable = 7,
    Bus_Error_Enable = 17,
    Arbitration_Lost_Enable = 19,
    Error_Passive_Enable = 21,
    //Time Trigged Mode 
    Time_Trigger_Enable = 28,
    Watch_Trigger_Enable = 31,
}CAN_INTERRUPTS_EN;

typedef enum{
    Abort_Interrupt_Flag = 8, // from TPA and TSA 
    Error_Interrupt_Flag = 9,
    TS_Interrupt_Flag = 10,
    TP_Interrupt_Flag = 11,
    RB_Almost_Full_Interrupt_Flag = 12,
    RB_Full_Interrupt_Flag = 13,
    RB_Overrun_Interrupt_Flag = 14,
    Receive_Interrupt_Flag = 15,
    Bus_Error_Interrupt_Flag = 16,
    Arbitration_Lost_Interrupt_Flag = 18,
    Error_Passive_Interrupt_Flag = 20,
    //Time Trigged Mode 
    Time_Trigger_Interrupt_Flag = 27,
    Trigger_Error_Interrupt_Flag = 29,
    Watch_Trigger_Interrupt_Flag = 30,
}CAN_INTERRUPTS_FLAG;

typedef enum{
    BUFF_EMPTY = 0,
    BUFF_LESS_HALF = 1,
    BUFF_MORE_HALF = 2,
    BUFF_FULL = 3,
}CAN_BUFF_STATUS;

typedef enum{
    CAN_NO_ERROR = 0,
    CAN_BIT_ERROR = 1,
    CAN_FORM_ERROR = 2,
    CAN_STUFF_ERROR = 3,
    CAN_ACKNOWLEDGEMENT_ERROR = 4,
    CAN_CRC_ERROR = 5,
    CAN_OTHER_ERROR = 6,
}CAN_ERROR_TYPE;

typedef enum{
    TTCAN_Immediate_Trigger = 0,
    TTCAN_Time_Trigger = 1,
    TTCAN_OneShot_Trans_Trigger = 2,
    TTCAN_Transmit_Start_Trigger = 3,
    TTCAN_Transmit_Stop_Trigger = 4,
    TTCAN_No_Action = 5,
}CAN_TRIGGER_TYPE;

typedef enum{
    #ifdef NON_FD
    protocol_CAN_2_0B = 0,
    #else
    protocol_FD_BOSCH = 0, //Bosch CAN FD (non-ISO) mode
    protocol_FD_ISO = 1      //ISO CAN FD mode (ISO 11898-1:2015) 
    #endif
}CAN_PROTOCOL;

typedef enum {
    CAN_SRCCLK_160M,
    CAN_SRCCLK_40M,
    CAN_SRCCLK_20M,
} CAN_SOURCECLK;


typedef enum {
	CAN_500K_1M, // 0x00
	CAN_500K_2M, // 0x01
	CAN_500K_3M,
	CAN_500K_4M,
	CAN_500K_5M,
	//CAN_500K_6M7,
	//CAN_500K_8M
	//CAN_500K_10M,
	CAN_250K_500K,
	CAN_250K_833K,
	CAN_250K_1M,
	CAN_250K_1M5,
	CAN_250K_2M,
	CAN_250K_3M,
	CAN_250K_4M,
	CAN_1000K_4M,
	//CAN_1000K_8M,
	CAN_125K_500K,
} CAN_BAUDRATE;

typedef enum {
    CAN_ACFE0 = 0,
    CAN_ACFE1,
    CAN_ACFE2,
    CAN_ACFE3,
    CAN_ACFE4,
    CAN_ACFE5,
    CAN_ACFE6,
    CAN_ACFE7,
    CAN_ACFE8,
    CAN_ACFE9,
    CAN_ACFE10,
    CAN_ACFE11,
    CAN_ACFE12,
    CAN_ACFE13,
    CAN_ACFE14,
    CAN_ACFE15,
} CAN_ACFE;

typedef enum {
    CAN_ACODE0 = 0,
    CAN_ACODE1,
    CAN_ACODE2,
    CAN_ACODE3,
    CAN_ACODE4,
    CAN_ACODE5,
    CAN_ACODE6,
    CAN_ACODE7,
    CAN_ACODE8,
    CAN_ACODE9,
    CAN_ACODE10,
    CAN_ACODE11,
    CAN_ACODE12,
    CAN_ACODE13,
    CAN_ACODE14,
    CAN_ACODE15,
} CAN_ACODE;

typedef enum {
    CAN_AMASK0 = 0,
    CAN_AMASK1,
    CAN_AMASK2,
    CAN_AMASK3,
    CAN_AMASK4,
    CAN_AMASK5,
    CAN_AMASK6,
    CAN_AMASK7,
    CAN_AMASK8,
    CAN_AMASK9,
    CAN_AMASK10,
    CAN_AMASK11,
    CAN_AMASK12,
    CAN_AMASK13,
    CAN_AMASK14,
    CAN_AMASK15,
} CAN_AMASK;

typedef struct{
    uint32_t PSP;
    uint32_t SSP;
    uint32_t Prescaler;
    uint32_t Bit_Time;
    uint32_t Seg_1;
    uint32_t Seg_2;
    uint32_t SJW;
    uint32_t SSPOFF;
}CAN_BTATTR;//Bit Time Attributes

typedef struct{
    bool      FilterEnable;
    uint32_t  ACode;
    uint32_t  AMask;
    bool      AIDEE;
    bool      AIDE;
}CAN_FILTEROBJ;

typedef struct{
    uint32_t       Instance;                 //device instance
    uint32_t       ADDR;                     //hardware baseaddress  
	CAN_BAUDRATE   BaudRate;                 //bus baud rate
    CAN_SOURCECLK  SourceClock;              //can clk
    CAN_PROTOCOL   ProtocolType;             //can protocol
    /* debug mode */
    bool           ExternalLoopBackMode;     //Loop Back Mode, External(self-tests used)
    bool           InternalLoopBackMode;     //Loop Back Mode, Internal(self-tests used)
    bool           ListenOnlyMode;           //Listen Only Mode
    CAN_FILTEROBJ* TPtr;
}CAN_HANDLE;

//=============================================================================
//                Public Function Definition
//=============================================================================
void ithCANEnableIntr(CAN_HANDLE* base, CAN_INTERRUPTS_EN intr);
void ithCANDisableIntr(CAN_HANDLE* base, CAN_INTERRUPTS_EN intr);
void ithCANClearIntrFlag(CAN_HANDLE* base, CAN_INTERRUPTS_FLAG flag);
uint32_t ithCANGetIntrFlag(CAN_HANDLE* base);
uint32_t ithTTCANGetIntrFlag(CAN_HANDLE* base) ;
void ithCANSWReset(CAN_HANDLE* base, bool OnOff);
void ithCANSetLoopBack(CAN_HANDLE* base, bool external, bool internal);
void ithCANSetListenOnlyMode(CAN_HANDLE* base, bool onoff);
void ithCANSetTransStandbyMode(CAN_HANDLE* base, bool onoff);
void ithCANSetTransPTBSSMode(CAN_HANDLE* base, bool onoff);
void ithCANSetTransSTBSSMode(CAN_HANDLE* base, bool onoff);
void ithCANSetFDISO(CAN_HANDLE* base);
void ithCANSetBitRate(CAN_HANDLE* base, CAN_BTATTR s_bt, CAN_BTATTR f_bt);
void ithCANSetCIA603(CAN_HANDLE* base, bool Onoff, bool position);
void ithCANSetACODE(CAN_HANDLE* base, CAN_ACODE c_index, uint32_t value);
void ithCANSetAMASK(CAN_HANDLE* base, CAN_AMASK m_index, uint32_t value , bool AIDEE, bool AIDE);
void ithCANACFEnable(CAN_HANDLE* base, CAN_ACFE index, bool OnOff);
char ithCANGetRBStatus(CAN_HANDLE* base);
char ithCANGetTBStatus(CAN_HANDLE* base);
void ithCANPrimarySendCtrl(CAN_HANDLE* base, bool OnOff);
void ithCANSecondarySendCtrl(CAN_HANDLE* base, bool OnOff);
uint32_t ithCANGetReceiveErrorCouNT(CAN_HANDLE* base);
uint32_t ithCANGetTransmitErrorCouNT(CAN_HANDLE* base);
uint32_t ithCANGetKindOfError(CAN_HANDLE* base);
uint32_t ithCANGetALC(CAN_HANDLE* base);
uint32_t ithCANGetTransTimeStamp(CAN_HANDLE* base);
void ithTTCANCrtlON(CAN_HANDLE* base, uint16_t PRESCaler, uint16_t Trigger_Time);
void ithTTCANCrtlOFF(CAN_HANDLE* base);
void ithTTCANSetREFID(CAN_HANDLE* base, uint32_t REFID, bool IDformat);
void ithTTCANSetTrig(CAN_HANDLE* base, CAN_TRIGGER_TYPE trigtype);
uint32_t ithTTCANGetTrig(CAN_HANDLE* base);
void ithTTCANSetTrigTime(CAN_HANDLE* base, uint16_t trigtime);
void ithTTCANSetWatchTrigTime(CAN_HANDLE* base, uint16_t watchtrigtime);
void ithCANSetTimeStampDiv(CAN_HANDLE* base, uint16_t value);


#ifdef __cplusplus
}
#endif

#endif //CAN_HW_H

