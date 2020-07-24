/*
*  (C) Copyright 2014-2016 Shenzhen South Silicon Valley microelectronics co.,limited
*
*  All Rights Reserved
*/


#define __SFILE__ "testcase.c"

#include <ssv_types.h>
#include <hw/config.h>
#include <log.h>


#include <ssv_ether.h>

#include "ap_sta_info.h"
#include "ap_info.h"
#include "ap_def.h"
#include "beacon.h"
#include "common/bitops.h"
#include <log.h>
#include "common/ap_common.h"
#include "ap_mlme.h"

#include <host_apis.h>
#include "ap_tx.h"

#include "common/ieee802_11_defs.h"

#if (AP_MODE_ENABLE == 1)



#ifndef MAC2STR
#define MAC2STR(a) (a)[0], (a)[1], (a)[2], (a)[3], (a)[4], (a)[5]
#define MACSTR "%02x:%02x:%02x:%02x:%02x:%02x"
#endif


typedef struct ApInfo ApInfo_st;
typedef struct APStaInfo APStaInfo_st;
extern struct APStaInfo *APStaInfo_add(struct ApInfo *pApInfo, const ssv_type_u8 *addr);

#ifdef __TEST_DATA__


extern ssv_type_s32 if_sim_recv(void *dat, ssv_type_s32 len);


char QoSData[]={	0x88 ,0x01 ,0xda ,0x00 ,			//Qos Data
					0x00 ,0x11 ,0x22 ,0x33 ,0x44 ,0x55 ,//RA
					0x02 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,//TA
					0x02 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,//DA
					0x60 ,0x01 ,						//Seq
					0x00 ,0x00 ,						//QoS
					0xaa ,0xaa ,0x03 ,0x00 ,0x00 ,0x00 ,
					0x86 ,0xdd ,0x60 ,0x00 ,0x00 ,0x00 ,0x00 ,0x2f ,0x11 ,0xff ,
					0xfe ,0x80 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0xff ,0xfe ,0x00 ,0x01 ,0x00 ,
					0xff ,0x02 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0xfb ,0x14 ,0xe9 ,0x14 ,
					0xe9 ,0x00 ,0x2f ,0x34 ,0x11 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x01 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x0a ,0x5f ,0x73 ,0x61 ,0x6e ,
					0x65 ,0x2d ,0x70 ,0x6f ,0x72 ,0x74 ,0x04 ,0x5f ,0x74 ,0x63 ,0x70 ,0x05 ,0x6c ,0x6f ,0x63 ,0x61 ,0x6c ,0x00 ,0x00 ,0x0c ,0x00 ,0x01 ,
					0x04, 0x03, 0x02, 0x01};

char QoSNullData[]={	0xc8 ,0x01 ,0xda ,0x00 ,			//Qos NULL Data
	0x00 ,0x11 ,0x22 ,0x33 ,0x44 ,0x55 ,//RA
	0x02 ,0x00 ,0x00 ,0x00 ,0x01 ,0x00 ,//TA
	0x02 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,//DA
	0x60 ,0x01 ,						//Seq
	0x04 ,0x00 ,							//QoS
	0x04, 0x03, 0x02, 0x01					//FCS
};


// struct ieee80211_hdr_3addr {
// 	__le16 frame_control;
// 	__le16 duration_id;
// 	u8 addr1[6];
// 	u8 addr2[6];
// 	u8 addr3[6];
// 	__le16 seq_ctrl;
// } STRUCT_PACKED;


int count=0;

void data_test_case(void)
{
	struct ieee80211_hdr_3addr *hdr = (struct ieee80211_hdr_3addr *)&QoSData;

	hdr->addr3[4]=count%2+1;
	if_sim_recv(&QoSData, sizeof(QoSData));
	count++;
}

struct ieee80211_pspoll_fcs {
	__le16_ssv_type frame_control;
	__le16_ssv_type aid;
	ssv_type_u8 bssid[6];
	ssv_type_u8 ta[6];
	ssv_type_u8 fcs[4];
} ;

void TestCase_SendPSPoll(void)
{	
	struct ieee80211_pspoll_fcs pspoll;
	
	
	//pspoll = (struct ieee80211_pspoll *) skb_put(skb, sizeof(*pspoll));
	ssv6xxx_memset((void *)&pspoll, 0, sizeof(pspoll));
	pspoll.frame_control = cpu_to_le16(IEEE80211_FTYPE_CTL | IEEE80211_STYPE_PSPOLL);
	pspoll.aid = cpu_to_le16(2);

	/* aid in PS-Poll has its two MSBs each set to 1 */
	pspoll.aid |= cpu_to_le16(1 << 15 | 1 << 14);

	pspoll.bssid[0]=0x00; pspoll.bssid[1]=0x11; pspoll.bssid[2]=0x22;
	pspoll.bssid[3]=0x33; pspoll.bssid[4]=0x44; pspoll.bssid[5]=0x55;

	pspoll.ta[0]=0x02; pspoll.ta[1]=0x00; pspoll.ta[2]=0x00;
	pspoll.ta[3]=0x00; pspoll.ta[4]=0x01; pspoll.ta[5]=0x00;

	if_sim_recv(&pspoll, sizeof(pspoll));


	pspoll.aid = cpu_to_le16(3);
	pspoll.ta[4]=0x02;
	if_sim_recv(&pspoll, sizeof(pspoll));
	
}



void TestCase_SendTriggerFrame(void)
{
	QoSNullData[1] |= 0x10;//pwr Mgmt
	if_sim_recv(&QoSNullData, sizeof(QoSNullData));

	QoSNullData[14] = 0x02;//Mac Address
	if_sim_recv(&QoSNullData, sizeof(QoSNullData));
}



void TestCase_FillSTAReg(void)
{
	ssv_type_u8 addr[6];
	//Set to HW
	int i;


	for (i=0;i<3;i++)
	{

		addr[0]=0x02;
		addr[1]=0x00;
		addr[2]=0x00;
		addr[3]=0x00;
		addr[4]=i;
		addr[5]=0x00;
		GHW_REGS.TBL_WSID[i].valid = 1;
		OS_MemCPY(&GHW_REGS.TBL_WSID[i].peer_mac, addr, 6);
		GHW_REGS.TBL_WSID[i].peer_op_mode = 0;
		LOG_TRACE(SSV_AP,"STA: " MACSTR " add to HW register table.\n", MAC2STR(addr));
	}







}


void TestCase_AddAPSta(void)
{
	int i;
	ssv_type_u8 addr[6];
	for (i=0;i<3;i++)
	{
		APStaInfo_st *sta;
		addr[0]=0x02;
		addr[1]=0x00;
		addr[2]=0x00;
		addr[3]=0x00;
		addr[4]=i;
		addr[5]=0x00;
		sta = APStaInfo_add(gAPInfo, (const ssv_type_u8*)&addr);
		
		set_sta_flag(sta, WLAN_STA_AUTH);
		set_sta_flag(sta, WLAN_STA_ASSOC);
		set_sta_flag(sta, WLAN_STA_WMM);
		if(i)
			set_sta_flag(sta, WLAN_STA_PS_STA);


		ap_sta_set_authorized(gDeviceInfo->APInfo, sta, 1);
		if (hostapd_get_aid(gDeviceInfo->APInfo, sta) < 0) {									//---------->get aid------------------------------------------------------------------------>
			LOG_INFO(SSV_AP, "No room for more AIDs\r\n");
		}
	}




	gDeviceInfo->APInfo->num_sta_ps=2;



	TestCase_FillSTAReg();

}
#endif//__TEST_DATA__
#endif












