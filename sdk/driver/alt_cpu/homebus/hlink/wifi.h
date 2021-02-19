#ifndef WIFI_H
#define WIFI_H


#ifdef   WIFI_GLOBALS
#define  WIFI_EXT
#else
#define  WIFI_EXT extern
#endif



WIFI_EXT unsigned char wifi_tx[1000],wifi_rx[1500];
WIFI_EXT unsigned char wifi_tx_flag;
WIFI_EXT unsigned char wifi_tx_start;
WIFI_EXT  unsigned int wifi_tx_nuber,wifi_tx_total;

WIFI_EXT unsigned char  wifi_rx_start_cnt,wifi_rx_clear_cnt;
WIFI_EXT  unsigned char wifi_rx_check_data[15],wifi_rx_start_flag,wifi_rx_finish_flag;
WIFI_EXT  unsigned int wifi_rx_cnt,wifi_rx_nuber;
WIFI_EXT  unsigned char wifi_moudle_set;
WIFI_EXT  unsigned char wifi_signal,wifi_connecting,wifi_connect_ok,wifi_moudle_version[3];

WIFI_EXT  unsigned char did0_data,did0_lenth;

WIFI_EXT  unsigned char unix_time[8],unix_time_const[8],cloud_servers_cmd,cloud_servers_seq;
WIFI_EXT  unsigned char cloud_servers_61,cloud_servers_62,cloud_servers_64,cloud_servers_65,cloud_servers_100,cloud_servers_101,cloud_servers_102,cloud_servers_103,cloud_servers_106,cloud_servers_107,cloud_servers_108;
WIFI_EXT  unsigned char cloud_servers_104[28],cloud_servers_105[402];

WIFI_EXT  unsigned char wifi_connect_10ms,wifi_connect_check_sec,wifi_connect_set_sec;
WIFI_EXT  unsigned char cloud_servers_65_10ms,cloud_servers_65_sec,cloud_servers_65_minu;

WIFI_EXT  unsigned char item_total,item_data;
WIFI_EXT  unsigned int  item_lenth;

WIFI_EXT  unsigned char wifi_cloud_servers_connect_flag;
WIFI_EXT  unsigned int  wifi_cloud_servers_connect_cnt;
WIFI_EXT  unsigned long filetotalbytes,ota_rx_byte,ota_pack_nuber;
WIFI_EXT  unsigned char ota_status_flag;
WIFI_EXT  unsigned int  ota_status_cnt;
WIFI_EXT  unsigned int ota_get_crc;
WIFI_EXT  unsigned char ota_down_finish_flag,ota_down_finish_cnt;
WIFI_EXT  unsigned char ota_cover_flag,ota_cover_cnt;

uint8_t cal_all_flashdata_crc(uint32_t addr,uint32_t length,uint16_t crc);
void write_update_flag(void);

void wifi_tx_moudle(void);
void wifi_tx_all_status(unsigned int start_did0,unsigned int lenth);
void wifi_tx_status_report(void);



void wifi_tx_moudle_ota_cmd(void);
void wifi_tx_moudle_ota_start(void);
void wifi_tx_moudle_ota_data(void);
void wifi_tx_data(void);
void wifi_rx_data(void);

void wifi_rx_moudle_ota_start(void);
void wifi_rx_moudle_ota_data(void);


#endif