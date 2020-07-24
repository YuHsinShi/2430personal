/*
*  (C) Copyright 2014-2016 Shenzhen South Silicon Valley microelectronics co.,limited
*
*  All Rights Reserved
*/


#define __SFILE__ "ieee802_11_ht.c"


#include "ap_config.h"
    
#include "common/ieee802_11_defs.h"
#include "common/ieee802_11_common.h"
#include <ieee80211.h>
#include <ssv_devinfo.h>
#include <channel.h>
#include "ap_mlme.h"    
#include "ap_info.h"
#include "ap_sta_info.h"
#include "beacon.h"
#include "ieee802_11_mgmt.h"
#include "wmm.h"

#if (AP_MODE_ENABLE == 1)
extern struct Host_cfg g_host_cfg;

ssv_type_u8 * hostapd_eid_ht_capabilities(ApInfo_st *pApInfo, ssv_type_u8 *eid)
{
    struct ieee80211_ht_capabilities *cap;
    ssv_type_u8 *pos = eid;
    ssv_type_u16 ht_capab=0;

    if (!pApInfo->config.b80211n|| !pApInfo->eCurrentApMode|| pApInfo->config.disable_11n)
        return eid;

    SSV_ASSERT(HT_CAP_SIZE == sizeof(struct ieee80211_ht_capabilities));

    *pos++ = WLAN_EID_HT_CAPABILITY;
    *pos++ = sizeof(struct ieee80211_ht_capabilities);

    cap = (struct ieee80211_ht_capabilities *) pos;
    OS_MemSET((void*)cap, 0, sizeof(*cap));
    //cap->ht_capabilities_info = host_to_le16(pApInfo->config.ht_capab);
    ht_capab=pApInfo->config.ht_capab;
    if(IS_40MHZ_AVAILABLE())    
    {
        ht_capab|=(1<<1); //Set support channel width = 1;
    }

    OS_MemCPY((void*)pos, (void*)&host_to_le16(ht_capab), 2);

    cap->a_mpdu_params = pApInfo->config.a_mpdu_params;

    OS_MemCPY((void*)cap->supported_mcs_set, pApInfo->config.mcs_set,16);

    /* TODO: ht_extended_capabilities (now fully disabled) */
    /* TODO: tx_bf_capability_info (now fully disabled) */
    /* TODO: asel_capabilities (now fully disabled) */

    //LOG_PRINTF("ht_capabilities size=%d\r\n",sizeof(struct ieee80211_ht_capabilities));
    pos += sizeof(struct ieee80211_ht_capabilities);
	return pos;
}



ssv_type_u8 * hostapd_eid_ht_operation(ApInfo_st *pApInfo, ssv_type_u8 *eid)
{
	struct ieee80211_ht_operation *oper;
	ssv_type_u8 *pos = eid;
    ssv_type_s32 i=0;
    ssv_type_u8 flags=0;
    if (!pApInfo->config.b80211n || pApInfo->config.disable_11n)
		return eid;

	*pos++ = WLAN_EID_HT_OPERATION;
	*pos++ = sizeof(struct ieee80211_ht_operation);

	oper = (struct ieee80211_ht_operation *) pos;
	OS_MemSET((void *)oper, 0, sizeof(*oper));

	oper->primary_chan = pApInfo->config.nChannel;
    if(IS_40MHZ_AVAILABLE())
    {
        i=ssv6xxx_wifi_ch_to_bitmask(oper->primary_chan);
        if(-1!=i)
        {
            if(IS_AP_IN_5G_BAND())
            {
                flags=ssv6xxx_5ghz_chantable[i].flags;
            }
            else
            {
                flags=ssv6xxx_2ghz_chantable[i].flags;
            }

                
            switch(flags)
            {
                case SSV_80211_CHAN_HT40PLUS:
                    oper->ht_param=1;
                    break;                    
                case SSV_80211_CHAN_HT40MINUS:
                    oper->ht_param=3;
                    break;  
                case SSV_80211_CHAN_HT20:
                default:
                    oper->ht_param=0;
                    break;
            }
        }
        oper->ht_param |= (1<<2); //Sec STA Channel Width = 1        
    }
    
    //oper->operation_mode = host_to_le16(pApInfo->ht_op_mode);
    OS_MemCPY((void*)&oper->operation_mode, (void*)&host_to_le16(pApInfo->ht_op_mode), 2);
    pos += sizeof(struct ieee80211_ht_operation);

	return pos;
}


/*
op_mode
Set to 0 (HT pure) under the followign conditions
	- all STAs in the BSS are 20/40 MHz HT in 20/40 MHz BSS or
	- all STAs in the BSS are 20 MHz HT in 20 MHz BSS
Set to 1 (HT non-member protection) if there may be non-HT STAs
	in both the primary and the secondary channel
Set to 2 if only HT STAs are associated in BSS,
	however and at least one 20 MHz HT STA is associated
Set to 3 (HT mixed mode) when one or more non-HT STAs are associated
*/
int hostapd_ht_operation_update(ApInfo_st *pApInfo)
{
	ssv_type_u16 cur_op_mode, new_op_mode;
	int op_mode_changes = 0;

	if (!pApInfo->config.b80211n)
		return 0;

	if (!(pApInfo->ht_op_mode & HT_OPER_OP_MODE_NON_GF_HT_STAS_PRESENT)
	    && pApInfo->num_sta_ht_no_gf) {
		pApInfo->ht_op_mode |= HT_OPER_OP_MODE_NON_GF_HT_STAS_PRESENT;
		op_mode_changes++;
	} else if ((pApInfo->ht_op_mode &
		    HT_OPER_OP_MODE_NON_GF_HT_STAS_PRESENT) &&
		   pApInfo->num_sta_ht_no_gf == 0) {
		pApInfo->ht_op_mode &= ~HT_OPER_OP_MODE_NON_GF_HT_STAS_PRESENT;
		op_mode_changes++;
	}

	if (!(pApInfo->ht_op_mode & HT_OPER_OP_MODE_OBSS_NON_HT_STAS_PRESENT) &&
	    (pApInfo->num_sta_no_ht || pApInfo->olbc_ht)) {
		pApInfo->ht_op_mode |= HT_OPER_OP_MODE_OBSS_NON_HT_STAS_PRESENT;
		op_mode_changes++;
	} else if ((pApInfo->ht_op_mode &
		    HT_OPER_OP_MODE_OBSS_NON_HT_STAS_PRESENT) &&
		   (pApInfo->num_sta_no_ht == 0 && !pApInfo->olbc_ht)) {
		pApInfo->ht_op_mode &= ~HT_OPER_OP_MODE_OBSS_NON_HT_STAS_PRESENT;
		op_mode_changes++;
	}

	if (pApInfo->num_sta_no_ht)
		new_op_mode = HT_PROT_NON_HT_MIXED;
	else if (pApInfo->config.secondary_channel && pApInfo->num_sta_ht_20mhz)
		new_op_mode = HT_PROT_20MHZ_PROTECTION;
	else if (pApInfo->olbc_ht)
		new_op_mode = HT_PROT_NONMEMBER_PROTECTION;
	else
		new_op_mode = HT_PROT_NO_PROTECTION;

	cur_op_mode = pApInfo->ht_op_mode & HT_OPER_OP_MODE_HT_PROT_MASK;
	if (cur_op_mode != new_op_mode) {
		pApInfo->ht_op_mode &= ~HT_OPER_OP_MODE_HT_PROT_MASK;
		pApInfo->ht_op_mode |= new_op_mode;
		op_mode_changes++;
	}

	return op_mode_changes;
}

ssv_type_u16 copy_sta_ht_capab(ApInfo_st *pApInfo, APStaInfo_st *sta,
		      const ssv_type_u8 *ht_capab)
{
	/*
	 * Disable HT caps for STAs associated to no-HT BSSes, or for stations
	 * that did not specify a valid WMM IE in the (Re)Association Request
	 * frame.
	 */
	if (!ht_capab ||
	    !(sta->_flags & WLAN_STA_WMM) || pApInfo->config.disable_11n) {
		sta->_flags &= ~WLAN_STA_HT;
        OS_MemSET((void *)&sta->ht_capabilities,0,sizeof(struct ieee80211_ht_capabilities));
		return WLAN_STATUS_SUCCESS;
	}

	sta->_flags |= WLAN_STA_HT;
	OS_MemCPY((void *)&sta->ht_capabilities, ht_capab,
		  sizeof(struct ieee80211_ht_capabilities));

	return WLAN_STATUS_SUCCESS;
}

static void update_sta_ht(ApInfo_st *pApInfo, APStaInfo_st *sta)
{
	ssv_type_u16 ht_capab;

	ht_capab = le_to_host16(sta->ht_capabilities.ht_capabilities_info);
	if ((ht_capab & HT_CAP_INFO_GREEN_FIELD) == 0) {
		if (!sta->no_ht_gf_set) {
			sta->no_ht_gf_set = 1;
			pApInfo->num_sta_ht_no_gf++;
		}
	}
	if ((ht_capab & HT_CAP_INFO_SUPP_CHANNEL_WIDTH_SET) == 0) {
		if (!sta->ht_20mhz_set) {
			sta->ht_20mhz_set = 1;
			pApInfo->num_sta_ht_20mhz++;
		}

	}

}


static void update_sta_no_ht(ApInfo_st *pApInfo, APStaInfo_st *sta)
{
	if (!sta->no_ht_set) {
		sta->no_ht_set = 1;
		pApInfo->num_sta_no_ht++;
	}
}


void update_ht_state(ApInfo_st *pApInfo, APStaInfo_st *sta)
{
	if (sta->_flags & WLAN_STA_HT)
		update_sta_ht(pApInfo, sta);
	else
		update_sta_no_ht(pApInfo, sta);

	if (hostapd_ht_operation_update(pApInfo) > 0)
    {   
        ssv_type_u16 operation_mode = host_to_le16(pApInfo->ht_op_mode);
        _ssv6xxx_wifi_ioctl_Ext(SSV6XXX_HOST_CMD_SET_HT_PROTECT, &operation_mode, sizeof(ssv_type_bool),TRUE,FALSE);      
		ieee802_11_set_beacon(pApInfo,TRUE);//update beacon
    }
}
#endif
