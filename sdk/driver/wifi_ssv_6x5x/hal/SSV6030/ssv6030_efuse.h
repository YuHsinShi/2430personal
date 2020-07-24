#ifndef _SSV6030_EFUSE_H_
#define _SSV6030_EFUSE_H_

#define EFUSE_MAX_SECTION_MAP (EFUSE_HWSET_MAX_SIZE>>5)
#define SSV_EFUSE_POWER_SWITCH_BASE	0xc0000328

#define EFUSE_HWSET_MAX_SIZE (256-32)	//224bit
#define SSV_EFUSE_ID_READ_SWITCH   0xC2000128
#define SSV_EFUSE_ID_READY_CHECK   0xC2000148
#define SSV_EFUSE_ID_RAW_DATA_BASE 0xC200014C
#define SSV_EFUSE_READ_SWITCH	0xc200012c
#define SSV_EFUSE_RAW_DATA_BASE	0xc2000150


enum efuse_data_item {
    EFUSE_R_CALIBRATION_RESULT = 1,
    EFUSE_SAR_RESULT,
    EFUSE_MAC,
    EFUSE_CRYSTAL_FREQUENCY_OFFSET,
    EFUSE_IQ_CALIBRATION_RESULT,	//orig is marked
    EFUSE_TX_POWER_INDEX_1,
    EFUSE_TX_POWER_INDEX_2,
    EFUSE_CHIP_IDENTITY
};

struct ssv6xxx_efuse_cfg {
    ssv_type_u32     r_calbration_result;
    ssv_type_u32     sar_result;
    ssv_type_u32     crystal_frequency_offset;
    //u16 iq_calbration_result;
    ssv_type_u32     tx_power_index_1;
    ssv_type_u32     tx_power_index_2;
    ssv_type_u32     chip_identity;
};

struct efuse_map {
    ssv_type_u8 offset;
    ssv_type_u8 byte_cnts;
    ssv_type_u16 value;
};

ssv_type_u32 ssv6030_hal_read_chip_id(void);
int ssv6030_hal_read_efuse_macaddr(ssv_type_u8* mcdr);

#endif /* _SSV6030_EFUSE_H_ */

