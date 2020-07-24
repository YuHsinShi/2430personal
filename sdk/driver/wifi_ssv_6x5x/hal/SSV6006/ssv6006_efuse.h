#ifndef _SSV6006_EFUSE_H_
#define _SSV6006_EFUSE_H_

#define EFUSE_MAX_SECTION_MAP (EFUSE_HWSET_MAX_SIZE>>5)
#define SSV_EFUSE_POWER_SWITCH_BASE	0xc0000328

#define EFUSE_HWSET_MAX_SIZE (256-32)	//224bit
#define SSV_EFUSE_ID_READ_SWITCH   0xC2000128
#define SSV_EFUSE_ID_RAW_DATA_BASE 0xC200014C
#define SSV_EFUSE_READ_SWITCH	0xc200012c
#define SSV_EFUSE_RAW_DATA_BASE	0xc2000150


#if 0
enum efuse_data_item 
{
   EFUSE_R_CALIBRATION_RESULT = 1,
   EFUSE_SAR_RESULT,
   EFUSE_MAC,
   EFUSE_CRYSTAL_FREQUENCY_OFFSET,
   EFUSE_TX_POWER_INDEX_1,
   EFUSE_TX_POWER_INDEX_2,
   EFUSE_CHIP_ID,
   EFUSE_PKG_INDICATOR,
   EFUSE_USB_VID,
   EFUSE_USB_PID,
};
#else   //work around
enum efuse_data_item 
{
   EFUSE_R_CALIBRATION_RESULT = 1,
   EFUSE_SAR_RESULT,
   EFUSE_RESERVED_MAC,  //work around for usb romcode
   EFUSE_CRYSTAL_FREQUENCY_OFFSET,
   EFUSE_TX_POWER_INDEX_1,
   EFUSE_TX_POWER_INDEX_2,
   EFUSE_CHIP_ID,
   EFUSE_PKG_INDICATOR,
   EFUSE_USB_VID,
   EFUSE_USB_PID,
   EFUSE_MAC,
   EFUSE_RATE_GAIN_B_N40,
   EFUSE_RATE_GAIN_G_N20
};
#endif

struct ssv6xxx_efuse_cfg {
    ssv_type_u32     r_calbration_result;
    ssv_type_u32     sar_result;
    ssv_type_u32     crystal_frequency_offset;
    //u16 iq_calbration_result;
    ssv_type_u32     tx_power_index_1;
    ssv_type_u32     tx_power_index_2;
    ssv_type_u32     chip_identity;
};

struct icomm_efuse_map 
{
    ssv_type_u8 offset;
    ssv_type_u8 bit_cnts;
    ssv_type_u16 value;
};

typedef union USB_VPID {
  ssv_type_u8  u8_d[2];			
  ssv_type_u16 u16_d;
} usb_pvid;

ssv_type_u32 ssv6006_hal_read_chip_id(void);
int ssv6006_hal_read_efuse_macaddr(ssv_type_u8* mcdr);
int ssv6006_hal_write_efuse_macaddr(ssv_type_u8* mcdr);
int ssv6006_hal_dump_efuse_data(void);
int ssv6006_hal_efuse_read_usb_vid(ssv_type_u16 *pvid);
int ssv6006_hal_efuse_read_usb_pid(ssv_type_u16 *ppid);
int ssv6006_hal_efuse_read_tx_power1(ssv_type_u8* pvalue);
int ssv6006_hal_efuse_read_tx_power2(ssv_type_u8* pvalue);
int ssv6006_hal_efuse_read_xtal(ssv_type_u8* pxtal);
int ssv6006_hal_efuse_write_xtal(ssv_type_u8 xtal);
int ssv6006_hal_efuse_read_rate_gain_b_n40(ssv_type_u8 * pvalue);
int ssv6006_hal_efuse_write_rate_gain_b_n40(ssv_type_u8 value);
int ssv6006_hal_efuse_read_rate_gain_g_n20(ssv_type_u8 * pvalue);
int ssv6006_hal_efuse_write_rate_gain_g_n20(ssv_type_u8 value);

#endif /* _SSV6006_EFUSE_H_ */

