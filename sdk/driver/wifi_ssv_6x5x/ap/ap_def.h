/*
*  (C) Copyright 2014-2016 Shenzhen South Silicon Valley microelectronics co.,limited
*
*  All Rights Reserved
*/


#ifndef _AP_DEF_H_
#define _AP_DEF_H_



#include <log.h>
#include "common/ap_common.h"




#define AP_WMM_ENABLED  (TRUE)


#define AP_TABLE_EXPIRATION_TIME (360000) //360 sec
#define AP_LIST_TIMER_INTERVAL   (10000)  //10 SEC



#define AP_TOTAL_MAX_TX_BUFFER 	(g_host_cfg.pool_size)

/* Maximum number of broadcast/multicast frames to buffer when some of the
 * associated stations are using power saving. */
#define AP_MAX_BC_BUFFER 	30//		128

/* Maximum number of frames to buffer per power saving station per AC */
#define AP_STA_MAX_TX_BUFFER	128


/* Maximum number of tx descriptor used in tx transmit*/
#define AP_TOTAL_MAX_TX_DSP	(AP_TOTAL_MAX_TX_BUFFER+10)



/* Minimum buffered frame expiry time. If STA uses listen interval that is
 * smaller than this value, the minimum value here is used instead. */
#define AP_STA_TX_BUFFER_EXPIRE 10000 //10 sec //(10 * HZ)




/* How often station data is cleaned up (e.g., expiration of buffered frames)
 */
#define AP_STA_INFO_CLEANUP_INTERVAL (10000)//10 sec


#define AP_MAX_BSS        1

#define AP_MAX_SSID_LEN   32
#define AP_MAX_CHANNEL	  14

//#define IEEE80211_MAX_AID			AP_MAX_STA//2007
//#define IEEE80211_MAX_TIM_LEN		(sizeof(unsigned long) * BITS_TO_LONGS(IEEE80211_MAX_AID + 1))//2//251
extern ssv_type_u32 gMaxAID;
extern ssv_type_u32 gMaxTimLen;


/* Maximum size of Supported Rates info element. IEEE 802.11 has a limit of 8,
 * but some pre-standard IEEE 802.11g products use longer elements. */
#define AP_SUPP_RATES_MAX 32






//# Maximum allowed Listen Interval (how many Beacon periods STAs are allowed to
//# remain asleep). Default: 65535 (no limit apart from field size)
#define AP_MAX_LISTEN_INTERVAL 65534

//-----------------------------------
//Beacon 
//#define AP_DEFAULT_BEACON_INT	(50)
#define AP_DEFAULT_DTIM_PERIOD  (1)



#define AP_NEIGHBOR_AP_TABLE_MAX_SIZE 8

#define AP_BEACON_TAIL_BUF_SIZE 256

//------------------------------------------

/* Default value for maximum station inactivity. After AP_MAX_INACTIVITY has
 * passed since last received frame from the station, a nullfunc data frame is
 * sent to the station. If this frame is not acknowledged and no other frames
 * have been received, the station will be disassociated after
 * AP_DISASSOC_DELAY seconds. Similarly, the station will be deauthenticated
 * after AP_DEAUTH_DELAY seconds has passed after disassociation. */
#define AP_MAX_INACTIVITY (g_host_cfg.ApStaInactiveTime*1000) //(15 * 1000) //msec
#define AP_DISASSOC_DELAY (1*1000)//msec
#define AP_DEAUTH_DELAY (1*1000) //msec
/* Number of seconds to keep STA entry with Authenticated flag after it has
 * been disassociated. */
#define AP_MAX_INACTIVITY_AFTER_DISASSOC (1 * 30)
/* Number of seconds to keep STA entry after it has been deauthenticated. */
#define AP_MAX_INACTIVITY_AFTER_DEAUTH (1 * 5)


//-------------------------------------
//Preallocated buffer size

#define AP_MGMT_PKT_LEN			(1024)//(2348) //Spec Figure 8-34
#define AP_MGMT_BEACON_LEN		(512)

#endif//_AP_DEF_H_
