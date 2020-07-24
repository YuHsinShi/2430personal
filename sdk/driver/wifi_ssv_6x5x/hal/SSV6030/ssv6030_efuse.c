#define SSV6030_EFUSE_C
#include <host_config.h>

#if((CONFIG_CHIP_ID==SSV6051Q)||(CONFIG_CHIP_ID==SSV6051Z)||(CONFIG_CHIP_ID==SSV6030P))
#include <config.h>
#include <rtos.h>
#include "ssv6030_hal.h"
#include "regs/ssv6051_reg.h"
#include "ssv6030_efuse.h"
#include <ssv_ether.h>
#include <ssv_dev.h>

struct efuse_map SSV_EFUSE_ITEM_TABLE[] = {
    {4, 0, 0},
    {4, 8, 0},
    {4, 8, 0},
    {4, 48, 0},//Mac address
    {4, 8, 0},
    //{4, 8, 0},
    {4, 12, 0},//EFUSE_IQ_CALIBRAION_RESULT
    {4, 8, 0},
    {4, 8, 0},
    {4, 8, 0},
};

#if 0
static void _ssv6030_hal_efuse_power_switch(u32 on)
{
    u32 temp_value;
    MAC_REG_READ(SSV_EFUSE_POWER_SWITCH_BASE,temp_value);

    temp_value&=(~(0x000000ff));

    if(on)
    {
        temp_value|=0x11;
    }
    else
    {
        temp_value|=0x0a;
    }

    MAC_REG_WRITE(SSV_EFUSE_POWER_SWITCH_BASE,temp_value);
}
#endif
static ssv_type_u8 _ssv6030_hal_read_efuse(ssv_type_u8 *pbuf)
{
    ssv_type_u32 *pointer32, i;
    pointer32 = (ssv_type_u32 *)pbuf;

    //_ssv6030_hal_efuse_power_switch(1);

    MAC_REG_WRITE(SSV_EFUSE_READ_SWITCH,0x1);
    MAC_REG_READ(SSV_EFUSE_RAW_DATA_BASE,*pointer32);
    if (*pointer32 == 0x00)
    {
        return 0;
    }

    /*get 8 section value*/
    for (i=0; i<EFUSE_MAX_SECTION_MAP; i++, pointer32++)
    {
        MAC_REG_WRITE(SSV_EFUSE_READ_SWITCH+i*4,0x1);
        MAC_REG_READ(SSV_EFUSE_RAW_DATA_BASE+i*4,*pointer32);
    }

    //_ssv6030_hal_efuse_power_switch(0);
    return 1;
}

static ssv_type_u16 _ssv6030_hal_parser_efuse(ssv_type_u8 *pbuf, ssv_type_u8 *mac_addr)
{
    ssv_type_u8 *rtemp8,idx=0;
    ssv_type_u16 shift=0,i;
    ssv_type_u16 efuse_real_content_len = 0;
    ssv_type_u8 temp1,temp2;

    rtemp8 = pbuf;

    if (*rtemp8 == 0x00)
    {
        return efuse_real_content_len;
    }

    do
    {
        /*get efuse index, EX: 3=mac address*/
        idx = (*(rtemp8) >> shift)&0xf;
        switch(idx)
        {
            ssv_type_u16 value = 0;
            //1 byte type
            case EFUSE_R_CALIBRATION_RESULT:
            case EFUSE_CRYSTAL_FREQUENCY_OFFSET:
            case EFUSE_TX_POWER_INDEX_1:
            case EFUSE_TX_POWER_INDEX_2:
            case EFUSE_SAR_RESULT:
            case EFUSE_CHIP_IDENTITY:
            if(shift)
            {
                rtemp8 ++;
                OS_MemCPY((void*)&value,(void*)rtemp8,2);
                SSV_EFUSE_ITEM_TABLE[idx].value = (ssv_type_u16)((ssv_type_u8)(value) & ((1<< SSV_EFUSE_ITEM_TABLE[idx].byte_cnts) - 1));
            }
            else
            {
                OS_MemCPY((void*)&value,(void*)rtemp8,2);
                SSV_EFUSE_ITEM_TABLE[idx].value = (ssv_type_u16)((ssv_type_u8)(value >> 4) & ((1<< SSV_EFUSE_ITEM_TABLE[idx].byte_cnts) - 1));
            }
            efuse_real_content_len += (SSV_EFUSE_ITEM_TABLE[idx].offset + SSV_EFUSE_ITEM_TABLE[idx].byte_cnts);
            break;

            case EFUSE_MAC:
            if(shift)
            {
                rtemp8 ++;
                OS_MemCPY(mac_addr,rtemp8,6);
            }
            else
            {
                for(i=0;i<6;i++)
                {
                    temp1=(*((ssv_type_u8*)rtemp8)>>4)&0x0F;
                    rtemp8 ++;
                    temp2 =(*((ssv_type_u8*)rtemp8)<<4)&0xF0;

                    mac_addr[i]=temp2|temp1;
                }
            }
            efuse_real_content_len += (SSV_EFUSE_ITEM_TABLE[idx].offset + SSV_EFUSE_ITEM_TABLE[idx].byte_cnts);
            break;

            default:
            idx = 0;
            break;
        }

        shift = efuse_real_content_len % 8;
        rtemp8 = &pbuf[efuse_real_content_len / 8];

    }while(idx != 0);

    return efuse_real_content_len;
}

/******************************************************
EFUSE_SPI_RD0_EN	0xc2000128  //data write form bit 31 (MSB)
EFUSE_SPI_RD1_EN	0xc200012c  //data write form bit 0 (LSB)
EFUSE_SPI_RD2_EN	0xc2000130  //data write form bit 0 (LSB)
EFUSE_SPI_RD3_EN	0xc2000134  //data write form bit 0 (LSB)
EFUSE_SPI_RD4_EN	0xc2000138  //data write form bit 0 (LSB)
EFUSE_SPI_RD5_EN	0xc200013c  //data write form bit 0 (LSB)
EFUSE_SPI_RD6_EN	0xc2000140  //data write form bit 0 (LSB)
EFUSE_SPI_RD7_EN	0xc2000144  //data write form bit 0 (LSB)

Example: C200012C=1FEDCBA3; C2000130=00065432
              index: 3
              MAC address=65.43.21.FE.DC.BA
Data format: index+data



*******************************************************/

ssv_type_u32 ssv6030_hal_read_chip_id(void)
{
    ssv_type_u32 val=0;
    ssv_type_u8  cnt=10;
    MAC_REG_WRITE(SSV_EFUSE_ID_READ_SWITCH,0x1);
    while(1)
    {
        MAC_REG_READ(SSV_EFUSE_ID_READY_CHECK,val);
        if(( val == 0x0)||(cnt==0)){
            break;
        }else{
            OS_TickDelay(1);
            cnt--;
            //LOG_PRINTF("ID_READY_CHECK");
        }
    }
    MAC_REG_READ(SSV_EFUSE_ID_RAW_DATA_BASE,val);
    return val;
}


int ssv6030_hal_read_efuse_macaddr(ssv_type_u8* mcdr)
{
	//struct ssv6xxx_efuse_cfg efuse_cfg;
    ssv_type_u8 efuse_mapping_table[EFUSE_HWSET_MAX_SIZE/8];

    OS_MemSET(mcdr,0x00,ETH_ALEN);
    OS_MemSET(efuse_mapping_table,0x00,EFUSE_HWSET_MAX_SIZE/8);

    _ssv6030_hal_read_efuse(efuse_mapping_table);

    _ssv6030_hal_parser_efuse(efuse_mapping_table,mcdr);
	#if 0
    efuse_cfg.r_calbration_result = (u8)SSV_EFUSE_ITEM_TABLE[EFUSE_R_CALIBRATION_RESULT].value;
    efuse_cfg.sar_result = (u8)SSV_EFUSE_ITEM_TABLE[EFUSE_SAR_RESULT].value;
    efuse_cfg.crystal_frequency_offset = (u8)SSV_EFUSE_ITEM_TABLE[EFUSE_CRYSTAL_FREQUENCY_OFFSET].value;
    efuse_cfg.tx_power_index_1 = (u8)SSV_EFUSE_ITEM_TABLE[EFUSE_TX_POWER_INDEX_1].value;
    efuse_cfg.tx_power_index_2 = (u8)SSV_EFUSE_ITEM_TABLE[EFUSE_TX_POWER_INDEX_2].value;
    efuse_cfg.chip_identity    = (u8)SSV_EFUSE_ITEM_TABLE[EFUSE_CHIP_IDENTITY].value;
    #endif
    return 0;

}

#endif //#if((CONFIG_CHIP_ID==SSV6051Q)||(CONFIG_CHIP_ID==SSV6051Z)||(CONFIG_CHIP_ID==SSV6030P))
