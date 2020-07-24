#ifndef _SSV6006_BEACON_H_
#define _SSV6006_BEACON_H_

struct ssv6xxx_beacon_info {
	ssv_type_u32 pubf_addr;
	ssv_type_u16 allocat_len;
	ssv_type_u8 tim_offset;
	ssv_type_u8 tim_cnt;
};

int ssv6006_hal_soc_set_bcn(enum ssv6xxx_tx_extra_type extra_type, void *frame, struct cfg_bcn_info *bcn_info, ssv_type_u8 dtim_cnt, ssv_type_u16 bcn_itv);
int ssv6006_hal_beacon_enable(ssv_type_bool bEnable,ssv_type_bool bFreePbuf,ssv_type_bool block);
ssv_type_bool ssv6006_hal_is_beacon_enable(void);
int ssv6006_hal_beacon_get_len(void *frame);
int ssv6006_hal_beacon_set(void* beacon_skb, int dtim_offset, ssv_type_bool block);

#endif /* _SSV6006_BEACON_H_ */

