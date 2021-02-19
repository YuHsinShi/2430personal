#define WIFI_GLOBALS


#include <stdarg.h>
#include <string.h>

#include "alt_cpu/homebus/homebus.h"
#include "alt_cpu/alt_cpu_utility.h"

#include "wifi.h"

//typedef short uint16_t;
typedef unsigned char uint8_t;
extern unsigned char option_hh;
extern unsigned char total_machine;
extern unsigned char  basedata[16][50];
extern unsigned char tempset_min_cool_rx;
extern unsigned char tempset_max_cool_rx;

extern unsigned char tempset_min_warm_rx;
extern unsigned char tempset_max_warm_rx;

extern unsigned char winddir_enable;
extern unsigned char save_used;
extern unsigned char mute_used;
extern unsigned char sleep_used;
extern unsigned char health_used;
extern unsigned char heat_used;
extern unsigned char forest_wind_used;
extern unsigned char self_clean_used;
extern unsigned char power_wind_flag_used;
extern unsigned char human_used;
extern unsigned char human_sensor_used;
//wind_auto_flag
extern unsigned char machine_type1;
extern unsigned char wind_enable;
//mode_check_flag
extern unsigned char system_status;
extern unsigned char tempset;
extern unsigned char dry_set;
extern unsigned char system_mode;
extern unsigned char system_wind;

extern unsigned char wind_board_status;
extern unsigned char wind_board_angle;
extern unsigned char wind_board2_status;
extern unsigned char wind_board2_angle;
extern unsigned char wind_horizontal_status;
extern unsigned char wind_horizontal_angle;
extern unsigned char wind_horizontal2_status;
extern unsigned char wind_horizontal2_angle;
extern unsigned char save_flag;
extern unsigned char mute_flag;
extern unsigned char sleep_flag;
extern unsigned char health_flag;
extern unsigned char heat_flag;
extern unsigned char forest_wind_flag;
extern unsigned char self_clean_flag;
extern unsigned char human_flag;
extern unsigned char wind_mode;
extern unsigned char power_wind_flag;
extern unsigned char filter_flag;
extern unsigned char center_control_all;
extern unsigned char center_control_onoff;
extern unsigned char center_control_mode;
extern unsigned char center_control_wind;
extern unsigned char center_control_windboard;
extern unsigned char center_control_tempset;
extern unsigned char tempset_write;
extern unsigned char dry_set_write;
extern unsigned char system_mode_write;
extern unsigned char system_wind_write;


extern unsigned char option_data[16][50];
extern unsigned char wind_board_angle_write;
extern unsigned char wind_board_status_write;
extern unsigned char wind_board2_angle_write;
extern unsigned char wind_horizontal_angle_write;
extern unsigned char wind_horizontal2_angle_write;
extern unsigned char wind_horizontal2_status_write;
extern unsigned char save_flag_write;
extern unsigned char mute_flag_write;
extern unsigned char pre_off_flag;
extern unsigned char option_preoff;
extern unsigned char pre_off_time;
extern unsigned char pre_off_time_write;
extern unsigned char pre_off_time_b;
extern unsigned char pre_off_time_b_write;
extern unsigned char try_run_flag;
extern unsigned char try_run_set;
extern unsigned char sleep_flag_write;
extern unsigned char health_flag_write;
extern unsigned char heat_flag_write;
extern unsigned char wind_mode_write;
extern unsigned char forest_wind_flag_write;
extern unsigned char self_clean_flag_b;
extern unsigned char self_clean_flag_b_write;
extern unsigned char try_run_set_write;
extern unsigned char self_clean_flag_write;
extern unsigned char human_flag_write;
extern unsigned char wind_board2_status_write;
extern unsigned char wind_horizontal_status_write;
extern unsigned char pre_off_flag_write;


unsigned char option_d1;
unsigned char compress_preheat_flag;
unsigned char heating_start;
unsigned char defrost_flag;
unsigned char option_l1;


unsigned char wind_auto_flag;
unsigned char mode_check_flag;

#define Start_APP2_Adress	0xa00000	//I/
unsigned char  ota_cmd_rx_flag;

unsigned char  wifi_tx_delay_cnt;

extern unsigned char err_now_flag;
extern unsigned char tx_change_flag;



const unsigned char wifi_const_data[] = 
{ 
 // 0x30,0x00,0x49,0xBD,0xB7,0x1D,0xC6,0x66,0x3F,0x08,0xC6,0x00,0x03,0x00,0x00,0x00,0x61,0x00,0x00,0x43,0x04,0xD3,0x43,0x04,0xD3,0x01,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x16,0x16,0x16,0x16,0x01,0x01,0x01,0x01,0xFF,0xFF,0x00,0xFF,0xFF,0x01,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x49,0xBD,0xB7,0x1D,0xC6,0x66,0x3F,0x08,0x01,0x01,0x00,0x00,0x00,0x00,0x16,0x16,0x16,0x16,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x16,0x00,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
 0x30,0x00,0x49,0xBD,0xB7,0x1D,0xC6,0x66,0x3F,0x08,0xC6,0x00,0x03,0x00,0x00,0x00,0x61,0x00,0x00,0x43,0x04,0xD3,0x43,0x04,0xD3,0x01,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x16,0x16,0x16,0x16,0x01,0x01,0x01,0x01,0xFF,0xFF,0x00,0xFF,0xFF,0x01,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x49,0xBD,0xB7,0x1D,0xC6,0x66,0x3F,0x08,0x01,0x01,0x00,0x00,0x00,0x00,0x16,0x16,0x16,0x16,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x16,0x00,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 
};
// 低字节CRC值表
const unsigned char auchCRCHi[] = 
{ 
  0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 
  0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 
  0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 
  0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 
  0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 
  0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 
  0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 
  0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
  0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 
  0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 
  0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 
  0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 
  0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 
  0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 
  0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 
  0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 
  0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 
  0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 
  0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 
  0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 
  0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 
  0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 
  0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 
  0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 
  0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 
  0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 
  0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 
  0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 
  0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 
  0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 
  0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 
  0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40 
} ; 

// 高字节CRC值表
const unsigned char auchCRCLo[] = 
{ 
  0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 
  0xC6, 0x06, 0x07, 0xC7, 0x05, 0xC5, 0xC4, 0x04, 
  0xCC, 0x0C, 0x0D, 0xCD, 0x0F, 0xCF, 0xCE, 0x0E, 
  0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09, 0x08, 0xC8, 
  0xD8, 0x18, 0x19, 0xD9, 0x1B, 0xDB, 0xDA, 0x1A, 
  0x1E, 0xDE, 0xDF, 0x1F, 0xDD, 0x1D, 0x1C, 0xDC,
  0x14, 0xD4, 0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6, 
  0xD2, 0x12, 0x13, 0xD3, 0x11, 0xD1, 0xD0, 0x10, 
  0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3, 0xF2, 0x32, 
  0x36, 0xF6, 0xF7, 0x37, 0xF5, 0x35, 0x34, 0xF4, 
  0x3C, 0xFC, 0xFD, 0x3D, 0xFF, 0x3F, 0x3E, 0xFE, 
  0xFA, 0x3A, 0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38, 
  0x28, 0xE8, 0xE9, 0x29, 0xEB, 0x2B, 0x2A, 0xEA, 
  0xEE, 0x2E, 0x2F, 0xEF, 0x2D, 0xED, 0xEC, 0x2C, 
  0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26, 
  0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0,
  0xA0, 0x60, 0x61, 0xA1, 0x63, 0xA3, 0xA2, 0x62, 
  0x66, 0xA6, 0xA7, 0x67, 0xA5, 0x65, 0x64, 0xA4, 
  0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F, 0x6E, 0xAE, 
  0xAA, 0x6A, 0x6B, 0xAB, 0x69, 0xA9, 0xA8, 0x68, 
  0x78, 0xB8, 0xB9, 0x79, 0xBB, 0x7B, 0x7A, 0xBA, 
  0xBE, 0x7E, 0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C, 
  0xB4, 0x74, 0x75, 0xB5, 0x77, 0xB7, 0xB6, 0x76, 
  0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71, 0x70, 0xB0, 
  0x50, 0x90, 0x91, 0x51, 0x93, 0x53, 0x52, 0x92, 
  0x96, 0x56, 0x57, 0x97, 0x55, 0x95, 0x94, 0x54,  
  0x9C, 0x5C, 0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E, 
  0x5A, 0x9A, 0x9B, 0x5B, 0x99, 0x59, 0x58, 0x98, 
  0x88, 0x48, 0x49, 0x89, 0x4B, 0x8B, 0x8A, 0x4A, 
  0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C, 
  0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 
  0x82, 0x42, 0x43, 0x83, 0x41, 0x81, 0x80, 0x40 
}; 

// 函数返回值是无符号短整型CRC值,待进行CRC校验计算的报文,待校验的报文长度
uint16_t CalCrcRTU(unsigned char *Buff_addr,uint16_t len)  /* CRC 16校验查表法 */
{                                              

  unsigned char uchCRCHi = 0xFF;                     // CRC高字节的初始化
  unsigned char uchCRCLo = 0xFF;                     // CRC低字节的初始化
  uint16_t  uIndex;                                  // CRC查找表的指针

  while (len--)								
  {  
    uIndex   = uchCRCHi ^ *Buff_addr++;              // 计算CRC
    uchCRCHi = uchCRCLo ^ auchCRCHi[uIndex];
    uchCRCLo = auchCRCLo[uIndex];
  }
  return(uchCRCLo <<8 | uchCRCHi);			         //这里面高低位定义反了
}

/********************************************************************************
********************************************************************************/
uint16_t Cal_Long_CRC(uint16_t crc, unsigned char *Buff_addr,uint16_t len)  /* CRC 16校验查表法 */
{                                              

	unsigned char uchCRCHi = (crc&0xff);		   
	unsigned char uchCRCLo = (crc>>8)&0xff;	

	uint16_t  uIndex;                               

	while (len--)								
	{  
		uIndex   = uchCRCHi ^ *Buff_addr++;             
		uchCRCHi = uchCRCLo ^ auchCRCHi[uIndex];
		uchCRCLo = auchCRCLo[uIndex];
	}
	return(uchCRCLo <<8 | uchCRCHi);
}
uint16_t ota_cur_crc=0xFFFF;

/********************************************************************************
********************************************************************************/

void FLASH_BufferRead(uint32_t flash_update_addr, uint8_t* flash_buf_crc, uint32_t len)
{
//LAW // need porting

}

uint8_t cal_all_flashdata_crc(uint32_t addr,uint32_t length,uint16_t crc)
{
	uint16_t ota_count_msb;
	uint16_t ota_count_lsb;
	//uint16_t ota_cur_crc=0xFFFF;
	uint8_t  flash_buf_crc[256];
    uint32_t flash_update_addr;
	uint16_t i;
	
	ota_count_msb = length / 256;
	ota_count_lsb = length % 256;
	flash_update_addr = addr;
	ota_cur_crc       = 0xFFFF;
	//memset(flash_buf, 0, 256);	
	for(i=0;i<256;i++)
	{
		flash_buf_crc[i]=0;
	}
	
	if (ota_count_msb>0)
	{
		while (ota_count_msb--)
		{
			FLASH_BufferRead(flash_update_addr,flash_buf_crc, 256);
			ota_cur_crc = Cal_Long_CRC (ota_cur_crc, flash_buf_crc, 256); 	
			flash_update_addr+=256;					  
		}
	}
	if(0 != ota_count_lsb)
	{
		FLASH_BufferRead(flash_update_addr,flash_buf_crc, ota_count_lsb);
		ota_cur_crc = Cal_Long_CRC (ota_cur_crc, flash_buf_crc, ota_count_lsb);
	}
	
	if (ota_cur_crc == crc)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}
/********************************************************************************
********************************************************************************/
void write_update_flag(void)
{
	
	unsigned char App_Rx_buf[8],App_Write_Length;
	
	App_Write_Length=0x02;
	/*
	//LAW  need porting //
	prFslStart();
	
	App_Rx_buf[0]=0x11;//update flag 0x11
	App_Rx_buf[1]=ota_get_crc>>8;
	App_Rx_buf[2]=ota_get_crc;
	App_Rx_buf[3]=filetotalbytes/0x1000000;
	App_Rx_buf[4]=filetotalbytes/0x10000;
	App_Rx_buf[5]=filetotalbytes/0x100;
	App_Rx_buf[6]=filetotalbytes%0x100;
	App_Rx_buf[7]=0;


	
	
    prFslErase( 0xfe,1 );
    prFslWrite(update_flag_adress,&App_Rx_buf[0],App_Write_Length);
     prFslEnd();
    */ //END
 

}
/********************************************************************************
********************************************************************************/
void wifi_tx_all_status(unsigned int start_did0,unsigned int lenth)
{
  unsigned int i,j,total_data;
  unsigned int temp_data;
  
  wifi_tx[0]=0x30;
  
 
  if((cloud_servers_cmd>=5)&&(cloud_servers_cmd<18))
  {
    wifi_tx[1]=cloud_servers_seq|0x80;
     for(i=0;i<8;i++)
     {
        wifi_tx[2+i]=unix_time_const[i];
     }
  }
  else
  {
    wifi_tx[1]=0;
    for(i=0;i<8;i++)
    {
        wifi_tx[2+i]=unix_time[i];
    }
  }

  
  total_data=17;
    
 for(i=start_did0;i<(start_did0+lenth);i++)
 {
   temp_data=0;
   switch(i)
  {
  case 0:wifi_tx[total_data++]=0;
         break;
  case 1:
         switch(option_hh) 
         {
         case 0:temp_data=1;break;
         case 1:temp_data=0;break;
         case 2:temp_data=2;break;
         }
         wifi_tx[total_data++]=temp_data;
         break;
  case 2:wifi_tx[total_data++]=0xd3;//C1235
         wifi_tx[total_data++]=0x04;
         wifi_tx[total_data++]='C';
         break;
  case 3:wifi_tx[total_data++]=wifi_moudle_version[0];//0xd3;//C1235
         wifi_tx[total_data++]=wifi_moudle_version[1];//0x04;
         wifi_tx[total_data++]=wifi_moudle_version[2];//'W';
         break;         
  case 4:
         wifi_tx[total_data++]=total_machine;
         
         for(j=0;j<16;j++)
         {
             wifi_tx[total_data++]=basedata[j][1];
             wifi_tx[total_data++]=basedata[j][2];         
         }        
         break;  
  case 5:
         wifi_tx[total_data++]=tempset_min_cool_rx;
         wifi_tx[total_data++]=tempset_max_cool_rx;               
         break;           
  case 6:
         wifi_tx[total_data++]=tempset_min_warm_rx;
         wifi_tx[total_data++]=tempset_max_warm_rx;               
         break;     
  case 7:
         switch(winddir_enable)
         {
         case 0:temp_data=0;break;
         case 1:temp_data=1;break;
         case 2:temp_data=1;break;
         case 3:temp_data=1;break;
         case 4:temp_data=1;break;
         case 5:temp_data=1;break;
         }
         wifi_tx[total_data++]=temp_data;           
         break;     
  case 8:
         switch(winddir_enable)
         {
         case 0:temp_data=0;break;
         case 1:temp_data=0;break;
         case 2:temp_data=1;break;
         case 3:temp_data=1;break;
         case 4:temp_data=1;break;
         case 5:temp_data=1;break;
         }
         wifi_tx[total_data++]=temp_data;      
         break;     
  case 9:
         switch(winddir_enable)
         {
         case 0:temp_data=0;break;
         case 1:temp_data=0;break;
         case 2:temp_data=1;break;
         case 3:temp_data=0;break;
         case 4:temp_data=0;break;
         case 5:temp_data=0;break;
         }
         wifi_tx[total_data++]=temp_data;              
         break;     
  case 10:
         switch(winddir_enable)
         {
         case 0:temp_data=0;break;
         case 1:temp_data=0;break;
         case 2:temp_data=1;break;
         case 3:temp_data=0;break;
         case 4:temp_data=0;break;
         case 5:temp_data=0;break;
         }
         wifi_tx[total_data++]=temp_data;            
         break;     
  case 11:
         if(save_used)temp_data|=0x0001;
         if(mute_used)temp_data|=0x0002;
         if(sleep_used)temp_data|=0x0004;
         if(health_used)temp_data|=0x0008;
         if(heat_used)temp_data|=0x0010;
         if(forest_wind_used)temp_data|=0x0020;
         if(self_clean_used)temp_data|=0x0040;
         //if()temp_data|=0x0080;//加温
         if(power_wind_flag_used)temp_data|=0x0100;
         //if()temp_data|=0x0200;//氛围灯
         //if()temp_data|=0x0400;//新风
         if(winddir_enable==4)temp_data|=0x0800;
         if((winddir_enable==3)||(winddir_enable==4))temp_data|=0x1000;
         if(human_used)temp_data|=0x2000;
         if(human_sensor_used)temp_data|=0x4000;
         
         wifi_tx[total_data++]=temp_data; 
         wifi_tx[total_data++]=temp_data>>8; 
         break;     
  case 12:
         wifi_tx[total_data++]=0;   //预留        
         break;     
  case 13:
         if(winddir_enable)temp_data|=0x0001;
         if((winddir_enable==5)||(winddir_enable==2))temp_data|=0x0002;
         if(winddir_enable==2)temp_data|=0x0004;
         
         
         temp_data|=0x0020;//支持0.5设定
         if(wind_auto_flag)temp_data|=0x0040;
         temp_data|=0x0080;//遥控器有
         
         if(machine_type1==4)temp_data|=0x0700;//三管制 DC除湿机
         if(machine_type1==3)temp_data|=0x0400;//AC除湿机
         
    
         wifi_tx[total_data++]=temp_data; 
         wifi_tx[total_data++]=temp_data>>8;        
         break;     
  case 14:
         switch(wind_enable)
         {
         case 0:temp_data=0x23;break;//
         case 1:temp_data=0x23;break;//
         case 2:temp_data=0x03;break;//
         case 3:temp_data=0x03;break;//
         case 4:temp_data=0x00;break;//
         case 5:temp_data=0x00;break;//
         }
         wifi_tx[total_data++]=temp_data;           
         break;     
  case 15:
         if((mode_check_flag&0x02)==0)temp_data|=0x01;
         if((mode_check_flag&0x04)==0)temp_data|=0x02;
         if((mode_check_flag&0x08)==0)temp_data|=0x04;
         if((mode_check_flag&0x10)==0)temp_data|=0x08;
         if((mode_check_flag&0x01)==0)temp_data|=0x10;
         
         wifi_tx[total_data++]=temp_data;           
         break;     
  case 16:
         wifi_tx[total_data++]=0;           
         break;     
  case 17:
         switch(option_hh) 
         {
         case 0:temp_data=1;break;
         case 1:temp_data=0;break;
         case 2:temp_data=2;break;
         }
         wifi_tx[total_data++]=temp_data;    
         break;     
  case 18:
         wifi_tx[total_data++]=system_status;           
         break;     
  case 19:
         wifi_tx[total_data++]=tempset;           
         break;     
  case 20:
         wifi_tx[total_data++]=dry_set;           
         break;     
  case 21:
         switch(system_mode)
          {
          case 0x01:temp_data=0x01;break; 
          case 0x02:temp_data=0x02;break; 
          case 0x04:temp_data=0x04;break; 
          case 0x08:temp_data=0x08;break; 
          case 0x10:temp_data=0x10;break; 
          case 0x44:temp_data=0x20;break;
          default:break;
          }
         wifi_tx[total_data++]=temp_data; 
         wifi_tx[total_data++]=temp_data>>8; 
         wifi_tx[total_data++]=0;
         break;     
  case 22:
         wifi_tx[total_data++]=0;     //预留       
         break;     
  case 23:
        if(wind_enable>3)
        {
          switch(system_wind)
          {
          case 0x01:temp_data=0x01;break; 
          case 0x18:temp_data=0x02;break; 
          case 0x08:temp_data=0x04;break; 
          case 0x14:temp_data=0x08;break; 
          case 0x04:temp_data=0x10;break; 
          case 0x02:temp_data=0x20;break;
          case 0x12:temp_data=0x40;break;
          }
        }
        else
        {
           switch(system_wind)
          {
          case 0x01:temp_data=0x01;break; 
          case 0x08:temp_data=0x08;break; 
          case 0x04:temp_data=0x10;break; 
          case 0x02:temp_data=0x20;break;
          case 0x12:temp_data=0x40;break;
          }
        }
         wifi_tx[total_data++]=temp_data;           
         break;     
  case 24:
         if(wind_board_status)
           temp_data=0x01;
         else
           temp_data=(wind_board_angle-1)<<1;
         
         wifi_tx[total_data++]=temp_data;           
         break;     
  case 25:
         if(wind_board2_status)
           temp_data=0x01;
         else
           temp_data=(wind_board2_angle-1)<<1;
         
         wifi_tx[total_data++]=temp_data;            
         break;  
  case 26:
         if(wind_horizontal_status)
           temp_data=0x01;
         else
           temp_data=(wind_horizontal_angle-1)<<1;
         
         wifi_tx[total_data++]=temp_data;        
         break;  
  case 27:
         if(wind_horizontal2_status)
           temp_data=0x01;
         else
           temp_data=(wind_horizontal2_angle-1)<<1;
         
         wifi_tx[total_data++]=temp_data;        
         break;  
  case 28:
         if(save_flag)temp_data=1;
         wifi_tx[total_data++]=temp_data;           
         break;  
  case 29:
         if(mute_flag)temp_data=1;
         wifi_tx[total_data++]=temp_data;          
         break;  
  case 30:
         if(sleep_flag)temp_data=1;
         wifi_tx[total_data++]=temp_data;             
         break;  
  case 31:
         if(health_flag)temp_data=1;
         wifi_tx[total_data++]=temp_data;         
         break;  
  case 32:
         if(heat_flag)temp_data=1;
         wifi_tx[total_data++]=temp_data;              
         break;  
  case 33:
         if(forest_wind_flag)temp_data=1;
         wifi_tx[total_data++]=temp_data;           
         break;  
  case 34:
         if(self_clean_flag)temp_data=1;
         wifi_tx[total_data++]=temp_data;             
         break;  
  case 35://加湿
         //if()temp_data=1;
         wifi_tx[total_data++]=temp_data;        
         break;  
  case 36:
         temp_data=human_flag;
         wifi_tx[total_data++]=temp_data;              
         break;  
  case 37:
         temp_data=0;//人感2
         wifi_tx[total_data++]=temp_data;        
         break;  
  case 38:
         if(wind_mode==2)temp_data=1;
         wifi_tx[total_data++]=temp_data;        
         break;  
  case 39:
         if(wind_mode==1)temp_data=1;
         wifi_tx[total_data++]=temp_data;         
         break;  
  case 40:
         if(power_wind_flag)temp_data=1;
         wifi_tx[total_data++]=temp_data;              
         break;  
  case 41:
         if(filter_flag)temp_data=1;
         wifi_tx[total_data++]=temp_data;       
         break;  
  case 42://新风
         
         wifi_tx[total_data++]=0;       
         break;  
  case 43://氛围灯
         wifi_tx[total_data++]=0;           
         break;  
  case 44://按键锁
         wifi_tx[total_data++]=0;           
         break;  
  case 45://指示灯模式
         wifi_tx[total_data++]=0;           
         break;  
  case 46://无极指示灯
         wifi_tx[total_data++]=0;           
         break;  
  case 47://背光源模式
         wifi_tx[total_data++]=0;           
         break;  
  case 48://无极背光源
         wifi_tx[total_data++]=0;           
         break;  
  case 49://背光时长
         wifi_tx[total_data++]=0;           
         break;  
  case 50://按键音
         wifi_tx[total_data++]=0;           
         break;  
  case 51://关机显示内容
         wifi_tx[total_data++]=0;           
         break;  
  case 52://
         if(center_control_all)temp_data=1;
         wifi_tx[total_data++]=temp_data;             
         break;  
  case 53:
         if(center_control_onoff)temp_data=1;
         wifi_tx[total_data++]=temp_data;           
         break;  
  case 54:
         if(center_control_mode)temp_data=1;
         wifi_tx[total_data++]=temp_data;               
         break;  
  case 55:
         if(center_control_wind)temp_data=1;
         wifi_tx[total_data++]=temp_data;         
         break;  
  case 56:
         if(center_control_windboard)temp_data=1;
         wifi_tx[total_data++]=temp_data;           
         break;  
  case 57:
         if(center_control_tempset)temp_data=1;
         wifi_tx[total_data++]=temp_data;                 
         break;  
  case 58://option_d1=1时自启动
         if(option_d1==0)temp_data=1;
         wifi_tx[total_data++]=temp_data;      
         break;  
  case 59:
         if(compress_preheat_flag||heating_start)temp_data=1;
         wifi_tx[total_data++]=temp_data;          
         break;  
  case 60:
         if(defrost_flag)temp_data=1;
         wifi_tx[total_data++]=temp_data;           
         break;  
  case 61:
         wifi_tx[total_data++]=cloud_servers_61;  
         cloud_servers_61=0;
         break;         
  case 62:
         wifi_tx[total_data++]=cloud_servers_62;   
         cloud_servers_62=0;
         break;         
  case 63:
         wifi_tx[total_data++]=unix_time[0];  
         wifi_tx[total_data++]=unix_time[1];   
         wifi_tx[total_data++]=unix_time[2];   
         wifi_tx[total_data++]=unix_time[3];   
         wifi_tx[total_data++]=unix_time[4];   
         wifi_tx[total_data++]=unix_time[5];   
         wifi_tx[total_data++]=unix_time[6];   
         wifi_tx[total_data++]=unix_time[7];      
         break;         
  case 64:
         wifi_tx[total_data++]=cloud_servers_64;           
         break;          
  case 65:
         wifi_tx[total_data++]=0;  //wifi线控器
         switch(option_hh) 
         {
         case 0:temp_data=1;break;
         case 1:temp_data=0;break;
         case 2:temp_data=2;break;
         }
         wifi_tx[total_data++]=temp_data;
         wifi_tx[total_data++]=cloud_servers_65;
         break;          
  case 66:
         wifi_tx[total_data++]=0;           
         break;          
  case 67:
         switch(option_hh) 
         {
         case 0:temp_data=1;break;
         case 1:temp_data=0;break;
         case 2:temp_data=2;break;
         }
         wifi_tx[total_data++]=temp_data;        
         break;          
  case 68://滤网
         wifi_tx[total_data++]=0;    
         wifi_tx[total_data++]=0;  
         wifi_tx[total_data++]=0; 
         wifi_tx[total_data++]=0; 
         wifi_tx[total_data++]=0; 
         break;    
  case 69://室外温湿度
         wifi_tx[total_data++]=0;    
         wifi_tx[total_data++]=0;  
         break;    
  case 70://室内温湿度
         wifi_tx[total_data++]=0;    
         wifi_tx[total_data++]=0;  
         break;    
  case 71://室外PM2.5
         wifi_tx[total_data++]=0;    
         wifi_tx[total_data++]=0;  
         break;    
  case 72://室内PM2.5
         wifi_tx[total_data++]=0;    
         wifi_tx[total_data++]=0;  
         break;    
  case 73://室外CO2
         wifi_tx[total_data++]=0;    
         wifi_tx[total_data++]=0;  
         break;    
  case 74://室内CO2
         wifi_tx[total_data++]=0;    
         wifi_tx[total_data++]=0;  
         break;    
  case 75://室内TVOC
         wifi_tx[total_data++]=0;    
         wifi_tx[total_data++]=0;  
         break;    
  case 76://新风温度
         wifi_tx[total_data++]=0;    
         
         break;           
  case 77://新风PM2.5
         wifi_tx[total_data++]=0;    
         wifi_tx[total_data++]=0;  
         break;           
  case 78:
        if(err_now_flag)temp_data=option_data[0][10];
        else temp_data=0x00;
         wifi_tx[total_data++]=temp_data;    
         wifi_tx[total_data++]=basedata[0][1];  
         wifi_tx[total_data++]=basedata[0][2];  
         break;          
  case 79:
         if(err_now_flag)temp_data=option_data[1][10];
         else temp_data=0x00;
         wifi_tx[total_data++]=temp_data;    
         wifi_tx[total_data++]=basedata[1][1];  
         wifi_tx[total_data++]=basedata[1][2];   
         break;          
  case 80:
         if(err_now_flag)temp_data=option_data[2][10];
         else temp_data=0x00;
         wifi_tx[total_data++]=temp_data;    
         wifi_tx[total_data++]=basedata[2][1];  
         wifi_tx[total_data++]=basedata[2][2];   
         break;          
  case 81:
         if(err_now_flag)temp_data=option_data[3][10];
         else temp_data=0x00;
         wifi_tx[total_data++]=temp_data;   
         wifi_tx[total_data++]=basedata[3][1];  
         wifi_tx[total_data++]=basedata[3][2];  
         break;          
  case 82:
         if(err_now_flag)temp_data=option_data[4][10];
         else temp_data=0x00;
         wifi_tx[total_data++]=temp_data;     
         wifi_tx[total_data++]=basedata[4][1];  
         wifi_tx[total_data++]=basedata[4][2];   
         break;          
  case 83:
         if(err_now_flag)temp_data=option_data[5][10];
         else temp_data=0x00;
         wifi_tx[total_data++]=temp_data;      
         wifi_tx[total_data++]=basedata[5][1];  
         wifi_tx[total_data++]=basedata[5][2];  
         break;          
  case 84:
         if(err_now_flag)temp_data=option_data[6][10];
         else temp_data=0x00;
         wifi_tx[total_data++]=temp_data;     
         wifi_tx[total_data++]=basedata[6][1];  
         wifi_tx[total_data++]=basedata[6][2];  
         break;          
  case 85:
         if(err_now_flag)temp_data=option_data[7][10];
         else temp_data=0x00;
         wifi_tx[total_data++]=temp_data;  
         wifi_tx[total_data++]=basedata[7][1];  
         wifi_tx[total_data++]=basedata[7][2];  
         break;          
  case 86:
         if(err_now_flag)temp_data=option_data[8][10];
         else temp_data=0x00;
         wifi_tx[total_data++]=temp_data;  
         wifi_tx[total_data++]=basedata[8][1];  
         wifi_tx[total_data++]=basedata[8][2];   
         break;          
  case 87:
         if(err_now_flag)temp_data=option_data[9][10];
         else temp_data=0x00;
         wifi_tx[total_data++]=temp_data;    
         wifi_tx[total_data++]=basedata[9][1];  
         wifi_tx[total_data++]=basedata[9][2];   
         break;          
  case 88:
         if(err_now_flag)temp_data=option_data[10][10];
         else temp_data=0x00;
         wifi_tx[total_data++]=temp_data;  
         wifi_tx[total_data++]=basedata[10][1];  
         wifi_tx[total_data++]=basedata[10][2];  
         break;          
  case 89:
         if(err_now_flag)temp_data=option_data[11][10];
         else temp_data=0x00;
         wifi_tx[total_data++]=temp_data;      
         wifi_tx[total_data++]=basedata[11][1];  
         wifi_tx[total_data++]=basedata[11][2];  
         break;          
  case 90:
         if(err_now_flag)temp_data=option_data[12][10];
         else temp_data=0x00;
         wifi_tx[total_data++]=temp_data;  
         wifi_tx[total_data++]=basedata[12][1];  
         wifi_tx[total_data++]=basedata[12][2];  
         break;          
  case 91:
         if(err_now_flag)temp_data=option_data[13][10];
         else temp_data=0x00;
         wifi_tx[total_data++]=temp_data;   
         wifi_tx[total_data++]=basedata[13][1];  
         wifi_tx[total_data++]=basedata[13][2];  
         break;          
  case 92:
         if(err_now_flag)temp_data=option_data[14][10];
         else temp_data=0x00;
         wifi_tx[total_data++]=temp_data;    
         wifi_tx[total_data++]=basedata[14][1];  
         wifi_tx[total_data++]=basedata[14][2];  
         break;          
  case 93:
         if(err_now_flag)temp_data=option_data[15][10];
         else temp_data=0x00;
         wifi_tx[total_data++]=temp_data;      
         wifi_tx[total_data++]=basedata[15][1];  
         wifi_tx[total_data++]=basedata[15][2];  
         break;          
  case 94:
         if(option_l1)temp_data=1;
         wifi_tx[total_data++]=temp_data;
         break;          
  case 95://订阅
         wifi_tx[total_data++]=0;    
         break;   
  case 96:
         wifi_tx[total_data++]=0;
         wifi_tx[total_data++]=0;
         wifi_tx[total_data++]=0;
         wifi_tx[total_data++]=0;
         wifi_tx[total_data++]=0;
         wifi_tx[total_data++]=0;
         wifi_tx[total_data++]=0;
         wifi_tx[total_data++]=0;
         break;  
         
   case 97://预留
         wifi_tx[total_data++]=0;    
         break;   
   case 98://线控器设备类型
         wifi_tx[total_data++]=0;    
         break;  
   case 99://品牌
         switch(option_hh) 
         {
         case 0:temp_data=1;break;
         case 1:temp_data=0;break;
         case 2:temp_data=2;break;
         }
         wifi_tx[total_data++]=temp_data;   
         break; 
         
  case 100://请求url
         wifi_tx[total_data++]=cloud_servers_100;  
		 cloud_servers_100=0;
         break;     
 case 101://下载状态
         wifi_tx[total_data++]=cloud_servers_101;    
		 cloud_servers_101=0;
         break;    
 case 102://升级状态
         wifi_tx[total_data++]=cloud_servers_102;  
		 cloud_servers_102=0;
         break;   
 case 103://策略状态
         wifi_tx[total_data++]=cloud_servers_103;  
		 cloud_servers_103=0;  
         break;  
 case 104://策略内容
         for(j=0;j<28;j++)
         {
            wifi_tx[total_data++]=cloud_servers_104[j];    
         }
         break;   
 case 106://升级用户确认
         wifi_tx[total_data++]=cloud_servers_106;  
		 cloud_servers_106=0;
         break;          
 case 107://确认下载
         wifi_tx[total_data++]=cloud_servers_107;  
		 cloud_servers_107=0;
         break;          
 case 108://确认升级
         wifi_tx[total_data++]=cloud_servers_108;   
		 cloud_servers_108=0;
         break;          
         
         
         
  }
  
  
 }
  
  
  wifi_tx[10]=total_data-12;
  wifi_tx[11]=0;
  wifi_tx[12]=cloud_servers_cmd;
  wifi_tx[13]=0;
  wifi_tx[14]=start_did0;
  wifi_tx[15]=0;
  wifi_tx[16]=lenth;
  
  
  wifi_tx_total=total_data+2;
//  nop();
  /*
  wifi_tx_total=212;
  
  for(i=0;i<210;i++)
  {
    wifi_tx[i]=0x30;//wifi_const_data[i];
  }
  */
  
  
  
}
/********************************************************************************
********************************************************************************/
void wifi_tx_status_report(void)
{
  unsigned int i;
  
    wifi_tx[0]=0x30;
    wifi_tx[1]=cloud_servers_seq|0x80;
     for(i=0;i<8;i++)
     {
        wifi_tx[2+i]=unix_time_const[i];
     }
  
  

  
  wifi_tx[10]=0x03;
  wifi_tx[11]=0;
  wifi_tx[12]=cloud_servers_cmd;
  wifi_tx[13]=0;
  wifi_tx[14]=0;

  wifi_tx_total=wifi_tx[10]+12+2;
}
/********************************************************************************
********************************************************************************/
//24 15 04 00 48 69 73 65 6E 73 65 2D 50 43 2D 50 31 48 45 51 32 AB AB 查询
//24 15 04 01 48 69 73 65 6E 73 65 2D 50 43 2D 50 31 48 45 51 32 FA 57 配网
void wifi_tx_moudle(void)
{

  wifi_tx_total=24;
  
  wifi_tx[0]=0x24;
  wifi_tx[1]=0x13;
  wifi_tx[2]=0x00;
  wifi_tx[3]=0x04;
  
  if(wifi_moudle_set)wifi_tx[4]=0x01;
  else wifi_tx[4]=0x00;
  
  wifi_tx[5]=0x48;
  wifi_tx[6]=0x69;
  wifi_tx[7]=0x74;
  wifi_tx[8]=0x61;
  wifi_tx[9]=0x63;
  wifi_tx[10]=0x68;
  wifi_tx[11]=0x69;
  wifi_tx[12]=0x2d;
  wifi_tx[13]=0x50;
  wifi_tx[14]=0x43;
  wifi_tx[15]=0x2d;
  wifi_tx[16]=0x50;
  wifi_tx[17]=0x31;
  wifi_tx[18]=0x48;
  wifi_tx[19]=0x45;
  wifi_tx[20]=0x51;
  wifi_tx[21]=0x32;
  
  wifi_moudle_set=0;
  
}
/********************************************************************************
********************************************************************************/
void wifi_tx_moudle_ota_cmd(void)
{
  unsigned int i;
  
  wifi_tx_total=431+5;
  
  wifi_tx[0]=0x24;
  wifi_tx[1]=431&0x00ff;
  wifi_tx[2]=431>>8;
  wifi_tx[3]=0x09;
  
  for(i=0;i<28;i++)
  {
    wifi_tx[4+i]=cloud_servers_104[i]; 
  }
  
 
  for(i=0;i<402;i++)
  {
    wifi_tx[32+i]=cloud_servers_105[i]; 
  }

 	if(cloud_servers_104[9]==1)//升级对象为1（线控器时）进入升级模式
 	{
  	ota_status_flag=1;
  	ota_status_cnt=0;
 	}
  
  
}
/********************************************************************************
********************************************************************************/
void wifi_tx_moudle_ota_start(void)
{

  wifi_tx_total=2+5;
  
  wifi_tx[0]=0x24;
  wifi_tx[1]=2;
  wifi_tx[2]=0;
  wifi_tx[3]=0x0d; 
  wifi_tx[4]=1;
  

}
/********************************************************************************
********************************************************************************/
void wifi_tx_moudle_ota_data(void)
{

 
  wifi_tx_total=7+5;
  
  wifi_tx[0]=0x24;
  wifi_tx[1]=7;
  wifi_tx[2]=0;
  wifi_tx[3]=0x0e;
  
  wifi_tx[4]=ota_pack_nuber;
  wifi_tx[5]=ota_pack_nuber>>8;
  wifi_tx[6]=ota_pack_nuber>>16;
  wifi_tx[7]=ota_pack_nuber>>24;
  
  wifi_tx[8]=1;
  
  if(ota_rx_byte<filetotalbytes)
  {
    	wifi_tx[9]=1;
  }
  else if(ota_rx_byte==filetotalbytes)
  {
	  ota_get_crc=(cloud_servers_104[19]<<8)+cloud_servers_104[18];
	  //uint8_t cal_all_flashdata_crc(uint32_t addr,uint32_t length,uint16_t crc)
	  if(cal_all_flashdata_crc(Start_APP2_Adress,filetotalbytes,ota_get_crc)==1)
	  {
	  	wifi_tx[9]=2; 
		
		write_update_flag();
		ota_down_finish_cnt=0;
		ota_down_finish_flag=1;
	  }
	  else
	  {
	  	wifi_tx[9]=3; 
		ota_down_finish_cnt=0;
		ota_down_finish_flag=2;
	  }
	  ota_status_flag=0;
      ota_status_cnt=0;
  }   
  else 
  {
    	wifi_tx[9]=3;
  }
  
}
/********************************************************************************
********************************************************************************/
void wifi_tx_data(void)
{

  unsigned int i,crc16;
  
  if(wifi_tx_flag==0)return;
  
  if(wifi_tx_delay_cnt<10)return;
  	wifi_tx_delay_cnt=0;
  
 // if(wifi_tx_start)return;
  
  for(i=0;i<512;i++)
  {
    wifi_tx[0]=0x0;
  }
  
  if(wifi_tx_flag==1)
  {
    wifi_tx_moudle();
    
  }
  else if(wifi_tx_flag==3)
  {
    //wifi_tx_all_status(0,97);
    wifi_tx_all_status(did0_data,did0_lenth);
  }
  else if(wifi_tx_flag==4)
  {
    wifi_tx_status_report();
  }
  
   else if(wifi_tx_flag==5)
  {
    wifi_tx_moudle_ota_cmd();
  }
  else if(wifi_tx_flag==6)
  {
    wifi_tx_moudle_ota_start();
  }
  else if(wifi_tx_flag==7)
  {
    wifi_tx_moudle_ota_data();
  }

  
  
  crc16= CalCrcRTU(wifi_tx,wifi_tx_total-2) ;  
  
  wifi_tx[wifi_tx_total-2]=crc16;
  wifi_tx[wifi_tx_total-1]=crc16>>8;
  
//  wifi_tx_nuber=0;
//  wifi_tx_start=1;
  //LAW
  //send a frame out 
  //TXD2=wifi_tx[wifi_tx_nuber];
	wifi_tx_frame_out(wifi_tx,wifi_tx_total);

  
	wifi_tx_flag=0;
}
/********************************************************************************
********************************************************************************/
void wifi_rx_moudle_information(void)
{
      wifi_signal=wifi_rx[4];
      wifi_connecting=wifi_rx[5];
      
      if((wifi_connect_ok==0)&&(wifi_rx[6]))//收到配网成功，发送全状态
      {
             cloud_servers_cmd=3;
             did0_data=0;
             did0_lenth=97;
             wifi_tx_flag=3;
      }
      wifi_connect_ok=wifi_rx[6];
      
      wifi_moudle_version[0]=wifi_rx[7];
      wifi_moudle_version[1]=wifi_rx[8];
      wifi_moudle_version[2]=wifi_rx[9];
}
/********************************************************************************
********************************************************************************/
void wifi_rx_moudle_ota_start(void)
{
//LAW　Porting issue//
/*
    //ota_flash_erase_flag=1;
    ota_flash_erase_deal();
    
    filetotalbytes=((unsigned long)wifi_rx[7]<<24)+((unsigned long)wifi_rx[6]<<16)+((unsigned long)wifi_rx[5]<<8)+(unsigned long)wifi_rx[4];
    ota_status_cnt=0;
*/
}
/********************************************************************************
********************************************************************************/
void wifi_rx_moudle_ota_data(void)
{ 
      unsigned int i,j,lenth;
  
  lenth=((unsigned int)wifi_rx[9]<<8)+wifi_rx[8];
  // LAW need porting // 
  /*
  for(i=0;i<lenth;i++)
  {
  	 ota_rx_data[ota_rx_nuber++]=wifi_rx[10+i];
  	 ota_rx_byte++;
    if((ota_rx_nuber==1024)||(ota_rx_byte==filetotalbytes))
     {
     	for(j=0;j<1024;j++)
     	{
     		ota_flash_data[j]=ota_rx_data[j];
		ota_rx_data[j]=0xff;
     		
     	}
	//ota_flash_write_flag=1;

	    ota_flash_write_deal();
     	ota_rx_nuber=0;
     }
     
  }
  */
	// ======================== //
    ota_pack_nuber=((unsigned long)wifi_rx[7]<<24)+((unsigned long)wifi_rx[6]<<16)+((unsigned long)wifi_rx[5]<<8)+(unsigned long)wifi_rx[4];
  ota_status_cnt=0;
}
/********************************************************************************
********************************************************************************/
//03 00 B9 88 79 71 74 01 00 00 0F 00 05 01 00 01 3F 00 08 B9 88 79 71 74 01 00 00 7E BF  心跳设定65
void wifi_rx_data_deal(void)
{
  //unsigned int i,j,item_total,item_data,item_lenth;
  unsigned int i,j;

  
  item_total=wifi_rx[15];
  item_lenth=16;
    
  for(i=0;i<item_total;i++)
  {
    item_data=wifi_rx[item_lenth];
    switch(item_data)
    {
    case 18:
          if(wifi_rx[item_lenth+3])
          {
            system_on_deal();
          }
          else
          {
            system_off_deal();
          }
          break;
    case 19:
          tempset=wifi_rx[item_lenth+3];
          tempset_write=1;
          break;
    case 20:
          dry_set=wifi_rx[item_lenth+3];
          dry_set_write=1;
          break;
    case 21:
          switch(wifi_rx[item_lenth+3])
          {
          case 0x01:system_mode=0x01;break; 
          case 0x02:system_mode=0x02;break; 
          case 0x04:system_mode=0x04;break; 
          case 0x08:system_mode=0x08;break; 
          case 0x10:system_mode=0x10;break; 
          }
          system_mode_write=1;
          break;
    case 22:
          break;      
    case 23:
        if(wind_enable>3)
        {
          switch(wifi_rx[item_lenth+3])
          {
          case 0x01:system_wind=0x01;break; 
          case 0x02:system_wind=0x18;break; 
          case 0x04:system_wind=0x08;break; 
          case 0x08:system_wind=0x14;break; 
          case 0x10:system_wind=0x04;break; 
          case 0x20:system_wind=0x02;break;
          case 0x40:system_wind=0x12;break;
          }
        }
        else
        {
          switch(wifi_rx[item_lenth+3])
          {
          case 0x01:system_wind=0x01;break; 
          case 0x08:system_wind=0x08;break; 
          case 0x10:system_wind=0x04;break; 
          case 0x20:system_wind=0x02;break;
          case 0x40:system_wind=0x12;break;
          }
        }
          system_wind_write=1;
          break;
    case 24:
          if(wifi_rx[item_lenth+3]&0x01)
          {
            wind_board_status=1;
            wind_board_angle=1;    
          }
          else
          {
            wind_board_status=0;
            wind_board_angle=(wifi_rx[item_lenth+3]>>1)+1;
          }
          wind_board_angle_write=1;
          wind_board_status_write=1;
          break; 
    case 25:
          if(wifi_rx[item_lenth+3]&0x01)
          {
            wind_board2_status=1;
            wind_board2_angle=1;    
          }
          else
          {
            wind_board2_status=0;
            wind_board2_angle=(wifi_rx[item_lenth+3]>>1)+1;
          }
          wind_board2_angle_write=1;
          wind_board2_status_write=1;
          break;           
    case 26:
          if(wifi_rx[item_lenth+3]&0x01)
          {
            wind_horizontal_status=1;
            wind_horizontal_angle=1;    
          }
          else
          {
            wind_horizontal_status=0;
            wind_horizontal_angle=(wifi_rx[item_lenth+3]>>1)+1;
          }
          wind_horizontal_angle_write=1;
          wind_horizontal_status_write=1;
          break;            
    case 27:
          if(wifi_rx[item_lenth+3]&0x01)
          {
            wind_horizontal2_status=1;
            wind_horizontal2_angle=1;    
          }
          else
          {
            wind_horizontal2_status=0;
            wind_horizontal2_angle=(wifi_rx[item_lenth+3]>>1)+1;
          }
          wind_horizontal2_angle_write=1;
          wind_horizontal2_status_write=1;
          break;   
    case 28:
          if(wifi_rx[item_lenth+3]==1)
          {
            save_flag=1;
          }
          else
          {
            save_flag=0;
          }
          save_flag_write=1;
          break; 
    case 29:
          if(wifi_rx[item_lenth+3]==1)
          {
            mute_flag=1;
          }
          else
          {
            mute_flag=0;
          }
          mute_flag_write=1;
          break; 
    case 30:
          if(wifi_rx[item_lenth+3]==1)
          {
            sleep_flag=1;
                  pre_off_flag=1;
                  pre_off_flag_write=1;
                  if(option_preoff)pre_off_time=option_preoff;
                  else pre_off_time=16;
                  pre_off_time_write=1;
                  pre_off_time_b=pre_off_time;
                  pre_off_time_b_write=1;
                  
                  if(wind_enable>3)system_wind=0x18;
                  else system_wind=0x08;
                  system_wind_write=1;
          }
          else
          {
            sleep_flag=0;
                  if((try_run_flag==0)&&(try_run_set==0)&&(option_preoff==0))
                  {
                    pre_off_flag=0;
                    pre_off_flag_write=1;
                  }
          }
          sleep_flag_write=1;
          break; 
    case 31:
          if(wifi_rx[item_lenth+3]==1)
          {
            health_flag=1;
          }
          else
          {
            health_flag=0;
          }
          health_flag_write=1;
          break; 
    case 32:
          if(wifi_rx[item_lenth+3]==1)
          {
            heat_flag=1;
          }
          else
          {
            heat_flag=0;
          }
          heat_flag_write=1;
          break; 
    case 33:
          if(wifi_rx[item_lenth+3]==1)
          {
            forest_wind_flag=1;
                  if((winddir_enable==3)||(winddir_enable==4))
                  {
                    wind_mode=1;
                    wind_mode_write=1;
                  }
                  else
                  {
                    wind_board_status=1;
                    wind_board_status_write=1;
                    wind_board2_status=1;
                    wind_board2_status_write=1;
                    wind_horizontal_status=1;
                    wind_horizontal_status_write=1;
                    wind_horizontal2_status=1;
                    wind_horizontal2_status_write=1;
                  }
          }
          else
          {
            forest_wind_flag=0;
          }
          forest_wind_flag_write=1;
          break;  
    case 34:
          if(wifi_rx[item_lenth+3]==1)
          {
            self_clean_flag=1;
            self_clean_flag_b=0xffff>>(16-total_machine);
                   self_clean_flag_b_write=1;
                   if(try_run_set)
                   {
                    try_run_set=0;
                    try_run_set_write=1;
                   }
          }
          else
          {
            self_clean_flag=0;
            self_clean_flag_b=0;
            self_clean_flag_b_write=1;
          }
          self_clean_flag_write=1;
          break; 
    case 35:
          
          break;
    case 36:
          switch(wifi_rx[item_lenth+3])
          {
          case 0:human_flag=0;break;
          case 1:human_flag=1;break;
          case 2:human_flag=2;break;
          case 3:human_flag=3;break;
          }
          human_flag_write=1;
          break;
    case 37:
          
          break;
          
    case 38:
          if(wifi_rx[item_lenth+3])
          {
            wind_mode=2;
          }
          else
          {
            if(wind_mode==2)wind_mode=0;
          }
          wind_mode_write=1;
          break;
    case 39:
          if(wifi_rx[item_lenth+3])
          {
            wind_mode=1;
          }
          else
          {
            if(wind_mode==1)wind_mode=0;
          }
          wind_mode_write=1;
          break;          
          
    case 61:          
             cloud_servers_61=wifi_rx[item_lenth+3];
             cloud_servers_cmd=14;
             did0_data=0;
             did0_lenth=97;
             wifi_tx_flag=3;
           break;  
    case 62:  
           
            cloud_servers_62=wifi_rx[item_lenth+3];
            if(cloud_servers_62==2)
            {
             cloud_servers_cmd=13;
             did0_data=0;
             did0_lenth=97;
             wifi_tx_flag=3;
            }
            else if(cloud_servers_62==1)
            {
              wifi_tx_flag=1;
              wifi_moudle_set=1;
            }
           break; 
    case 63:
          for(j=0;j<8;j++)
          {
            unix_time[j]=wifi_rx[item_lenth+3+j];
          }
           break;
    case 64:          
            cloud_servers_64=wifi_rx[item_lenth+3];
           break;
    case 65:          
            cloud_servers_65=wifi_rx[item_lenth+3+2];//cloud_servers_65=wifi_rx[item_lenth+3];
            /*
             cloud_servers_cmd=18;
             did0_data=65;
             did0_lenth=1;
             wifi_tx_flag=3;
            */
           break;  
           
 
 
    case 104:                  
          for(j=0;j<28;j++)
          {
            cloud_servers_104[j]=wifi_rx[item_lenth+3+j];
          }
           break;  
    case 105:                  
          for(j=0;j<402;j++)
          {
            cloud_servers_105[j]=wifi_rx[item_lenth+4+j];
          }
          ota_cmd_rx_flag=1;
          
             cloud_servers_cmd=16;
             did0_data=98;
             did0_lenth=7;
             wifi_tx_flag=3;
             cloud_servers_100=0;
             cloud_servers_101=1;
          
           break;  
     case 106:          
            cloud_servers_106=wifi_rx[item_lenth+3];
           break;         
     case 107:          
            cloud_servers_107=wifi_rx[item_lenth+3];
           break;         
     case 108:          
            cloud_servers_108=wifi_rx[item_lenth+3];
           break;         
          
          
    }
    
    if(item_data==105)item_lenth+=((unsigned int)wifi_rx[item_lenth+3]+wifi_rx[item_lenth+2]+4);
    else item_lenth+=wifi_rx[item_lenth+2]+3;
  }
  
  
   
   
   
   
}
/********************************************************************************
********************************************************************************/
void wifi_rx_data(void)
{
  unsigned int i;
  unsigned int crc_16,crc_data;
  
  if(wifi_rx_finish_flag==0)return;
  
  crc_16= CalCrcRTU((unsigned char*)wifi_rx,wifi_rx_cnt-2); 
  crc_data=(unsigned int)(wifi_rx[wifi_rx_cnt-1]<<8)|(wifi_rx[wifi_rx_cnt-2]);
  
  if(crc_16==crc_data)
  {
    if(wifi_rx_finish_flag&0x01)
    {
      if(wifi_rx[3]==0x02)
      {
        wifi_rx_moudle_information();
      }
      else if(wifi_rx[3]==0x0a)
      {
        
      }
      else if(wifi_rx[3]==0x0b)
      {
        wifi_rx_moudle_ota_start();
        wifi_tx_flag=6;
      }
      else if(wifi_rx[3]==0x0c)
      {
        wifi_rx_moudle_ota_data();
        wifi_tx_flag=7;
      }
    }
    else if(wifi_rx_finish_flag&0x02)
    {
      wifi_cloud_servers_connect_cnt=0;
      wifi_cloud_servers_connect_flag=1;
      
      cloud_servers_seq=wifi_rx[1];
      for(i=0;i<8;i++)
      {
        unix_time_const[i]=wifi_rx[2+i];
      }
      cloud_servers_cmd=wifi_rx[12];
      
      switch(cloud_servers_cmd)
      {
      
      case 5: 
             //
             cloud_servers_cmd=2;
             did0_data=16;
             did0_lenth=97-16;
             wifi_tx_flag=3;
             //
             wifi_rx_data_deal();
             tx_change_flag=1;
             //wifi_tx_flag=4;   目前不需要回应
             
             break;
      case 6:
             did0_data=wifi_rx[14];
             did0_lenth=wifi_rx[16];
             wifi_tx_flag=3;
             break;
      case 7:
             wifi_rx_data_deal();
             //
             cloud_servers_cmd=16;
             did0_data=98;
             did0_lenth=7;
             wifi_tx_flag=3;
             if(cloud_servers_106)cloud_servers_100=1;
             else cloud_servers_100=0;
             //
             break;  
      case 13:
             cloud_servers_62=2;
             did0_data=0;
             did0_lenth=97;
             wifi_tx_flag=3;
             break;
      case 14:
             cloud_servers_61=1;
             did0_data=0;
             did0_lenth=97;
             wifi_tx_flag=3;
             break;
      case 16:
             wifi_rx_data_deal();
             break;  
      case 18:
             did0_data=65;
             did0_lenth=1;
             wifi_tx_flag=3;
             break;       
             
      default:break;
      }
    
    }
  }
  else
  {
//    nop();
  }
  
  
   for(i=0;i<512;i++)
    {
      wifi_rx[i]=0;
    }
  
  for(i=0;i<15;i++)
    {
      wifi_rx_check_data[i]=0;
    }

    wifi_rx_finish_flag=0;
    wifi_rx_start_flag=0;
  
  
}


int wifi_rx_frame_crc_check(unsigned char* inbuf,unsigned char frame_len)
{
	uint16_t crc_16;
	uint16_t crc_data;

	crc_16= CalCrcRTU(inbuf,frame_len-2); 
	crc_data=(uint16_t)(inbuf[frame_len-1]<<8)|(inbuf[frame_len-2]);
	
	if(crc_16== crc_data)
		return 1;
	else
		{
		printf("crc_16=0x%x,crc_data=0x%x\n",crc_16,crc_data);
		return -1;
		}

}

//neet to consider mutex add for thread safe issue
void wifi_rx_frame_in(unsigned char* inbuf,unsigned char frame_len)
{
	memcpy(wifi_rx,inbuf,frame_len);
	wifi_rx_cnt  = frame_len;
	wifi_rx_finish_flag =1;//let wifi_rx_data in

}


void wifi_tx_frame_out(unsigned char* inbuf,unsigned char frame_len)
{
	uart_module_framesend(inbuf,frame_len);
}

//set all global variable values
void wifi_module_logic_ini()
{
	wifi_rx_finish_flag=0;
	wifi_tx_flag=0;

}

/********************************************************************************
********************************************************************************/
