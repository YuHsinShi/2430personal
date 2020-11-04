#define UART_RX_GLOBALS
#include "uart_rx.h"
#include "uart_tx.h"


#define CONST_16 16
#define nop() 

#if 1
extern unsigned char d1_d3_check;
extern unsigned char initialize_10ms;
extern unsigned char initialize_flag;
extern unsigned char initialize_cnt;
extern unsigned char initialize_step;
extern unsigned char line_init_flag;
extern unsigned char tx_system_nuber1;
extern unsigned char tx_dress_nuber1;
extern unsigned char tx_system_nuber2;
extern unsigned char tx_dress_nuber2;
extern unsigned char line_control_double;
extern unsigned char master_flag;
extern unsigned char master_flag_write;
extern unsigned char already_tx_nuber;
extern unsigned char basedata[16][50];
extern unsigned char option_data[16][50];
extern unsigned char option_data_new[16][50];
extern unsigned char option_backup[16][50];
extern unsigned char option_new_backup[16][50];
extern unsigned char option_data_new1[16][50];
extern unsigned char option_new1_backup[16][50];
extern unsigned char option_all_set_new[16];
extern unsigned char option_all_set_new1[16];
extern unsigned char option_all_set[16];
extern unsigned char 	line_g25_flag;
extern unsigned char inroom_2g5_protocol;
extern unsigned char	inroom_2g_protocol;
extern unsigned char	total_machine;
extern unsigned char	line_control_dress;
extern unsigned char tt_nuber;
extern unsigned char tt_data;
extern unsigned char system_mode;
extern unsigned char  system_status;
extern unsigned char tx_change_flag;
extern unsigned char tx_time_1s_cnt;
extern unsigned char initialize1_check;
extern unsigned char wifi_power_control;
extern unsigned char wifi_tx_flag;
extern unsigned char wifi_moudle_set;
extern unsigned char confirm_system_status;
extern unsigned char server_set_status;
extern unsigned char a3d_wind_use;
extern unsigned char option_tx_flag;
extern unsigned char option_set_flag;
extern unsigned char single_line_tx_flag;
extern unsigned char half_set_change;
extern unsigned char wind_mode;
extern unsigned char self_tx_data;
extern unsigned char  confirm_run_nuber;
extern unsigned char system_wind;
extern unsigned char    all_package_windnot_flag;
extern unsigned char 	   ignore_flag;
extern unsigned char 	   wind_board_enable;
extern unsigned char 	   wind_board_status;
extern unsigned char compress_preheat_reset;
extern unsigned char emergency_set_status;
extern unsigned char emergency_flag;
extern unsigned char try_run_flag;
extern unsigned char try_run_set;
extern unsigned char try_run_fre;
extern unsigned char wind_board_angle;
extern unsigned char hh_flag;
extern unsigned char save_flag;
extern unsigned char sleep_flag;
extern unsigned char human_flag;
extern unsigned char wind_horizontal2_status;
extern unsigned char wind_horizontal2_angle;
extern unsigned char wind_board2_status;
extern unsigned char wind_board2_angle;
extern unsigned char center_control_tempset;
extern unsigned char tempset;
extern unsigned char dry_set;
extern unsigned char  machine_type;
extern unsigned char  machine_type1;
extern unsigned char  err_reset_flag;
extern unsigned char  filter_reset;
extern unsigned char  temp_ad_value;
extern unsigned char  wind_board_set;
extern unsigned char  winddir_enable;
extern unsigned char 	wind_horizontal_angle;
extern unsigned char 	wind_horizontal_status;
extern unsigned char 	mute_flag;
extern unsigned char 	health_flag;
extern unsigned char 	heat_flag;
extern unsigned char 	forest_wind_flag;
extern unsigned char 	self_clean_flag_b;
extern unsigned char 	high_temp_disinfect_flag_b;
extern unsigned char 	power_wind_check_flag_b;
extern unsigned char human_sensor;
extern unsigned char human_sensor_used;
extern unsigned char inroom_3g_protocol;
extern unsigned char line_protocol;
extern unsigned char human_used;
extern unsigned char option_hh;
extern unsigned char temporary_system;
extern unsigned char temporary_dress;
extern unsigned char backlight_cnt;
extern unsigned char wind_board_set_3s;
extern unsigned char 	half_set;
extern unsigned char 	option_dress;
extern unsigned char try_run_status;
extern unsigned char 	wind_enable;
extern unsigned char 	line_06_sent;
extern unsigned char 	center_onoff_flag;
extern unsigned char 	por_06_flag;
extern unsigned char 	option_f1;
extern unsigned char self_clean_flag;
extern unsigned char err_now_flag;
extern unsigned char a3d_wind_led_flag;
extern unsigned char line_change_flag;
extern  unsigned char a3d_wind_err;

#endif

#if 1 //local
unsigned int option_backcome_txflag;
unsigned char uart_connect_err_cnt=0;
unsigned char uart_connect_err_flag=0;
unsigned char ee_nuber;
 //ee_nuber=dress_assign();
unsigned char power_wind_check_flag_b_write;
unsigned char option_hh_write;
unsigned int option_data_new_write[16];
unsigned char option_data_write[16];
unsigned char	deal_data[16];
unsigned char	boardcheck_data[16];

unsigned char	restart_flag;
unsigned char	restart_cnt;
unsigned char	boardcheck_status;

unsigned char	server_dresschange_ah_display;
unsigned char	dresschange_request_already_tx;
unsigned char	dresschange_request_already_tx_cnt;
unsigned char	line_double_check_cnt;
unsigned char	center_control_all;
unsigned char	system_off_1s_flag;
unsigned char	center_useon_flag;
unsigned char	center_control_onoff;
unsigned char	center_control_rx_flag;
unsigned char	system_mode_canntchange;
unsigned char	center_control_mode;
unsigned char	option_b7;
unsigned char	system_mode_write;
unsigned char	option_b6;
unsigned char tempset_auto;
unsigned char tempset_cool;
unsigned char tempset_heat;
unsigned char half_mode;
unsigned char err_status_nuber;
unsigned char ee_err_flag;
unsigned char err_rest_cnt;
unsigned char tempset_canntchange;
unsigned char tempset_write;
unsigned char half_set_write;
unsigned char 	power_wind_flag;
unsigned char 	power_wind_flag_write;
unsigned char 	try_run_set_write;
unsigned char 	try_run_flag_write;
unsigned char 	pre_on_flag;
unsigned char 	pre_on_flag_write;
unsigned char 	pre_off_flag;

unsigned char pre_off_time;
unsigned char pre_off_time_write;
unsigned char pre_off_minu;
unsigned char pre_off_time_b;
unsigned char pre_off_time_b_write;
unsigned char tempset_min_warm_rx;
unsigned char tempset_max_warm_rx;
unsigned char tempset_min_cool_rx;
unsigned char tempset_max_cool_rx;
unsigned char pointcheck1_data[16];
unsigned char pointcheck2_data[16];
unsigned char wind_board_status_write;
unsigned char wind_board_angle_write;
unsigned char wind_horizontal_status_write;
unsigned char wind_board2_status_write;
unsigned char wind_horizontal2_status_write;
unsigned char basedata_7_backup[16];
unsigned char option_data_new1_write[16];
unsigned char initialize1_flag;
unsigned char self_clean_flag_b_write;
unsigned char high_temp_disinfect_flag_b_write;
unsigned char temp_value;
unsigned char a3d_wind_cnt;
unsigned char clear_eeprom_flag;
unsigned char clear_eeprom_flag_write;
unsigned char system_wind_write;
unsigned char wind_horizontal_angle_write;
unsigned char wind_board2_angle_write;
unsigned char wind_horizontal2_angle_write;
unsigned char system_wind_canntchange;
unsigned char high_temp_disinfect_flag;
unsigned char high_temp_disinfect_flag_write;
unsigned char forest_wind_flag_write;
unsigned char filter_flag;
unsigned char wind_board_changing;
unsigned char pre_off_flag_write;
unsigned char fre_machine;
unsigned char dry_set_write;
unsigned char line_err_reset_flag;
unsigned char machine_type_write;
unsigned char line_06_flag;
unsigned char wind_board_set_write;			  

unsigned char option_f1_write;
unsigned char sleep_flag_write;
unsigned char option_preoff;
unsigned char save_flag_write;
unsigned char mute_flag_write;
unsigned char heat_flag_write;
unsigned char health_flag_write;
unsigned char wind_mode_write;
unsigned char human_flag_write;
unsigned char self_clean_flag_write;

unsigned char set_status;
unsigned char pre_set_status;
unsigned char systemon_cnt;
unsigned char systemon_flag;
unsigned char system_status_write;
unsigned char 	health_used;
extern unsigned char self_check_flag;

#endif 
/********************************************************************************
********************************************************************************/
unsigned int mark_deal(unsigned int value,unsigned int nuber)
{
  unsigned int i,j,data;

  i=(value<<1)&(0xffff<<(nuber+1));
  j=value&(0xffff>>(16-nuber));
  data=i|j;
  
  return data;
}
/********************************************************************************
********************************************************************************/
void dress_confirm(void)
{
	unsigned char i,j,k,l,_already_on,_total;

        _total=total_machine+1;
        _already_on=0;

        if(total_machine>CONST_16)return;

       for(i=0;i<_total;i++)
	{
          if((rx_data[3]==basedata[i][1])&&(rx_data[4]==basedata[i][2]))
	 {	
           _already_on=1;
	 }
        }

      
	
      if(_already_on==0)
      {

	for(i=0;i<_total;i++)
	{
	
		//只有地址小的 才能往前排入
	         if((rx_data[3]<basedata[i][1])||((rx_data[3]==basedata[i][1])&&(rx_data[4]<basedata[i][2])))
		{
			for(j=_total;j>i;j--)//因为地址小的往前插入 所以所有值往后移一位
			{				
				if(j<CONST_16)
				{
				  basedata[j][1]=basedata[j-1][1];
				  basedata[j][2]=basedata[j-1][2];
                                  basedata[j][3]=basedata[j-1][3];//室外机的机型，对应第37字节
                                  
                                  for(l=0;l<11;l++)
                                  {
                                    option_backup[j][l]=option_backup[j-1][l];
                                  }
                                  
                                  //////////////
                                   for(k=0;k<13;k++)
                                  {
                                    option_new_backup[j][k]=option_new_backup[j-1][k];///                                   
                                    option_new1_backup[j][k]=option_new1_backup[j-1][k];///
                                  }
                                  ////////////////
                                  
				}
								
			}
                       //新的地址值写入
			basedata[i][1]=rx_data[3];
			basedata[i][2]=rx_data[4];
                        basedata[i][3]=rx_data[36];//室外机的机型，对应第37字节
                        
                        option_backcome_txflag=mark_deal(option_backcome_txflag,i);

                        ////////////////////////////////////////
                        //先将值保存下来，再做默认值判断
                        for(j=0;j<10;j++)
                        {
                                option_backup[i][j]=rx_data[18+j];
                        }
                        
                        /////////////////////////////  
                        //如果从室内机发来的数据为默认值，且eeprom有系统地址与之对应的数据 则从eeprom的数据
                        //如果不是，则保存从室内机发过来的数据，
                        //这些数据先用一个地方临时保存，然后再按地址顺序存入option_data里面
                        for(k=0;k<CONST_16;k++)
                        {
                          if((rx_data[3]==option_data[k][11])&&(rx_data[4]==option_data[k][12]))
                          {
                            ///////////////                       
                             if(((rx_data[18]==0x00)&&((rx_data[19]&0x3d)==0x00)&&(rx_data[20]==0x00)
                                &&((rx_data[21]&0x7f)==0x00)&&(rx_data[22]==0x00)&&(rx_data[23]==0x04)&&(rx_data[24]==0x20)&&(rx_data[25]==0x01)&&
                                  (rx_data[26]==0x02)&&(rx_data[27]==0x20))&&
                               ((option_data[k][0]!=0x00)||((option_data[k][1]&0x3d)!=0x00)||(option_data[k][2]!=0x00)
                                ||((option_data[k][3]&0x7f)!=0x00)||(option_data[k][4]!=0x00)||(option_data[k][5]!=0x04)||(option_data[k][6]!=0x20)||(option_data[k][7]!=0x01)
                               ||(option_data[k][8]!=0x02)||(option_data[k][9]!=0x20)))
                            {

                              for(j=0;j<10;j++)
                              {
                                option_backup[i][j]=option_data[k][j];
                              }
                              option_backup[i][3]|=0x80;
                              option_backcome_txflag|=(0x0001<<i);
  
                            }
                            option_backup[i][10]=option_data[k][10];//错误代码
                            ///////////////////
                             //新协议中用到的机能设定6-14
                            for(j=0;j<13;j++)
                              {
                                option_new_backup[i][j]=option_data_new[k][j];
                                option_new1_backup[i][j]=option_data_new1[k][j];
                              }
                          }
                       
                        }
                        /////////////////
			
			i=_total;//跳出循环
		}
	
	}
        total_machine++;
      }
}
/********************************************************************************
********************************************************************************/
unsigned char  dress_assign(void)
{
  unsigned char i,nuber;

  nuber=0xff;

  for(i=0;i<total_machine;i++)
  {
 	if((basedata[i][1]==rx_data[3])&&(basedata[i][2]==rx_data[4])) 		
 	{
 	  nuber=i; 	
 	}
  }
  
  return nuber;
}
/********************************************************************************
********************************************************************************/
void rx_deal(void)
{
  unsigned char i;
	int count;
  check_data=0;
/*
  rx_data_lenth = ioctl(ITP_DEVICE_ALT_CPU, ITP_IOCTL_HOMEBUS_READ_DATA, &rx_data);
  if(rx_data_lenth > 5) {
	  printf("Homebus Read(%d) :\n", rx_data_lenth);
	  for(count = 0; count < len; count++) {
		  printf("0x%x ", rx_data[count]);
	  }
	  printf("\r\n");
  }
  else
  {
		return;
  }
*/
	homebus_recvdata(rx_data,&rx_data_lenth);

return;

//  if(rx_finish)
  {
  //    rx_finish=0;
      
      
    
       for(i=1;i<rx_data_lenth-1;i++)
       {
         check_data^=rx_data[i];
       }

       if(rx_data[rx_data_lenth-1]==check_data)
       {
        //接收数据     
        if(rx_data[0]==0x12)
        {
          
         if((rx_data[2]==0x2d)&&(rx_data[5]==0x01)&&(rx_data[6]==line_control_dress)&&(rx_data[9]==0x01)&&(initialize_flag==0))
         {
           dress_confirm();
         }
         
         uart_connect_err_cnt=0;
         uart_connect_err_flag=0;
          
          ee_nuber=dress_assign();
          if(ee_nuber>=CONST_16)
          {
            if((rx_data[9]!=0x0c)&&(rx_data[9]!=0xbc)&&(rx_data[9]!=0xac)&&(rx_data[1]!=0xe2)) 
            return;
          }
          
          
          //基本数据
         if((rx_data[2]==0x2d)&&(rx_data[5]==0x01)&&(rx_data[6]==line_control_dress)&&(rx_data[9]==0x01))
         {        
           base_data_rx_deal();             
         }
         //基本数据--new
         else if((rx_data[2]==0x27)&&(rx_data[6]==line_control_dress)&&(rx_data[9]==0xb1))
         {            
            base_data_rx_new_deal();
         }
          //基本数据--new1
         else if((rx_data[2]==0x30)&&(rx_data[6]==line_control_dress)&&(rx_data[9]==0xa1))
         {
             base_data_rx_new1_deal();
         }
         //基本扩展数据
         else if((rx_data[2]==0x24)&&(rx_data[6]==line_control_dress)&&(rx_data[9]==0xb2))
         {
             extendbase_data_rx_new_deal();
         }
         //基本扩展数据-new1
         else if((rx_data[2]==0x30)&&(rx_data[6]==line_control_dress)&&(rx_data[9]==0xa2))
         {
             extendbase_data_rx_new1_deal();
         }
	 //G25基本数据
         //else if((rx_data[2]==0x30)&&(rx_data[6]==line_control_dress)&&(rx_data[9]==0xbd))
         else if((rx_data[2]==0x30)&&(rx_data[9]==0xbd))
         {
             g25_basedata_rx_deal();
         }
	 //G25扩展数据
         //else if((rx_data[2]==0x30)&&(rx_data[6]==line_control_dress)&&(rx_data[9]==0xbe))
         else if((rx_data[2]==0x30)&&(rx_data[9]==0xbe))
         {
             g25_extenddata_rx_deal();
         }
         //基本数据要求
         else if((rx_data[1]==0x00)&&(rx_data[2]==0x0b)&&((rx_data[9]==0x00)||(rx_data[9]==0xb0)||(rx_data[9]==0xa0)))
         {
                 timing_tx_flag=1;
                 already_tx_nuber=total_machine;
                 tx_system_nuber1=rx_data[3];tx_dress_nuber1=rx_data[4];
                 tx_system_nuber2=rx_data[3];tx_dress_nuber2=rx_data[4];
         }       
         //地址变更成功后重启要求
         else if((rx_data[1]==0x00)&&(rx_data[2]==0x0b)&&((rx_data[9]==0x0c)||(rx_data[9]==0xbc)||(rx_data[9]==0xac)))
         {
                restart_flag=1;restart_cnt=0;
         }
         //点检1
         else if((rx_data[2]==0x2d)&&((rx_data[9]==0x06)||(rx_data[9]==0xb6)||(rx_data[9]==0xa6)))
         {
           point1_data_rx_deal();
         }
         //点检2
         else if((rx_data[2]==0x1e)&&((rx_data[9]==0x08)||(rx_data[9]==0xb8)||(rx_data[9]==0xa8)))
         {
           point2_data_rx_deal();
         }
          //型式情报数据
         else if((rx_data[2]==0x30)&&((rx_data[9]==0xBA)||(rx_data[9]==0xAA)))
         {
           machine_type_rx_deal();
         }
         //节电机能数据(3G)
         else if((rx_data[2]==0x13)&&(rx_data[9]==0x90))
         {
           save_option_rx_deal();
         }
         //接线信息2G 3G 收到
         else if((rx_data[2]==0x1c)&&((rx_data[9]==0xb4)||(rx_data[9]==0xa4)))
         {
           
              
         }
         //接线信息1G 收到
         else if((rx_data[2]==0x14)&&(rx_data[9]==0x04))
         {
           
           

         }
         //诊断中
         else if((rx_data[2]==0x0a)&&(rx_data[1]==0x20))
         {

         }
         //诊断结果
         else if((rx_data[2]==0x0d)&&(rx_data[1]==0x21)&&(rx_data[6]==line_control_dress))
         {   
           if(boardcheck_status)boardcheck_status=3;
           boardcheck_data[0]=rx_data[9];
           boardcheck_data[1]=rx_data[10];
           boardcheck_data[2]=rx_data[11];
         }
         //诊断中断结束
         else if((rx_data[2]==0x0a)&&(rx_data[1]==0x22))
         {

         }
         //地址冷媒系统变更完成
         else if((rx_data[2]==0x0a)&&(rx_data[1]==0xe2)&&(rx_data[6]==line_control_dress))
         {
           
           dresschange_confirm_tx=1;
           tx_system_nuber1=temporary_system;tx_dress_nuber1=temporary_dress;
           tx_system_nuber2=temporary_system;tx_dress_nuber2=temporary_dress;
           basedata[option_dress][1]=rx_data[3];basedata[option_dress][2]=rx_data[4];
           option_data[option_dress][11]=basedata[option_dress][1];
           option_data[option_dress][12]=basedata[option_dress][2];
           option_data_write[option_dress]|=0x1800;
           
           if(server_set_status==0x7f)server_set_status=0x7b;
           server_dresschange_ah_display=1;
           
           dresschange_request_already_tx=0;
           dresschange_request_already_tx_cnt=0;
         }         
         
        }
        //////////////////////////////////////////
        //3d rx
        else if(rx_data[0]==0x17)
        {
               if((rx_data[1]==0x00)&&(rx_data[2]==0x18)&&(rx_data[9]==0xb1))
              {
                 a3d_wind_rx_deal();
              }
        }
         //////////////////////////////////////////
        //其他线控器指令
        else if(rx_data[0]==0x41)
        {
          //1G亲子遥控器基本格式
          if((rx_data[1]==0x00)&&(rx_data[2]==0x1e)&&(rx_data[9]==0x01))
          {
               if((rx_data[5]!=0xff)&&(rx_data[6]!=0xff))
               {
                 line_base_data_rx_deal();
               }

                if((master_flag)&&(rx_data[4]==0x02))//握手结束后，都为子遥控 自己变为主
                {   
                  
                    master_flag=0;
                    master_flag_write=1;
                    if(initialize_flag)line_control_tx=1;
                }
                else if((master_flag)&&(rx_data[4]==0x01))
                {
                  if((rx_data[5]==0xff)&&(rx_data[6]==0xff))//自身为子，收到亲的广播，为了避免对方也是子，所以发送基本格式
                  {
                    //////
                    if((initialize_flag==0)&&(line_control_tx))//自身广播没发出前，重新进入第一步
                    {
                    initialize_10ms=0;
                    initialize_cnt=3;
                    initialize_step=0;
                    line_init_flag=0;
                    }
                    //////
                    line_control_tx=1;
                    single_line_tx_flag=1;
                  }
                }
                else if((master_flag==0)&&(rx_data[4]==0x01))//都为亲遥控
                {
                  if((rx_data[5]==0xff)&&(rx_data[6]==0xff))//在广播时收到，叫别人改为子
                  {
                    line_dresschange_request_tx=1;   
                    if(initialize_flag)g25_line_control_tx_flag=1;
                  }
                  else  //握手结束后收到，自己改为子
                  {
                    master_flag=1;
                    master_flag_write=1;
                    line_control_tx=1;
                  }
                }
                line_control_double=1;//有亲子遥控器   

          }
          //2G亲子遥控器基本格式
          else if((rx_data[1]==0x00)&&(rx_data[2]==0x18)&&(rx_data[9]==0xb1))
          {

               if((rx_data[5]!=0xff)&&(rx_data[6]!=0xff))
               {
                 line_base_data_rx_new_deal();
               }

              //////////////////////////////////////////////
                if((master_flag)&&(rx_data[4]==0x02))//都为子遥控
                {
                    master_flag=0;
                    master_flag_write=1;           
                }
                else if((master_flag==0)&&(rx_data[4]==0x01))//都为亲遥控
                {
                    master_flag=1;
                    master_flag_write=1;                 
                }
                line_control_double=1;//有亲子遥控器

             ///////////////////////////////////////            
          }
          //3G线控器基本格式
          //else if((rx_data[1]==0x00)&&(rx_data[2]==0x20)&&(rx_data[9]==0xa1))
          else if((rx_data[1]==0x00)&&((rx_data[2]==0x20)||(rx_data[2]==0x1A))&&(rx_data[9]==0xa1))//兼容矫工
          {
            
               if((rx_data[5]!=0xff)&&(rx_data[6]!=0xff))
               {
                 line_base_data_rx_new1_deal();
               }

              //////////////////////////////////////////////
                if((master_flag)&&(rx_data[4]==0x02))//都为子遥控
                {
                    master_flag=0;
                    master_flag_write=1;
                }
                else if((master_flag==0)&&(rx_data[4]==0x01))//都为亲遥控
                {
                    master_flag=1;
                    master_flag_write=1;
                }
                line_control_double=1;//有亲子遥控器

             ///////////////////////////////////////             
          }         
           //2G亲子线控器扩张格式
         else if((rx_data[2]==0x20)&&(rx_data[6]==line_control_dress)&&(rx_data[9]==0xb2))
         {         
               extendbase_line_data_rx_new_deal();
         }
         //3G亲子线控器扩张格式
          else if((rx_data[2]==0x2c)&&(rx_data[6]==line_control_dress)&&(rx_data[9]==0xa2))
         {         
               extendbase_line_data_rx_new1_deal();
         }
         //2.5G亲子线控器基本格式
         else if((rx_data[2]==0x30)&&(rx_data[6]==line_control_dress)&&(rx_data[9]==0xbd))
         {         
               g25_line_control_rx_deal();            
         }
         //2.5G亲子线控器扩张格式
          else if((rx_data[2]==0x30)&&(rx_data[6]==line_control_dress)&&(rx_data[9]==0xbe))
         {
               g25_extenddata_line_rx_deal();            
         }
          else if((rx_data[1]==0xf1)&&(rx_data[2]==0x0c))//遥控器地址变更要求
          { 
            line_dresschange_request_tx=0;//收到对方的地址变更要求后，自身要发的变更要求要清0
            line_dresschange_finish_tx=1;     
            if(rx_data[10]==0x01)
            {
              master_flag=0;
              master_flag_write=1;
              /*
              tx_system_nuber1=0x01;
              tx_dress_nuber1=0x02;
              tx_system_nuber2=0x01;
              tx_dress_nuber2=0x02;
              */
            }
            else if(rx_data[10]==0x02)
            {
              master_flag=1;
              master_flag_write=1;
              /*
              tx_system_nuber1=0x01;
              tx_dress_nuber1=0x01;
              tx_system_nuber2=0x01;
              tx_dress_nuber2=0x01;
              */
            }
            line_control_double=1;//有亲子遥控器    

          }
          else if((rx_data[1]==0xe2)&&(rx_data[2]==0x0a))//遥控器地址变更完成
          {
            
            line_dresschange_confirm_tx=1;
            /*
            if(master_flag)
            {
              tx_system_nuber1=0x01;
              tx_dress_nuber1=0x01;
              tx_system_nuber2=0x01;
              tx_dress_nuber2=0x01;
            }
            else
            {
              tx_system_nuber1=0x01;
              tx_dress_nuber1=0x02;
              tx_system_nuber2=0x01;
              tx_dress_nuber2=0x02;
            }
            */
            line_control_double=1;//有亲子遥控器

          }
          else if((rx_data[1]==0xf2)&&(rx_data[2]==0x0a))//遥控器地址变更确认
          {

          }
         
        }
        //接收自己的数据进行校验
        else if(rx_data[0]==0x21)
        {
           if((rx_data[2]==tx_total)&&(check_data==xor_data)&&(rx_data[4]==line_control_dress))
           {
                
              nop();
              if(initialize_flag==0)
              {
                tt_nuber=tt_nuber_dress();
                option_backup[tt_nuber][3]&=0x7f;
              }
           }
           else if(rx_data[4]!=line_control_dress)
           {
             nop();
             line_double_check_cnt=0;
           }
        }
         //////////////////
       }
       else  //数据CRC校验不对的处理
       {
        
       }
       

    
  }
}



/********************************************************************************
********************************************************************************/
void system_on_deal(void)
{
  if((system_status==0)&&(self_check_flag==0)&&(server_set_status==0)&&(boardcheck_status==0)&&
     (system_off_1s_flag)&&((por_06_flag==0)||(center_control_rx_flag))&&
       ((center_control_onoff==0)||(center_useon_flag)))
  {
   
    if(a3d_wind_use)
    {
       a3d_wind_tx_flag=1;
       systemon_cnt=0;
       systemon_flag=1;
    }
    else
    {
      tx_time_1s_cnt=0;
      tx_change_flag=1;
    }
  

  system_status=1;
  system_status_write=1;
  center_useon_flag=0;
  
  set_status=0;
  pre_set_status=0;
  //濡傛灉娌℃湁鎸変笅mode閿?寮�鏈洪粯璁ゆ槸鍒跺喎
  if(system_mode==0)
  {
    system_mode=0x02;
    system_mode_write=1;
    tempset=28;
    tempset_write=1;
  }


    if(pre_on_flag)
    {
    pre_on_flag=0;
    pre_on_flag_write=1;
    }
 

       if(try_run_set)
        {
          try_run_set=0;
          try_run_set_write=1;
          try_run_flag|=0x01;
          try_run_flag_write=1;
          pre_on_flag=0;
          pre_on_flag_write=1;
          
          //if(pre_off_flag==0)
          //{
          pre_off_flag=1;
          pre_off_flag_write=1;
          pre_off_time=4;
          pre_off_time_write=1;
          //}
          
          pre_off_minu=0;
          pre_off_time_b=pre_off_time;
          pre_off_time_b_write=1;
        }
  
         else if(option_preoff)
         {
          pre_off_flag=1;
          pre_off_flag_write=1;
          pre_off_minu=0;         
          pre_off_time=option_preoff;
          pre_off_time_write=1;
          pre_off_time_b=pre_off_time;
          pre_off_time_b_write=1;
         }


  if((health_flag==0)&&(health_used)&&(self_clean_flag==0)&&(high_temp_disinfect_flag==0))
  {
  health_flag=1;
  health_flag_write=1;
  }


  }
}

/********************************************************************************
********************************************************************************/
void fuction_clear_deal(void)
{
      
      if(sleep_flag)
      {
       
          sleep_flag=0;
          sleep_flag_write=1;
        
        if((try_run_flag==0)&&(try_run_set==0)&&(option_preoff==0))
        {
          if(pre_off_flag)
            {
             pre_off_flag=0;
             pre_off_flag_write=1;
            }
        }
          tx_change_flag=1;
          tx_time_1s_cnt=0;
      }
      if(save_flag)
      {
      save_flag=0;
      save_flag_write=1;
      tx_change_flag=1;
      tx_time_1s_cnt=0;
      }
      if(heat_flag)
      {
      heat_flag=0;
      heat_flag_write=1;
      tx_change_flag=1;
      tx_time_1s_cnt=0;
      }
      /*
      if(health_flag)
      {
      health_flag=0;
      health_flag_write=1;
      tx_change_flag=1;
      tx_time_1s_cnt=0;
      }
     */
      if(mute_flag)
      {
      mute_flag=0;
      mute_flag_write=1;
      tx_change_flag=1;
      tx_time_1s_cnt=0;
      }
      if(forest_wind_flag)
      {
      forest_wind_flag=0;
      forest_wind_flag_write=1;
      tx_change_flag=1;
      tx_time_1s_cnt=0;
      }
      
      if(power_wind_flag)
      {
      power_wind_flag=0;
      power_wind_flag_write=1;
      power_wind_check_flag_b=0;
      power_wind_check_flag_b_write=1;
      tx_change_flag=1;
      tx_time_1s_cnt=0;
      }
      
            /*
      if(high_temp_disinfect_flag)
      {
      high_temp_disinfect_flag=0;
      high_temp_disinfect_flag_write=1;
      high_temp_disinfect_flag_b=0;
      high_temp_disinfect_flag_b_write=1;
      tx_change_flag=1;
      tx_time_1s_cnt=0;
      }
      */
}
/********************************************************************************
********************************************************************************/
void fuction_clear1_deal(void)
{
      
      if(sleep_flag)
      {
       
          sleep_flag=0;
          sleep_flag_write=1;
        
        if((try_run_flag==0)&&(try_run_set==0)&&(option_preoff==0))
        {
          if(pre_off_flag)
            {
             pre_off_flag=0;
             pre_off_flag_write=1;
            }
        }
          tx_change_flag=1;
          tx_time_1s_cnt=0;
      }
      if(save_flag)
      {
      save_flag=0;
      save_flag_write=1;
      tx_change_flag=1;
      tx_time_1s_cnt=0;
      }
      if(heat_flag)
      {
      heat_flag=0;
      heat_flag_write=1;
      tx_change_flag=1;
      tx_time_1s_cnt=0;
      }
      /*
      if(health_flag)
      {
      health_flag=0;
      health_flag_write=1;
      tx_change_flag=1;
      tx_time_1s_cnt=0;
      }
      */
      if(mute_flag)
      {
      mute_flag=0;
      mute_flag_write=1;
      tx_change_flag=1;
      tx_time_1s_cnt=0;
      }
      if(forest_wind_flag)
      {
      forest_wind_flag=0;
      forest_wind_flag_write=1;
      tx_change_flag=1;
      tx_time_1s_cnt=0;
      }
      
      if(power_wind_flag)
      {
      power_wind_flag=0;
      power_wind_flag_write=1;
      power_wind_check_flag_b=0;
      power_wind_check_flag_b_write=1;
      tx_change_flag=1;
      tx_time_1s_cnt=0;
      }
      /*
      if(high_temp_disinfect_flag)
      {
      high_temp_disinfect_flag=0;
      high_temp_disinfect_flag_write=1;
      high_temp_disinfect_flag_b=0;
      high_temp_disinfect_flag_b_write=1;
      tx_change_flag=1;
      tx_time_1s_cnt=0;
      }
      */
}
/********************************************************************************
********************************************************************************/



/********************************************************************************
********************************************************************************/
void system_off_deal(void)
{
  unsigned char i;

  if(system_status)
  {
            
            tx_time_1s_cnt=0;
            tx_change_flag=1; 
            
            system_status=0;
            system_status_write=1;
            center_useon_flag=0;
            a3d_wind_led_flag=0;
            
            set_status=0;
            pre_set_status=0;
            
             if(pre_off_flag)
            {
             pre_off_flag=0;
             pre_off_flag_write=1;
            }
          

            if(err_now_flag)
            {
                a3d_wind_err=0;
                err_now_flag=0;
                ee_err_flag=0;
                for(i=0;i<CONST_16;i++)
                {
                   basedata[i][4]&=0xfe;
                }
            }

            //鍏虫満 閫�鍑鸿瘯杩愯浆
            try_run_fre=30;
            try_run_status=0;
            try_run_flag=0;
            try_run_flag_write=1;
            try_run_set=0;
            try_run_set_write=1;
            
            if(human_flag)
            {
              human_flag=0;
              human_flag_write=1;
            }
            
            fuction_clear1_deal();
            
  if(health_flag)
  {
  health_flag=0;
  health_flag_write=1;
  }
      
      if(self_clean_flag)
      {
      self_clean_flag=0;
      self_clean_flag_write=1;
      self_clean_flag_b=0;
      self_clean_flag_b_write=1;
      }
  
                 if(high_temp_disinfect_flag)
                   {
                    high_temp_disinfect_flag=0;
                    high_temp_disinfect_flag_b=0;
                    high_temp_disinfect_flag_write=1;
                   high_temp_disinfect_flag_b_write=1;
                   }
  }
}
/********************************************************************************
********************************************************************************/

/********************************************************************************
********************************************************************************/
void rx_data_system_status(void)
{
  unsigned char i;
  
  //7日定时器
          if((deal_data[0]&0x20)&&((basedata[ee_nuber][5]&0x20)==0))
          {
            i=1;
          }
          else if((deal_data[0]&0x10)&&((basedata[ee_nuber][5]&0x10)==0))
          {
            i=2;
            center_onoff_flag=0;;
          }
          //室外机
          else if((deal_data[1]&0x20)&&((basedata[ee_nuber][6]&0x20)==0))
          {
            i=1;
          }
          else if(((deal_data[1]&0x20)==0)&&(basedata[ee_nuber][6]&0x20))
          {
            i=2;
            center_onoff_flag=0;
          }
          //i1 i2-->03 04 05
          else if((deal_data[0]&0x02)&&((basedata[ee_nuber][5]&0x02)==0))
          {
             if((center_control_all==0)||(center_onoff_flag))
            {
              i=1;
              system_off_1s_flag=1;
              if(center_onoff_flag&0x02)center_useon_flag=1;
            }
          }
          else if((deal_data[0]&0x01)&&((basedata[ee_nuber][5]&0x01)==0))
          {
            i=2;
            if((center_control_onoff)&&(system_status))center_onoff_flag=0x02;             

          }
          //集中控制器
          else if((deal_data[2]&0x01)&&(deal_data[2]&0x02)&&
                  (((basedata[ee_nuber][12]&0x01)==0)||((basedata[ee_nuber][12]&0x02)==0)))
          {
            
            if((system_status==0)&&(center_onoff_flag==0))
            {
              center_useon_flag=1;
              center_control_rx_flag=1;
              i=1;
            }

          }
          else if(((deal_data[2]&0x01)==0)&&(deal_data[2]&0x02))
          {
            center_onoff_flag=0;            
            if(system_status)i=2;           
          }



       if(i==1)
       {
         system_on_deal();
         
       }
       else if(i==2)
       {
         system_off_deal();
        
       }
       center_control_rx_flag=0;
       
}
/********************************************************************************
********************************************************************************/
void rx_data_system_mode(void)
{
  unsigned char i;
  //模式
         i=system_mode;
         if((deal_data[0]&0x04)&&((basedata[ee_nuber][5]&0x04)==0))
            
         {
           i=0x02;
         }
         else if((deal_data[0]&0x08)&&((basedata[ee_nuber][5]&0x08)==0))
                 
         {
           i=0x10;
         }
         //室外机01 02
         if((deal_data[1]&0x04)&&((basedata[ee_nuber][6]&0x04)==0))
         {
            i=0x02;
         }
         else if((deal_data[1]&0x10)&&((basedata[ee_nuber][6]&0x10)==0))
         {
           i=0x10;
         }
         else if((deal_data[1]&0x08)&&((basedata[ee_nuber][6]&0x08)==0))
         {
           i=0x08;
         }

         if(deal_data[2]&0x80)
         {
           //i=(deal_data[2]>>2)&0x1f;//集中控制器
           if(deal_data[2]&0x7c)i=(deal_data[2]>>2)&0x1f;//集中控制器
           center_control_rx_flag=1;
           
           if(deal_data[3]&0x04)
           {
             switch(deal_data[3]&0x03)
            {
            case 0:i=0x84;break;
            case 1:i=0xa4;break;
            case 2:i=0xc4;break;
            }
           }
         }


         if((system_mode!=i)&&(system_mode_canntchange==0)&&
            (((por_06_flag==0)&&(center_control_mode==0))||(center_control_rx_flag))&&(initialize_flag))
           {
             
              if((i==0x10)&&(option_b7))
              {             
              }
              else if(((try_run_flag)||(try_run_set))&&(i==0x01))
              {
                
              }
              else 
              {
                system_mode=i;
                system_mode_write=1;     
                
                fuction_clear1_deal();
                //
     if(option_b6==0)
     {
        switch(system_mode)
   	{
   		case 0x01: tempset=tempset_auto;break;
   		case 0x02: tempset=tempset_cool;break;
   		case 0x04: tempset=tempset_cool;break;
   		case 0x08: tempset=tempset_cool;break;
   		case 0x10: tempset=tempset_heat;break;
   		default:tempset=tempset_cool;break;
   	}

      if(system_mode&0x10)
      {
        if(half_mode&0x01)half_set=1;
        else half_set=0;
      }
      else if(system_mode&0x01)
      {
        if(half_mode&0x02)half_set=1;
        else half_set=0;
      }
      else
      {
        if(half_mode&0x04)half_set=1;
        else half_set=0;
      }
     }
             //   
            tx_change_flag=1;    
                
              }
              //tx_change_flag=1;

           }
         
         center_control_rx_flag=0;
}
/********************************************************************************
********************************************************************************/
void rx_data_system_err(void)
{

  //错误代码
  if((system_status==0)&&(confirm_system_status==0))return;
  
           if((deal_data[1])&&(err_reset_flag==0))
           {
             if(err_now_flag==0)
             {
               err_status_nuber=ee_nuber;
               tx_change_flag=1;
             }

             if(deal_data[1]==0xee)
             {
               ee_err_flag=1;
             }
             else
             {
              if(option_data[ee_nuber][10]!=deal_data[1])option_data_write[ee_nuber]|=(0x0001<<10);        
              option_data[ee_nuber][10]=deal_data[1];
             }
           }
           
           
 
           if((deal_data[0]&0x60)==0x60)
           {
             if(err_reset_flag==0)
             {
                err_now_flag=1;
                err_rest_cnt=0;
                basedata[ee_nuber][4]|=0x01;              
             }
           }
           else
           {
             basedata[ee_nuber][4]&=0xfe;
           }
          
           
         
         
}
/********************************************************************************
********************************************************************************/
void rx_data_system_tempset(void)
{
   unsigned char i;
   
   //设定温度
           i=tempset;
          if(deal_data[0]&0x04)
          {
            i=deal_data[1];
            center_control_rx_flag=1;
          }

          if(i!=tempset)
           {
              if(((por_06_flag==0)||(center_control_rx_flag))&&(tempset_canntchange==0))
              {
                if((i>10)&&(i<40))
                {
                  tempset=i;
                  tempset_write=1;
                  tx_change_flag=1;
                }
                
                if(half_set)
                {
                  half_set=0;
                  half_set_write=1;
                }
                
                  if(power_wind_flag)
                 {
                  power_wind_flag=0;
                  power_wind_flag_write=1;
                  power_wind_check_flag_b=0;
                  power_wind_check_flag_b_write=1;
                 }
                
              }
             
           }
          center_control_rx_flag=0;
    
   
}
/********************************************************************************
********************************************************************************/
void rx_data_system_tryrun(void)
{

   
   //试运转 关机时接到水模块发来的信号 开始进入试运转
          if(((deal_data[0]&0x40)&&((basedata[ee_nuber][6]&0x40)==0))||((deal_data[1]&0x01)&&((basedata[ee_nuber][14]&0x01)==0)))
           {
           try_run_set|=0x02;

            system_on_deal();
            

            if(system_status)
            {
            try_run_set=0;
            try_run_set_write=1;
            try_run_flag|=0x02;
            try_run_flag_write=1;
            pre_on_flag=0;
            pre_on_flag_write=1;
            pre_off_flag=1;
            pre_off_flag_write=1;
            pre_off_time=4;
            pre_off_time_write=1;
            pre_off_minu=0;
            pre_off_time_b=pre_off_time;
            pre_off_time_b_write=1;
            }
            else if(por_06_flag==0)
            {
              try_run_set|=0x02;
              try_run_set_write=1;
            }

            if(try_run_status==0)try_run_fre=30;

           }
           else if((((deal_data[0]&0x40)==0)&&(basedata[ee_nuber][6]&0x40))||(((deal_data[1]&0x01)==0)&&(basedata[ee_nuber][14]&0x01)))
           {
             try_run_flag=0;
             try_run_flag_write=1;
             try_run_set=0;
             try_run_set_write=1;
             try_run_fre=30;
             try_run_status=0;
             center_onoff_flag=0;center_useon_flag=0;
             system_off_deal();
             
           }
   
}
/********************************************************************************
********************************************************************************/
void rx_data_system_maxmin(void)
{
                 
           if(system_mode&0x10)
           {
              tempset_min_warm_rx=deal_data[0];
              tempset_max_warm_rx=deal_data[1];
           }
          else
          {
              tempset_min_cool_rx=deal_data[0];
              tempset_max_cool_rx=deal_data[1];
          }
          
           
}
/********************************************************************************
********************************************************************************/
void rx_data_system_windboard(void)
{
      unsigned char i,j,k;
      
          i=0;j=0;
            //自动导风板状态
           if(deal_data[0]&0x80)
           {
            pointcheck1_data[19]=0xff;
           }
           else
           {
             pointcheck1_data[19]=0;
           }
           
           
           if(wind_board_enable)
           {
             //集中控制导风板
             if(deal_data[1]&0x08)
             {
               if(deal_data[1]&0x10)i=1;
               else i=2;
               j=(deal_data[1]>>5)&0x07;
             }

             if(i==1)
             {
               if(wind_board_status==0)
               {
               wind_board_status=1;
               wind_board_status_write=1;
               tx_change_flag=1;
               }
             }
             else if(i==2)
             {
               if(wind_board_status)
               {
                wind_board_status=0;
                wind_board_status_write=1;
                tx_change_flag=1;
               }    
               k=j+1;
               if(wind_board_angle!=k)
               {
                  wind_board_angle=k;
                  wind_board_angle_write=1;
                  tx_change_flag=1;
               }
             }
             
           }
}
/********************************************************************************
********************************************************************************/
void rx_data_center_windboard(void)
{
     
          
             //集中控制导风板
         if(deal_data[1]&0x08)
         {
              //四个导风机一致时
             if(winddir_enable==2) 
             {           
             if(wind_horizontal_status!=wind_board_status)
             wind_horizontal_status_write=1;    
             wind_horizontal_status=wind_board_status;
             
             if(wind_board2_status!=wind_board_status)
             wind_board2_status_write=1;
             wind_board2_status=wind_board_status;
             
             if(wind_horizontal2_status!=wind_board_status)
             wind_horizontal2_status_write=1;
             wind_horizontal2_status=wind_board_status;
             
             if(wind_horizontal_angle!=wind_board_angle)
             wind_horizontal_angle_write=1; 
             wind_horizontal_angle=wind_board_angle;
             
             if(wind_board2_angle!=wind_board_angle)
             wind_board2_angle_write=1;  
             wind_board2_angle=wind_board_angle;
             
             if(wind_horizontal2_angle!=wind_board_angle)
             wind_horizontal2_angle_write=1;  
             wind_horizontal2_angle=wind_board_angle;
                       
             }
        
           }

           
}
/********************************************************************************
********************************************************************************/
void rx_data_system_wind(void)
{
  unsigned char i;
  
  //风量
              if(deal_data[0]&0x10)
              {
                i=deal_data[0]&0x0f;
                if((system_wind_canntchange==0)&&(i!=system_wind))
                {
                  system_wind=i;
                  system_wind_write=1;
                  tx_change_flag=1;
                  
                  
                 if(power_wind_flag)
                 {
                  power_wind_flag=0;
                  power_wind_flag_write=1;
                  power_wind_check_flag_b=0;
                  power_wind_check_flag_b_write=1;
                 }
                if(high_temp_disinfect_flag)
                {
                  high_temp_disinfect_flag=0;
                  high_temp_disinfect_flag_write=1;
                  high_temp_disinfect_flag_b=0;
                  high_temp_disinfect_flag_b_write=1;
                }
                 if(forest_wind_flag)
                {
                  forest_wind_flag=0;
                  forest_wind_flag_write=1;
                 }
                 
                 
                 
                }
              }
              
}
/********************************************************************************
********************************************************************************/
void rx_data_system_filterreset(void)
{
  if((filter_flag)&&(deal_data[0]&0x02))
   {
          filter_reset=1;
          tx_time_1s_cnt=0;
          tx_change_flag=1;
   }
}
/********************************************************************************
********************************************************************************/
void rx_data_system_other(void)
{
           pointcheck1_data[14]=deal_data[0];//停机理由
           basedata[ee_nuber][0]=deal_data[1];//同一冷媒系统室内机台数
           basedata[ee_nuber][3]=deal_data[2];//室外机机种代码
}
/********************************************************************************
********************************************************************************/
void base_data_rx_deal(void)
{
  
          unsigned char j;

         
          
          
          deal_data[0]=rx_data[28];
          deal_data[1]=rx_data[29];
          deal_data[2]=rx_data[30];
          rx_data_system_status();
          

          deal_data[0]=rx_data[28];
          deal_data[1]=rx_data[29];
          deal_data[2]=rx_data[30];
          rx_data_system_mode();
        
          deal_data[0]=rx_data[11];
          deal_data[1]=rx_data[14];
          rx_data_system_err();
       


         //错误代码不同时 发基本数据
           //if(err_backup[ee_nuber]!=rx_data[14])
           //{
            // tx_change_flag=1;
           //}
          // err_backup[ee_nuber]=rx_data[14];
           /////////////////////////////////////////////
          

           
           deal_data[0]=rx_data[32];
           deal_data[1]=rx_data[33];
           rx_data_system_tempset();

           

           deal_data[0]=rx_data[29];
           deal_data[1]=rx_data[32];
           rx_data_system_tryrun();
           
           deal_data[0]=rx_data[34];
           deal_data[1]=rx_data[35];
           rx_data_system_maxmin();
          
           
           
           
           deal_data[0]=rx_data[10];
           deal_data[1]=rx_data[32];
           rx_data_system_windboard();
         
           
           deal_data[0]=rx_data[31];
           rx_data_system_wind();
          

           deal_data[0]=rx_data[32];
           rx_data_system_filterreset();
          
           deal_data[0]=rx_data[15];
           deal_data[1]=rx_data[16];
           deal_data[2]=rx_data[36];
           rx_data_system_other();
           
          
            
            

          if(((option_data[ee_nuber][3]&0x80)==0)&&(server_set_status==0)&&(initialize_flag))
            {
              //加热器制热 C1
              if(option_data[ee_nuber][0]&0x40)rx_data[18]|=0x40;
              else rx_data[18]&=0xbf;
              //滤网时间 b4
              rx_data[19]=(rx_data[19]&0xf0)+(option_data[ee_nuber][1]&0x0f);
              for(j=0;j<10;j++)
               {
                 if(option_data[ee_nuber][j]!=rx_data[18+j])
                   //option_data_write[ee_nuber]|=0x0001<<j;
                 option_data[ee_nuber][j]=rx_data[18+j];
               }

            }

          
          //////////////////////////
         basedata_7_backup[ee_nuber]=rx_data[37];//
          basedata[ee_nuber][7]=rx_data[37];
          basedata[ee_nuber][5]=rx_data[28];
          basedata[ee_nuber][6]=rx_data[29];
          basedata[ee_nuber][16]=rx_data[10];
          basedata[ee_nuber][17]=rx_data[11];
          basedata[ee_nuber][8]=rx_data[38];
          basedata[ee_nuber][9]=rx_data[39];
          basedata[ee_nuber][10]=rx_data[40];
          basedata[ee_nuber][11]=rx_data[41];
          basedata[ee_nuber][12]=rx_data[30];
          basedata[ee_nuber][13]=rx_data[31];
          basedata[ee_nuber][14]=rx_data[32];
          basedata[ee_nuber][15]=rx_data[33];
          basedata[ee_nuber][18]=rx_data[17];      
          basedata[ee_nuber][28]=rx_data[13];
          
          if(inroom_2g5_protocol&(0x0001<<ee_nuber)==0)initialize1_check|=0x0001<<ee_nuber;
          
          
  
}
/********************************************************************************
********************************************************************************/
void base_data_rx_new_deal(void)
{
  
         

                 
          
          
          deal_data[0]=rx_data[18];
          deal_data[1]=rx_data[19];
          deal_data[2]=rx_data[20];
          rx_data_system_status();
          
          deal_data[0]=rx_data[18];
          deal_data[1]=rx_data[19];
          deal_data[2]=rx_data[20];
          deal_data[3]=rx_data[24];
          rx_data_system_mode();
          
          deal_data[0]=rx_data[11];
          deal_data[1]=rx_data[14];
          rx_data_system_err();
          
          deal_data[0]=rx_data[22];
          deal_data[1]=rx_data[23];
          rx_data_system_tempset();
          
          deal_data[0]=rx_data[19];
          deal_data[1]=rx_data[22];
          rx_data_system_tryrun();
          
          deal_data[0]=rx_data[25];
          deal_data[1]=rx_data[26];
          rx_data_system_maxmin();
          
          deal_data[0]=rx_data[10];
          deal_data[1]=rx_data[22];
          rx_data_system_windboard();
          
          deal_data[0]=rx_data[21];
          rx_data_system_wind();
          
          deal_data[0]=rx_data[22];
          rx_data_system_filterreset();
          
          deal_data[0]=rx_data[15];
          deal_data[1]=rx_data[16];
          deal_data[2]=rx_data[27];
          rx_data_system_other();
          
          
          basedata[ee_nuber][7]=rx_data[28];
          basedata[ee_nuber][5]=rx_data[18];
          basedata[ee_nuber][6]=rx_data[19];  
          basedata[ee_nuber][16]=rx_data[10];
          basedata[ee_nuber][17]=rx_data[11];
          basedata[ee_nuber][12]=rx_data[20];
          basedata[ee_nuber][13]=rx_data[21];
          basedata[ee_nuber][14]=rx_data[22];
          basedata[ee_nuber][15]=rx_data[23];
          basedata[ee_nuber][18]=rx_data[17];
          basedata[ee_nuber][19]=rx_data[31];   
          basedata[ee_nuber][24]=rx_data[33];
          basedata[ee_nuber][25]=rx_data[34];       
          basedata[ee_nuber][26]=rx_data[14];
          basedata[ee_nuber][27]=rx_data[37];       
          basedata[ee_nuber][28]=rx_data[13];       
          basedata[ee_nuber][29]=rx_data[32];        
          basedata[ee_nuber][36]=rx_data[36];
          
          
          if(inroom_2g5_protocol&(0x0001<<ee_nuber)==0)initialize1_check|=0x0001<<ee_nuber;
}
/********************************************************************************
********************************************************************************/
void base_data_rx_new1_deal(void)
{
  
          deal_data[0]=rx_data[18];
          deal_data[1]=rx_data[19];
          deal_data[2]=rx_data[20];
          rx_data_system_status();
          
          deal_data[0]=rx_data[18];
          deal_data[1]=rx_data[19];
          deal_data[2]=rx_data[20];
          deal_data[3]=rx_data[24];
          rx_data_system_mode();
          
          deal_data[0]=rx_data[11];
          deal_data[1]=rx_data[14];
          rx_data_system_err();
          
          deal_data[0]=rx_data[22];
          deal_data[1]=rx_data[23];
          rx_data_system_tempset();
          
          deal_data[0]=rx_data[19];
          deal_data[1]=rx_data[22];
          rx_data_system_tryrun();
          
          deal_data[0]=rx_data[25];
          deal_data[1]=rx_data[26];
          rx_data_system_maxmin();
          
          deal_data[0]=rx_data[10];
          deal_data[1]=rx_data[22];
          rx_data_system_windboard();
                   
          rx_data_center_windboard();
          
          deal_data[0]=rx_data[21];
          rx_data_system_wind();
          
          deal_data[0]=rx_data[22];
          rx_data_system_filterreset();
          
          deal_data[0]=rx_data[15];
          deal_data[1]=rx_data[16];
          deal_data[2]=rx_data[27];
          rx_data_system_other();
          
            
          
          basedata[ee_nuber][7]=rx_data[28];
          basedata[ee_nuber][5]=rx_data[18];
          basedata[ee_nuber][6]=rx_data[19];
          basedata[ee_nuber][16]=rx_data[10];
          basedata[ee_nuber][17]=rx_data[11];
          basedata[ee_nuber][12]=rx_data[20];
          basedata[ee_nuber][13]=rx_data[21];
          basedata[ee_nuber][14]=rx_data[22];
          basedata[ee_nuber][15]=rx_data[23];
          basedata[ee_nuber][18]=rx_data[17];
          basedata[ee_nuber][19]=rx_data[31];
          basedata[ee_nuber][24]=rx_data[33];
          basedata[ee_nuber][25]=rx_data[34];       
          basedata[ee_nuber][26]=rx_data[14];
          basedata[ee_nuber][27]=rx_data[37];       
          basedata[ee_nuber][28]=rx_data[13];       
          basedata[ee_nuber][29]=rx_data[32]; 
          basedata[ee_nuber][35]=rx_data[39];
          basedata[ee_nuber][36]=rx_data[36];
          
          if(inroom_2g5_protocol&(0x0001<<ee_nuber)==0)initialize1_check|=0x0001<<ee_nuber;
}
/********************************************************************************
********************************************************************************/
void extend_tx_enable(void)
{
  /*
  baseextend_tx_flag=1;
  already_tx_nuber=0;
  tx_system_nuber1=basedata[already_tx_nuber][1];tx_dress_nuber1=basedata[already_tx_nuber][2];
  tx_system_nuber2=basedata[already_tx_nuber][1];tx_dress_nuber2=basedata[already_tx_nuber][2];
  */
}
/********************************************************************************
********************************************************************************/
void extendbase_data_rx_new_deal(void)
{
  unsigned char j;


  if((option_data[ee_nuber][3]&0x80)==0)   
   {
        
              for(j=0;j<5;j++)
               {
                 if(option_data[ee_nuber][j]!=rx_data[10+j])
                 {
                   option_data_write[ee_nuber]|=0x0001<<j;
                    if(initialize_flag)extend_tx_enable();                       
                 }
                 option_data[ee_nuber][j]=rx_data[10+j];
               }
              for(j=0;j<5;j++)
               {
                 if(option_data[ee_nuber][5+j]!=rx_data[25+j])
                 {
                   option_data_write[ee_nuber]|=0x0001<<(5+j);
                  if(initialize_flag)extend_tx_enable();                
                 }
                 option_data[ee_nuber][5+j]=rx_data[25+j];
               }
              for(j=0;j<9;j++)
               {
                 if(option_data_new[ee_nuber][j]!=rx_data[15+j])
                 {
                   option_data_new_write[ee_nuber]|=0x0001<<j;
                  if(initialize_flag)  extend_tx_enable();         
                 }
                 option_data_new[ee_nuber][j]=rx_data[15+j];
                 
                 if((initialize_flag==0)&&((option_backcome_txflag&(0x0001<<ee_nuber))==0))option_new_backup[ee_nuber][j]=rx_data[15+j];
               }
              
              
                option_data_new[ee_nuber][3]&=0x7f; 
                option_new_backup[ee_nuber][3]&=0x7f;  //option清除
              
              
     }

   //室内机生产编号
          basedata[ee_nuber][8]=rx_data[30];
          basedata[ee_nuber][9]=rx_data[31];
          basedata[ee_nuber][10]=rx_data[32];
          basedata[ee_nuber][11]=rx_data[33];
  
  
}
/********************************************************************************
********************************************************************************/
void extendbase_data_rx_new1_deal(void)
{
  unsigned char j;

 if((option_data[ee_nuber][3]&0x80)==0)
  {
              for(j=0;j<5;j++)
              {
                 if(option_data[ee_nuber][j]!=rx_data[10+j])
                {
                   option_data_write[ee_nuber]|=0x0001<<j;
                   if(initialize_flag)extend_tx_enable();           
                 }
                 option_data[ee_nuber][j]=rx_data[10+j];
               }
              for(j=0;j<5;j++)
               {
                 if(option_data[ee_nuber][5+j]!=rx_data[25+j])
                {
                   option_data_write[ee_nuber]|=0x0001<<(5+j);
                   if(initialize_flag)extend_tx_enable(); 
                 }
                 option_data[ee_nuber][5+j]=rx_data[25+j];
               }        
              for(j=0;j<9;j++)
               {
                 if(option_data_new[ee_nuber][j]!=rx_data[15+j])
                 {
                  option_data_new_write[ee_nuber]|=0x0001<<j;
                  if(initialize_flag)extend_tx_enable(); 
                 }
                 option_data_new[ee_nuber][j]=rx_data[15+j];     
                 
                 if((initialize_flag==0)&&((option_backcome_txflag&(0x0001<<ee_nuber))==0))option_new_backup[ee_nuber][j]=rx_data[15+j];
               }
              for(j=0;j<7;j++)
               {
                 if(option_data_new1[ee_nuber][j]!=rx_data[34+j])
                 {
                   option_data_new1_write[ee_nuber]|=0x0001<<j;
                   if(initialize_flag)extend_tx_enable();          
                 }
                 option_data_new1[ee_nuber][j]=rx_data[34+j];
                 
                 if((initialize_flag==0)&&((option_backcome_txflag&(0x0001<<ee_nuber))==0))option_new1_backup[ee_nuber][j]=rx_data[34+j];
               }
              
              
              option_data_new[ee_nuber][3]&=0x7f; 
              option_new_backup[ee_nuber][3]&=0x7f;  //option清除
     }

   //室内机生产编号
          basedata[ee_nuber][8]=rx_data[30];
          basedata[ee_nuber][9]=rx_data[31];
          basedata[ee_nuber][10]=rx_data[32];
          basedata[ee_nuber][11]=rx_data[33];
  
  
}
/********************************************************************************
********************************************************************************/
void g25_basedata_rx_deal(void)
{
 
  
  

  

  

  

  

  

  
  
  basedata[ee_nuber][30]=rx_data[10];
  basedata[ee_nuber][31]=rx_data[11];
  basedata[ee_nuber][32]=rx_data[12];
  basedata[ee_nuber][33]=rx_data[13];
  basedata[ee_nuber][34]=rx_data[16];
  
  basedata[ee_nuber][38]=rx_data[17];
  basedata[ee_nuber][39]=rx_data[18];
  
  basedata[ee_nuber][37]=rx_data[19];
  
  basedata[ee_nuber][40]=rx_data[21];
  basedata[ee_nuber][41]=rx_data[24];//2.5G 集控 
  basedata[ee_nuber][42]=rx_data[22];
  basedata[ee_nuber][43]=rx_data[23];
  
  
  if(initialize1_flag)
  {
    
    
  if((rx_data[17]&0x60)==0x40)//自清洁终止
  {
    self_clean_flag_b&=((0x0001<<ee_nuber)^0xffff); 
    self_clean_flag_b_write=1;
  }
  
    if((rx_data[23]&0x03)==0x02)//强力终止
  {
    power_wind_check_flag_b&=((0x0001<<ee_nuber)^0xffff); 
    power_wind_check_flag_b_write=1;
  }
  
  if((rx_data[23]&0x0c)==0x08)//高温杀菌终止
  {
    high_temp_disinfect_flag_b&=((0x0001<<ee_nuber)^0xffff); 
    high_temp_disinfect_flag_b_write=1;
  }
  
  }
  
  
  initialize1_check|=0x0001<<ee_nuber;
  
}
/********************************************************************************
********************************************************************************/
void g25_extenddata_rx_deal(void)
{
   if((option_data[ee_nuber][3]&0x80)==0)
  {
    if(option_data_new[ee_nuber][10]!=rx_data[10])
    {
      option_data_new[ee_nuber][10]=rx_data[10];
      option_data_new_write[ee_nuber]|=(0x0001<<10);
    }
    if((option_data_new[ee_nuber][11]&0x7f)!=(rx_data[11]&0x7f))
    {
      option_data_new[ee_nuber][11]=rx_data[11];
      option_data_new_write[ee_nuber]|=(0x0001<<11);
    }
    
    if((initialize_flag==0)&&((option_backcome_txflag&(0x0001<<ee_nuber))==0))
    {
      option_new_backup[ee_nuber][10]=rx_data[10];
      option_new_backup[ee_nuber][11]=rx_data[11];
    }
    
   
    
  }

  
}
/********************************************************************************
********************************************************************************/
void point1_data_rx_deal(void)
{
  
  if(system_mode==0)pointcheck1_data[0]=0;
  else
    pointcheck1_data[0]= tempset;//设定温度 b 1

  pointcheck1_data[1]= rx_data[10];//回风空气温度 b 2
  pointcheck1_data[2]= rx_data[11];//出风空气温度 b 3
  pointcheck1_data[3]= rx_data[12];//液管温度  b 4
  pointcheck1_data[4]= rx_data[13];//远程传感器温度  b 5
  pointcheck1_data[5]= rx_data[14];//环境温度 b 6
  pointcheck1_data[6]= rx_data[15];//气管温度 b 7
  pointcheck1_data[7]= rx_data[16];//蒸发温度 b 8
  pointcheck1_data[8]= rx_data[17];//冷凝温度 b 9
  pointcheck1_data[9]= rx_data[18];//排气温度 b A
  pointcheck1_data[10]=temp_value; //temp_value;//线控器传感器温度 b b
  pointcheck1_data[11]=rx_data[33];//湿度
  pointcheck1_data[12]= rx_data[36];//室内芯片IO状态 C1
  pointcheck1_data[13]= rx_data[37];//室外芯片I/O状态 C2
  //pointcheck1_data[14]= rx_data[41];//停机理由  d 1
  pointcheck1_data[15]= rx_data[28];//报警发生次数 E 1
  pointcheck1_data[16]= rx_data[29];//瞬停次数 E 2
  pointcheck1_data[17]= rx_data[30];//停线控器与水模块通信错误次数 E 3
  pointcheck1_data[18]= rx_data[31];//变频器故障次数 E 4
  //pointcheck1_data[19]= rx_data[28];//自动导风板状态F1
  pointcheck1_data[20]= rx_data[19];//排气压力 H1
  pointcheck1_data[21]= rx_data[20];//吸气压力 H2
  pointcheck1_data[22]= rx_data[21];//要求频率 H3
  pointcheck1_data[23]= rx_data[22];//实际运转频率 H4
  pointcheck1_data[24]= rx_data[32];//室内机容量 J1
  //pointcheck1_data[25]= rx_data[21];//室外机种代码 J 2
  pointcheck1_data[26]= rx_data[3];//冷媒系统（显示方式1） J 3
  pointcheck1_data[27]= rx_data[3];//冷媒系统（显示方式2） J 4
  pointcheck1_data[28]= rx_data[23];//室内膨胀阀开度 L 1
  pointcheck1_data[29]= rx_data[24];//室外膨胀阀1开度 L 2
  pointcheck1_data[30]= rx_data[25];//室外膨胀阀2开度 L 2
  pointcheck1_data[31]= rx_data[26];//室外膨胀阀B开度 L 2
  pointcheck1_data[32]= rx_data[27];//压机电流 P1 
  pointcheck1_data[33]= rx_data[38];//压机运转时间高位 P2
  pointcheck1_data[34]= rx_data[40];//人感传感器反应率q1
  pointcheck1_data[35]= rx_data[41];//地板温度传感器反应率q2 
  
}
/********************************************************************************
********************************************************************************/
void point2_data_rx_deal(void)
{
    pointcheck2_data[0]=rx_data[10];//回风空气温度 q1
    pointcheck2_data[1]=rx_data[11];//出风空气温度 q2
    pointcheck2_data[2]=rx_data[12];//冻结温度 q3
    pointcheck2_data[3]=rx_data[13];//外气温度 q4
    pointcheck2_data[4]=rx_data[14];//室内热交换器气管温度 q5
    pointcheck2_data[5]=rx_data[15];//制热蒸发温度 q6
    pointcheck2_data[6]=rx_data[16];//冷凝温度 q7
    pointcheck2_data[7]=rx_data[17];//压缩机顶部温度  q8
    pointcheck2_data[8]=rx_data[18];//排气（高压）压力 q9
    pointcheck2_data[9]=rx_data[19];//吸气（低压）压力 qa
    pointcheck2_data[10]=rx_data[20];//要求频率 qb
    pointcheck2_data[11]=rx_data[21];//运转实际频率 qc
    pointcheck2_data[12]=rx_data[22];//室内机膨胀阀开度 qd
    pointcheck2_data[13]=rx_data[23];//室外机膨胀阀开度1 qe
    pointcheck2_data[14]=rx_data[24];//压缩机）电流 qf
  
  
}
/********************************************************************************
********************************************************************************/
void machine_type_rx_deal(void)
{
  
  
  
}
/********************************************************************************
********************************************************************************/
void a3d_wind_rx_deal(void)
{
   
      

     a3d_wind_use=1;
     a3d_wind_cnt=0;


     if(rx_data[17]) err_now_flag=1;
     a3d_wind_err= rx_data[17];

     if(rx_data[18]&0x01)
     {
       a3d_wind_tx_flag=1;
       
     }
  
  
}
/********************************************************************************
********************************************************************************/
void rx_data_line_10to13(void)
{
  unsigned char i,_temp1;
  
   _temp1=(deal_data[0]>>2)&0x1f;
  if((_temp1!=(system_mode&0x1f))&&(initialize_flag))
  {
    system_mode=_temp1;
    system_mode_write=1;
    
    if(system_mode==0)
    {
     
      clear_eeprom_flag=1;
      clear_eeprom_flag_write=1;
          
    }
  
  }
  
  if((system_mode==0x04)&&(machine_type1==3))
   {
     system_mode=0x44;
   }

   if(deal_data[0]&0x01)
   {
     center_control_rx_flag=1;
     center_useon_flag=1;
     system_off_1s_flag=1;
     i=tx_change_flag;
     system_on_deal(); 
     tx_change_flag=i;
     //
     if(system_status==0)
     {
       line_control_tx=1;
       single_line_tx_flag=1;
     }
     //
     center_control_rx_flag=0;
   }
   else 
   {
     i=tx_change_flag;
     system_off_deal();
     tx_change_flag=i;
   }
     
   



    _temp1=deal_data[1]&0x0f;  
    if(((system_wind&0x0f)!=_temp1)&&(initialize_flag))
    {     
      system_wind_write=1;  
      system_wind=_temp1;        
    }
 


         if(wind_board_enable)
           {
             if((deal_data[1]&0x10)==0)
             {
               if(wind_board_status)
               {
                  wind_board_status=0;
                  wind_board_status_write=1;
               }

               i=((deal_data[1]>>5)&0x07)+1;
               if(wind_board_angle!=i)
               {
               wind_board_angle=i;
               wind_board_angle_write=1;
               }


             }
             else
             {
               if(wind_board_status==0)
               {
               wind_board_status=1;
               wind_board_status_write=1;
               wind_board_changing=wind_board_angle;/////////

               }

             }
           }


     if(deal_data[3]&0x80)
     {
       if((try_run_flag==0)&&(system_status))
       {
        try_run_flag=1;
        try_run_flag_write=1;     
        
        try_run_set=0;
          try_run_set_write=1;

          pre_on_flag=0;
          pre_on_flag_write=1;
          pre_off_flag=1;
          pre_off_flag_write=1;
          pre_off_time=4;
          pre_off_time_write=1;
          pre_off_minu=0;
          pre_off_time_b=pre_off_time;
          pre_off_time_b_write=1;
          try_run_fre=30;
       }
       else
       {
        if((try_run_set==0)&&(system_status==0))
         {
         try_run_set=1;
         try_run_set_write=1;
         try_run_fre=30;
        
         }
       }
      }
     else
     {
       if(try_run_set)
       {
         try_run_set=0;
         try_run_set_write=1;
       }
       if(try_run_flag)
       {
          try_run_flag=0;
          try_run_flag_write=1;
       }
     }

      if(deal_data[3]&0x80)
      {
        if(deal_data[2]==0)
       {
         try_run_status=0;
       }
       else if(fre_machine)
       {
         try_run_status=1;
         try_run_fre=deal_data[2];
       }
      }
     else if(system_mode==0x44)
     {
       if(dry_set!=deal_data[2])
       {
         dry_set=deal_data[2];
         dry_set_write=1;
       }
     }
      else if(initialize_flag)
      {
        if(tempset!=deal_data[2])
        {
          tempset=deal_data[2];
          tempset_write=1;
             
        }
        if((clear_eeprom_flag)&&(initialize1_flag)&&(tempset!=0xff))
            {
              clear_eeprom_flag=0;
              clear_eeprom_flag_write=1;
            }      
        
        if((half_set)&&(line_g25_flag==0))
        {
          half_set=0;
          half_set_write=1;
        }
       
      }

   


  if(deal_data[3]&0x01)
  {
     if(err_now_flag)
    {
      a3d_wind_err=0;
      err_now_flag=0;
      err_reset_flag=0xffff>>(16-total_machine);

      line_err_reset_flag=1;
      /*
      for(i=0;i<CONST_16;i++)
      {
        basedata[i][4]&=0xfe;
      }
      */
    }
    
   }
   
   if(deal_data[3]&0x02)filter_reset=1;

  switch((deal_data[3]>>4)&0x03)
  {
  case 0x01:i=0x01;break;
  case 0x02:i=0x02;break;
  case 0x03:i=0x03;break;
  default:i=0x01;break;
  }

  if(i!=machine_type)
  {
    machine_type=i;
    machine_type_write=1;
  }
  
}
/********************************************************************************
********************************************************************************/
void  rx_data_line_16(void)
{
  //if(deal_data[0]&0x80)line_g25_flag=1;
  //else line_g25_flag=0;
  
  if((wind_enable>1)&&(initialize_flag))
  {
    if(deal_data[0]&0x40)system_wind|=0x10;
    else system_wind&=0xef;
    system_wind_write=1;  
  }

}
/********************************************************************************
********************************************************************************/
void line_base_data_rx_deal(void)
{

  if((rx_data[10]&0x80)==0)
  {
    line_protocol=0;//线控器为1G
  }
  if((rx_data[16]&0x03)==0)
  {
    line_protocol&=0xfd;//线控器为2G
  }
  
  if(rx_data[16]&0x80)line_g25_flag=1;
  else line_g25_flag=0;
  

  deal_data[0]=rx_data[10];
  deal_data[1]=rx_data[11];
  deal_data[2]=rx_data[12];
  deal_data[3]=rx_data[13];
  rx_data_line_10to13();
  
  deal_data[0]=rx_data[16];
  rx_data_line_16();
  
  //if(rx_data[25]&0x01)line_control_tx=1;
 

     if(initialize_flag)line_change_flag=1;
  
}
/********************************************************************************
********************************************************************************/
void line_base_data_rx_new_deal(void)
{
  
  unsigned char i;

  if((rx_data[10]&0x80)==0)
  {
    line_protocol=0;//线控器为1G 
  }
  if((rx_data[16]&0x03)==0)
  {
    line_protocol&=0xfd;//线控器为2G
  }

  if(rx_data[16]&0x80)line_g25_flag=1;
  else line_g25_flag=0;

  
  deal_data[0]=rx_data[10];
  deal_data[1]=rx_data[11];
  deal_data[2]=rx_data[12];
  deal_data[3]=rx_data[13];
  rx_data_line_10to13();
  
  deal_data[0]=rx_data[16];
  rx_data_line_16();

  
  
  
 if((machine_type1==4)&&(system_mode&0x04))
 {
   i=system_mode;
  switch(rx_data[15]&0x03)
  {
  case 0x00:i=0x84;break; 
  case 0x01:i=0xa4;break; 
  case 0x02:i=0xc4;break;
  }
  if(system_mode!=i)
  {
    system_mode=i;
    system_mode_write=1;
  }
 }
  
  if(rx_data[15]&0x10)compress_preheat_reset=1;
 
  //if(rx_data[18]&0x01)line_control_tx=1;


  if(master_flag)
  {
    if(rx_data[21]==0)
    {
      line_06_flag=1;
      por_06_flag=1;     
      //////////////////
      for(i=0;i<total_machine;i++)
      {
       basedata[i][5]|=0x40;
      }
      //////////////////
    }
    else
    {
      line_06_flag=0;
      por_06_flag=0;     
      //////////////////
      for(i=0;i<total_machine;i++)
      {
       basedata[i][5]&=0xbf;
      }
      //////////////////
      
    }
  }


  if(rx_data[20]&0x10)center_onoff_flag=0x01;
  else if(rx_data[20]&0x80)center_onoff_flag=0x02;


 
  if(initialize_flag)line_change_flag=1;
  
}
/********************************************************************************
********************************************************************************/
void line_base_data_rx_new1_deal(void)
{
  
   unsigned char i,j;
   
  if(rx_data[16]&0x80)line_g25_flag=1;
  else line_g25_flag=0;
  
  deal_data[0]=rx_data[10];
  deal_data[1]=rx_data[11];
  deal_data[2]=rx_data[12];
  deal_data[3]=rx_data[13];
  rx_data_line_10to13();
  
  deal_data[0]=rx_data[16];
  rx_data_line_16();

  
  
  
 if((machine_type1==4)&&(system_mode&0x04))
 {
   i=system_mode;
  switch(rx_data[15]&0x03)
  {
  case 0x00:i=0x84;break; 
  case 0x01:i=0xa4;break; 
  case 0x02:i=0xc4;break;
  }
  if(system_mode!=i)
  {
    system_mode=i;
    system_mode_write=1;
  }
 }
  
  if(rx_data[15]&0x10)compress_preheat_reset=1;
 
  //if(rx_data[18]&0x01)line_control_tx=1;


  if(master_flag)
  {
    if(rx_data[21]==0)
    {
      line_06_flag=1;
      por_06_flag=1;     
      //////////////////
      for(i=0;i<total_machine;i++)
      {
       basedata[i][5]|=0x40;
      }
      //////////////////
    }
    else
    {
      line_06_flag=0;
      por_06_flag=0;     
      //////////////////
      for(i=0;i<total_machine;i++)
      {
       basedata[i][5]&=0xbf;
      }
      //////////////////
      
    }
  }


  if(rx_data[20]&0x10)center_onoff_flag=0x01;
  else if(rx_data[20]&0x80)center_onoff_flag=0x02;
  
  
  //24字节
 if(winddir_enable)
 {
  if((rx_data[23]&0xf0)==0)
  {
    if(wind_board_set==0)
    {
      wind_board_set=1;
      wind_board_set_write=1;
    }
  }
  else
  {
    if(wind_board_set)
    {
      wind_board_set=0;
      wind_board_set_write=1;
    }
  }
 }
  
  
  //25字节
  if((rx_data[23]&0x10)&&((winddir_enable==2)||(winddir_enable==5)))
  {
  i=rx_data[24]&0x0f;
  if(i&0x01)
  {
    if(wind_board_status==0)
     {
        wind_board_status=1;
        wind_board_status_write=1;
     }   
  }
  else
  {
    if(wind_board_status)
    {
      wind_board_status=0;
      wind_board_status_write=1;
    }
    j=(i>>1)+1;
    if(wind_board_angle!=j)
    {
      wind_board_angle=j;
      wind_board_angle_write=1;
    }
  }
  }
  
  if((rx_data[23]&0x20)&&((winddir_enable==2)||(winddir_enable==5)))
  {
  i=(rx_data[24]>>4)&0x0f;
  if(i&0x01)
  {
     if(wind_horizontal_status==0)
     {
     wind_horizontal_status=1;
     wind_horizontal_status_write=1;
     }
  }
  else
  {
     if(wind_horizontal_status)
    {
    wind_horizontal_status=0;
    wind_horizontal_status_write=1;
    }
    j=(i>>1)+1;
    if(wind_horizontal_angle!=j)
    {
      wind_horizontal_angle=j;
      wind_horizontal_angle_write=1;
    }
  }
  }
  
  
  //26字节
  if((rx_data[23]&0x40)&&((winddir_enable==2)||(winddir_enable==5)))
  {
  i=rx_data[25]&0x0f;
  if(i&0x01)
  {
    if(wind_board2_status==0)
     {
        wind_board2_status=1;
        wind_board2_status_write=1;
     }   
  }
  else
  {
    if(wind_board2_status)
    {
      wind_board2_status=0;
      wind_board2_status_write=1;
    }
    j=(i>>1)+1;
    if(wind_board2_angle!=j)
    {
      wind_board2_angle=j;
      wind_board2_angle_write=1;
    }
  }
  }
  
  
  if((rx_data[23]&0x80)&&((winddir_enable==2)||(winddir_enable==5)))
  {
  i=(rx_data[25]>>4)&0x0f;
  if(i&0x01)
  {
     if(wind_horizontal2_status==0)
     {
     wind_horizontal2_status=1;
     wind_horizontal2_status_write=1;
     }
  }
  else
  {
     if(wind_horizontal2_status)
    {
    wind_horizontal2_status=0;
    wind_horizontal2_status_write=1;
    }
    j=(i>>1)+1;
    if(wind_horizontal2_angle!=j)
    {
      wind_horizontal2_angle=j;
      wind_horizontal2_angle_write=1;
    }
  }
  }
  
  if(initialize_flag)line_change_flag=1;
  
}
/********************************************************************************
********************************************************************************/
void extendbase_line_data_rx_new_deal(void)
{
   unsigned char i;

              if(option_f1!=(rx_data[16]))
              {
                option_f1=rx_data[16];
                option_f1_write=1;
                
                for(i=0;i<total_machine;i++)
                {
                  option_data_new[i][1]=option_f1;
                  option_data_new_write[i]|=0x0002;
                }
              }
              /////////////////
              
              for(i=0;i<total_machine;i++)
                {
                  if(option_data_new[i][5]!=rx_data[20])
                  {
                  option_data_new[i][5]=rx_data[20];
                  option_data_new_write[i]|=0x0001<<5;
                  }
                  
                  if(option_data_new[i][6]!=rx_data[21])
                  {
                  option_data_new[i][6]=rx_data[21];
                  option_data_new_write[i]|=0x0001<<6;
                  }
                  
                  if(option_data_new[i][7]!=rx_data[22])
                  {
                  option_data_new[i][7]=rx_data[22];
                  option_data_new_write[i]|=0x0001<<7;
                  }
                  
                  if(option_data_new[i][8]!=rx_data[23])
                  {
                  option_data_new[i][8]=rx_data[23];
                  option_data_new_write[i]|=0x0001<<8;
                  }
                  
                }

  
  
}
/********************************************************************************
********************************************************************************/
void extendbase_line_data_rx_new1_deal(void)
{
  unsigned char i;
  
              if(option_f1!=(rx_data[16]))
              {
                option_f1=rx_data[16];
                option_f1_write=1;

                for(i=0;i<total_machine;i++)
                {
                  option_data_new[i][1]=option_f1;
                  option_data_new_write[i]|=0x0002;
                }
              }

              
               for(i=0;i<total_machine;i++)
                {
                  if(option_data_new[i][5]!=rx_data[20])
                  {
                  option_data_new[i][5]=rx_data[20];
                  option_data_new_write[i]|=0x0001<<5;
                  }
                  
                  if(option_data_new[i][6]!=rx_data[21])
                  {
                  option_data_new[i][6]=rx_data[21];
                  option_data_new_write[i]|=0x0001<<6;
                  }
                  
                  if(option_data_new[i][7]!=rx_data[22])
                  {
                  option_data_new[i][7]=rx_data[22];
                  option_data_new_write[i]|=0x0001<<7;
                  }
                  
                  if(option_data_new[i][8]!=rx_data[23])
                  {
                  option_data_new[i][8]=rx_data[23];
                  option_data_new_write[i]|=0x0001<<8;
                  }
                  
                  ///
                  if(option_data_new1[i][0]!=rx_data[30])
                  {
                  option_data_new1[i][0]=rx_data[30];
                  option_data_new1_write[i]|=0x0001<<0;
                  }
                  
                  if(option_data_new1[i][1]!=rx_data[31])
                  {
                  option_data_new1[i][1]=rx_data[31];
                  option_data_new1_write[i]|=0x0001<<1;
                  }
                  
                   if(option_data_new1[i][2]!=rx_data[32])
                  {
                  option_data_new1[i][2]=rx_data[32];
                  option_data_new1_write[i]|=0x0001<<2;
                  }
                  
                   if(option_data_new1[i][3]!=rx_data[33])
                  {
                  option_data_new1[i][3]=rx_data[33];
                  option_data_new1_write[i]|=0x0001<<3;
                  }
                  
                   if(option_data_new1[i][4]!=rx_data[34])
                  {
                  option_data_new1[i][4]=rx_data[34];
                  option_data_new1_write[i]|=0x0001<<4;
                  }
                  
                   if(option_data_new1[i][5]!=rx_data[35])
                  {
                  option_data_new1[i][5]=rx_data[35];
                  option_data_new1_write[i]|=0x0001<<5;
                  }
                }

  
  
}
/********************************************************************************
********************************************************************************/
void g25_line_fuction_deal(void)
{
  unsigned char i;
    //睡眠
  i=rx_data[10]&0x03;
  if(i)
  {
    if(sleep_flag==0)
    {
      //fuction_clear_deal();
      sleep_flag=1;
      sleep_flag_write=1;
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
         
      //return;
    }
  }
  else
  {
    if(sleep_flag)
    {
      sleep_flag=0;
      sleep_flag_write=1;
      if(option_preoff==0)
      {
      pre_off_flag=0;
      pre_off_flag_write=1;
      }
    }
  }
   
  //节能
  i=(rx_data[10]>>2)&0x03;
  if(i)
  {
    if(save_flag==0)
    {
      //fuction_clear_deal();
      save_flag=1;
      save_flag_write=1;
      //return;
    }
  }
  else
  {
    if(save_flag)
    {
      save_flag=0;
      save_flag_write=1;
    }
  }
  
   //静音
    if(rx_data[14]&0x04)
    {
      if(mute_flag==0)
      {
        //fuction_clear_deal();
        mute_flag=1;
        mute_flag_write=1;
        //return;
      }
    }
    else
    {
      if(mute_flag)
      {
        mute_flag=0;
        mute_flag_write=1;
      }
    }
    

    //辅热
    if(rx_data[14]&0x08)
    {
      if(heat_flag==0)
      {
        //fuction_clear_deal();
        heat_flag=1;
        heat_flag_write=1;
        //return;
      }
    }
    else
    {
      if(heat_flag)
      {
        heat_flag=0;
        heat_flag_write=1;
      }
    }
    
   
     //健康
    if(rx_data[14]&0x20)
    {
      if(health_flag==0)
      {
        //fuction_clear_deal();
        health_flag=1;
        health_flag_write=1;
        //return;
      }
    }
    else
    {
      if(health_flag)
      {
        health_flag=0;
        health_flag_write=1;
      }
    }

  
     //18字节   
    //森林风
      if(rx_data[17]&0x02)
      {
        if(forest_wind_flag==0)
        {
          //fuction_clear_deal();
          forest_wind_flag=1;
          forest_wind_flag_write=1;
          
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
          //return;
        }
      }
      else
      {
        if(forest_wind_flag)
        {
          forest_wind_flag=0;
          forest_wind_flag_write=1;
        }
      }  
  
  
  
   
  
}
/********************************************************************************
********************************************************************************/
void g25_line_control_rx_deal(void)
{
  unsigned char i,j;
  
  line_g25_flag=1;

  if(initialize_flag==0)return;
  
 
 
  g25_line_fuction_deal();

  
  switch(rx_data[11]&0x07)
  {
  case 0:i=0;break;
  case 1:i=1;break;
  case 2:i=2;break;
  case 4:i=3;break;
  case 5:i=4;break;
  case 6:i=5;break;
  default:i=0;break;
  }
  if(human_flag!=i)
  {
    human_flag=i;
    human_flag_write=1;
  }
 
  
  //13字节
  
  i=rx_data[12]&0x0f;
  if(i&0x01)
  {
    if(wind_board_status==0)
     {
        wind_board_status=1;
        wind_board_status_write=1;
     }   
  }
  else
  {
    if(wind_board_status)
    {
      wind_board_status=0;
      wind_board_status_write=1;
    }
    j=(i>>1)+1;
    if(wind_board_angle!=j)
    {
      wind_board_angle=j;
      wind_board_angle_write=1;
    }
  }
  
  i=(rx_data[12]>>4)&0x0f;
  if(i&0x01)
  {
     if(wind_horizontal_status==0)
     {
     wind_horizontal_status=1;
     wind_horizontal_status_write=1;
     }
  }
  else
  {
     if(wind_horizontal_status)
    {
    wind_horizontal_status=0;
    wind_horizontal_status_write=1;
    }
    j=(i>>1)+1;
    if(wind_horizontal_angle!=j)
    {
      wind_horizontal_angle=j;
      wind_horizontal_angle_write=1;
    }
  }
  
  
  //14字节
  
  i=rx_data[13]&0x0f;
  if(i&0x01)
  {
    if(wind_board2_status==0)
     {
        wind_board2_status=1;
        wind_board2_status_write=1;
     }   
  }
  else
  {
    if(wind_board2_status)
    {
      wind_board2_status=0;
      wind_board2_status_write=1;
    }
    j=(i>>1)+1;
    if(wind_board2_angle!=j)
    {
      wind_board2_angle=j;
      wind_board2_angle_write=1;
    }
  }
  
  i=(rx_data[13]>>4)&0x0f;
  if(i&0x01)
  {
     if(wind_horizontal2_status==0)
     {
     wind_horizontal2_status=1;
     wind_horizontal2_status_write=1;
     }
  }
  else
  {
     if(wind_horizontal2_status)
    {
    wind_horizontal2_status=0;
    wind_horizontal2_status_write=1;
    }
    j=(i>>1)+1;
    if(wind_horizontal2_angle!=j)
    {
      wind_horizontal2_angle=j;
      wind_horizontal2_angle_write=1;
    }
  }
  
    i=rx_data[14]&0x03; 
    if(wind_mode!=i)
    {
      wind_mode=i;
      wind_mode_write=1;
    }
    
    
   
 
    //0.5度
    if(rx_data[14]&0x40)i=1;
    else i=0;      
    if((half_set!=i)&&(half_set_change==0))
    {
      half_set=i;
      half_set_write=1;
    }
  
  
    
      //自清洁
      if(rx_data[17]&0x04)
      {
        if(self_clean_flag==0)
        {
          self_clean_flag=1;
          self_clean_flag_write=1;
          self_clean_flag_b=0xffff>>(16-total_machine);
          self_clean_flag_b_write=1;
        }
      }
      else
      {
        if(self_clean_flag)
        {
          self_clean_flag=0;
          self_clean_flag_write=1;
          self_clean_flag_b=0;
          self_clean_flag_b_write=1;
        }
      } 
  
}
/********************************************************************************
********************************************************************************/
void g25_extenddata_line_rx_deal(void)
{
  
  if((option_data[ee_nuber][3]&0x80)==0)
  {
    if(option_data_new[ee_nuber][10]!=rx_data[10])
    {
      option_data_new[ee_nuber][10]=rx_data[10];
      option_data_new_write[ee_nuber]|=(0x0001<<10);
    }
    if((option_data_new[ee_nuber][11]&0x7f)!=(rx_data[11]&0x7f))
    {
      option_data_new[ee_nuber][11]=rx_data[11];
      option_data_new_write[ee_nuber]|=(0x0001<<11);
    }
    
  }
  
  if(rx_data[12]&0x01)
  {
    if(option_hh==0)
      {
        option_hh=1;
        option_hh_write=1;
      }
  }
  else
  {
    if(option_hh)
      {
        option_hh=0;
        option_hh_write=1;
      }
  }
  
  
  
}
/********************************************************************************
********************************************************************************/
void save_option_rx_deal(void)
{
  
}
/********************************************************************************
********************************************************************************/



