/*
*  (C) Copyright 2014-2016 Shenzhen South Silicon Valley microelectronics co.,limited
*
*  All Rights Reserved
*/


#define __SFILE__ "neighbor_ap_list.c"

#include "common/ieee802_11_defs.h"
#include "common/ieee802_11_common.h"
#include "ieee802_11_ht.h"

#include "beacon.h"
#include "neighbor_ap_list.h"
#include "ssv_timer.h"
#include <ssv_devinfo.h>

#if (AP_MODE_ENABLE == 1)

extern struct task_info_st g_host_task_info[];
extern struct Host_cfg g_host_cfg;

/* AP list is a double linked list with head->prev pointing to the end of the
 * list and tail->next = NULL. Entries are moved to the head of the list
 * whenever a beacon has been received from the AP in question. The tail entry
 * in this link will thus be the least recently used entry. */
static int ap_list_beacon_olbc(ApInfo_st *pApInfo, struct neighbor_ap_info *ap)
{
	int i;

	if (pApInfo->eCurrentApMode != AP_MODE_IEEE80211G ||
		pApInfo->nCurrentChannel != ap->channel)
		return 0;

	if (ap->erp != -1 && (ap->erp & ERP_INFO_NON_ERP_PRESENT))
		return 1;

	for (i = 0; i < AP_SUPP_RATES_MAX; i++) {
		int rate = (ap->supported_rates[i] & 0x7f) * 5;
		if (rate == 60 || rate == 90 || rate > 110)
			return 0;
	}

	return 1;
}


struct neighbor_ap_info * ap_get_ap(ApInfo_st *pApInfo, const ETHER_ADDR *mac)
{
	int i;
//	ETHER_ADDR NULLADDR = {0};

	for(i=0; i<AP_NEIGHBOR_AP_TABLE_MAX_SIZE; i++)
	{
		struct neighbor_ap_info *s = &pApInfo->neighbor_ap_list[i];

		if (IS_EQUAL_MACADDR((char*)mac, (char*)&s->addr))
			return s;
	}

	return NULL;
}


static void ap_free_ap(ApInfo_st *pApInfo, struct neighbor_ap_info *ap)
{
	OS_MemSET(ap, 0, sizeof(struct neighbor_ap_info));
	pApInfo->num_neighbor_ap--;
}

static struct neighbor_ap_info * ap_ap_add(ApInfo_st *pApInfo, const ETHER_ADDR *addr)
{
	struct neighbor_ap_info *ap;

	pApInfo->num_neighbor_ap++;
	if (pApInfo->num_neighbor_ap > AP_NEIGHBOR_AP_TABLE_MAX_SIZE) 
	{
		//neighbor AP storage is full. need to replace least one.
		int i;//, nLeastIdx = 0;
		

		ap = &pApInfo->neighbor_ap_list[0];

		//Find least AP and reset it.
		for (i=1;i<AP_NEIGHBOR_AP_TABLE_MAX_SIZE;i++)
		{
			struct neighbor_ap_info *sIdx = &pApInfo->neighbor_ap_list[i];
			if(time_before(sIdx->last_beacon ,ap->last_beacon) )
			{	
				ap = sIdx;
			}
		}

		//Free AP
		ap_free_ap(pApInfo, ap);

	}
	else
	{
		ETHER_ADDR NULLADDR = {0};
		ap = ap_get_ap(pApInfo, &NULLADDR);
	}

	OS_MemCPY((void*)ap->addr, (void*)addr, ETHER_ADDR_LEN);

	return ap;
}


void neighbor_ap_list_process_beacon(ApInfo_st *pApInfo,
	const struct ieee80211_mgmt *mgmt,
struct ieee802_11_elems *elems)
{
 	struct neighbor_ap_info *ap;
	int new_ap = 0;
	ssv_type_size_t len;
	int set_beacon = 0;


	ap = ap_get_ap(pApInfo, (ETHER_ADDR*)mgmt->bssid);
	if (!ap) {
		ap = ap_ap_add(pApInfo, (ETHER_ADDR*)mgmt->bssid);
		if (!ap) {
			LOG_ERROR("%s Failed to allocate AP information entry\r\n",__func__);
			return;
		}
		new_ap = 1;
	}
 
	ap->beacon_int = le_to_host16(mgmt->u.beacon.beacon_int);
	ap->capability = le_to_host16(mgmt->u.beacon.capab_info);

	if (elems->ssid) {
		len = elems->ssid_len;
		if (len > sizeof(ap->ssid))
			len = sizeof(ap->ssid);
		OS_MemCPY(ap->ssid, elems->ssid, len);
		//ap->ssid[len] = '\0';
		ap->ssid_len = len;
	}

	OS_MemSET(ap->supported_rates, 0, AP_SUPP_RATES_MAX);
	len = 0;
	if (elems->supp_rates) {
		len = elems->supp_rates_len;
		if (len > AP_SUPP_RATES_MAX)
			len = AP_SUPP_RATES_MAX;
		OS_MemCPY(ap->supported_rates, elems->supp_rates, len);
	}
	if (elems->ext_supp_rates) {
		int len2;
		if (len + elems->ext_supp_rates_len > AP_SUPP_RATES_MAX)
			len2 = AP_SUPP_RATES_MAX - len;
		else
			len2 = elems->ext_supp_rates_len;
		OS_MemCPY(ap->supported_rates + len, elems->ext_supp_rates,
			  (ssv_type_u32)len2);
	}

	ap->wpa = elems->wpa_ie != NULL;

	if (elems->erp_info && elems->erp_info_len == 1)
		ap->erp = elems->erp_info[0];
	else
		ap->erp = -1;

	if (elems->ds_params && elems->ds_params_len == 1)
		ap->channel = elems->ds_params[0];

	if (elems->ht_capabilities)
		ap->ht_support = 1;
	else
		ap->ht_support = 0;

	ap->num_beacons++;

	ap->last_beacon = OS_GetSysTick(); //now.sec;

    if(g_host_cfg.erp==1)
    {
        if (!pApInfo->olbc &&
        ap_list_beacon_olbc(pApInfo, ap)) {

            //Send ERP protect cmd
            ssv_type_bool erp_protect = TRUE;
            _ssv6xxx_wifi_ioctl_Ext(SSV6XXX_HOST_CMD_SET_ERP_PROTECT, &erp_protect, sizeof(ssv_type_bool),TRUE,FALSE); 

            pApInfo->olbc = 1;

            set_beacon++;
        }
    }

#if AP_SUPPORT_80211N

	if (!pApInfo->olbc_ht && !ap->ht_support) {
        ssv_type_u16 operation_mode;
		pApInfo->olbc_ht = 1;
		hostapd_ht_operation_update(pApInfo);
        operation_mode = host_to_le16(pApInfo->ht_op_mode);
        _ssv6xxx_wifi_ioctl_Ext(SSV6XXX_HOST_CMD_SET_HT_PROTECT, &operation_mode, sizeof(ssv_type_bool),TRUE,FALSE);
		set_beacon++;
	}
#endif /* AP_SUPPORT_80211N */

	if (set_beacon)
		ieee802_11_set_beacon(pApInfo,TRUE);
 }
// 
// 
 static void ap_list_timer(void *data1, void *data2)
 {
	ApInfo_st *pApInfo = (ApInfo_st *)data1;
	ssv_type_u32 now = OS_GetSysTick();
	int set_beacon = 0;
	int olbc = 0;			//Overlapping Legacy BSS Condition 
	int olbc_ht = 0;
    int i = 0;
    
    for (i = 0; i < MAX_VIF_NUM; i++)
    {
	    if (SSV6XXX_HWM_AP == gDeviceInfo->vif[i].hw_mode)
		    break; 
    }

    if (i >= MAX_VIF_NUM)
    {
        return;
    }
    
    os_create_timer(AP_LIST_TIMER_INTERVAL, ap_list_timer, pApInfo, NULL, (void*)TIMEOUT_TASK);

	for (i=0;i<AP_NEIGHBOR_AP_TABLE_MAX_SIZE;i++)
	{
		struct neighbor_ap_info *ap = &pApInfo->neighbor_ap_list[i];


		//If this ap doest not store data, skip it.
		if (!ap->last_beacon)
			continue;


		//Release AP that is too long no update	
		if (time_before(ap->last_beacon + OS_MS2TICK(AP_TABLE_EXPIRATION_TIME), now))		
			ap_free_ap(pApInfo, ap);	
		else
		{
			//After free some AP, check exist ap if need to set olbc or olbc_hc to 1
			if ((pApInfo->olbc || pApInfo->olbc_ht))
			{

				if(1 == olbc && 1 ==olbc_ht)
				{
					//Already found. don't need to check
					continue;
				}

				if (ap_list_beacon_olbc(pApInfo, ap))
					olbc = 1;
				if (!ap->ht_support)//neighbor AP not support HT
					olbc_ht = 1;
			}			
		}	
	}


    if(g_host_cfg.erp==1)
    {
        if (!olbc && pApInfo->olbc) {
            LOG_TRACE( "OLBC not detected anymore %s\r\n",__func__);
            pApInfo->olbc = 0;
            if (pApInfo->num_sta_non_erp == 0) 
            {
                //Send ERP protect cmd
                ssv_type_bool erp_protect = FALSE;
                _ssv6xxx_wifi_ioctl_Ext(SSV6XXX_HOST_CMD_SET_ERP_PROTECT, &erp_protect, sizeof(ssv_type_bool),TRUE,FALSE);  
            }
            set_beacon++;
        }
    }
#if AP_SUPPORT_80211N
		if (!olbc_ht && pApInfo->olbc_ht) {
            ssv_type_u16 operation_mode;
			LOG_TRACE( "OLBC HT not detected anymore %s\r\n",__func__);
			pApInfo->olbc_ht = 0;
			hostapd_ht_operation_update(pApInfo);
            operation_mode = host_to_le16(pApInfo->ht_op_mode);
            _ssv6xxx_wifi_ioctl_Ext(SSV6XXX_HOST_CMD_SET_HT_PROTECT, &operation_mode, sizeof(ssv_type_bool),TRUE,FALSE);
			set_beacon++;
		}		
#endif /* AP_SUPPORT_80211N */
	

	if (set_beacon)
		ieee802_11_set_beacon(pApInfo,TRUE);//update beacon
 }


 void reset_neighbor_apinfo(ApInfo_st *pApInfo)
 {
	 pApInfo->olbc = 0;
	 pApInfo->olbc_ht = 0;	
	 pApInfo->num_neighbor_ap = 0;
	 OS_MemSET(pApInfo->neighbor_ap_list, 0, sizeof(struct neighbor_ap_info)*AP_NEIGHBOR_AP_TABLE_MAX_SIZE);
 }


int neighbor_ap_list_init(ApInfo_st *pApInfo)
{

    os_create_timer(AP_LIST_TIMER_INTERVAL, ap_list_timer, pApInfo, NULL, (void*)TIMEOUT_TASK);
	reset_neighbor_apinfo(pApInfo);
	return 0;
}


void neighbor_ap_list_deinit(ApInfo_st *pApInfo)
{
	os_cancel_timer(ap_list_timer, (ssv_type_u32)pApInfo, (ssv_type_u32)0);
	reset_neighbor_apinfo(pApInfo);
}



//-------------------------------------------------
#ifdef __AP_DEBUG__


struct ap_sta_info_tbl
{
	const char *string;
};

static const struct ap_sta_info_tbl g_ap_mode[]=
{
	{"A"},
	{"B"},
	{"G"},
};


static const struct ap_sta_info_tbl g_ap_sec[]=
{
	{"NONE"},
	{"WEP40"},
	{"WEP104"},
	{"WPA_PSK"},
	{"WPA2_PSK"},
	{"WPS"},
};






extern void ieee802_11_print_ssid(char *buf, const ssv_type_u8 *ssid, ssv_type_u8 len);

void APStaInfo_PrintNeiborInfo(void)
{
//	int index=0;
	int i;

	LOG_TRACE("Neighbor AP info:%d AP(s)\r\n", gDeviceInfo->APInfo->num_neighbor_ap);



	
	for(i=0; i<AP_NEIGHBOR_AP_TABLE_MAX_SIZE; i++)
	{
		struct neighbor_ap_info *s = &gDeviceInfo->APInfo->neighbor_ap_list[i];
		ssv_type_u8 nulladdr[6] = {0};
		char ssid_txt[33]= {0};

		LOG_TRACE("AP:%d \r\n", i);


		if (IS_EQUAL_MACADDR(&nulladdr, &s->addr))
        {
        	LOG_PRINTF("=>Invalid\n");
        	continue;
		}
		
		ieee802_11_print_ssid(ssid_txt, s->ssid, s->ssid_len);
		LOG_PRINTF("=>addr:"MACSTR" ssid:%s \r\n",MAC2STR(s->addr), ssid_txt);		
		LOG_PRINTF("=>beacon_int:%d capability:%d \r\n  ",  s->beacon_int, s->capability);
		LOG_PRINTF("=>erp 0x%x channel:%d ht_support:%d last_beacon:%s\r\n\n", s->erp, s->channel, s->ht_support, s->last_beacon);		
		
	}



}


void APStaInfo_PrintAPInfo(void)
{
	
	LOG_TRACE("======================%s================\r\n",__func__);	
	LOG_TRACE("=>AP SSID:%s MAC:"MACSTR"\r\n", gDeviceInfo->APInfo->config.ssid, MAC2STR(gDeviceInfo->APInfo->own_addr));


	LOG_TRACE("=>eCurrentApMode:%s b80211n:%d	 nCurrentChannel:%d\r\n",g_ap_mode[gDeviceInfo->APInfo->eCurrentApMode], gDeviceInfo->APInfo->b80211n, gDeviceInfo->APInfo->nCurrentChannel);
	//LOG_TRACE("=>dtim_count:%d     dtim_bc_mc:%d total_ps_buffered:%d\r\n", gDeviceInfo->APInfo->dtim_count, gDeviceInfo->APInfo->dtim_bc_mc, gDeviceInfo->APInfo->total_ps_buffered);

	LOG_TRACE("-----------------STA(s)Info------------------------\r\n",__LINE__);
	LOG_TRACE("=>num_sta:%d	 num_sta_ps:%d  num_sta_authorized:%d  num_sta_non_erp:%d \r\n",gDeviceInfo->APInfo->num_sta, gDeviceInfo->APInfo->num_sta_ps, gDeviceInfo->APInfo->num_sta_authorized, gDeviceInfo->APInfo->num_sta_non_erp);
	LOG_TRACE("=>num_sta_no_short_preamble:%d  num_sta_no_short_slot_time:%d \r\n",gDeviceInfo->APInfo->num_sta_no_short_preamble, 
																				gDeviceInfo->APInfo->num_sta_no_short_slot_time);

	LOG_TRACE("-------------------HT Info--------------------------\r\n",__LINE__);	
	LOG_TRACE("=>num_sta_ht_20mhz:%d num_sta_ht_no_gf:%d num_sta_no_ht:%d \r\n",gDeviceInfo->APInfo->num_sta_ht_20mhz, 
																				gDeviceInfo->APInfo->num_sta_ht_no_gf, 
																				gDeviceInfo->APInfo->num_sta_no_ht);
	
	LOG_TRACE("-------------------AP Security--------------------------\r\n",__LINE__);
	LOG_TRACE("=>auth_algs:%d tkip_countermeasures:%d sec_type:%s\r\n",gDeviceInfo->APInfo->auth_algs, gDeviceInfo->APInfo->tkip_countermeasures, g_ap_sec[gDeviceInfo->APInfo->sec_type]);
	LOG_TRACE("=>password:%s\r\n",gDeviceInfo->APInfo->password);


	LOG_TRACE("-------------------AP Neibor--------------------------\r\n",__LINE__);
	LOG_TRACE("=>olbc:%d olbc_ht:%d num_neighbor_ap:%d\r\n",gDeviceInfo->APInfo->olbc, gDeviceInfo->APInfo->olbc_ht, gDeviceInfo->APInfo->num_neighbor_ap);

	LOG_TRACE("========================================\r\n",__LINE__);	

}






#endif//#ifdef __AP_DEBUG__
#endif
