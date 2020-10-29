#ifndef UART_TX_H
#define UART_TX_H


#ifdef   UART_TX_GLOBALS
#define  UART_TX_EXT 	
#else
#define  UART_TX_EXT extern
#endif

#define NO_DATA_TIME     100
#define REAP_DATA_TIME   9



UART_TX_EXT unsigned char tx_buffer[50*3];
UART_TX_EXT unsigned char tx_data[50],tx_start,tx_nuber,tx_total,tx_finish_flag,next_tx_flag,xor_data;
UART_TX_EXT unsigned char tx_clear_flag,tx_clear_cnt;



UART_TX_EXT unsigned char no_data_cnt,tx_age1,random_temp,AAH_already_sent;
UART_TX_EXT unsigned char tx_repeat_cnt;

UART_TX_EXT unsigned char  Index,tx_multi;

union char_oper
{
    struct bit_feild
    {
        char bit0:1;
        char bit1:1;
        char bit2:1;
        char bit3:1;
        char bit4:1;
        char bit5:1;
        char bit6:1;
        char bit7:1;
    } bits;
    unsigned char data;
};
union char_oper tx_short_flag;
union char_oper tx_data_flag[4];





#define	ack_tx_flag  	          (tx_short_flag.bits.bit0)
#define	nak_tx_flag  	          (tx_short_flag.bits.bit1)
#define	line_ack_tx_flag  	 	  (tx_short_flag.bits.bit2)
#define	line_nak_tx_flag  	 	  (tx_short_flag.bits.bit3)
#define	ack_3d_tx_flag  	  	  (tx_short_flag.bits.bit4)
#define	nak_3d_tx_flag  	  	  (tx_short_flag.bits.bit5)
#define	AAH_sent_flag  	          (tx_short_flag.bits.bit6)
#define	self_tx_flag  	          (tx_short_flag.bits.bit7)


#define	line_control_tx  	 	  		(tx_data_flag[0].bits.bit0)
#define	timing_tx_flag  	 	  		(tx_data_flag[0].bits.bit1)
#define	baseextend_tx_flag  	  		(tx_data_flag[0].bits.bit2)
#define	connect_information_tx    		(tx_data_flag[0].bits.bit3)
#define	group_message_tx  	  	  		(tx_data_flag[0].bits.bit4)
#define	group_message_extend_tx   		(tx_data_flag[0].bits.bit5)
#define	point_check_tx1           		(tx_data_flag[0].bits.bit6)
#define	point_check_tx2           		(tx_data_flag[0].bits.bit7)
		
#define	dresschange_request_tx 	        (tx_data_flag[1].bits.bit0)
#define	dresschange_confirm_tx 	        (tx_data_flag[1].bits.bit1)
#define	baseextend_line_tx_flag	        (tx_data_flag[1].bits.bit2)
#define	line_dresschange_request_tx     (tx_data_flag[1].bits.bit3)
#define	line_dresschange_finish_tx      (tx_data_flag[1].bits.bit4)
#define	line_dresschange_confirm_tx     (tx_data_flag[1].bits.bit5)
#define	line_group_message_tx           (tx_data_flag[1].bits.bit6)
#define	line_group_message_extend_tx    (tx_data_flag[1].bits.bit7)

#define	boardcheck_start_tx  	    	(tx_data_flag[2].bits.bit0)
#define	boardcheck_end_tx  	        	(tx_data_flag[2].bits.bit1)
#define	g25_line_control_tx_flag  		(tx_data_flag[2].bits.bit2)
#define	g25_extend_line_tx_flag     	(tx_data_flag[2].bits.bit3)
#define	g25_basedata_tx_flag  	    	(tx_data_flag[2].bits.bit4)
#define	g25_extenddata_tx_flag      	(tx_data_flag[2].bits.bit5)
#define	a3d_wind_tx_flag            	(tx_data_flag[2].bits.bit6)
#define	auto_clear_tx               	(tx_data_flag[2].bits.bit7)

#define	machine_type_tx                 (tx_data_flag[3].bits.bit0)
#define	save_option_tx                  (tx_data_flag[3].bits.bit1)







void init_tx_deal(void);
void tx_ack_next_deal(void);
void tx_deal(void);

void set_data_deal(void);
void tx_data_table(void);



unsigned char tt_nuber_dress(void);
unsigned char tx_data_10(void);
unsigned char tx_data_11(void);
unsigned char tx_data_12(void);
unsigned char tx_data_13(void);
unsigned char tx_data_14(void);
unsigned char tx_data_15(void);
unsigned char tx_data_16(void);
unsigned char tx_data_17(void);
unsigned char tx_data_18(void);
unsigned char tx_data_19(void);
unsigned char tx_data_21(void);


void line_control_tx_deal2(void);
void timing_tx_deal2(void);
void baseextend_tx_deal2(void);
void baseextend_line_tx_deal2(void);


unsigned char get_nuber(void);
unsigned char get_index(void);
void base_data_tx_deal(void);
void base_data_tx_new_deal(void);
void base_data_tx_new1_deal(void);
void extendbase_data_tx_new_deal(void);
void extendbase_data_tx_new1_deal(void);
void g25_basedata_tx_deal(void);
void g25_extenddata_tx_deal(void);
void dresschange_request_tx_deal(void);
void dresschange_confirm_tx_deal(void);
void point_check_tx1_deal(void);
void point_check_tx2_deal(void);
void connect_information_tx_deal(void);
void group_message_tx_deal(void);
void group_message_extend_tx_deal(void);
void machine_type_tx_deal(void);
void auto_clear_address(void);
void boardcheck_start_tx_deal(void);
void boardcheck_end_tx_deal(void);


unsigned char tx_line_10(void);
unsigned char tx_line_11(void);
unsigned char tx_line_12(void);
unsigned char tx_line_13(void);
unsigned char tx_line_14(void);
unsigned char tx_line_15(void);
unsigned char tx_line_16(void);
unsigned char tx_line_17(void);
unsigned char tx_line_18(void);
unsigned char tx_line_19(void);
unsigned char tx_line_23(void);
unsigned char tx_line_24(void);
unsigned char tx_line_25(void);

void line_control_tx_deal(void);
void line_control_tx_new_deal(void);
void line_control_tx_new1_deal(void);
void extendbase_line_control_tx_new_deal(void);
void extendbase_line_control_tx_new1_deal(void);
void g25_line_control_tx_deal(void);
void g25_extenddata_line_tx_deal(void);
void line_dresschange_request_tx_deal(void);
void line_dresschange_finish_tx_deal(void);
void line_dresschange_confirm_tx_deal(void);
void line_group_message_tx_deal(void);
void line_group_message_extend_tx_deal(void);

void a3d_wind_tx_deal(void);
void save_option_tx_deal(void);


#endif
