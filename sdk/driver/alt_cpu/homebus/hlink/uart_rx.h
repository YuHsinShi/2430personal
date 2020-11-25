#ifndef UART_RX_H
#define UART_RX_H


#ifdef   UART_RX_GLOBALS
#define  UART_RX_EXT 	
#else
#define  UART_RX_EXT extern
#endif



UART_RX_EXT unsigned char  rx_check_data[3],rx_data[150],rx_start,rx_cnt,rx_data_lenth,rx_finish,check_data;

unsigned int mark_deal(unsigned int value,unsigned int nuber);
void dress_confirm(void);
unsigned char  dress_assign(void);
void rx_deal(void);

void rx_data_system_status(void);
void rx_data_system_mode(void);
void rx_data_system_err(void);
void rx_data_system_tempset(void);
void rx_data_system_tryrun(void);
void rx_data_system_maxmin(void);
void rx_data_system_windboard(void);
void rx_data_center_windboard(void);
void rx_data_system_wind(void);
void rx_data_system_filterreset(void);
void rx_data_system_other(void);
void extend_tx_enable(void);

void base_data_rx_deal(void);
void base_data_rx_new_deal(void);
void base_data_rx_new1_deal(void);
void extendbase_data_rx_new_deal(void);
void extendbase_data_rx_new1_deal(void);
void g25_basedata_rx_deal(void);
void g25_extenddata_rx_deal(void);
void point1_data_rx_deal(void);
void point2_data_rx_deal(void);
void machine_type_rx_deal(void);

void a3d_wind_rx_deal(void);

void rx_data_line_10to13(void);
void  rx_data_line_16(void);

void line_base_data_rx_deal(void);
void line_base_data_rx_new_deal(void);
void line_base_data_rx_new1_deal(void);
void extendbase_line_data_rx_new_deal(void);
void extendbase_line_data_rx_new1_deal(void);
void g25_line_fuction_deal(void);
void g25_line_control_rx_deal(void);
void g25_extenddata_line_rx_deal(void);


void save_option_rx_deal(void);






#endif