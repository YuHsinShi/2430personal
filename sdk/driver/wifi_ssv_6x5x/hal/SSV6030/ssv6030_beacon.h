#ifndef _SSV6030_BEACON_H_
#define _SSV6030_BEACON_H_


int ssv6030_hal_soc_set_bcn(enum ssv6xxx_tx_extra_type extra_type, void *frame, struct cfg_bcn_info *bcn_info, ssv_type_u8 dtim_cnt, ssv_type_u16 bcn_itv);
int ssv6030_hal_beacon_enable(ssv_type_bool bEnable);
ssv_type_bool ssv6030_hal_is_beacon_enable(void);
int ssv6030_hal_beacon_get_len(void *frame);
int ssv6030_hal_beacon_set(void* beacon_skb, int dtim_offset);

#endif /* _SSV6030_BEACON_H_ */

