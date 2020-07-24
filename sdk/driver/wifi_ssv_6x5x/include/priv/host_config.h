/*
*  (C) Copyright 2014-2016 Shenzhen South Silicon Valley microelectronics co.,limited
*
*  All Rights Reserved
*/


#ifndef _SIM_CONFIG_H_
#define _SIM_CONFIG_H_

//#include <ssv_types.h>
#include <porting.h>
#include <msgevt.h>
// ------------------------- debug ---------------------------

#define CONFIG_STATUS_CHECK         0

//-----------------------------------------------------------
#define Sleep                       OS_MsDelay
#define AUTO_INIT_STATION           1


#define CONFIG_CHECKSUM_DCHECK      0
#define TARGET						prj1_config

// ------------------------- log -------------------------------
#define SSV_LOG_DEBUG           1


/** lower two bits indicate debug level
 * - 0 all
 * - 1 warning
 * - 2 serious
 * - 3 severe
 */
#define LOG_LEVEL_ALL     0x00
#define LOG_LEVEL_WARNING 0x01 	/* bad checksums, dropped packets, ... */
#define LOG_LEVEL_SERIOUS 0x02 	/* memory allocation failures, ... */
#define LOG_LEVEL_SEVERE  0x03
#define LOG_LEVEL_MASK_LEVEL 0x03


#define LOG_NONE	       			   (0)
#define LOG_MEM	       			       (1<<16)
#define LOG_L3_SOCKET				   (1<<17)
#define LOG_L3_API				   	   (1<<18)
#define LOG_L3_TCP					   (1<<19)
#define LOG_L3_UDP					   (1<<20)
#define LOG_L3_IP					   (1<<21)
#define LOG_L3_OTHER_PROTO			   (1<<22)
#define LOG_L2_DATA			           (1<<23)
#define LOG_L2_AP			           (1<<24)
#define LOG_L2_STA			           (1<<25)
#define LOG_CMDENG			           (1<<26)
#define LOG_TXRX			           (1<<27)
#define LOG_SCONFIG			           (1<<28)

#define LOG_L4_HTTPD                   (1<<29)
#define LOG_L4_NETMGR   	           (1<<30)
#define LOG_L4_DHCPD			       ((unsigned int)1<<31)

#define LOG_ALL_MODULES                (0xffffffff)

//***********************************************//
//Modify default log config
#define CONFIG_LOG_MODULE				(LOG_ALL_MODULES)
#define CONFIG_LOG_LEVEL				(LOG_LEVEL_SERIOUS)

// ------------------------- chip -------------------------------
#define SSV6051Q    0
#define SSV6051Z    1
#define SSV6030P    2
#define SSV6052Q    3
#define SSV6006B    4
//#define SSV6006C    5

#define SV6155P     0
#define SV6156P     1
#define SV6166P     2
#define SV6167Q     3
#define SV6166F     4
#define SV6166M     5
#define SV6255P     6
#define SV6256P     7
#define SV6266P     8
#define SV6267Q     9
#define SV6266F     10
#define SV6266M     11
#define SV6166FS    12
#define SV6151P     13
#define SV6152P     14

#define SSV6006C    ((1<<SV6266P)|(1<<SV6166P)|(1<<SV6166F)|(1<<SV6156P)|(1<<SV6256P)|(1<<SV6151P)|(1<<SV6152P)|(1<<SV6267Q)|(1<<SV6167Q))

#define CONFIG_CHIP_ID     SSV6006C

// ------------------------PHY mode ----------------------------
#define PHY_GB_MODE    0
#define PHY_NB_MODE    1

#define PHY_SUPPORT_HT PHY_NB_MODE

// ------------------------RF Band ----------------------------
#define RF_2G_BAND    0
#define RF_5G_BAND    1

#if(PHY_SUPPORT_HT==PHY_GB_MODE)
#define RF_BAND RF_2G_BAND
#else
#define RF_BAND RF_5G_BAND
#endif

// ------------------------AP HT20/HT40 ----------------------------
// TRUE: AP force to disable HT40
// FALSE: AP config HT40 by Regulatory, and HT40 only support on 5G Band
#define AP_HT20_ONLY 0
#define AP_HT40_ONLY 1
#define AP_HT2040    2

#define AP_BW AP_HT40_ONLY

// ------------------------- rtos -------------------------------
#define OS_MUTEX_SIZE				10              // Mutex
#define OS_COND_SIZE				5               // Conditional variable
#define OS_TIMER_SIZE				5               // OS Timer
#define OS_MSGQ_SIZE				5               // Message Queue

// ------------------------- cli -------------------------------
#define CLI_ENABLE					1               // Enable/Disable CLI
#if (CLI_ENABLE==1)
#define CLI_HISTORY_ENABLE			1               // Enable/Disable CLI history log. only work in SIM platofrm for now.
#define CLI_HISTORY_NUM				10


#define CLI_BUFFER_SIZE				80              // CLI Buffer size
#define CLI_ARG_SIZE				22             // The number of arguments
#define CLI_PROMPT					"wifi-host> "
#endif

// ------------------------- misc ------------------------------
#define _INTERNAL_MCU_SUPPLICANT_SUPPORT_
#define CONFIG_HOST_PLATFORM         1
/*test for sdio/spi drv*/
#define BUS_TEST_MODE 0
#define SSV_TMR_MAX   24

//STA mode change the RX sensitive dynamically
#define ENABLE_DYNAMIC_RX_SENSITIVE 1

//TX power mode , only workable for 6030P
#define TX_POWER_NORMAL          0
#define TX_POWER_B_HIGH_ONLY     1
#define TX_POWER_ENHANCE_ALL     2

#define CONFIG_TX_PWR_MODE     TX_POWER_NORMAL

//Voltage setting: LDO or DCDC
#define VOLT_LDO_REGULATOR  0
#define VOLT_DCDC_CONVERT   1

#define SSV_VOLT_REGULATOR  VOLT_DCDC_CONVERT

#define USE_EXT_PA  0
// ------------------------- mlme ------------------------------
/*Regular iw scan or regulare ap list update*/
#define CONFIG_AUTO_SCAN            0               // 1=auto scan, 0=auto flush ap_list table
#define MLME_TASK                   0               // MLME function 0=close,1=open. PS:If MLME_TASK=1, please re-set the stack size to 64 in porting.h

#if (MLME_TASK == 0)
#define TIMEOUT_TASK                MBOX_CMD_ENGINE               // MLME function 0=close,1=open
#else
#define TIMEOUT_TASK                MBOX_MLME_TASK               // MLME function 0=close,1=open
#endif

#define NUM_AP_INFO                 20
// ------------------------- txrx ------------------------------
#define RXFLT_ENABLE                1               // Rx filter for data frames
#define ONE_TR_THREAD               0               // one txrx thread. PS: If ONE_TR_THREAD=1, please re-set the value of WIFI_RX_STACK_SIZE to 0 in porting.h
#define TX_TASK_SLEEP               1               // Tx task go to sleep 1 tick when get no tx resource 200 time
#define TX_TASK_SLEEP_TICK          1               // how long does the tx task sleep? The default time is 1 tick
#define TX_TASK_RETRY_CNT           200             // Tx task retry count when tx resource of wifi-chip is not enough

#if(CONFIG_CHIP_ID==SSV6006C)||(CONFIG_CHIP_ID==SSV6006B)
#define TX_RESOURCE_PAGE            128 //82 //164 //128
#define RX_RESOURCE_PAGE            118  //164 //82 //118, 4 pages for secure, 6 pages for beacon
#define HCI_RX_AGGR    0
#define HCI_AGGR_TX    0    
#define RXINTGPIO      20
#else
#define TX_RESOURCE_PAGE            60
#define RX_RESOURCE_PAGE            61
#define RXINTGPIO      8  
#endif

#define TXDUTY_AT_HOST 0
#define TXDUTY_AT_FW   1
#define TXDUTY_MODE    TXDUTY_AT_FW
 
#if(CONFIG_CHIP_ID!=SSV6006C)
#undef HCI_RX_AGGR
#define HCI_RX_AGGR 0
#undef HCI_AGGR_TX
#define HCI_AGGR_TX 0
#endif
#define SW_Q_FORCE_UNLOCK_TIME   200 //unit: 10ms. 0 means never force unlock, 100=1000ms

// ------------------------- network ---------------------------
#define CONFIG_MAX_NETIF        1
#define AP_MODE_ENABLE          0
#if(CONFIG_CHIP_ID==SSV6006C)||(CONFIG_CHIP_ID==SSV6006B)
#define AP_SUPPORT_CLIENT_NUM     4 
#else
#define AP_SUPPORT_CLIENT_NUM     2 
#endif
#define CONFIG_PLATFORM_CHECK   1

// ------------------------- mac address  ------------------------------
#define CONFIG_EFUSE_MAC            0
#define	CONFIG_RANDOM_MAC           0

// ------------------------- Deafult channel in STA mode ---------------------------
#define STA_DEFAULT_CHANNEL 6

// ------------------------- STA channel mask  ------------------------------
//This macro is for STA mode, evey bit corrsebonds to a channel, for example: bit[0] -> channel 0, bit[1] ->cahnnel 1.
//If user assign 0 to channel mask to netmgr_wifi_scan or netmgr_wifi_sconfig, we use the default value.
//If youe set ch0 and ch15, we will filter it automatically
#define DEFAULT_STA_CHANNEL_MASK 0x7FFE //from 1~14

#if 0
BIT[0]=CHAN5G(5180, 36)
BIT[1]=CHAN5G(5200, 40)
BIT[2]=CHAN5G(5220, 44)
BIT[3]=CHAN5G(5240, 48)

BIT[4]=CHAN5G(5260, 52)
BIT[5]=CHAN5G(5280, 56)
BIT[6]=CHAN5G(5300, 60)
BIT[7]=CHAN5G(5320, 64)

BIT[8]=CHAN5G(5500, 100)
BIT[9]=CHAN5G(5520, 104)
BIT[10]=CHAN5G(5540, 108)
BIT[11]=CHAN5G(5560, 112)

BIT[12]=CHAN5G(5660, 132)
BIT[13]=CHAN5G(5680, 136)
BIT[14]=CHAN5G(5700, 140)
BIT[15]=CHAN5G(5745, 149)

BIT[16]=CHAN5G(5765, 153)
BIT[17]=CHAN5G(5785, 157)
BIT[18]=CHAN5G(5805, 161)
BIT[19]=CHAN5G(5825, 165)
#endif
#define DEFAULT_STA_5G_CHANNEL_MASK 0x1FFFFFF 
#define SCANNING_CH_INTERVAL 200 //200ms
#define INFO_SCAN_RESULT_TO_APP 0

// ------------------------- auto channel selection ---------------------------
//The ACS_CHANNEL_MASK is for auto channel selection in AP  mode, evey bit corrsebonds to a channel, for example: bit[0] -> channel 0, bit[1] ->cahnnel 1.
//Now, we set 0xFFE, this value means we do auto channel selection from channel 1 to channel 11.
//If you just want to choose a channel from 1, 6,11, you must set ACS_CHANNEL_MASK  to 0x842
//If youe set ch0, ch12, ch13, ch14, or ch15, we will filter it automatically
#define ACS_CHANNEL_MASK 0xFFE
#define ACS_5G_CHANNEL_MASK 0x1FFFFFF

//The g_acs_channel_scanning_interval is for auto channel secltion in AP mode, if you set 10, it means AP will stay in one channel for 10x10ms,
//and then change to the next channel.
#define ACS_SCANNING_INTERVAL 100 //100ms

//This macro is used to set the number of times that you want to do the channel scanning.
//If this macro is 1, we do channel scanning one times, if this macro is 2, we do channel scanning two times, and then we choose a channel by all datas
#define ACS_SCANNING_LOOP 2

// ------------------------- Smart Config ---------------------------
#define ENABLE_SMART_CONFIG 0
#define SMART_CONFIG_AUTO_JOIN 1 //Auto join AP after getting the result of smart config
//The SmartConfig's solutions
#define WECHAT_AIRKISS_IN_FW 0
#define WECHAT_AIRKISS_ON_HOST 1
#define ICOMM_SMART_LINK 2
#define CUSTOMER_SOLUTION  3

//Define the solution
#define SMART_CONFIG_SOLUTION ICOMM_SMART_LINK

#ifndef __SSV_UNIX_SIM__
    #if(SMART_CONFIG_SOLUTION==WECHAT_AIRKISS_ON_HOST)
    //# ERROR Configuration ...
    #endif
#endif

//This macro is for SmartConfig mode, evey bit corrsebonds to a channel, for example: bit[0] -> channel 0, bit[1] ->cahnnel 1.
//If user assign 0 to channel mask to netmgr_wifi_scan or netmgr_wifi_sconfig, we use the default value.
//If youe set ch0 and ch15, we will filter it automatically
#define DEFAULT_SCONFIG_CHANNEL_MASK 0x7FFE //from 1~14
#define DEFAULT_SCONFIG_5G_CHANNEL_MASK 0x1FFFFFF 

// ------------------------- ---------------------------------------
#if (AP_MODE_ENABLE == 1)

	//#define __TEST__
	//#define __TEST_DATA__  //Test data flow
	//#define __TEST_BEACON__

#else
	#define __STA__

	//#define __TCPIP_TEST__
#endif


//#define __AP_DEBUG__

// ------------------------- rate control ---------------------------

#define RC_DEFAULT_RATE_MSK 0x0FFD
#define RC_DEFAULT_RESENT_REPORT 1  // feature, 0 or 1
#define RC_DEFAULT_UP_PF 10         // percentage, smaller than RC_DEFAULT_DOWN_PF
#define RC_DEFAULT_DOWN_PF 27       // percentage, bigger than RC_DEFAULT_UP_PF
#define RC_DEFAULT_PREPRBFRM 0      // feature, 0 or 1
#define RC_DIRECT_DOWN 0            // 1: TRUE, 0:False
#define RC_DEFAULT_DRATE_ENDIAN 0 //1: The default data rate is from the lowest rate of rate mask. 0:from highest index of rate mask
#define RC_DEFAULT_TX_SGI 0         // 1: TRUE, 0:False
// ------------------------- AMPDU ---------------------------
#define AMPDU_TX_ENABLE 1
#define AMPDU_RX_ENABLE 1
#if(CONFIG_CHIP_ID==SSV6006C)||(CONFIG_CHIP_ID==SSV6006B)
#define AMPDU_RX_MAX_STATION AP_SUPPORT_CLIENT_NUM
#else
#define AMPDU_RX_MAX_STATION 1
#endif
#define AMPDU_RX_BUF_SIZE 64

//==============EDCA===============
//#define EDCA_PATTERN_TEST
#ifdef EDCA_PATTERN_TEST
#define EDCA_DBG						1		//Enable to test edca function
#define MACTXRX_CONTROL_ENABLE			1		//To simulate MAC TX operation. It's also enable ASIC queue empty interrupt.
#define MACTXRX_CONTROL_DURATION_SIM	1		//TX control use softmain edca handler to test MAC TX EDCA function
//#define __EDCA_INT_TEST__						//
//#define __EDCA_NOTIFY_HOST__					//When TX done send an event to nofity host to know
#define BEACON_DBG						1
#else
#define EDCA_DBG						0		//Enable to test edca function
#define MACTXRX_CONTROL_ENABLE			0		//To simulate MAC TX operation. It's also enable ASIC queue empty interrupt.
#define MACTXRX_CONTROL_DURATION_SIM	0		//TX control use softmain edca handler to test MAC TX EDCA function
//#define __EDCA_INT_TEST__						//
//#define __EDCA_NOTIFY_HOST__					//When TX done send an event to nofity host to know
#define BEACON_DBG						0
#endif


//=================================

//#define PACKED

/* default ip */
#define DEFAULT_IPADDR   "192.168.25.1"
#define DEFAULT_SUBNET   "255.255.255.0"
#define DEFAULT_GATEWAY  "192.168.25.1"
#define DEFAULT_DNS      "192.168.25.1"

/* default dhcp server info */
#define DEFAULT_DHCP_START_IP    "192.168.25.101"
#define DEFAULT_DHCP_END_IP      "192.168.25.110"
#define DEFAULT_DHCP_MAX_LEASES  10

#define DEFAULT_DHCP_AUTO_TIME       (7200)
#define DEFAULT_DHCP_DECLINE_TIME    (3600)
#define DEFAULT_DHCP_CONFLICT_TIME   (3600)
#define DEFAULT_DHCP_OFFER_TIME      (60)
#define DEFAULT_DHCP_MIN_LEASE_SEC   (60)
#define DEFAULT_DHCP_MAX_LEASE_SEC   (60*60*24 * 10)


/* watchdog */
#if((CONFIG_CHIP_ID==SSV6006B)||(CONFIG_CHIP_ID==SSV6006C))
#define RECOVER_ENABLE                  0
#else
#define RECOVER_ENABLE                  0
#endif
#ifndef __SSV_UNIX_SIM__
#define RECOVER_MECHANISM               1 //0:Used GPIO interrupt, 1:Used Timer interrupt
#else
#define RECOVER_MECHANISM               0 //0:Used GPIO interrupt, 1:Used Timer interrupt
#endif
#define IPC_CHECK_TIMER                 1000 //For IPC interrupt

// ------------------------- Power Saving ---------------------------
#define BUS_IDLE_TIME 0 //5000 //mswpw

/* netmgr auto retry default times and delay value */
#define NET_MGR_AUTO_RETRY_TIMES 0xFFFF
#define NET_MGR_AUTO_RETRY_DELAY 5 // unit s

//------------------------wpa2--------------------
//#define CONFIG_NO_WPA2

//------------------------AP mode--------------------
#define AUTO_BEACON 0
#define AP_BEACON_INT	(100)
#define AP_ERP                   0
#define AP_B_SHORT_PREAMBLE      0
#define AP_RX_SHORT_GI           1
#define AP_RX_SUPPORT_BASIC_RATE     0xFFF
#define AP_RX_SUPPORT_MCS_RATE       0xFF

#define AP_MODE_BEACON_VIRT_BMAP_0XFF         0		// 1: FIX VIRT BMAP 0XFF, 0:DYNAMIC VIRT BMAP
#define AP_SAT_INACTIVE                       60    //sec

// ------------------------- Update features depends on TCPIP stack ---------------------------
/* TCP/IP Configuration: */
#ifndef USE_ICOMM_LWIP
#define USE_ICOMM_LWIP          1
#endif
/*
    1: LWIP ignore the pcb->cwnd when LWIP output tcp packet
    0: LWIP refer the pcb->cwnd when LWIP output tcp packet
*/
#define LWIP_TCP_IGNORE_CWND     0
/* 0 for minimal resources, 1 for double resource of setting 0 with higher performance, 2 for default, 3 for maximal resources */
/* RAM Usage:
   0: 46KB
   1: 77KB
   2: 99KB
   3: 177KB
*/
#define LWIP_PARAM_SET                  3
#if USE_ICOMM_LWIP
#define HTTPD_SUPPORT           0
#define DHCPD_SUPPORT           0
#define CONFIG_USE_LWIP_PBUF    1
#define CONFIG_MEMP_DEBUG       0
#define PING_SUPPORT            0
#define IPERF3_ENABLE           0
#define IPERF3_UDP_TEST         0
#else //#if USE_ICOMM_LWIP
#define PING_SUPPORT 0
#undef IPERF3_ENABLE
#define IPERF3_ENABLE  0
#define IPERF3_UDP_TEST 0
#undef  CONFIG_USE_LWIP_PBUF
#define CONFIG_USE_LWIP_PBUF    0
#undef  CONFIG_MEMP_DEBUG
#define CONFIG_MEMP_DEBUG       0
#define HTTPD_SUPPORT           0
#define DHCPD_SUPPORT           0

#endif //#if USE_ICOMM_LWIP

#if CONFIG_USE_LWIP_PBUF
#define POOL_SIZE PBUF_POOL_SIZE
#define POOL_SEC_SIZE           36
#define RECV_BUF_SIZE           (TCP_MSS+40+PBUF_LINK_HLEN)
#define TRX_HDR_LEN             40

#else
#define RECV_BUF_SIZE           1516
#define POOL_SIZE               64
#define POOL_SEC_SIZE           32
#define TRX_HDR_LEN             40

#endif //#if USE_ICOMM_LWIP

//------------------------Net APP--------------------
//Disable NETAPP, and we need to manually set the netapp stack size to zero in porting.h
#define NETAPP_SUPPORT  1
#define NETMGR_SUPPORT  1
#if NETMGR_SUPPORT
#define NET_MGR_DEBUG   1
#endif

//------------------------8023 <-> 80211--------------------
#define SW_8023TO80211 0 

#if((CONFIG_CHIP_ID!=SSV6006B)&&(CONFIG_CHIP_ID!=SSV6006C))
#undef SW_8023TO80211
#define SW_8023TO80211 0
#endif

//------------------------Bus Loop Back--------------------
#define CONFIG_BUS_LOOPBACK_TEST 1

//------------------------Deafult Country--------------------
#define DEFAULT_COUNTRY_CODE "00"

//------------------------DFS--------------------
#define AP_NO_DFS_FUN 1

//Set a timeout periord for STA auto disconnect when AP power off or change channel
//The unit is 500ms, the default value is 40, it means STA will disconnect after AP power off for 24s(40*600ms)
#define STA_NO_BCN_TIMEOUT 40

//------------------------XTAL--------------------
#define XTAL_16M        0
#define XTAL_24M        1
#define XTAL_26M        2
#define XTAL_40M        3
#define XTAL_12M        4
#define XTAL_20M        5
#define XTAL_25M        6
#define XTAL_32M        7
#define XTAL_19P2M      8	
#define XTAL_38P4M      9
#define XTAL_52M        10
#define XTAL_SETTING	XTAL_26M

//------------------------KRACK--------------------
#define KRACK_PATCH 0

#define FAST_CONNECT_PROFILE 0
#endif /* _SIM_CONFIG_H_ */
