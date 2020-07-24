#define SSV6006_BECISION_TBL_C
#include <host_config.h>
#if((CONFIG_CHIP_ID==SSV6006B)||(CONFIG_CHIP_ID==SSV6006C))
#include <ssv_hal.h>
#include <ssv_dev.h>
#include <ssv_drv.h>
#include <msgevt.h>
#include <rtos.h>
#include "regs/ssv6006_hw_regs.h"
#include "ssv6006_decision_tbl.h"

//action
#define ACTION_DO_NOTHING   0   //00
#define ACTION_UPDATE_NAV   1   //01
#define ACTION_RESET_NAV    2   //10
#define ACTION_SIGNAL_ACK   3   //11

//frame
#define FRAME_ACCEPT        0
#define FRAME_DROP          1

#define SET_DEC_TBL( _type, _mask, _action, _drop)      \
                (_type<<9|          \
                _mask <<3|          \
                _action<<1|         \
                _drop)

#define _RF_MODE_SHUTDOWN_  0
#define _RF_MODE_STANDBY_   1
#define _RF_MODE_TRX_EN_    2

//#ifndef USE_GENERIC_DECI_TBL
/**
* SSV6200 MAC Decision Table for STA mode. The first and
* second tables are aggregated into a single table here.
*/
ssv_type_u16 sta_deci_tbl[] =
{
//---------------------------------------------------------------------------------------------------------------------------
/* The first decision table content: */
//---------------------------------------------------------------------------------------------------------------------------
    //-0
    SET_DEC_TBL(0x1e, 0x3e, ACTION_RESET_NAV, FRAME_DROP),     //CF-End (1110) +   CF-End + CF-Ack (1111) --> drop
    SET_DEC_TBL(0x18, 0x3e, ACTION_SIGNAL_ACK, FRAME_ACCEPT),  //BlockAckReq (1000) + BlockAck (1001)
    SET_DEC_TBL(0x1a, 0x3f, ACTION_SIGNAL_ACK, FRAME_DROP),    //PS-Poll (1010) --> drop
    SET_DEC_TBL(0x10, 0x38, ACTION_DO_NOTHING, FRAME_DROP),    //Control Wrapper (0111) --> drop
    //-4
    SET_DEC_TBL(0x25, 0x3f, ACTION_DO_NOTHING, FRAME_DROP),    //
    SET_DEC_TBL(0x26, 0x36, ACTION_DO_NOTHING, FRAME_DROP),    //
    SET_DEC_TBL(0x08, 0x3f, ACTION_DO_NOTHING, FRAME_ACCEPT),  //Beacon
    SET_DEC_TBL(0x05, 0x3f, ACTION_SIGNAL_ACK, FRAME_ACCEPT),  //Probe Response
    //-8
    SET_DEC_TBL(0x0b, 0x3f, ACTION_SIGNAL_ACK, FRAME_ACCEPT),  //Auth
    SET_DEC_TBL(0x01, 0x3d, ACTION_SIGNAL_ACK, FRAME_ACCEPT),  //Assoc/ReAssoc
    SET_DEC_TBL(0x04, 0x3F, ACTION_DO_NOTHING, FRAME_DROP),    //Probe Request
    SET_DEC_TBL(0x00, 0x00, ACTION_SIGNAL_ACK, FRAME_ACCEPT),  //
    //-12
    SET_DEC_TBL(0x00, 0x00, ACTION_DO_NOTHING, FRAME_DROP),
    SET_DEC_TBL(0x00, 0x00, ACTION_UPDATE_NAV, FRAME_DROP),     //CTS for self
    SET_DEC_TBL(0x00, 0x00, ACTION_RESET_NAV, FRAME_DROP),      //
    SET_DEC_TBL(0x00, 0x00, ACTION_SIGNAL_ACK, FRAME_DROP),     //??? RTS (1011) + CTS (1100 ) + ACK (1101) --> drop

//---------------------------------------------------------------------------------------------------------------------------
/* The second decision table contents: decision mask */
//---------------------------------------------------------------------------------------------------------------------------
    0x2008,
    0x1001,
//    0x8408, //0x0808,  <-- for dhcp offer data ( DA --> broadcast )
//    0x1000,//when user request to scan, it should set to 0x1040.
#if 0
    0x8408, //0x0808,  <-- for dhcp offer data ( DA --> broadcast )
    0x1000, //when user request to scan, it should set to 0x1040.
#else
    0x0C08, //0x0808,  <-- for dhcp offer data ( DA --> broadcast )
    0x1040, //when user request to scan, it should set to 0x1040.
#endif

    0x2008,
    0x800E,
    0x0BB8,//unicat data/mgmt to me in the same bssid or wildcardbssid ( Auth/Assoc/ )
    0x2B88,//unicat data/mgmt to me in different bssid ( Probe Response/Auth )
    0x0800,
};


/**
* SSV6200 MAC Decision Table for AP mode:
*/
ssv_type_u16 ap_deci_tbl[] =
{
//---------------------------------------------------------------------------------------------------------------------------
/* The first decision table content: */
//---------------------------------------------------------------------------------------------------------------------------
    //-0
    SET_DEC_TBL(0x1e, 0x3e, ACTION_RESET_NAV, FRAME_DROP),      //CF-End (1110) +   CF-End + CF-Ack (1111) --> drop                    //Reset NAV         //Drop
    SET_DEC_TBL(0x18, 0x3e, ACTION_SIGNAL_ACK, FRAME_ACCEPT),   //BlockAckReq (1000) + BlockAck (1001)                             //Signal(Ack)           //Accept
    SET_DEC_TBL(0x1a, 0x3f, ACTION_SIGNAL_ACK, FRAME_ACCEPT),   //PS-Poll (1010) --> accept                                      //Do nothing            //Accept
    SET_DEC_TBL(0x10, 0x38, ACTION_DO_NOTHING, FRAME_DROP),     //Control Wrapper (0111) --> drop                                   //Do nothing            //Drop
    //-4
    SET_DEC_TBL(0x25, 0x3f, ACTION_DO_NOTHING, FRAME_DROP),     //([Data]CF-ACK)                                               //Do nothing            //Drop
    SET_DEC_TBL(0x26, 0x36, ACTION_DO_NOTHING, FRAME_DROP),     //([Data]all CF-Poll (no data)    )                                 //Do nothing            //Drop
    SET_DEC_TBL(0x08, 0x3f, ACTION_DO_NOTHING, FRAME_ACCEPT),   //Beacon            ---->drop                                       //Do nothing            //Drop
    SET_DEC_TBL(0x20, 0x30, ACTION_DO_NOTHING, FRAME_DROP),     //Drop all mc/broadcast data frames.(AP don't need to receive bc/multi)     //Do nothing            //Drop
    //-8
    SET_DEC_TBL(0x00, 0x00, ACTION_DO_NOTHING, FRAME_ACCEPT),   //Empty                                                      //Do nothing            //Accept
    SET_DEC_TBL(0x00, 0x00, ACTION_DO_NOTHING, FRAME_ACCEPT),   //Empty                                                      //Do nothing            //Accept
    SET_DEC_TBL(0x20, 0x30, ACTION_SIGNAL_ACK, FRAME_ACCEPT),   //Data                                                       //Signal(Ack)           //Accept
    SET_DEC_TBL(0x00, 0x00, ACTION_SIGNAL_ACK, FRAME_ACCEPT),   //All frames                                                    //Signal(Ack)           //Accept
    //-12
    SET_DEC_TBL(0x00, 0x00, ACTION_DO_NOTHING, FRAME_DROP),     //All frames                                                    //Do nothing            //Drop
    SET_DEC_TBL(0x00, 0x00, ACTION_UPDATE_NAV, FRAME_DROP),     //CTS for self                                                 //Update NAV            //Drop
    SET_DEC_TBL(0x00, 0x00, ACTION_RESET_NAV, FRAME_DROP),      //All frames                                                    //Reset NAV         //Drop
    SET_DEC_TBL(0x00, 0x00, ACTION_SIGNAL_ACK, FRAME_DROP),     //??? RTS (1011) + CTS (1100 ) + ACK (1101) --> drop                   //Signal(Ack)           //Drop


//---------------------------------------------------------------------------------------------------------------------------
/* The second decision table contents: decision mask */
//---------------------------------------------------------------------------------------------------------------------------

    0x2008, //ToDS, FromDS error with AP mode/station mode
    0x1001, //Control frames with BC/MC RA. CF-End, CF-End+CF-Ack
    0x0888, //Mcast or Bcast data/mgmt frames with same or wildcard BSSID.           0x0808,  <-- for dhcp offer data ( DA --> broadcast )
    0x1040, //Mcast data/mgmt frames with different BSSID.                                 when user request to scan, it should set to 0x1040.

    0x2008, //Unicast frames not to me
    0x800E, //Unicast Control Frames to me
    //0x0838, //Unicat data/mgmt to me in the same bssid or wildcardbssid

    0x0800,
    0x2008, //0x2808, //Unicat data/mgmt to me in different bssid
    0x0800, //WDS
};

ssv_type_u16 null_deci_tbl[] =
{
//---------------------------------------------------------------------------------------------------------------------------
/* The first decision table content: */
//---------------------------------------------------------------------------------------------------------------------------
    //-0
    SET_DEC_TBL(0, 0, ACTION_DO_NOTHING, FRAME_DROP),      //CF-End (1110) +   CF-End + CF-Ack (1111) --> drop                    //Reset NAV         //Drop
    SET_DEC_TBL(0, 0, ACTION_DO_NOTHING, FRAME_ACCEPT),   //BlockAckReq (1000) + BlockAck (1001)                             //Signal(Ack)           //Accept
    SET_DEC_TBL(0, 0, ACTION_DO_NOTHING, FRAME_ACCEPT),   //PS-Poll (1010) --> accept                                      //Do nothing            //Accept
    SET_DEC_TBL(0, 0, ACTION_DO_NOTHING, FRAME_DROP),     //Control Wrapper (0111) --> drop                                   //Do nothing            //Drop
    //-4
    SET_DEC_TBL(0, 0, ACTION_DO_NOTHING, FRAME_DROP),     //([Data]CF-ACK)                                               //Do nothing            //Drop
    SET_DEC_TBL(0, 0, ACTION_DO_NOTHING, FRAME_DROP),     //([Data]all CF-Poll (no data)    )                                 //Do nothing            //Drop
    SET_DEC_TBL(0, 0, ACTION_DO_NOTHING, FRAME_ACCEPT),   //Beacon            ---->drop                                       //Do nothing            //Drop
    SET_DEC_TBL(0, 0, ACTION_DO_NOTHING, FRAME_DROP),     //Drop all mc/broadcast data frames.(AP don't need to receive bc/multi)     //Do nothing            //Drop
    //-8
    SET_DEC_TBL(0, 0, ACTION_DO_NOTHING, FRAME_ACCEPT),   //Empty                                                      //Do nothing            //Accept
    SET_DEC_TBL(0, 0, ACTION_DO_NOTHING, FRAME_ACCEPT),   //Empty                                                      //Do nothing            //Accept
    SET_DEC_TBL(0, 0, ACTION_DO_NOTHING, FRAME_ACCEPT),   //Data                                                       //Signal(Ack)           //Accept
    SET_DEC_TBL(0, 0, ACTION_DO_NOTHING, FRAME_ACCEPT),   //All frames                                                    //Signal(Ack)           //Accept
    //-12
    SET_DEC_TBL(0, 0, ACTION_DO_NOTHING, FRAME_DROP),     //All frames                                                    //Do nothing            //Drop
    SET_DEC_TBL(0, 0, ACTION_DO_NOTHING, FRAME_DROP),     //CTS for self                                                 //Update NAV            //Drop
    SET_DEC_TBL(0, 0, ACTION_DO_NOTHING, FRAME_DROP),      //All frames                                                    //Reset NAV         //Drop
    SET_DEC_TBL(0, 0, ACTION_DO_NOTHING, FRAME_DROP),     //??? RTS (1011) + CTS (1100 ) + ACK (1101) --> drop                   //Signal(Ack)           //Drop


//---------------------------------------------------------------------------------------------------------------------------
/* The second decision table contents: decision mask */
//---------------------------------------------------------------------------------------------------------------------------

    0x0001, //ToDS, FromDS error with AP mode/station mode
    0x0001, //Control frames with BC/MC RA. CF-End, CF-End+CF-Ack
    0x0001, //Mcast or Bcast data/mgmt frames with same or wildcard BSSID.           0x0808,  <-- for dhcp offer data ( DA --> broadcast )
    0x0001, //Mcast data/mgmt frames with different BSSID.                                 when user request to scan, it should set to 0x1040.

    0x0001, //Unicast frames not to me
    0x0001, //Unicast Control Frames to me
    //0x0838, //Unicat data/mgmt to me in the same bssid or wildcardbssid

    0x0001,
    0x0001, //0x2808, //Unicat data/mgmt to me in different bssid
    0x0001, //WDS
};

//#else // USE_GENERIC_DECI_TBL
ssv_type_u16 generic_deci_tbl[] =
{
//---------------------------------------------------------------------------------------------------------------------------
/* The first decision table content: */
//---------------------------------------------------------------------------------------------------------------------------
    /* 0 */ SET_DEC_TBL(0x1e, 0x3e, ACTION_RESET_NAV,  FRAME_DROP),     //0x3DF5, /* 0 011110 111110 10 1 */    //CF-End (1110) + CF-End + CF-Ack (1111) --> drop               //Reset NAV
    /* 1 */ SET_DEC_TBL(0x18, 0x3e, ACTION_SIGNAL_ACK, FRAME_ACCEPT),   //0x31F6, /* 0 011000 111110 11 0 */    //BlockAckReq (1000) + BlockAck (1001)                                  //Signal(Ack)
    /* 2 */ SET_DEC_TBL(0x1a, 0x3f, ACTION_DO_NOTHING, FRAME_ACCEPT),   //0x35F8, /* 0 011010 111111 00 0 */    //PS-Poll (1010) --> accept                                                         //Do nothing
    /* 3 */ SET_DEC_TBL(0x10, 0x38, ACTION_DO_NOTHING, FRAME_DROP),     //0x21C1, /* 0 010000 111000 00 1 */    //Control Wrapper (0111) --> drop

    /* 4 */ SET_DEC_TBL(0x25, 0x3f, ACTION_DO_NOTHING, FRAME_DROP),    //
    /* 5 */ SET_DEC_TBL(0x26, 0x36, ACTION_DO_NOTHING, FRAME_DROP),    //
    /* 6 */ SET_DEC_TBL(0x08, 0x3f, ACTION_DO_NOTHING, FRAME_ACCEPT),  //Beacon
    /* 7 */ SET_DEC_TBL(0x05, 0x3f, ACTION_SIGNAL_ACK, FRAME_ACCEPT),  //Probe Response

    /* 8 */ SET_DEC_TBL(0x00, 0x00, ACTION_DO_NOTHING, FRAME_ACCEPT),   //Empty                                                      //Do nothing            //Accept
    /* 9 */ SET_DEC_TBL(0x00, 0x00, ACTION_DO_NOTHING, FRAME_ACCEPT),   //Empty                                                      //Do nothing            //Accept
    /* a */ SET_DEC_TBL(0x20, 0x30, ACTION_SIGNAL_ACK, FRAME_ACCEPT),   //Data                                                       //Signal(Ack)           //Accept
    /* b */ SET_DEC_TBL(0x00, 0x00, ACTION_SIGNAL_ACK, FRAME_ACCEPT),   //All frames                                                    //Signal(Ack)           //Accept

    /* c */ SET_DEC_TBL(0x00, 0x00, ACTION_DO_NOTHING, FRAME_DROP),     //0x0001, /* 0 000000 000000 00 1 */    //All frames
    /* d */ SET_DEC_TBL(0x00, 0x00, ACTION_UPDATE_NAV, FRAME_DROP),     //0x0003, /* 0 000000 000000 01 1 */    //CTS for self
    /* e */ SET_DEC_TBL(0x00, 0x00, ACTION_RESET_NAV,  FRAME_DROP),     //0x0005, /* 0 000000 000000 10 1 */    //All frames
    /* f */ SET_DEC_TBL(0x00, 0x00, ACTION_SIGNAL_ACK, FRAME_DROP),     //0x0007, /* 0 000000 000000 11 1 */    //??? RTS (1011) + CTS (1100 ) + ACK (1101) --> drop    //Signal(Ack)           //Drop

    //---------------------------------------------------------------------------------------------------------------------------
    /* The second decision table contents: decision mask */
    //---------------------------------------------------------------------------------------------------------------------------
    //             fedc ba98 7654 3210
    //           -----------------------
    0x2008, // 0 - 0010 0000 0000 1000 - ToDS, FromDS error with AP mode/station mode
    0x1001, // 1 - 0001 0000 0000 0001 - Control frames with BC/MC RA. CF-End, CF-End+CF-Ack
    0x0808, // 2 - 0000 1000 0000 0000 - Mcast or Bcast data/mgmt frames with same or wildcard BSSID.
    0x1040, // 3 - 0000 1000 0000 0000 - Mcast or Bcast data/mgmt frames with different BSSID. To support dual interface, accept as BSSID matched.
    0x2000, // 4 - 0010 0000 0000 0000 - Unicast frames not to me
    0x800E, // 5 - 1000 0000 0000 1110 - Unicast Control Frames to me
    0x0800, // 6 - 0000 1000 0000 0000 - Unicast frames to me
    0x0800, // 7 - 0000 1000 0000 0000 - Unicat data/mgmt to me in different bssid. To support dual interface, accept as BSSID matched.
    0x0800, // 8 - 0000 1000 0000 0000 - WDS
    0x0800, // 9 - 0000 1000 0000 0000 - Mcast or Bcast data/mgmt frames with same or wildcard BSSID of vif 1
    0x0808, // 10 - 0000 1000 0000 0000 - Unicast frames to me of vif 1
};
//#endif // USE_GENERIC_DECI_TBL


typedef enum{
    EN_STA_BCN_RECEIVE_ALL,
    EN_STA_BCN_RECEIVE_SPECIFIC, //Only receive the bcns from the specific ap that is connected with STA
    EN_STA_BCN_RECEIVE_NONE,
    EN_STA_BCN_RECEIVE_INVALID,
    EN_STA_BCN_RECEIVE_MAX=EN_STA_BCN_RECEIVE_INVALID
}EN_STA_BCN_RECEIVE;


static int _ssv6006_hal_set_bcn_received(EN_STA_BCN_RECEIVE enVal)
{
    ssv_type_u32 val=0;


#define DROP_BCN_IN_TB6()           \
        do{                         \
            MAC_REG_READ(ADR_MRX_FLT_TB6,val);\
            val&=0x00007fff;\
            val|=0x01;\
            MAC_REG_WRITE(ADR_MRX_FLT_TB6,val);\
        }while(0)

#define ACCEPT_BCN_IN_TB6()           \
        do{                         \
            MAC_REG_READ(ADR_MRX_FLT_TB6,val);\
            val&=0x00007fff;\
            val&=~0x01;              \
            MAC_REG_WRITE(ADR_MRX_FLT_TB6,val);\
        }while(0)

//There is a rx decsision flow in HW, HW arrange the all rx frames to 8 groups by this decision flow
//Becasons are always hitting the two of 8 groups. We called resulst 3 and result 2 below....
//Result3: Mcast data and mgmt frame wifth different BSSID
//Result2: Mcast or Bcast data/mgmt frame with same or wildcard BSSID

/*Setup the 6th field in rx decision table for accept beacon*/
/*The frames that came from result 2 must run the case 6*/
#define RECEIVE_BCN_FROM_RESULT2()    \
            do{                     \
                ACCEPT_BCN_IN_TB6();    \
                MAC_REG_READ(ADR_MRX_FLT_EN2,val);\
                val&=0x0000ffff; \
                val|=0x40;            \
                MAC_REG_WRITE(ADR_MRX_FLT_EN2,val);\
            }while(0)

/*Setup the 6th field in rx decision table for drop beacon*/
/*The frames that came from result 2 must run the case 6*/
#define BLOCK_BCN_FROM_RESULT2()  \
            do{                 \
                DROP_BCN_IN_TB6();    \
                MAC_REG_READ(ADR_MRX_FLT_EN2,val);\
                val&=0x0000ffff; \
                val|=0x40;            \
                MAC_REG_WRITE(ADR_MRX_FLT_EN2,val);\
            }while(0)

/*Setup the 6th field in rx decision table for accept beacon*/
/*The frames that came from result 3 must run the case 6*/
#define RECEIVE_BCN_FROM_RESULT3() \
            do{                         \
                ACCEPT_BCN_IN_TB6();    \
                MAC_REG_READ(ADR_MRX_FLT_EN3,val);\
                val&=0x0000ffff; \
                val|=0x40;              \
                MAC_REG_WRITE(ADR_MRX_FLT_EN3,val);\
            }while(0)

/*The frames that came from result 3 don't run the case 6*/
/*Drop the beacons that came from result3 by case 12*/
#define BLOCK_BCN_FROM_RESULT3()  \
            do{                 \
                MAC_REG_READ(ADR_MRX_FLT_EN3,val);\
                val&=0x0000ffff; \
                val&=~0x40;            \
                val|=0x1000;            \
                MAC_REG_WRITE(ADR_MRX_FLT_EN3,val);\
            }while(0)


    //OS_TASK_ENTER_CRITICAL();

    #if 0 //Ian.Wu: need to modify
    //SET_RG_MODE(_RF_MODE_STANDBY_);
    MAC_REG_READ(ADR_3_WIRE_REGISTER,val);
    val&=0xffffcfff;
    val|=(_RF_MODE_STANDBY_<< 8);
    MAC_REG_WRITE(ADR_3_WIRE_REGISTER,val);
    #endif
    switch(enVal)
    {
        case EN_STA_BCN_RECEIVE_ALL:
            RECEIVE_BCN_FROM_RESULT3();
            RECEIVE_BCN_FROM_RESULT2();
            break;
        case EN_STA_BCN_RECEIVE_SPECIFIC:
            BLOCK_BCN_FROM_RESULT3();
            RECEIVE_BCN_FROM_RESULT2();
            break;
        case EN_STA_BCN_RECEIVE_NONE:
            BLOCK_BCN_FROM_RESULT3();
            BLOCK_BCN_FROM_RESULT2();
            break;
        default:
            break;
    }
    #if 0 //Ian.Wu: need to modify
    //SET_RG_MODE(_RF_MODE_TRX_EN_);
    MAC_REG_READ(ADR_3_WIRE_REGISTER,val);
    val&=0xffffcfff;
    val|=(_RF_MODE_TRX_EN_<< 8);
    MAC_REG_WRITE(ADR_3_WIRE_REGISTER,val);
    #endif
    //OS_TASK_EXIT_CRITICAL();

    return 0;

}

int ssv6006_hal_sta_rcv_all_bcn(void)
{
    return _ssv6006_hal_set_bcn_received(EN_STA_BCN_RECEIVE_ALL);
}

int ssv6006_hal_sta_rcv_specific_bcn(void)
{
    return _ssv6006_hal_set_bcn_received(EN_STA_BCN_RECEIVE_SPECIFIC);
}

int ssv6006_hal_sta_reject_bcn(void)
{
    return _ssv6006_hal_set_bcn_received(EN_STA_BCN_RECEIVE_NONE);
}

int ssv6006_hal_accept_none_wsid_frame(void)
{
    ssv_type_u32 _regval;

    //For group frame
    MAC_REG_READ(ADR_MRX_FLT_EN3, _regval);
    _regval=_regval & 0x0000ffff;
    _regval|=0x0800;
    MAC_REG_WRITE(ADR_MRX_FLT_EN3,_regval);

    //For unicast frame
    MAC_REG_READ(ADR_MRX_FLT_TB12, _regval);
    _regval=_regval & 0x0000ffff;
    _regval&=~0x0001;
    MAC_REG_WRITE(ADR_MRX_FLT_TB12, _regval);

    MAC_REG_READ(ADR_MRX_FLT_EN0, _regval);
    _regval=_regval & 0x0000ffff;
    _regval|=0x1000;
    MAC_REG_WRITE(ADR_MRX_FLT_EN0, _regval);

    //It's in order to get the ampdu packets from smart phone
    MAC_REG_READ(ADR_GLBLE_SET, _regval);
    _regval=_regval&AMPDU_SNIFFER_I_MSK;
    MAC_REG_WRITE(ADR_GLBLE_SET, _regval);
    return 0;
}
int ssv6006_hal_drop_none_wsid_frame(void)
{
    ssv_type_u32 _regval;

    //For group frame
    MAC_REG_READ(ADR_MRX_FLT_EN3, _regval);
    _regval=_regval & 0x0000ffff;
    _regval&=~0x0800;
    MAC_REG_WRITE(ADR_MRX_FLT_EN3,_regval);


    //For unicast frame
    MAC_REG_READ(ADR_MRX_FLT_TB12, _regval);
    _regval=_regval & 0x0000ffff;
    _regval|=0x0001;
    MAC_REG_WRITE(ADR_MRX_FLT_TB12, _regval);

    MAC_REG_READ(ADR_MRX_FLT_EN0, _regval);
    _regval=_regval & 0x0000ffff;
    _regval&=~0x1000;
    MAC_REG_WRITE(ADR_MRX_FLT_EN0,_regval);

    //Prevent from RX hang because of receiving AMPDU
    MAC_REG_READ(ADR_GLBLE_SET, _regval);
    _regval=_regval|AMPDU_SNIFFER_MSK;
    MAC_REG_WRITE(ADR_GLBLE_SET, _regval);

    return 0;
}

int ssv6006_hal_ap_listen_neighborhood(ssv_type_bool en)
{
//Accept Beacon
//#define FRAME_ACCEPT 0
#define ACCEPT_BEACON_IN_AP_MODE() \
    MAC_REG_READ(ADR_MRX_FLT_TB6, _regval); \
    _regFltTb6=_regval; \
    _regval&=~0x01; \
    _regval|=0; \
    MAC_REG_WRITE(ADR_MRX_FLT_TB6, _regval);

//Accept unicat data/mgmt to me in different bssid
#define ACCEPT_UNICAST_DATA_MGMT_FRAME_FROM_DIFFERENT_BSSID() \
    MAC_REG_READ(ADR_MRX_FLT_EN7, _regval); \
    _regFltEn7=_regval; \
    _regval |= 0x800; \
    MAC_REG_WRITE(ADR_MRX_FLT_EN7, _regval);

//Drop Beacon
//Restore ADR_MRX_FLT_EN7
#define DROP_BEACON_IN_AP_MODE() \
    MAC_REG_WRITE(ADR_MRX_FLT_EN7, _regFltEn7);

//Restore ADR_MRX_FLT_TB6
#define DROP_UNICAST_DATA_MGMT_FRAME_FROM_DIFFERENT_BSSID() \
    MAC_REG_WRITE(ADR_MRX_FLT_TB6, _regFltTb6);

    ssv_type_u32 _regval=0;
    ssv_type_u32 _regFltTb6=0,_regFltEn7=0;
    if(TRUE==en){
        ACCEPT_BEACON_IN_AP_MODE();
        ACCEPT_UNICAST_DATA_MGMT_FRAME_FROM_DIFFERENT_BSSID();
    }else{
        DROP_BEACON_IN_AP_MODE();
        DROP_UNICAST_DATA_MGMT_FRAME_FROM_DIFFERENT_BSSID();
    }
    return 0;
}

#endif //#if((CONFIG_CHIP_ID==SSV6006B)||(CONFIG_CHIP_ID==SSV6006C))
