#define SSV6006_EFUSE_C
#include <host_config.h>
#if((CONFIG_CHIP_ID==SSV6006B)||(CONFIG_CHIP_ID==SSV6006C))
#include <rtos.h>
#include "regs/ssv6006_hw_regs.h"
#include "ssv6006_efuse.h"
#include <ssv_ether.h>
#include <ssv_dev.h>
#include <ssv_drv.h>
#include <log.h>

static struct icomm_efuse_map ICOMM_EFUSE_ITEM_TABLE[] = {
	{4, 0, 0}, //No data (Efuse default value)
	{4, 8, 0}, //R calibration result
	{4, 8, 0}, //Thermal sensor SAR code result
	{4, 48, 0},//Mac address
	{4, 8, 0}, //Crystal frequency offset register
	{4, 8, 0}, //TX power index_1 register
	{4, 8, 0}, //TX power index_2 register
	{4, 4, 0}, //Chip identity
	{4, 4, 0}, //Package indicator
	{4, 16, 0}, //USB VID
	{4, 16, 0}, //USB PID
	{4, 48, 0},//Mac address	
	{4, 8, 0 }, //rate gain b n40
	{4, 8, 0 }, //rate gain g n20
};


signed char hal_efuse_write(ssv_type_u8 *data, ssv_type_u8 data_length) {

    signed char ret = 0x0;
    ssv_type_u8 i = 0x0;
    ssv_type_u8 loop = 0x0;
    ssv_type_u32 temp_value = 0x0;
    ssv_type_u32 align_4_byte = 0x0;
    ssv_type_u32 efuse_start_addr = 0x0;
    
    efuse_start_addr = ADR_EFUSE_WDATA_0_0;//&(HW_EFS_REG->EFUSE_WDATA_0_0);
    loop = data_length / 4;
    align_4_byte = (data_length % 4)?(data_length % 4):0;

    // clean efuse data buffer
    for(i = 0;i < 8; i++) { 
        MAC_REG_WRITE(efuse_start_addr + (i * 4), 0);
    }
    
    for(i = 0;i < loop; i++) { 
        temp_value = (data[(i * 4)+3] << 24) + (data[(i * 4)+2] << 16) + (data[(i * 4)+1] << 8) + (data[(i * 4)] << 0);
        MAC_REG_WRITE(efuse_start_addr + (i * 4), temp_value);
    }

    temp_value = 0;
    for (i = 0; i < align_4_byte; i++) {
        temp_value += data[loop * 4 + i] << (i * 8);
        MAC_REG_WRITE(efuse_start_addr + (loop * 4), temp_value);
    }
    
    //temp_value = HW_CSR_TU_PMU->PMU_REG_3;
    //temp_value |= (1 << 16);    
    //HW_CSR_TU_PMU->PMU_REG_3 = temp_value; 
    SET_RG_EN_LDO_EFUSE(1);

    //HW_EFS_REG->EFUSE_VDDQ_EN = 0x01;
    SET_EFS_VDDQ_EN(1);

    //HW_EFS_REG->EFUSE_WR_KICK = 0x01;
    SET_EFS_WR_KICK(0x01);

    do {            
        temp_value = GET_EFS_PROGRESS_DONE;//HW_EFS_REG->EFUSE_STATUS2;        
    } while (0 == temp_value);

    //temp_value = HW_CSR_TU_PMU->PMU_REG_3;
    //temp_value &= ~(1 << 16);    
    //HW_CSR_TU_PMU->PMU_REG_3 = temp_value;
    SET_RG_EN_LDO_EFUSE(0);

    //HW_EFS_REG->EFUSE_VDDQ_EN = 0x00;
    SET_EFS_VDDQ_EN(0);
    
    return ret;
}

signed char hal_efuse_read(ssv_type_u8 *data, ssv_type_u8 data_length) {

    signed char ret = 0x0;
    ssv_type_u8 i = 0x0;
    ssv_type_u8 loop = 0x0;
    ssv_type_u32 align_4_byte = 0x0;
    ssv_type_u32 temp_value = 0x0;
    ssv_type_u32 efuse_start_addr = 0x0;
    
    efuse_start_addr = ADR_EFUSE_WDATA_0_0;//&(HW_EFS_REG->EFUSE_WDATA_0_0);

    loop = data_length / 4;
    align_4_byte = (data_length % 4)?(data_length % 4):0;

    //HW_EFS_REG->EFUSE_RD_KICK = 0x01;
    SET_EFS_RD_KICK(1);

    do {            
        temp_value = GET_EFS_PROGRESS_DONE;//HW_EFS_REG->EFUSE_STATUS2;        
    } while (0 == temp_value);
    
    for(i = 0;i < loop; i++) { 
        temp_value = REG32(efuse_start_addr + (i * 4));
        data[i*4+0] = (temp_value >>  0) - ((temp_value >>  8) << 8);
        data[i*4+1] = (temp_value >>  8) - ((temp_value >> 16) << 8);
        data[i*4+2] = (temp_value >> 16) - ((temp_value >> 24) << 8); 
        data[i*4+3] = (temp_value >> 24) - 0x0/*((temp_value >> 32) << 8)*/; 
    }

    temp_value = REG32(efuse_start_addr + (loop * 4));
    
    for (i = 0; i < align_4_byte; i++) {
        data[loop * 4 + i] = (temp_value >>  (i * 8)) - ((temp_value >>  ((i + 1) * 8)) << 8);
    }

    //HW_EFS_REG->EFUSE_VDDQ_EN = 0x00;
    SET_EFS_VDDQ_EN(0);
    return ret;
}

#define EFUSE_BUFFER_BITS_SIZE 256
#define EFUSE_BUFFER_SIZE 32
ssv_type_u8 efuse_buf[EFUSE_BUFFER_SIZE] =  {0};
ssv_type_u16 efuse_real_content_len = 0;
ssv_type_u8* efuse_mapping_table = &efuse_buf[0];

int read_efuse_buf(void)
{    
    
    int ret = -1;
#if 1    
    ret = hal_efuse_read(efuse_buf, EFUSE_BUFFER_SIZE);

    /*
        LOG_PRINTF("Read efuse data: "); 
        for(i=0; i<EFUSE_BUFFER_SIZE; i++)
        {
            LOG_PRINTF("%02x ", efuse_buf[i]);
        }
        LOG_PRINTF("\n"); 
	*/
#else
    {
        int i=0;
        static u8 efuse_test_data[32] = {0x00, 0x00, 0x00, 0x7a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
                                         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
                                         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        i = 0;
        ret = 0;
        if(efuse_buf[3] == 0)
        {
            LOG_PRINTF("load test data\r\n");
            OS_MemSET(efuse_buf, 0, EFUSE_BUFFER_SIZE);
            OS_MemCPY(efuse_buf, efuse_test_data, EFUSE_BUFFER_SIZE);
        }
        LOG_PRINTF("Read efuse data: "); 
        for(i=0; i<EFUSE_BUFFER_SIZE; i++)
        {
            LOG_PRINTF("%02x ", efuse_buf[i]);
        }
        LOG_PRINTF("\r\n");
    }
#endif
    return ret;    
}
int write_efuse_buf(void)
{
    int ret = -1;
#if 1        
    ret = hal_efuse_write(efuse_buf, EFUSE_BUFFER_SIZE);
#else
	int i = 0;
    i = 0;
    ret = 0;
    LOG_PRINTF("Write efuse data: ");
    for(i=0; i<EFUSE_BUFFER_SIZE; i++)
    {
        LOG_PRINTF("%02x ", efuse_buf[i]);
    }
    LOG_PRINTF("\r\n");
#endif
    return ret;    
}

int parse_efuse_buf(void)
{
    //int i =0;
    ssv_type_u8 *rtemp8, idx = 0;
    ssv_type_u16 shift = 0;
    //ssv_type_u16 value;

    if(read_efuse_buf() != 0)
    {
        return -1;
    }
        
    efuse_real_content_len = 32;    //chip id section 
    rtemp8 = efuse_buf+efuse_real_content_len/8;   //skip 4 bytes for chip id section
    
    if(*rtemp8 == 0x00)
    {
        return efuse_real_content_len;
    }

    do
    {
        idx = (*(rtemp8) >> shift)&0xf;
        switch(idx)
        {
            case EFUSE_R_CALIBRATION_RESULT:
            case EFUSE_SAR_RESULT:
            case EFUSE_CRYSTAL_FREQUENCY_OFFSET:
            case EFUSE_TX_POWER_INDEX_1:
            case EFUSE_TX_POWER_INDEX_2:
            case EFUSE_RATE_GAIN_B_N40:
            case EFUSE_RATE_GAIN_G_N20:
                //LOG_PRINTF("Find 8 bit index = %d\rn", idx);
                efuse_real_content_len += (ICOMM_EFUSE_ITEM_TABLE[idx].offset + ICOMM_EFUSE_ITEM_TABLE[idx].bit_cnts);
                break;
                
            case EFUSE_CHIP_ID:
            case EFUSE_PKG_INDICATOR:
                //LOG_PRINTF("Find 4 bit index = %d\r\n", idx);
                efuse_real_content_len += (ICOMM_EFUSE_ITEM_TABLE[idx].offset + ICOMM_EFUSE_ITEM_TABLE[idx].bit_cnts);
                break;
                
            case EFUSE_USB_VID:
            case EFUSE_USB_PID:
                //LOG_PRINTF("Find 16 bit index = %d\r\n", idx);
                efuse_real_content_len += (ICOMM_EFUSE_ITEM_TABLE[idx].offset + ICOMM_EFUSE_ITEM_TABLE[idx].bit_cnts);
                break;
                
            case EFUSE_MAC:
                //LOG_PRINTF("Find 48 bit index = %d\r\n", idx);
                efuse_real_content_len += (ICOMM_EFUSE_ITEM_TABLE[idx].offset + ICOMM_EFUSE_ITEM_TABLE[idx].bit_cnts);
                break;
                
            default:
                idx = 0;
                break;
            
        }        
        shift = efuse_real_content_len %8;
        rtemp8 = &efuse_buf[efuse_real_content_len/8];
        //LOG_PRINTF("shift = %d, content_len = %d, loc = %d\r\n", shift, efuse_real_content_len, efuse_real_content_len/8);
    }while(idx !=0);

    //LOG_PRINTF("parse_efuse_buf return %d\r\n", efuse_real_content_len);
    return efuse_real_content_len;

}
void efuse_write_8(ssv_type_u32 address, ssv_type_u8 value)
{

    if(address)
        efuse_mapping_table[address] = value | efuse_mapping_table[address];
    else
        efuse_mapping_table[address] = value;
}

void efuse_write_16(ssv_type_u32 address, ssv_type_u16 value)
{
    ssv_type_u8 *pointer;
    pointer = (ssv_type_u8 *)(address);

    efuse_write_8(address,(value>>0)&0xff);
    efuse_write_8((address+1),(value>>8)&0xff);
}

void efuse_write_32(ssv_type_u32 address, ssv_type_u32 value)
{
    
    efuse_write_8(address,(value>>0)&0xff);
    efuse_write_8((address+1),(value>>8)&0xff);
    efuse_write_8((address+2),(value>>16)&0xff);
    efuse_write_8((address+3),(value>>24)&0xff);
}

int read_efuse_item(ssv_type_u8 item, ssv_type_u8 *pbuf)
{
    int i = 0;
    int result = -1;
    ssv_type_u8 *rtemp8, idx = 0;
    ssv_type_u16 shift = 0;
    ssv_type_u16 value;
    efuse_real_content_len = 32;
    
    rtemp8 = efuse_buf+efuse_real_content_len/8;   //skip 4 bytes for chip id section

    //LOG_PRINTF("read_efuse_item = %d\r\n", item);
    //get chip id 
    if (item == EFUSE_CHIP_ID)
    {
        if( ((efuse_buf[3]>>4) & 0xf) == EFUSE_CHIP_ID)
        {
            result = 0;
            OS_MemCPY(pbuf, efuse_buf, 4);
        }
        else
        {
            LOG_PRINTF("No chip id\r\n");
        }
        return result;
    }
    
    if(*rtemp8 == 0x00)
    {
        return result;
    }
                
    do
    {
        idx = (*(rtemp8) >> shift)&0xf;
        switch(idx)
        {
            //16 bit data
            case EFUSE_USB_VID:
            case EFUSE_USB_PID:
                if(item == idx)
                {             
                    result = 0;
                    if(shift)
                    {
                        rtemp8 ++;
                        OS_MemCPY(pbuf, rtemp8, 2);
                        //LOG_PRINTF("%02x %02x\n", pbuf[0], pbuf[1]);
                    }
                    else
                    {   
                        for(i=0;i<2;i++)
                        {
                            value= *rtemp8 | (*(rtemp8+1) << 8);
                            //u8 mac1= (value>>4)&0xff;
                            pbuf[i] =  (value>>4)&0xff;
                            //LOG_PRINTF("%02x ",  pbuf[i]);                     
                            rtemp8++;
                        }   
                        //LOG_PRINTF("\n");
                    }
                }
                efuse_real_content_len += (ICOMM_EFUSE_ITEM_TABLE[idx].offset + ICOMM_EFUSE_ITEM_TABLE[idx].bit_cnts);             
                break;                

            //8 bit data    
            case EFUSE_TX_POWER_INDEX_1:
            case EFUSE_TX_POWER_INDEX_2:
            case EFUSE_RATE_GAIN_B_N40:
            case EFUSE_RATE_GAIN_G_N20:
            case EFUSE_CRYSTAL_FREQUENCY_OFFSET:
                if(item == idx)
                {
                    result = 0;
                    if(shift)
                    {
                        rtemp8 ++;
                        OS_MemCPY(pbuf, rtemp8, 1);
                        //LOG_PRINTF("%02x %02x %02x %02x %02x %02x\n", pbuf[0], pbuf[1], pbuf[2], pbuf[3], pbuf[4], pbuf[5]);
                    }
                    else
                    {   
                        for(i=0;i<1;i++)
                        {
                            value= *rtemp8 | (*(rtemp8+1) << 8);
                            //u8 mac1= (value>>4)&0xff;
                            pbuf[i] =  (value>>4)&0xff;
                            //LOG_PRINTF("%02x ",  pbuf[i]);                     
                            rtemp8++;
                        }   
                        //LOG_PRINTF("\n");
                    }
                }
                efuse_real_content_len += (ICOMM_EFUSE_ITEM_TABLE[idx].offset + ICOMM_EFUSE_ITEM_TABLE[idx].bit_cnts);         
                break;
                
            case EFUSE_MAC:
                if(item == idx)
                {
                    result = 0;
                    if(shift)
                    {
                        rtemp8 ++;
                        OS_MemCPY(pbuf, rtemp8, 6);
                        //LOG_PRINTF("%02x %02x %02x %02x %02x %02x\r\n", pbuf[0], pbuf[1], pbuf[2], pbuf[3], pbuf[4], pbuf[5]);
                    }
                    else
                    {   
                        for(i=0;i<6;i++)
                        {
                            value= *rtemp8 | (*(rtemp8+1) << 8);
                            //u8 mac1= (value>>4)&0xff;
                            pbuf[i] =  (value>>4)&0xff;
                            //LOG_PRINTF("%02x ",  pbuf[i]);                     
                            rtemp8++;
                        }   
                        //LOG_PRINTF("\r\n");
                    }
                }
                    efuse_real_content_len += (ICOMM_EFUSE_ITEM_TABLE[idx].offset + ICOMM_EFUSE_ITEM_TABLE[idx].bit_cnts);
                break;
                
            default:
                idx = 0;
                break;
            
        }
      
        shift = efuse_real_content_len %8;
        rtemp8 = &efuse_buf[efuse_real_content_len/8];
        //LOG_PRINTF("shift = %d, content_len = %d, loc = %d\r\n", shift, efuse_real_content_len, efuse_real_content_len/8);
    }while(idx !=0);
    
    return result;
}

int write_efuse_item(ssv_type_u8 idx, ssv_type_u8 *value)
{       
    //int i=0;
    int ret = -1;
    ssv_type_u8 value8=0;
    ssv_type_u16 shift=0, value16=0;
    ssv_type_u32 value32=0;

    if((efuse_real_content_len + ICOMM_EFUSE_ITEM_TABLE[idx].offset + ICOMM_EFUSE_ITEM_TABLE[idx].bit_cnts) > EFUSE_BUFFER_BITS_SIZE)
    {
        LOG_PRINTF("There is not enough space, %d!!\n", EFUSE_BUFFER_BITS_SIZE);
        return ret;
    }

    shift = efuse_real_content_len % 8;
    LOG_PRINTF("write_efuse_item = %d, efuse_real_content_len = %d\r\n", idx, efuse_real_content_len);
    switch(idx)
    {
        //8 bit data
        case EFUSE_R_CALIBRATION_RESULT:                 //   1
        case EFUSE_SAR_RESULT:                                  //   2
        case EFUSE_CRYSTAL_FREQUENCY_OFFSET:        //   4           
        case EFUSE_TX_POWER_INDEX_1:                        // 5
        case EFUSE_TX_POWER_INDEX_2:                        // 6
        case EFUSE_RATE_GAIN_B_N40:
        case EFUSE_RATE_GAIN_G_N20:
            if(shift)
            {
                value8 = value[0] ;
                value16 = (idx << 4) | ((value8) << 8);
            }
            else
            {
                value8 = value[0] ;
                value16 = (idx << 0) | ((value8) << 4);
            }
            efuse_write_16(efuse_real_content_len / 8,value16);
            efuse_real_content_len += (ICOMM_EFUSE_ITEM_TABLE[idx].offset + ICOMM_EFUSE_ITEM_TABLE[idx].bit_cnts);
            break;

        //16 bit data    
        case EFUSE_USB_VID:                                         // 9
        case EFUSE_USB_PID:                                         // 10
            if(shift)
            {
                value16 = (value[1] << 8) | value[0] ;
                value32 = (idx << 4) | ((value16) << 8);
            }
            else
            {
                value16 = (value[1] << 8) | value[0] ;
                value32 = (idx << 0) | ((value16) << 4);
            }
            efuse_write_32(efuse_real_content_len / 8,value32);
            efuse_real_content_len += (ICOMM_EFUSE_ITEM_TABLE[idx].offset + ICOMM_EFUSE_ITEM_TABLE[idx].bit_cnts);
            break;

        // 4 bit data
        case EFUSE_PKG_INDICATOR:                           // 7
        case EFUSE_CHIP_ID:                                       // 8
            LOG_PRINTF("not implement now\n");
            break;
        case EFUSE_MAC:                                             //  3
            if(shift)
                value8 = (idx << 4);
            else
                value8 = (idx >> 0);
            
            efuse_write_8((efuse_real_content_len / 8), value8);
            efuse_real_content_len += ICOMM_EFUSE_ITEM_TABLE[idx].offset;
            
            shift = efuse_real_content_len % 8;    
         
            if(shift)
            {
                //value32 = (*(u32 *)value << 4);
                //value32 = value32 | (u32)value8;
                value32 = 0;
                OS_MemCPY((ssv_type_u8*)&value32, value, 4);
                value32 = value32<<4;
            }
            else
            {
                //value32 = (*(u32 *)value << 0);
                value32 = 0;
                OS_MemCPY((ssv_type_u8*)&value32, value, 4);
            } 
            efuse_write_32(efuse_real_content_len / 8,value32);
            efuse_real_content_len += 32;

            if(shift)
                value += 3;
            else
                value += 4;                   

            shift = efuse_real_content_len % 8;
            if(shift)
            {
                //value32 = (*(u32 *)value >> 4);
                value32 = 0;
                OS_MemCPY((ssv_type_u8*)&value32, value, 3);
                value32 = value32>>4;
            }
            else
            {
                //value32 = (*(u32 *)value << 0);
                value32 = 0;
                OS_MemCPY((ssv_type_u8*)&value32, value, 2);
            }        
            efuse_write_32(efuse_real_content_len / 8,value32);
            efuse_real_content_len += 16;
        
			break;

		default:
			break;
        }
    
    ret = write_efuse_buf();

    return ret;
}



//export API
int efuse_get_free_bits(void)
{

    int efuse_free_bits = -1;
    efuse_real_content_len = parse_efuse_buf();
    efuse_free_bits = 256 - efuse_real_content_len;
    return efuse_free_bits;       
}
int efuse_dump_data(void)
{

    int i = 0;
    int len = parse_efuse_buf();

    LOG_PRINTF("efuse free bits = %d\r\n", efuse_get_free_bits());
    LOG_PRINTF("Dump Efuse Data:\r\n");
    for(i=0;i<32;i++)
    {
        LOG_PRINTF("%02x ", efuse_buf[i]);
    }
    LOG_PRINTF("\r\n");
    
    return len;
}

int efuse_write_mac(ssv_type_u8 *mac_addr)
{
    int ret = -1;
    efuse_real_content_len = parse_efuse_buf();
    ret = write_efuse_item(EFUSE_MAC, mac_addr);
    return ret;    
}
int efuse_write_chip_id(ssv_type_u32 chip_id)
{
    LOG_PRINTF("efuse_write_chip_id isn't supprot now!!!\n");
    return -1;
}
int efuse_write_usb_vid(ssv_type_u16 usb_vid)
{
    int ret = -1;
    usb_pvid vid;
    vid.u16_d = usb_vid;
    parse_efuse_buf();
    ret = write_efuse_item(EFUSE_USB_VID, (ssv_type_u8*)vid.u8_d);
    return ret;
}
int efuse_write_usb_pid(ssv_type_u16 usb_pid)
{
    int ret = -1;
    usb_pvid pid;
    pid.u16_d = usb_pid;    
    parse_efuse_buf();
    ret = write_efuse_item(EFUSE_USB_PID, (ssv_type_u8*)pid.u8_d);
    return ret;
}
int efuse_write_tx_power1(ssv_type_u8 value)
{
    int ret = -1; 
    parse_efuse_buf();
    ret = write_efuse_item(EFUSE_TX_POWER_INDEX_1, &value);
    return ret;
}
int efuse_write_tx_power2(ssv_type_u8 value)
{
    int ret = -1; 
    parse_efuse_buf();
    ret = write_efuse_item(EFUSE_TX_POWER_INDEX_2, &value);
    return ret;
}
int efuse_write_xtal(ssv_type_u8 xtal)
{
    int ret = -1; 
    parse_efuse_buf();
    ret = write_efuse_item(EFUSE_CRYSTAL_FREQUENCY_OFFSET, &xtal);
    return ret;

}


int efuse_read_mac(ssv_type_u8 *mac_addr)
{
    int ret = -1;
    parse_efuse_buf();
    ret = read_efuse_item(EFUSE_MAC, mac_addr);
    return ret;
}
int efuse_read_mac_increase(ssv_type_u8 *mac_addr)
{
    int ret = -1;
    ssv_type_u8 efuse_mac[6] = {0};
    parse_efuse_buf();
    ret = read_efuse_item(EFUSE_MAC, efuse_mac);
    if(ret == 0)
    {
#if 0
        mac_addr[0] = efuse_mac[0];
        mac_addr[1] = efuse_mac[1] | 0x2;
        mac_addr[2] = efuse_mac[2];
        mac_addr[3] = efuse_mac[3];
        mac_addr[4] = efuse_mac[4];
        mac_addr[5] = efuse_mac[5];
#else    
        mac_addr[0] = efuse_mac[0];
        mac_addr[1] = efuse_mac[1];
        mac_addr[2] = efuse_mac[2];
        mac_addr[3] = efuse_mac[3];
        mac_addr[4] = efuse_mac[4];
        if (efuse_mac[5]&0x1 )
            mac_addr[5] = efuse_mac[5] - 1;    
        else        
            mac_addr[5] = efuse_mac[5] + 1;      
#endif        
    }
    
    return ret;
}
int efuse_read_chip_id(ssv_type_u32 *p_chip_id)
{
    int ret = -1;
    parse_efuse_buf();
    ret = read_efuse_item(EFUSE_CHIP_ID, (ssv_type_u8*)p_chip_id);
    return ret;
}
int efuse_read_usb_vid(ssv_type_u16 *pvid)
{
    int ret = -1;
    usb_pvid vid;
    vid.u16_d = *pvid;
    parse_efuse_buf();
    ret = read_efuse_item(EFUSE_USB_VID, (ssv_type_u8*)vid.u8_d);
    *pvid = vid.u16_d;
    return ret;
}
int efuse_read_usb_pid(ssv_type_u16 *ppid)
{
    int ret = -1;
    usb_pvid pid;
    pid.u16_d = *ppid;    
    parse_efuse_buf();
    ret = read_efuse_item(EFUSE_USB_PID, (ssv_type_u8*)pid.u8_d);
    *ppid = pid.u16_d;
    return ret;
}
int efuse_read_tx_power1(ssv_type_u8* pvalue)
{
    int ret = -1;
    parse_efuse_buf();
    ret = read_efuse_item(EFUSE_TX_POWER_INDEX_1, pvalue);
    return ret;
}
int efuse_read_tx_power2(ssv_type_u8* pvalue)
{

    int ret = -1;
    parse_efuse_buf();
    ret = read_efuse_item(EFUSE_TX_POWER_INDEX_2, pvalue);
    return ret;
}
int efuse_read_xtal(ssv_type_u8* pxtal)
{
    int ret = -1;
    parse_efuse_buf();
    ret = read_efuse_item(EFUSE_CRYSTAL_FREQUENCY_OFFSET, pxtal);
    return ret;

}

int efuse_write_rate_gain_b_n40(ssv_type_u8 value)
{
    int ret = -1;
    parse_efuse_buf();
    ret = write_efuse_item(EFUSE_RATE_GAIN_B_N40, &value);
    return ret;
}
int efuse_write_rate_gain_g_n20(ssv_type_u8 value)
{
    int ret = -1;
    parse_efuse_buf();
    ret = write_efuse_item(EFUSE_RATE_GAIN_G_N20, &value);
    return ret;
}
int efuse_read_rate_gain_b_n40(ssv_type_u8* pvalue)
{
    int ret = -1;
    parse_efuse_buf();
    ret = read_efuse_item(EFUSE_RATE_GAIN_B_N40, pvalue);
    return ret;
}
int efuse_read_rate_gain_g_n20(ssv_type_u8* pvalue)
{

    int ret = -1;
    parse_efuse_buf();
    ret = read_efuse_item(EFUSE_RATE_GAIN_G_N20, pvalue);
    return ret;
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

ssv_type_u32 ssv6006_hal_read_chip_id(void)
{
    ssv_type_u32 val=0;
    efuse_read_chip_id(&val);
    return val;
}


int ssv6006_hal_read_efuse_macaddr(ssv_type_u8* mcdr)
{
    efuse_read_mac(mcdr);
    return 0;

}

int ssv6006_hal_write_efuse_macaddr(ssv_type_u8* mcdr)
{
    efuse_write_mac(mcdr);
    return 0;
}

int ssv6006_hal_dump_efuse_data(void)
{
    return efuse_dump_data();
}

int ssv6006_hal_efuse_read_usb_vid(ssv_type_u16 *pvid)
{
    return efuse_read_usb_vid(pvid);
}
int ssv6006_hal_efuse_read_usb_pid(ssv_type_u16 *ppid)
{
    return efuse_read_usb_pid(ppid);
}
int ssv6006_hal_efuse_read_tx_power1(ssv_type_u8* pvalue)
{
    return efuse_read_tx_power1(pvalue);
}
int ssv6006_hal_efuse_read_tx_power2(ssv_type_u8* pvalue)
{
    return efuse_read_tx_power2(pvalue);
}
int ssv6006_hal_efuse_read_xtal(ssv_type_u8* pxtal)
{
    return efuse_read_xtal(pxtal);
}
int ssv6006_hal_efuse_write_xtal(ssv_type_u8 xtal)
{
    return efuse_write_xtal(xtal);
}

int ssv6006_hal_efuse_write_rate_gain_b_n40(ssv_type_u8 value)
{
    return efuse_write_rate_gain_b_n40(value);
}
int ssv6006_hal_efuse_write_rate_gain_g_n20(ssv_type_u8 value)
{
    return efuse_write_rate_gain_g_n20(value);
}
int ssv6006_hal_efuse_read_rate_gain_b_n40(ssv_type_u8* pvalue)
{
    return efuse_read_rate_gain_b_n40(pvalue);
}
int ssv6006_hal_efuse_read_rate_gain_g_n20(ssv_type_u8* pvalue)
{
    return efuse_read_rate_gain_g_n20(pvalue);
}


#endif //#if((CONFIG_CHIP_ID==SSV6006B)||(CONFIG_CHIP_ID==SSV6006C))
