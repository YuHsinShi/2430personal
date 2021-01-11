#define UART_TX_GLOBALS


#include "uart_tx.h"


#define LINE_CONTROL_TX          1
#define TIMEING_TX               2
#define BASEEXTEND_TX            3
#define CONNECT_INFORMATION_TX   4
#define GROUP_MESSAGE_TX         5
#define GROUP_MESSAGE_EXTEND_TX  6
#define POINT_CHECK_TX1          7
#define POINT_CHECK_TX2          8

#define DRESSCHANG_REQUEST_TX    9
#define DRESSCHANGE_CONFIRM_TX   10
#define BASEEXTEND_LINE_TX       11
#define LINE_DRESSCHANG_REQUEST_TX    12
#define LINE_DRESSCHANG_FINISH_TX     13
#define LINE_DRESSCHANG_CONFIRM_TX    14
#define LINE_GROUP_MESSAGE_TX         15 
#define LINE_GROUP_MESSAGE_EXTEND_TX  16 


#define BOARDCHECK_START_TX     17
#define BOARDCHECK_END_TX       18
#define G25_LINE_CONTROL_TX     19
#define G25_EXTEND_LINE_TX      20
#define G25_BASEDATA_TX         21
#define G25_EXTENDATA_TX        22
#define A3D_WIND_TX             23
#define AUTO_CLEAR_TX           24

#define MACHINE_TYPE_TX         25
#define SAVE_OPTION_TX         26



#if 1 // porting modified


unsigned char  system_status;//系统开关机
unsigned char system_mode;//系统模式  共20个模式
unsigned char system_wind;//系统风量
unsigned char tempset;//设定温度
unsigned char tempset_auto;//自动模式设定温度
unsigned char tempset_cool;//制冷模式设定温度
unsigned char tempset_heat;//制热模式设定温度
unsigned char half_set;//0.5度设定
unsigned char dry_set;//系统设定湿度

unsigned char 	mode_enable;//模式类型  大于40种组合
unsigned char 	wind_enable;//风量类型 0：3档   1：3档+自动   2：4档    3：4档+自动  4：6档    5：6档+自动


unsigned char temp_ad_value;//温度传感器ad值 
unsigned char temp_value;//温度传感器温度值


unsigned char wind_mode;//风向模式  0：普通风向  1：3D风  2：超远风
unsigned char  wind_board_enable;//是否有导风板
unsigned char  winddir_enable;//导风板类型 0：无导风板 1：有一个导风板  2：四方向导风板 3：一方向导风板  4：3D出风口 5：二方向导风板
unsigned char  wind_board_status;//导风板1是否摆动 1：摆动   0：停止
unsigned char  wind_board_angle;//导风板1摆动角度 范围1~7
unsigned char  wind_horizontal_status;//导风板2是否摆动 1：摆动   0：停止
unsigned char  wind_horizontal_angle;//导风板2摆动角度 范围1~7
unsigned char  wind_board2_status;//导风板3是否摆动 1：摆动   0：停止
unsigned char  wind_board2_angle;//导风板3摆动角度 范围1~7
unsigned char  wind_horizontal2_status;//导风板4是否摆动 1：摆动   0：停止
unsigned char  wind_horizontal2_angle;//导风板4摆动角度 范围1~7
unsigned char  wind_board_set;//导风板统一设定 1：为统一设定 0：为单独设定


unsigned char save_flag,save_used,save_set_enable;//节能标志,节能是否有效、节能是否可以操作
unsigned char sleep_flag,sleep_used,sleep_set_enable;//睡眠标志,睡眠是否有效、睡眠是否可以操作
unsigned char mute_flag,mute_used,mute_set_enable;//静音标志,静音是否有效、静音是否可以操作
unsigned char health_flag,health_used,health_set_enable;//健康标志,健康是否有效、健康是否可以操作
unsigned char heat_flag,heat_used,heat_set_enable;//辅热标志,辅热是否有效、辅热是否可以操作
unsigned char forest_wind_flag,forest_wind_used,forest_wind_set_enable;//森林风标志,森林风是否有效、森林风是否可以操作

unsigned char human_flag,human_used,human_set_enable;//人感标志,人感是否有效、人感是否可以操作
unsigned char human_sensor,human_sensor_used;//人感2标志,人感2有效

unsigned char 	self_clean_flag,self_clean_used,self_clean_enable;//自清洁标志,自清洁是否有效、自清洁是否可以操作
unsigned int 	  self_clean_flag_b;//各室内机自清洁标志
unsigned char 	high_temp_disinfect_flag,high_temp_disinfect_used,high_temp_disinfect_set_enable;//高温杀标志,高温杀是否有效、高温杀是否可以操作
unsigned int 	  high_temp_disinfect_flag_b;//各室内机高温杀标志
unsigned char 	power_wind_check_flag,power_wind_flag_used,power_wind_set_enable;//强力标志,强力是否有效、强力是否可以操作
unsigned int 	  power_wind_check_flag_b;//各室内机强力标志


unsigned char d1_d3_check;
unsigned char initialize_10ms;
unsigned char initialize_flag;  //系统初始化标志    1：表示初始化结束
unsigned char initialize_cnt;  //系统初始化计数
unsigned char initialize_step; //系统初始化步骤
unsigned char line_init_flag;
unsigned char tx_system_nuber1;//通信对象冷媒系统
unsigned char tx_dress_nuber1;//通信对象冷媒地址
unsigned char tx_system_nuber2;//通信对象冷媒系统
unsigned char tx_dress_nuber2;//通信对象冷媒地址
unsigned char line_control_double;//有亲子线控器
unsigned char master_flag;//亲子标志 0：亲线控器 1：子线控器



unsigned char already_tx_nuber;//正在发码的室内机序号 从0至（total_machine-1）
unsigned char basedata[16][50];//用于保存室内机数据  basedata[i][1]表示室内机系统  basedata[i][2]表示室内机地址

unsigned char option_data[16][50];//各台室内机机能设定数据1G
unsigned char option_data_new[16][50];//各台室内机机能设定数据2G
unsigned char option_data_new1[16][50];//各台室内机机能设定数据3G

unsigned char option_backup[16][50];//各台室内机机能设定数据1G临时数组
unsigned char option_new_backup[16][50];//各台室内机机能设定数据2G临时数组
unsigned char option_new1_backup[16][50];//各台室内机机能设定数据3G临时数组

unsigned char option_all_set[16];//各台室内机机能设定数据1G共同设定
unsigned char option_all_set_new[16];//各台室内机机能设定数据2G共同设定
unsigned char option_all_set_new1[16];//各台室内机机能设定数据3G共同设定



unsigned char line_protocol;//线控器1、2、3G支持标志
unsigned char line_g25_flag;//线控器2.5G支持标志
unsigned int  inroom_2g5_protocol;//各台室内机机2.5G支持标志
unsigned int	inroom_2g_protocol;//各台室内机机2G支持标志
unsigned int  inroom_3g_protocol;//各台室内机机3G支持标志
unsigned char	total_machine;//实际连接的室内机台数





unsigned char tx_change_flag;//状态改变发码标志 1：表示有改变 发码后为0
unsigned char tx_time_1s_cnt;//状态改变发码计数

unsigned char server_set_status;//服务设定状态 为1时表示正处于设定状态，不能开机
unsigned char a3d_wind_use;//是否有3D出风口
unsigned char a3d_wind_err;//3D出风口报警


unsigned char compress_preheat_reset;//压缩机预热解除
unsigned char emergency_set_status;
unsigned char emergency_flag;//应急运转标志
unsigned char try_run_flag;//试运行标志
unsigned char try_run_set;//试运行设定标志
unsigned char try_run_fre;//试运行频率



unsigned char hh_flag;//品牌 0：日立  1：海信  2：约克




unsigned char  center_control_all;//集控全禁止
unsigned char center_control_onoff;//集控开关机禁止
unsigned char 	center_onoff_flag;////集控开关机禁止下允许开机标志
unsigned char center_control_mode;//集控模式禁止
unsigned char center_control_wind;//集控风量禁止
unsigned char center_control_windboard;//集控导风板禁止
unsigned char center_control_tempset;//集控设定温度禁止



unsigned char  machine_type;//室内机机型
unsigned char  machine_type1;//室内机机型1

unsigned char err_now_flag;//正在报警标志
unsigned char  err_reset_flag;//报警复位
unsigned char  filter_reset;//滤网复位


unsigned char tempset_min_warm_rx;//制热设定温度下限
unsigned char tempset_max_warm_rx;//制热设定温度上限
unsigned char tempset_min_cool_rx;//制冷设定温度下限
unsigned char tempset_max_cool_rx;//制冷设定温度上限
unsigned char pointcheck1_data[50];//点检1接收数据
unsigned char pointcheck2_data[16];//点检2接收数据






unsigned char line_change_flag	;
unsigned char option_tx_flag;
unsigned char option_set_flag;
unsigned char single_line_tx_flag;
unsigned char half_set_change;


unsigned char initialize1_check;

unsigned char wifi_power_control;
unsigned char wifi_tx_flag;
unsigned char wifi_moudle_set;
unsigned char confirm_system_status;


unsigned char option_hh;
unsigned char temporary_system;
unsigned char temporary_dress;


unsigned char backlight_cnt;
unsigned char wind_board_set_3s;

unsigned char 	option_dress;
unsigned char try_run_status;

unsigned char 	line_06_sent;

unsigned char 	por_06_flag;
unsigned char  option_f1;
unsigned char self_clean_flag;



unsigned char a3d_wind_led_flag;

unsigned char system_tx_10ms;
unsigned char self_check_flag;
unsigned char tx_change_tx_flag;
unsigned int tx_time_systemon_cnt;
unsigned char tx_systemon_flag;
unsigned char wifi_connect_ok;
unsigned char cloud_servers_cmd;
unsigned char did0_data;
unsigned char did0_lenth;

unsigned char err_4s_cnt;
unsigned char  err_4s_nuber;
unsigned char self_tx_data;
unsigned char  confirm_run_nuber;

unsigned char    all_package_windnot_flag;
unsigned char 	   ignore_flag;
unsigned char	line_control_dress;
unsigned char tt_nuber;

unsigned char tt_data;
unsigned char master_flag_write;










#endif

void (*TxFuncPtr)(); 

typedef struct{
        unsigned char Tx_Index; 
        unsigned char Multi; 
        void (*CurrentOperate)(); 
} KbdTabStruct;

const KbdTabStruct TXTab[]={
{1,0,(*line_control_tx_deal2)},
{2,1,(*timing_tx_deal2)},
{3,1,(*baseextend_tx_deal2)},
{4,1,(*connect_information_tx_deal)},
{5,1,(*group_message_tx_deal)},
{6,1,(*group_message_extend_tx_deal)},
{7,0,(*point_check_tx1_deal)},
{8,0,(*point_check_tx2_deal)},

{9,0,(*dresschange_request_tx_deal)},
{10,0,(*dresschange_confirm_tx_deal)},
{11,0,(*baseextend_line_tx_deal2)},
{12,0,(*line_dresschange_request_tx_deal)},
{13,0,(*line_dresschange_finish_tx_deal)},
{14,0,(*line_dresschange_confirm_tx_deal)},
{15,0,(*line_group_message_tx_deal)},
{16,0,(*line_group_message_extend_tx_deal)},

{17,0,(*boardcheck_start_tx_deal)},
{18,0,(*boardcheck_end_tx_deal)},
{19,0,(*g25_line_control_tx_deal)},
{20,0,(*g25_extenddata_line_tx_deal)},
{21,2,(*g25_basedata_tx_deal)},
{22,2,(*g25_extenddata_tx_deal)},
{23,0,(*a3d_wind_tx_deal)},
{24,1,(*auto_clear_address)},
{25,2,(*machine_type_tx_deal)},
{26,2,(*save_option_tx_deal)},
};
/********************************************************************************
********************************************************************************/
void init_tx_deal(void)
{
  unsigned char i=0,j=0,step_time=0;

 // if(d1_d3_check==0)return;
  initialize_10ms++;

  if((initialize_10ms>=100)&&(initialize_flag==0))
  {
    initialize_10ms=0;
    initialize_cnt++;

    switch(initialize_step)
    {
    case 0:step_time=4;break;//4s 
    case 1:step_time=5;break;//5s   
    case 2:step_time=5;break;//5s
    case 3:step_time=5;break;//5s
    case 4:step_time=3;break;//3s
    case 5:step_time=3;break;//3s
    case 6:step_time=6;break;//6s
    default:step_time=4;break;//4s
    }


    if(initialize_cnt>=step_time)
    {
      initialize_cnt=0;
      initialize_step++;
     printf(" [Hlink] initialize_step=%d\r\n",initialize_step);
      switch(initialize_step)
      {
      case 1:     

	
	for(i=0;i<16;i++)
	{
	  basedata[i][1]=0xff;
	  basedata[i][2]=0xff;
	
		  for(j=3;j<40;j++)
		  {
			basedata[i][j]=0;
		  } 	
		  
	
		  
	}
            line_control_tx=1;line_init_flag=1;
            tx_system_nuber1=0xff;tx_dress_nuber1=0xff;
            tx_system_nuber2=0xff;tx_dress_nuber2=0xff;        
             break;
      case 2:
            if((line_control_double==0)&&(master_flag))
            {
              master_flag=0;
              master_flag_write=1;
            }
            connect_information_tx=0;
            group_message_tx=0;
            group_message_extend_tx=0;
            backlight_cnt=0;

            timing_tx_flag=1;
            tx_system_nuber1=0xff;tx_dress_nuber1=0xff;
            tx_system_nuber2=0xff;tx_dress_nuber2=0xff;
             break;
      case 3:
            a3d_wind_tx_flag=1;
            timing_tx_flag=1;
            tx_system_nuber1=0xff;tx_dress_nuber1=0xff;
            tx_system_nuber2=0xff;tx_dress_nuber2=0xff;
             break;
      case 4:     
	  	printf("total_machine=%d\r\n",total_machine);
            connect_information_tx=1;
            already_tx_nuber=0;
            tx_system_nuber1=basedata[already_tx_nuber][1];tx_dress_nuber1=basedata[already_tx_nuber][2];
            tx_system_nuber2=basedata[already_tx_nuber][1];tx_dress_nuber2=basedata[already_tx_nuber][2];
             break;
      case 5:
            //if((line_control_double)&&(master_flag==0))line_group_message_tx=1;
            //else group_message_tx=1;
            //printf("total_machine=(%d)",total_machine);
			
            group_message_tx=1;
            already_tx_nuber=0;
            tx_system_nuber1=basedata[already_tx_nuber][1];tx_dress_nuber1=basedata[already_tx_nuber][2];
            tx_system_nuber2=basedata[already_tx_nuber][1];tx_dress_nuber2=basedata[already_tx_nuber][2];
			 

             break;
     case 6:
          if((line_g25_flag)&&(line_control_double))
          {
            g25_extend_line_tx_flag=1;
          }
          else if(inroom_2g_protocol)
          {
              baseextend_tx_flag=1;
              already_tx_nuber=0;
              while((inroom_2g_protocol&0x0001<<already_tx_nuber)==0)
                {
                  already_tx_nuber++;
                  if(already_tx_nuber>=total_machine)
                  {
                    already_tx_nuber=0;
                    baseextend_tx_flag=0;
                    break;
                  }
                }    
            tx_system_nuber1=basedata[already_tx_nuber][1];tx_dress_nuber1=basedata[already_tx_nuber][2];
            tx_system_nuber2=basedata[already_tx_nuber][1];tx_dress_nuber2=basedata[already_tx_nuber][2];
          }
             break;
     case 7:
            initialize_flag=1;           
            tx_change_flag=1;
            tx_time_1s_cnt=100;
            initialize1_check=0;
            
            wifi_power_control=1;
            wifi_tx_flag=1;
            wifi_moudle_set=1;
            
             break;

      }

    }
  }
}
/********************************************************************************
********************************************************************************/
void tx_ack_next_deal(void)
{
  unsigned char nuber=0;
  
  if(next_tx_flag==0)return;
  
  printf("ok3 \n");

                
  if(next_tx_flag==TIMEING_TX)
  {
        nuber=get_nuber();
        if((server_set_status==0)&&(initialize_flag)&&((inroom_2g_protocol&(0x0001<<nuber))==0))option_data[nuber][3]&=0x7f; //如果没有2G室内机 则清0
        
       already_tx_nuber++;
 
       if(already_tx_nuber>=total_machine)
       {
         already_tx_nuber=0;
         timing_tx_flag=0;

         
         if(initialize_flag==0)
         {
           
         }
         else if(inroom_2g5_protocol)
         {
                    g25_basedata_tx_flag=1;                 
                    already_tx_nuber=0;       
                    while((inroom_2g5_protocol&(0x0001<<already_tx_nuber))==0)
                    {
                      already_tx_nuber++;
                      if(already_tx_nuber>=total_machine)
                      {
                        already_tx_nuber=0;
                        g25_basedata_tx_flag=0;
                        break;
                       }
                    }                                      
                 tx_system_nuber1=basedata[already_tx_nuber][1];tx_dress_nuber1=basedata[already_tx_nuber][2];
                 tx_system_nuber2=basedata[already_tx_nuber][1];tx_dress_nuber2=basedata[already_tx_nuber][2];

         }
         else if(a3d_wind_use)
         {
           a3d_wind_tx_flag=1;
         }
         else if(option_tx_flag&0x01)
         {
           if(line_control_double)
           {
            baseextend_line_tx_flag=1;
           }
           else
           {
           baseextend_tx_flag=1;
           already_tx_nuber=0;
           
                while((inroom_2g_protocol&0x0001<<already_tx_nuber)==0)
                {
                  already_tx_nuber++;
                  if(already_tx_nuber>=total_machine)
                  {
                    already_tx_nuber=0;
                    baseextend_tx_flag=0;
                    break;
                  }
               }    
           tx_system_nuber1=basedata[already_tx_nuber][1];tx_dress_nuber1=basedata[already_tx_nuber][2];
           tx_system_nuber2=basedata[already_tx_nuber][1];tx_dress_nuber2=basedata[already_tx_nuber][2];
           }
         }
       }
       else
       {
         tx_system_nuber1=basedata[already_tx_nuber][1];tx_dress_nuber1=basedata[already_tx_nuber][2];
         tx_system_nuber2=basedata[already_tx_nuber][1];tx_dress_nuber2=basedata[already_tx_nuber][2];
       }
  }
  else if(next_tx_flag==BASEEXTEND_TX)
  {
       nuber=get_nuber(); 
       if(((server_set_status==0)||(server_set_status==0x93))&&(initialize_flag)&&((inroom_2g5_protocol&(0x0001<<nuber))==0))option_data[nuber][3]&=0x7f;//如果没有2.5G室内机 则清0
    
     do
     {
       already_tx_nuber++;
       if(already_tx_nuber>=total_machine)
       {
         already_tx_nuber=0;
         baseextend_tx_flag=0;
	 /////////////////////////
	   if(inroom_2g5_protocol)
           {
	        g25_extenddata_tx_flag=1;

                 already_tx_nuber=0;    
                 while((inroom_2g5_protocol&(0x0001<<already_tx_nuber))==0)
                {
                  already_tx_nuber++;
                  if(already_tx_nuber>=total_machine)
                  {
                    already_tx_nuber=0;
                    g25_extenddata_tx_flag=0;
                    break;
                    }
                 }    
         
                 tx_system_nuber1=basedata[already_tx_nuber][1];tx_dress_nuber1=basedata[already_tx_nuber][2];
                 tx_system_nuber2=basedata[already_tx_nuber][1];tx_dress_nuber2=basedata[already_tx_nuber][2];

            }
           else 
           {
             option_tx_flag=0;
           }
           break;
	 //////////////////////////
       }
     }
     while((inroom_2g_protocol&0x0001<<already_tx_nuber)==0);
          
       tx_system_nuber1=basedata[already_tx_nuber][1];tx_dress_nuber1=basedata[already_tx_nuber][2];
       tx_system_nuber2=basedata[already_tx_nuber][1];tx_dress_nuber2=basedata[already_tx_nuber][2]; 
  }
  else if(next_tx_flag==G25_BASEDATA_TX)
  {
         do
         {
           already_tx_nuber++;
           if(already_tx_nuber>=total_machine)
           {
             already_tx_nuber=0;
             g25_basedata_tx_flag=0;
             
             if(a3d_wind_use)
             {
               a3d_wind_tx_flag=1;
             }
             else if(option_tx_flag&0x01)
             {              
               if(line_control_double)
              {
                baseextend_line_tx_flag=1;
              }
              else
              {
                baseextend_tx_flag=1;
                already_tx_nuber=0;
                
                while((inroom_2g_protocol&0x0001<<already_tx_nuber)==0)
                {
                  already_tx_nuber++;
                  if(already_tx_nuber>=total_machine)
                  {
                    already_tx_nuber=0;
                    baseextend_tx_flag=0;
                    break;
                  }
                }    
                
                tx_system_nuber1=basedata[already_tx_nuber][1];tx_dress_nuber1=basedata[already_tx_nuber][2];
                tx_system_nuber2=basedata[already_tx_nuber][1];tx_dress_nuber2=basedata[already_tx_nuber][2];
              }
           
             }
             break;
           }
         } 
         while((inroom_2g5_protocol&(0x0001<<already_tx_nuber))==0);              
         tx_system_nuber1=basedata[already_tx_nuber][1];tx_dress_nuber1=basedata[already_tx_nuber][2];
         tx_system_nuber2=basedata[already_tx_nuber][1];tx_dress_nuber2=basedata[already_tx_nuber][2];      
  }
  else if(next_tx_flag==G25_EXTENDATA_TX)
  {    
        nuber=get_nuber();  
        if((server_set_status==0)&&(initialize_flag))option_data[nuber][3]&=0x7f;       
  
         do
         {
           already_tx_nuber++;
           if(already_tx_nuber>=total_machine)
           {
             already_tx_nuber=0;
             g25_extenddata_tx_flag=0;           
             option_tx_flag=0;           
             break;
           }
         } 
         while((inroom_2g5_protocol&(0x0001<<already_tx_nuber))==0);
       
       tx_system_nuber1=basedata[already_tx_nuber][1];tx_dress_nuber1=basedata[already_tx_nuber][2];
       tx_system_nuber2=basedata[already_tx_nuber][1];tx_dress_nuber2=basedata[already_tx_nuber][2];
  }
  else if(next_tx_flag==DRESSCHANG_REQUEST_TX)
  {
      dresschange_request_tx=0;
  }
  else if(next_tx_flag==DRESSCHANGE_CONFIRM_TX)
  {
      dresschange_confirm_tx=0;
  }
  else if(next_tx_flag==POINT_CHECK_TX1)
  {
       point_check_tx1=0;
  }
  else if(next_tx_flag==POINT_CHECK_TX2)
  {
     point_check_tx2=0;
  }
  else if(next_tx_flag==CONNECT_INFORMATION_TX)
  {
       already_tx_nuber++;

       if(already_tx_nuber>=total_machine)
       {
         already_tx_nuber=0;
         connect_information_tx=0;
       } 
       else
       {
       tx_system_nuber1=basedata[already_tx_nuber][1];tx_dress_nuber1=basedata[already_tx_nuber][2];
       tx_system_nuber2=basedata[already_tx_nuber][1];tx_dress_nuber2=basedata[already_tx_nuber][2];
       }
  }
   else if(next_tx_flag==GROUP_MESSAGE_TX)
  {
       already_tx_nuber++;

       if(already_tx_nuber>=total_machine)
       {
         already_tx_nuber=0;
         group_message_tx=0;
       }   
       else
       {
       tx_system_nuber1=basedata[already_tx_nuber][1];tx_dress_nuber1=basedata[already_tx_nuber][2];
       tx_system_nuber2=basedata[already_tx_nuber][1];tx_dress_nuber2=basedata[already_tx_nuber][2];
       }
  }
  else if(next_tx_flag==GROUP_MESSAGE_EXTEND_TX)
  {
       already_tx_nuber++;
      
       if(already_tx_nuber>=total_machine)
       {
         already_tx_nuber=0;
         group_message_extend_tx=0;
       } 
       else
       {
        tx_system_nuber1=basedata[already_tx_nuber][1];tx_dress_nuber1=basedata[already_tx_nuber][2];
       tx_system_nuber2=basedata[already_tx_nuber][1];tx_dress_nuber2=basedata[already_tx_nuber][2];
       }
  }
  else if(next_tx_flag==MACHINE_TYPE_TX)
  {
       machine_type_tx=0;
  } 
  else if(next_tx_flag==SAVE_OPTION_TX)
  {
       save_option_tx=0;
  } 
  else if(next_tx_flag==AUTO_CLEAR_TX)
  {
       already_tx_nuber++;
       if(already_tx_nuber>=total_machine)
       {
         already_tx_nuber=0;
         auto_clear_tx=0;
       }
       else
       {
        tx_system_nuber1=basedata[already_tx_nuber][1];tx_dress_nuber1=basedata[already_tx_nuber][2];
       tx_system_nuber2=basedata[already_tx_nuber][1];tx_dress_nuber2=basedata[already_tx_nuber][2];
       }
      
  }
   
    else if(next_tx_flag==BOARDCHECK_START_TX)
    {
      boardcheck_start_tx=0;
    }
    else if(next_tx_flag==BOARDCHECK_END_TX)
    {
      boardcheck_end_tx=0;
    }
  else if(next_tx_flag==LINE_CONTROL_TX)
  {
     line_control_tx=0;

     if(initialize_flag)
     {
       option_set_flag=0;
       
       if(single_line_tx_flag)
       {
         single_line_tx_flag=0;
       }
       else if((line_g25_flag)&&(line_control_double))
       {
         g25_line_control_tx_flag=1;
       }
       else
       {
         timing_tx_flag=1;
         already_tx_nuber=0;
         tx_system_nuber1=basedata[0][1];tx_dress_nuber1=basedata[0][2];
         tx_system_nuber2=basedata[0][1];tx_dress_nuber2=basedata[0][2];
       }

     }
     else
     {
        if(single_line_tx_flag)
       {
         single_line_tx_flag=0;
       }
       else if(master_flag)
       { 
         timing_tx_flag=1;
         already_tx_nuber=0;
         tx_system_nuber1=basedata[0][1];tx_dress_nuber1=basedata[0][2];
         tx_system_nuber2=basedata[0][1];tx_dress_nuber2=basedata[0][2];

       }
     }
  }
  else if(next_tx_flag==BASEEXTEND_LINE_TX)
  {
       baseextend_line_tx_flag=0;      
       if((line_g25_flag)&&(line_control_double))
       {
         g25_extend_line_tx_flag=1;
       }
       else
       {
           baseextend_tx_flag=1;
           already_tx_nuber=0;
                while((inroom_2g_protocol&0x0001<<already_tx_nuber)==0)
                {
                  already_tx_nuber++;
                  if(already_tx_nuber>=total_machine)
                  {
                    already_tx_nuber=0;
                    baseextend_tx_flag=0;
                    break;
                  }
                }    
           tx_system_nuber1=basedata[already_tx_nuber][1];tx_dress_nuber1=basedata[already_tx_nuber][2];
           tx_system_nuber2=basedata[already_tx_nuber][1];tx_dress_nuber2=basedata[already_tx_nuber][2];
                
       }                 
  }
  else if(next_tx_flag==G25_LINE_CONTROL_TX)
  { 
        g25_line_control_tx_flag=0;
        half_set_change=0;////////////////////////////

       
         timing_tx_flag=1;
         already_tx_nuber=0;
         tx_system_nuber1=basedata[already_tx_nuber][1];tx_dress_nuber1=basedata[already_tx_nuber][2];
         tx_system_nuber2=basedata[already_tx_nuber][1];tx_dress_nuber2=basedata[already_tx_nuber][2];
    
  }
  else if(next_tx_flag==G25_EXTEND_LINE_TX)
  {      
       g25_extend_line_tx_flag=0;
     
       baseextend_tx_flag=1;
       already_tx_nuber=0;
       
                while((inroom_2g_protocol&0x0001<<already_tx_nuber)==0)
                {
                  already_tx_nuber++;
                  if(already_tx_nuber>=total_machine)
                  {
                    already_tx_nuber=0;
                    baseextend_tx_flag=0;
                    break;
                    }
                }    
       
       tx_system_nuber1=basedata[already_tx_nuber][1];tx_dress_nuber1=basedata[already_tx_nuber][2];
       tx_system_nuber2=basedata[already_tx_nuber][1];tx_dress_nuber2=basedata[already_tx_nuber][2]; 
  }
  else if(next_tx_flag==LINE_DRESSCHANG_REQUEST_TX)
  {
       line_dresschange_request_tx=0;      
  }
  else if(next_tx_flag==LINE_DRESSCHANG_FINISH_TX)
  {
       line_dresschange_finish_tx=0;    
  }
  else if(next_tx_flag==LINE_DRESSCHANG_CONFIRM_TX)
  {
       line_dresschange_confirm_tx=0;         
  }
  else if(next_tx_flag==LINE_GROUP_MESSAGE_TX)
  {
     line_group_message_tx=0;
     group_message_tx=1;
     //if(line_protocol==0)line_group_message_extend_tx=1;    
  }
  else if(next_tx_flag==LINE_GROUP_MESSAGE_EXTEND_TX)
  {
     line_group_message_extend_tx=0;
  }
   else if(next_tx_flag==A3D_WIND_TX)
    {
      a3d_wind_tx_flag=0;  
      if(option_tx_flag&0x01)
      {              
         if(line_control_double)
         {
           baseextend_line_tx_flag=1;
         }
        else
        {
         baseextend_tx_flag=1;
         already_tx_nuber=0;
               while((inroom_2g_protocol&0x0001<<already_tx_nuber)==0)
                {
                  already_tx_nuber++;
                  if(already_tx_nuber>=total_machine)
                  {
                    already_tx_nuber=0;
                    baseextend_tx_flag=0;
                    break;
                  }
                }    
         tx_system_nuber1=basedata[already_tx_nuber][1];tx_dress_nuber1=basedata[already_tx_nuber][2];
         tx_system_nuber2=basedata[already_tx_nuber][1];tx_dress_nuber2=basedata[already_tx_nuber][2];
        }
              
      }
       //tx_system_nuber1=basedata[already_tx_nuber][1];tx_dress_nuber1=basedata[already_tx_nuber][2];
       //tx_system_nuber2=basedata[already_tx_nuber][1];tx_dress_nuber2=basedata[already_tx_nuber][2];     
    }

  
  next_tx_flag=0;
  no_data_cnt=0;
}
/********************************************************************************
********************************************************************************/
unsigned char get_nuber(void)
{
  
  unsigned char i=0,_nuber=0;

  _nuber=0;
  for(i=0;i<total_machine;i++)
  {
 	if((basedata[i][1]==tx_system_nuber1)&&(basedata[i][2]==tx_dress_nuber1)) 		
 	{
 	  _nuber=i; 	
 	}
  }
        return _nuber;
}
/********************************************************************************
********************************************************************************/
unsigned char get_index(void)
{
	unsigned char i=0,j=0,_Index=0;
	
	for(i=0;i<4;i++)
	{
	  for(j=0;j<8;j++)
	  {
            if(tx_data_flag[i].data&(0x01<<j))
            {
            	_Index=i*8+j;
            	i=4;j=8;
            }
          }
        }
        return _Index;
}
/********************************************************************************
********************************************************************************/
void system_option_check(void)
{
  unsigned char i=0,j=0,k=0,l=0,i2=0,i3=0;
  unsigned int  room2g=0,room3g=0,room2g5=0;
  
  /*
    total_machine=1;
    basedata[0][1]=0x01;
    basedata[0][2]=0x01;
   
 
  basedata[0][30]|=0x04;//节能 
  basedata[0][30]|=0x01;//睡眠 
  //basedata[0][31]|=0x01;//人感
  basedata[0][31]|=0x02;//静音
  basedata[0][31]|=0x10;//辅电
  basedata[0][31]|=0x20;//健康
  basedata[0][37]|=0x08;//森林风
  basedata[0][37]|=0x10;//自清洁
  basedata[0][42]|=0x01;//强力
  basedata[0][42]|=0x10;//高温杀菌
   */


  
  //室内机世代判定
  if(total_machine)
  {
    room2g=0;room3g=0;room2g5=0;
    for(i=0;i<total_machine;i++)
    {
      if(basedata[i][28]&0x80)room2g|=(0x0001<<i);
      if(basedata[i][28]&0x03)room3g|=(0x0001<<i);
      if(basedata[i][28]&0x10)room2g5|=(0x0001<<i);
    }
    inroom_2g_protocol=room2g;
    inroom_3g_protocol=room3g;
    inroom_2g5_protocol=room2g5;
  }

}

/********************************************************************************
********************************************************************************/
void tx_deal(void)
{
	unsigned char i=0;

	line_control_dress=master_flag+1;
/*
	if(initialize_flag)
	{
		tx_system_nuber1=0x01;
		tx_dress_nuber1=0x04;
		tx_system_nuber2=0x01;
		tx_dress_nuber2=0x04;
	}
  */       
         //tx_ack_next_deal();
         system_option_check();
		 

         
         
         if((tx_data_flag[0].data==0)&&(tx_data_flag[1].data==0)&&(tx_data_flag[2].data==0)&&(tx_data_flag[3].data==0))return;

         //if(tx_start)return;
         

         /////////////////////////////////////////////////
         

	   	if((self_tx_flag)&&(no_data_cnt>=random_temp))
       {
           self_tx_flag=0;
           for(i=0;i<10;i++)
           {
             tx_data[i]=self_tx_data;
           }
            tx_total=10;
         
            tx_data_table();  
            set_data_deal();              
            return;
       }
       
        /*
		 for(i=0;i<50;i++)
		 {
		   tx_data[i]=0;
		 }*/ 
		 memset(tx_data,50,0);
		 
		Index=get_index();
		tx_clear_flag=0;
		tx_finish_flag=TXTab[Index].Tx_Index;
		tx_multi=TXTab[Index].Multi;
		TxFuncPtr=TXTab[Index].CurrentOperate;
		(*TxFuncPtr)();//执行当前按键的操作
		

			//TODO: send data
			//tx_total: toatal data send
			//tx_data: address 
		int ret;
			


		ret =homebus_senddata(&tx_data[0],tx_total);
		if(ret >=10)
		{
			printf("homebus_senddata fail\n");


		}
					
        
	
	next_tx_flag=tx_finish_flag; 
	tx_finish_flag=0;



		
}
/********************************************************************************
********************************************************************************/
void set_data_deal(void)
{
        tx_nuber=0;
        tx_start=1;
#if 0 //WAIT FOR PORTING to ALT CPU homebus
		SIO00=tx_buffer[tx_nuber];
#endif

		
}
/********************************************************************************
********************************************************************************/
void tx_data_table(void)
{
 unsigned char i=0,k=0,l=0;


  for(i=0;i<tx_total;i++)
  {
   tx_buffer[3*i]=0xaa;
   tx_buffer[3*i+1]=0xaa;
   tx_buffer[3*i+2]=0xfa;

   if(tx_data[i]&0x01)tx_buffer[3*i]|=0x04;
   if(tx_data[i]&0x02)tx_buffer[3*i]|=0x10;
   if(tx_data[i]&0x04)tx_buffer[3*i]|=0x40;

   if(tx_data[i]&0x08)tx_buffer[3*i+1]|=0x01;
   if(tx_data[i]&0x10)tx_buffer[3*i+1]|=0x04;
   if(tx_data[i]&0x20)tx_buffer[3*i+1]|=0x10;
   if(tx_data[i]&0x40)tx_buffer[3*i+1]|=0x40;

   if(tx_data[i]&0x80)tx_buffer[3*i+2]|=0x01;

     l=0;
    for(k=0;k<8;k++)
    {
      if((tx_data[i]>>k)&0x01)l=!l;
    }
   
    if((i==0)||(l))tx_buffer[3*i+2]|=0x04;


  }

}
/********************************************************************************
********************************************************************************/
unsigned char tt_nuber_dress(void)
{
  unsigned char i=0,j=0;
  
     for(i=0;i<total_machine;i++)
      {
 	if((basedata[i][1]==tx_data[5])&&(basedata[i][2]==tx_data[6]))
 	{
 	  j=i;
 	}
      }
     
     return j;
  
}
/********************************************************************************
********************************************************************************/
unsigned char tx_data_10(void)
{
  unsigned char tt_data=0;
  
      tt_data=0;
      tt_data=(system_mode&0x1f)<<2;
      tt_data|=0x80;//新遥控器
      if(system_status)tt_data|=0x01;
      
      if((confirm_system_status)&&(tt_nuber==confirm_run_nuber))tt_data|=0x03;
      
      return tt_data;
}
/********************************************************************************
********************************************************************************/
unsigned char tx_data_11(void)
{
  unsigned char tt_data=0,i=0;
  
      tt_data=0;
      
       i=(basedata[tt_nuber][27]>>2)&0x03;
   if(i)
  {
    switch(system_wind&0x1f)
     {
     case 0x00:tt_data=0x00;break;
     case 0x01:tt_data=0x01;break;   
     case 0x18:tt_data=0x08;break;
     case 0x08:tt_data=0x08;break;
     case 0x14:tt_data=0x04;break;
     case 0x04:tt_data=0x04;break;
     case 0x02:tt_data=0x02;break;
     case 0x12:tt_data=0x02;break;
     default:tt_data=0x04;break;
     }
  }
  else
  {
      switch(system_wind&0x0f)
     {
     case 0x00:tt_data=0x00;break;
     case 0x01:tt_data=0x01;break;
     case 0x02:tt_data=0x02;break;     
     case 0x04:tt_data=0x04;break;    
     case 0x08:tt_data=0x08;break;
     default:tt_data=0x04;break;
     }
  }

      if((all_package_windnot_flag)||((ignore_flag&0x70)==0x70))tt_data&=0xf1;

      if(wind_mode==1)
      {
        tt_data|=0x10;
        tt_data&=0x1f;
      }
      else if(wind_board_enable)
      {
        if(wind_board_status)
        {
          tt_data|=0x10;
          tt_data&=0x1f;
        }
        else
        {
          tt_data&=0xef;
          
          i=wind_board_angle;
          
          switch(i)
          {
          case 1:tt_data&=0x1f;break;
          case 2:tt_data&=0x3f;tt_data|=0x20;break;
          case 3:tt_data&=0x5f;tt_data|=0x40;break;
          case 4:tt_data&=0x7f;tt_data|=0x60;break;
          case 5:tt_data&=0x9f;tt_data|=0x80;break;
          case 6:tt_data&=0xbf;tt_data|=0xa0;break;
          case 7:tt_data&=0xdf;tt_data|=0xc0;break;
          default:tt_data&=0x1f;break;
          }
        }

      }
      else
      {
        tt_data&=0x0f;
      }

       if(hh_flag)tt_data=0;
      
      
      return tt_data;
}
/********************************************************************************
********************************************************************************/
unsigned char tx_data_12(void)
{
  unsigned char tt_data=0;
  
      tt_data=0;
      
      if((try_run_flag)||(try_run_set))
      {
          tt_data=try_run_fre;
      }
      else
      {
        if(system_mode==0)tt_data=0xff;
        else if(center_control_tempset)tt_data=tempset;
        else if(system_mode==0x44)tt_data=dry_set;
        else
        tt_data=tempset;
      }
      
      return tt_data;
}
/********************************************************************************
********************************************************************************/
unsigned char tx_data_13(void)
{
  unsigned char tt_data=0;
  
      tt_data=0;

     switch(machine_type)
      {
      case 1:tt_data|=0x10;break;
      case 2:tt_data|=0x20;break;
      case 3:tt_data|=0x30;break;
      default:break;
      }

      if(system_mode==0x28)tt_data|=0x40;
      if((try_run_flag)||(try_run_set))tt_data|=0x80;
      if(err_reset_flag)tt_data|=0x01;
      if(filter_reset)tt_data|=0x02;
      
      return tt_data;
}
/********************************************************************************
********************************************************************************/
unsigned char tx_data_14(void)
{
  unsigned char tt_data=0;

     tt_data=temp_ad_value;//temp_value;
  
      
      return tt_data;
}
/********************************************************************************
********************************************************************************/
unsigned char tx_data_15(void)
{
  unsigned char tt_data=0;

       tt_data=0;
      if(machine_type1==4)
      {
        switch(system_mode)
        {
        case 0x84:tt_data=0x00;break;
        case 0xa4:tt_data=0x01;break;
        case 0xc4:tt_data=0x02;break;
        default:tt_data=0x00;break;
        }
      }
      if(compress_preheat_reset) tt_data|=0x10;
      if((emergency_set_status)||(emergency_flag)) tt_data|=0x20;
      
      return tt_data;
}
/********************************************************************************
********************************************************************************/
unsigned char tx_data_16(void)
{
  unsigned char tt_data=0;

       tt_data=0;
   
  //  i=(basedata[tt_nuber][27]>>2)&0x03;
  // if(i)
 //  {
     switch(system_wind&0x1f)
     { 
     case 0x18:tt_data|=0x10;break;
     case 0x14:tt_data|=0x10;break;
     case 0x12:tt_data|=0x10;break;
     default:break;
     }
  // }
            
      if(system_mode==0x44)tt_data|=0x80;     
      tt_data|=(basedata[tt_nuber][27]<<3)&0x60;     
      tt_data|=0x01;

      
      return tt_data;
}
/********************************************************************************
********************************************************************************/
unsigned char tx_data_17(void)
{
  unsigned char tt_data=0;

       
   
   //18字节
  //对应室内机有导风板(1G 38字节与2G 29字节 bit3为1)时，才能设定 
   tt_data=0;
   if(basedata[tt_nuber][7]&0x08)
   {      
      if(wind_board_set==0)//导风板分别设定时，下面的设定才有效 统一设定时，下面的设定为0
      {
        switch(winddir_enable)
        {
        case 0:
            break;
        case 1:tt_data|=0x10;
            break;
        case 2:tt_data|=0xf0;
            break;
        case 3:tt_data|=0x30;
            break;
        case 4:tt_data|=0x70;
            break;  
        case 5:tt_data|=0x30;
            break;    
        }
        
        //if(wind_board_set)tx_data[17]|=0x08;
        if((wind_board_set_3s)&&(wind_board_set==0))tt_data|=0x08;
      }
    }

      
      return tt_data;
}
/********************************************************************************
********************************************************************************/
unsigned char tx_data_18(void)
{
  unsigned char tt_data=0,i=0,j=0,i1=0,j1=0;

  
       tt_data=0;
       //19字节
         if(wind_mode==1)
          {
            i=1;j=1;
          }
         else if(wind_board_set)//统一设定时，都与第1个导风板一致
         {
           i=wind_board_status;j=wind_board_status;
           i1=wind_board_angle;j1=wind_board_angle;
         }
          else
          {
            i=wind_board_status;j=wind_horizontal_status;
            if(a3d_wind_use)
            {
               i1=wind_board_angle;j1=wind_horizontal_angle;
            }
            else 
            {
              i1=wind_board_angle;j1=wind_horizontal_angle;
            }               
          }
          

      if(i)tt_data|=0x01;
      else  tt_data|=(i1-1)<<1;
      
      if(j)tt_data|=0x10;
      else  tt_data|=(j1-1)<<5;
      

      return tt_data;
}
/********************************************************************************
********************************************************************************/
unsigned char tx_line_18(void)
{
  unsigned char tt_data=0,i=0,j=0,i1=0,j1=0;

  
       tt_data=0;
       //19字节
          if(wind_board_set)//统一设定时，都与第1个导风板一致
         {
           i=wind_board_status;j=wind_board_status;
           i1=wind_board_angle;j1=wind_board_angle;
         }
          else
          {
            i=wind_board_status;j=wind_horizontal_status;
            if(a3d_wind_use)
            {
               i1=wind_board_angle;j1=wind_horizontal_angle;
            }
            else 
            {
              i1=wind_board_angle;j1=wind_horizontal_angle;
            }               
          }
          

      if(i)tt_data|=0x01;
      else  tt_data|=(i1-1)<<1;
      
      if(j)tt_data|=0x10;
      else  tt_data|=(j1-1)<<5;
      

      return tt_data;
}
/********************************************************************************
********************************************************************************/
unsigned char tx_data_19(void)
{
  unsigned char tt_data=0,i=0,j=0,i1=0,j1=0;

       tt_data=0;
       
         //20字节
      if(wind_mode==1)
      {
            i=1;j=1;
      }
       else if(wind_board_set)//统一设定时，都与第1个导风板一致
       {
           i=wind_board_status;j=wind_board_status;
           i1=wind_board_angle;j1=wind_board_angle;
       }
      else
      {
        i=wind_board2_status;j=wind_horizontal2_status;
        i1=wind_board2_angle;j1=wind_horizontal2_angle;
      }

      if(i)tt_data|=0x01;
      else tt_data|=(i1-1)<<1;

      if(j)tt_data|=0x10;
      else  tt_data|=(j1-1)<<5;  
      
      return tt_data;
}
/********************************************************************************
********************************************************************************/
unsigned char tx_line_19(void)
{
  unsigned char tt_data=0,i=0,j=0,i1=0,j1=0;

       tt_data=0;
       
         //20字节
       if(wind_board_set)//统一设定时，都与第1个导风板一致
       {
           i=wind_board_status;j=wind_board_status;
           i1=wind_board_angle;j1=wind_board_angle;
       }
      else
      {
        i=wind_board2_status;j=wind_horizontal2_status;
        i1=wind_board2_angle;j1=wind_horizontal2_angle;
      }

      if(i)tt_data|=0x01;
      else tt_data|=(i1-1)<<1;

      if(j)tt_data|=0x10;
      else  tt_data|=(j1-1)<<5;  
      
      return tt_data;
}
/********************************************************************************
********************************************************************************/
unsigned char tx_data_21(void)
{
  unsigned char tt_data=0;

       tt_data=0;
       
      tt_data=human_sensor;
      if(human_sensor_used==0)tt_data&=0xfe;
       
       return tt_data;
}
/********************************************************************************
********************************************************************************/
void line_control_tx_deal2(void)
{
         if((initialize_flag)&&(line_protocol&0x02))
          line_control_tx_new1_deal();
         else if((initialize_flag)&&(line_protocol&0x01))
          line_control_tx_new_deal();
         else
          line_control_tx_deal();
}
/********************************************************************************
********************************************************************************/
void timing_tx_deal2(void)
{
    unsigned char nuber=0;

    nuber=get_nuber();
  
        if((inroom_3g_protocol&(0x0001<<nuber))&&(initialize_flag))base_data_tx_new1_deal();
        else if((inroom_2g_protocol&(0x0001<<nuber))&&(initialize_flag))base_data_tx_new_deal();
        else  base_data_tx_deal();
}
/********************************************************************************
********************************************************************************/
void baseextend_tx_deal2(void)
{
    unsigned char nuber=0;

    nuber=get_nuber();
  
        if(inroom_3g_protocol&(0x0001<<nuber))extendbase_data_tx_new1_deal();
        else  extendbase_data_tx_new_deal();
}
/********************************************************************************
********************************************************************************/
void baseextend_line_tx_deal2(void)
{
         if(line_protocol&0x02)
           extendbase_line_control_tx_new1_deal();
         else 
           extendbase_line_control_tx_new_deal();
}
/********************************************************************************
********************************************************************************/
void base_data_tx_deal(void)
{
   unsigned char i=0;

      tx_data[0]=0x21;
      tx_data[1]=0x00;
      tx_data[2]=0x1c;
      tx_data[3]=0x02;
      tx_data[4]=line_control_dress;
      tx_data[5]=tx_system_nuber1;
      tx_data[6]=tx_dress_nuber1;
      tx_data[7]=tx_system_nuber2;
      tx_data[8]=tx_dress_nuber2;
      tx_data[9]=0x01;


     
      tt_nuber=tt_nuber_dress();
      tx_data[10]=tx_data_10();
      tx_data[11]=tx_data_11();
      tx_data[12]=tx_data_12();
      tx_data[13]=tx_data_13();
      tx_data[14]=tx_data_14();



            for(i=0;i<10;i++)
             {
               if(initialize_flag)
               tx_data[15+i]=option_data[tt_nuber][i];
               else if((tx_data[5]==0xff)||(tx_data[6]==0xff))
                 tx_data[15+i]=0;
               else
               {
                 tx_data[15+i]=option_backup[tt_nuber][i];
                  
               }
             }



      tx_total=tx_data[2];
      xor_data=0;
      for(i=1;i<tx_total-1;i++)
       {
         xor_data^=tx_data[i];
       }
      tx_data[tx_total-1]=xor_data;

}
/********************************************************************************
********************************************************************************/
void base_data_tx_new_deal(void)
{
     unsigned char i=0;

      tx_data[0]=0x21;
      tx_data[1]=0x00;
      tx_data[2]=0x12;
      tx_data[3]=0x02;
      tx_data[4]=line_control_dress;
      tx_data[5]=tx_system_nuber1;
      tx_data[6]=tx_dress_nuber1;
      tx_data[7]=tx_system_nuber2;
      tx_data[8]=tx_dress_nuber2;
      tx_data[9]=0xb1;
    
    
      tt_nuber=tt_nuber_dress();
      tx_data[10]=tx_data_10();
      tx_data[11]=tx_data_11();
      tx_data[12]=tx_data_12();
      tx_data[13]=tx_data_13();
      tx_data[14]=tx_data_14();
      tx_data[15]=tx_data_15();
      tx_data[16]=tx_data_16();
 
    

      
      

      tx_total=tx_data[2];
      xor_data=0;
      for(i=1;i<tx_total-1;i++)
       {
         xor_data^=tx_data[i];
       }
      tx_data[tx_total-1]=xor_data;
}
/********************************************************************************
********************************************************************************/
void base_data_tx_new1_deal(void)
{
  unsigned char i=0;

      tx_data[0]=0x21;
      tx_data[1]=0x00;
      tx_data[2]=0x1a;
      tx_data[3]=0x02;
      tx_data[4]=line_control_dress;
      tx_data[5]=tx_system_nuber1;
      tx_data[6]=tx_dress_nuber1;
      tx_data[7]=tx_system_nuber2;
      tx_data[8]=tx_dress_nuber2;
      tx_data[9]=0xa1;
      
      tt_nuber=tt_nuber_dress();
      tx_data[10]=tx_data_10();
      tx_data[11]=tx_data_11();
      tx_data[12]=tx_data_12();
      tx_data[13]=tx_data_13();
      tx_data[14]=tx_data_14();
      tx_data[15]=tx_data_15();
      tx_data[16]=tx_data_16();
      
      //tx_data[17]=tx_data_17();
      //tx_data[18]=tx_data_18();
      //tx_data[19]=tx_data_19();    
      tx_data[17]=0;
      tx_data[18]=0xff;
      tx_data[19]=0xff;
      
      tx_data[20]=0x00;
      if(half_set)tx_data[20]|=0x10;
      tx_data[21]=tx_data_21();

      tx_data[22]=0x00;
      tx_data[23]=0x00;
      tx_data[24]=0x00;
      
      
      

      tx_total=tx_data[2];
      xor_data=0;
      for(i=1;i<tx_total-1;i++)
       {
         xor_data^=tx_data[i];
       }
      tx_data[tx_total-1]=xor_data;
}
/********************************************************************************
********************************************************************************/
void extendbase_data_tx_new_deal(void)
{
  unsigned char i=0;

  tx_data[0]=0x21;
      tx_data[1]=0x00;
      tx_data[2]=0x20;
      tx_data[3]=0x02;
      tx_data[4]=line_control_dress;
      tx_data[5]=tx_system_nuber1;
      tx_data[6]=tx_dress_nuber1;
      tx_data[7]=tx_system_nuber2;
      tx_data[8]=tx_dress_nuber2;
      tx_data[9]=0xb2;


        tt_nuber=tt_nuber_dress();

         
          if(initialize_flag)
          {
           for(i=0;i<5;i++)
             {
               tx_data[10+i]=option_data[tt_nuber][i];
             }
           for(i=0;i<5;i++)
             {
               tx_data[25+i]=option_data[tt_nuber][5+i];
             }

           for(i=0;i<9;i++)
             {
               tx_data[15+i]=option_data_new[tt_nuber][i];
             }
          }
          else
          {
            for(i=0;i<5;i++)
             {
               tx_data[10+i]=option_backup[tt_nuber][i];
             }
           for(i=0;i<5;i++)
             {
               tx_data[25+i]=option_backup[tt_nuber][5+i];
             }

           for(i=0;i<9;i++)
             {
               tx_data[15+i]=option_new_backup[tt_nuber][i];
               
             }
          }

           
      ///////////////////////////////////////
      
           if(initialize_flag==0)
           {
             tx_data[13]&=0x7f;
             tx_data[18]&=0x7f;//option清除
           }
          

           tx_data[30]=0;


           tx_total=tx_data[2];
      xor_data=0;
      for(i=1;i<tx_total-1;i++)
       {
         xor_data^=tx_data[i];
       }
      tx_data[tx_total-1]=xor_data;

}
/********************************************************************************
********************************************************************************/
void extendbase_data_tx_new1_deal(void)
{
  unsigned char i=0;

      tx_data[0]=0x21;
      tx_data[1]=0x00;
      tx_data[2]=0x2c;
      tx_data[3]=0x02;
      tx_data[4]=line_control_dress;
      tx_data[5]=tx_system_nuber1;
      tx_data[6]=tx_dress_nuber1;
      tx_data[7]=tx_system_nuber2;
      tx_data[8]=tx_dress_nuber2;
      tx_data[9]=0xa2;


       tt_nuber=tt_nuber_dress();
       
          if(initialize_flag)
          {
           for(i=0;i<5;i++)
             {
               tx_data[10+i]=option_data[tt_nuber][i];
             }
           for(i=0;i<5;i++)
             {
               tx_data[25+i]=option_data[tt_nuber][5+i];
             }

           for(i=0;i<9;i++)
             {
               tx_data[15+i]=option_data_new[tt_nuber][i];
             }
          }
          else
          {
             for(i=0;i<5;i++)
             {
               tx_data[10+i]=option_backup[tt_nuber][i];
             }
           for(i=0;i<5;i++)
             {
               tx_data[25+i]=option_backup[tt_nuber][5+i];
             }

           for(i=0;i<9;i++)
             {
               tx_data[15+i]=option_new_backup[tt_nuber][i];
             }
          }
          

          
      ///////////////////////////////////////
      //设定选项有变化时
             
            
            //////////////////////////////////
           if(initialize_flag==0)
           {
             tx_data[13]&=0x7f;
             tx_data[18]&=0x7f;//option清除
           }


           tx_data[30]=0;
           
           
           //31-37字节
           if(initialize_flag)
           {
            for(i=0;i<7;i++)
             {
               tx_data[30+i]=option_data_new1[tt_nuber][i];
             }
           }
           else
           {
             for(i=0;i<7;i++)
             {
               tx_data[30+i]=option_new1_backup[tt_nuber][i];
             }
           }
           //37-43字节备用
          

           tx_total=tx_data[2];
      xor_data=0;
      for(i=1;i<tx_total-1;i++)
       {
         xor_data^=tx_data[i];
       }
      tx_data[tx_total-1]=xor_data;
}
/********************************************************************************
********************************************************************************/
void g25_basedata_tx_deal(void)
{
   unsigned char i=0;

      tx_data[0]=0x21;
      tx_data[1]=0x00;
      tx_data[2]=0x30;
      tx_data[3]=0x02;
      tx_data[4]=line_control_dress;
      tx_data[5]=tx_system_nuber1;
      tx_data[6]=tx_dress_nuber1;
      tx_data[7]=tx_system_nuber2;
      tx_data[8]=tx_dress_nuber2;
      tx_data[9]=0xbd;
      
      tt_nuber=tt_nuber_dress();

       tx_data[10]=((save_flag<<2)&0x0c)|(sleep_flag&0x03);    
       if(mute_flag)tx_data[10]|=0x10;//静音
       //0-无效、1-风吹人、2-风避人、3-无人、4-风吹人和房间无人运行模式同时、5-风避人和房间无人运行模式同时       
       i=0;
       if(human_used)
       {
        switch(human_flag)
        { 
        case 0:i=0;break;
        case 1:i=1;break;
        case 2:i=2;break;  
        case 3:i=4;break;  
        case 4:i=5;break;  
        case 5:i=6;break;  
        default:i=0;break;
        } 
       }        
       else 
       {
        i&=0xf8;
       }
       tx_data[11]=i;
       
       tx_data[12]=tx_data_18();
       tx_data[13]=tx_data_19();

       
      //15字节
      //wind_mode 0--普通 1--3D 2--超远
       tx_data[14]=0;
      if(wind_mode==1)tx_data[14]|=0x01;
      else if(wind_mode==2)tx_data[14]|=0x02;
      
      if(mute_flag)tx_data[14]|=0x04;//静音
      if(heat_flag)tx_data[14]|=0x08;//辅热
      if(health_flag)tx_data[14]|=0x20;//健康
      
       //16字节
      tx_data[15]=0;
      
       //17字节
      tx_data[16]=0;
      
      //18字节
      tx_data[17]=0;
      if(forest_wind_flag) tx_data[17]|=0x02;
      if(self_clean_flag_b&(0x0001<<tt_nuber)) tx_data[17]|=0x04;
      
            //19字节
      tx_data[18]=0; 
      if((high_temp_disinfect_flag_b&(0x0001<<tt_nuber)))tx_data[18]|=0x08;
      if((power_wind_check_flag_b&(0x0001<<tt_nuber)))tx_data[18]|=0x40;
     

       for(i=19;i<47;i++)
       {
         tx_data[i]=0;
       }


      tx_total=tx_data[2];
      xor_data=0;
      for(i=1;i<tx_total-1;i++)
       {
         xor_data^=tx_data[i];
       }
      tx_data[tx_total-1]=xor_data;
}
/********************************************************************************
********************************************************************************/
void g25_extenddata_tx_deal(void)
{
   unsigned char i=0;

      tx_data[0]=0x21;
      tx_data[1]=0x00;
      tx_data[2]=0x30;
      tx_data[3]=0x02;
      tx_data[4]=line_control_dress;
      tx_data[5]=tx_system_nuber1;
      tx_data[6]=tx_dress_nuber1;
      tx_data[7]=tx_system_nuber2;
      tx_data[8]=tx_dress_nuber2;
      tx_data[9]=0xbe;
      
      tt_nuber=tt_nuber_dress();

      tx_data[10]=option_data_new[tt_nuber][10];
      
      tx_data[11]=option_data_new[tt_nuber][11];;
      if(option_data[tt_nuber][3]&0x80)tx_data[11]|=0x80;      
      
      tx_data[12]=0;
       if(option_hh)tx_data[12]|=0x01;

      for(i=13;i<47;i++)
       {
         tx_data[i]=0;
       }

     tx_total=tx_data[2];

      xor_data=0;
      for(i=1;i<tx_total-1;i++)
       {
         xor_data^=tx_data[i];
       }
      tx_data[tx_total-1]=xor_data;
}
/********************************************************************************
********************************************************************************/
void dresschange_request_tx_deal(void)
{
  unsigned char i=0;

      tx_data[0]=0x21;
      tx_data[1]=0xf1;
      tx_data[2]=0x0c;
      tx_data[3]=0x02;
      tx_data[4]=line_control_dress;
      tx_data[5]=tx_system_nuber1;
      tx_data[6]=tx_dress_nuber1;
      tx_data[7]=tx_system_nuber2;
      tx_data[8]=tx_dress_nuber2;

      //变更成为这个地址
      tx_data[9]=temporary_system;
      tx_data[10]=temporary_dress;
      /////////////////
      tx_total=tx_data[2];

      xor_data=0;
      for(i=1;i<tx_total-1;i++)
       {
         xor_data^=tx_data[i];
       }
      tx_data[tx_total-1]=xor_data;
}
/********************************************************************************
********************************************************************************/
void dresschange_confirm_tx_deal(void)
{
  unsigned char i=0;

     tx_data[0]=0x21;
      tx_data[1]=0xf2;
      tx_data[2]=0x0a;
      tx_data[3]=0x02;
      tx_data[4]=line_control_dress;
      tx_data[5]=tx_system_nuber1;
      tx_data[6]=tx_dress_nuber1;
      tx_data[7]=tx_system_nuber2;
      tx_data[8]=tx_dress_nuber2;

      tx_total=tx_data[2];

      xor_data=0;
      for(i=1;i<tx_total-1;i++)
       {
         xor_data^=tx_data[i];
       }
      tx_data[tx_total-1]=xor_data;
}
/********************************************************************************
********************************************************************************/
void point_check_tx1_deal(void)
{
      unsigned char i=0;

      tx_data[0]=0x21;
      tx_data[1]=0x00;
      tx_data[2]=0x0b;
      tx_data[3]=0x02;
      tx_data[4]=line_control_dress;
      tx_data[5]=basedata[option_dress][1];
      tx_data[6]=basedata[option_dress][2];
      tx_data[7]=basedata[option_dress][1];
      tx_data[8]=basedata[option_dress][2];

      if(inroom_3g_protocol&(0x0001<<option_dress))tx_data[9]=0xa5;
      else if(inroom_2g_protocol&(0x0001<<option_dress))tx_data[9]=0xb5;
        else
      tx_data[9]=0x05;

      tx_total=tx_data[2];
      xor_data=0;
      for(i=1;i<tx_total-1;i++)
       {
         xor_data^=tx_data[i];
       }
      tx_data[tx_total-1]=xor_data;
}
/********************************************************************************
********************************************************************************/
void point_check_tx2_deal(void)
{
   unsigned char i=0;

      tx_data[0]=0x21;
      tx_data[1]=0x00;
      tx_data[2]=0x0b;
      tx_data[3]=0x02;
      tx_data[4]=line_control_dress;
      tx_data[5]=basedata[option_dress][1];
      tx_data[6]=basedata[option_dress][2];
      tx_data[7]=basedata[option_dress][1];
      tx_data[8]=basedata[option_dress][2];

      if(inroom_3g_protocol&(0x0001<<option_dress))tx_data[9]=0xa7;
      else if(inroom_2g_protocol&(0x0001<<option_dress))tx_data[9]=0xb7;
        else
      tx_data[9]=0x07;

      tx_total=tx_data[2];
      xor_data=0;
      for(i=1;i<tx_total-1;i++)
       {
         xor_data^=tx_data[i];
       }
      tx_data[tx_total-1]=xor_data;
}
/********************************************************************************
********************************************************************************/
void connect_information_tx_deal(void)
{
   unsigned char i=0;

      tx_data[0]=0x21;
      tx_data[1]=0x00;
      tx_data[2]=0x0b;
      tx_data[3]=0x02;
      tx_data[4]=line_control_dress;
      tx_data[5]=tx_system_nuber1;
      tx_data[6]=tx_dress_nuber1;
      tx_data[7]=tx_system_nuber2;
      tx_data[8]=tx_dress_nuber2;
      
      if(inroom_3g_protocol&(0x0001<<already_tx_nuber))tx_data[9]=0xa3;
      else if(inroom_2g_protocol&(0x0001<<already_tx_nuber))tx_data[9]=0xb3;
        else
      tx_data[9]=0x03;

      tx_total=tx_data[2];
      xor_data=0;
      for(i=1;i<tx_total-1;i++)
       {
         xor_data^=tx_data[i];
       }
      tx_data[tx_total-1]=xor_data;
}
/********************************************************************************
********************************************************************************/
void group_message_tx_deal(void)
{
   unsigned char i=0,j=0;

      tx_data[0]=0x21;
      tx_data[1]=0x00;
      tx_data[2]=0x2b;
      tx_data[3]=0x02;
      tx_data[4]=line_control_dress;
      tx_data[5]=tx_system_nuber1;
      tx_data[6]=tx_dress_nuber1;
      tx_data[7]=tx_system_nuber2;
      tx_data[8]=tx_dress_nuber2;

      if(inroom_3g_protocol&(0x0001<<already_tx_nuber))tx_data[9]=0xa4;
      else if(inroom_2g_protocol&(0x0001<<already_tx_nuber))tx_data[9]=0xb4;
        else
      tx_data[9]=0x04;

      for(i=10;i<tx_data[2];i++)
      {
        tx_data[i]=0;
      }

      if(inroom_2g_protocol)
      {
        for(j=0;j<total_machine;j++)
        {
          tx_data[10+2*j]=basedata[j][1];
          tx_data[11+2*j]=basedata[j][2];
        }
      }
      else
      {
        for(j=0;j<total_machine;j++)
        {

        if((basedata[j][2]>8)&&(basedata[j][2]<=16))
          tx_data[basedata[j][1]*2+9]|=(0x01<<(basedata[j][2]-9));
        else if(basedata[j][2]<=8)
         tx_data[basedata[j][1]*2+8]|=(0x01<<(basedata[j][2]-1));
        }
      }


      tx_total=tx_data[2];
      xor_data=0;
      for(i=1;i<tx_total-1;i++)
       {
         xor_data^=tx_data[i];
       }
      tx_data[tx_total-1]=xor_data;
}
/********************************************************************************
********************************************************************************/
void group_message_extend_tx_deal(void)
{
    unsigned char i=0,j=0;

      tx_data[0]=0x21;
      tx_data[1]=0x00;
      tx_data[2]=0x2b;
      tx_data[3]=0x02;
      tx_data[4]=line_control_dress;
      tx_data[5]=tx_system_nuber1;
      tx_data[6]=tx_dress_nuber1;
      tx_data[7]=tx_system_nuber2;
      tx_data[8]=tx_dress_nuber2;
      tx_data[9]=0x0b;

      for(i=10;i<tx_data[2];i++)
      {
        tx_data[i]=0;
      }

      for(j=0;j<total_machine;j++)
      {
        if((basedata[j][2]>24)&&(basedata[j][2]<=32))
        tx_data[basedata[j][1]*2+9]|=0x01<<(basedata[j][2]-25);
        else if((basedata[j][2]>16)&&(basedata[j][2]<=24))
         tx_data[basedata[j][1]*2+8]|=0x01<<(basedata[j][2]-17);
      }




      tx_total=tx_data[2];
      xor_data=0;
      for(i=1;i<tx_total-1;i++)
       {
         xor_data^=tx_data[i];
       }
      tx_data[tx_total-1]=xor_data;
}
/********************************************************************************
********************************************************************************/
void machine_type_tx_deal(void)
{
    unsigned char i=0;

      tx_data[0]=0x21;
      tx_data[1]=0x00;
      tx_data[2]=0x0b;
      tx_data[3]=0x02;
      tx_data[4]=line_control_dress;
      tx_data[5]=tx_system_nuber1;
      tx_data[6]=tx_dress_nuber1;
      tx_data[7]=tx_system_nuber2;
      tx_data[8]=tx_dress_nuber2;

     
      tx_data[9]=0xBA;


      tx_total=tx_data[2];
      xor_data=0;
      for(i=1;i<tx_total-1;i++)
       {
         xor_data^=tx_data[i];
       }
      tx_data[tx_total-1]=xor_data;
}
/********************************************************************************
********************************************************************************/
void auto_clear_address(void)
{
  unsigned char i=0;

      tx_data[0]=0x21;
      tx_data[1]=0xf3;
      tx_data[2]=0x0c;
      tx_data[3]=0x02;
      tx_data[4]=line_control_dress;
      tx_data[5]=tx_system_nuber1;
      tx_data[6]=tx_dress_nuber1;
      tx_data[7]=tx_system_nuber2;
      tx_data[8]=tx_dress_nuber2;
      
      tx_data[9]=0;
      tx_data[10]=0;

      tx_total=tx_data[2];
      xor_data=0;
      for(i=1;i<tx_total-1;i++)
       {
         xor_data^=tx_data[i];
       }
      tx_data[tx_total-1]=xor_data;
}
/********************************************************************************
********************************************************************************/
void boardcheck_start_tx_deal(void)
{
    unsigned char i=0;

      tx_data[0]=0x21;
      tx_data[1]=0x10;
      tx_data[2]=0x0a;
      tx_data[3]=0x02;
      tx_data[4]=line_control_dress;
      tx_data[5]=basedata[option_dress][1];
      tx_data[6]=basedata[option_dress][2];
      tx_data[7]=basedata[option_dress][1];
      tx_data[8]=basedata[option_dress][2];


      tx_total=tx_data[2];
      xor_data=0;
      for(i=1;i<tx_total-1;i++)
       {
         xor_data^=tx_data[i];
       }
      tx_data[tx_total-1]=xor_data;
}
/********************************************************************************
********************************************************************************/
void boardcheck_end_tx_deal(void)
{
    unsigned char i=0;

  tx_data[0]=0x21;
      tx_data[1]=0x11;
      tx_data[2]=0x0a;
      tx_data[3]=0x02;
      tx_data[4]=line_control_dress;
     tx_data[5]=basedata[option_dress][1];
      tx_data[6]=basedata[option_dress][2];
      tx_data[7]=basedata[option_dress][1];
      tx_data[8]=basedata[option_dress][2];


      tx_total=tx_data[2];
      xor_data=0;
      for(i=1;i<tx_total-1;i++)
       {
         xor_data^=tx_data[i];
       }
      tx_data[tx_total-1]=xor_data;
}
/********************************************************************************
********************************************************************************/
unsigned char tx_line_10(void)
{
  unsigned char tt_data=0;
  
      tt_data=0;
      tt_data=(system_mode&0x1f)<<2;
      tt_data|=0x80;//新遥控器
      if(system_status)tt_data|=0x01;
      
      return tt_data;
}
/********************************************************************************
********************************************************************************/
unsigned char tx_line_11(void)
{
   unsigned char tt_data=0;
  
      tt_data=0;
      
      
   if(wind_enable>1)
  {
     switch(system_wind&0x1f)
     {
     case 0x00:tt_data=0x00;break;
     case 0x01:tt_data=0x01;break;   
     case 0x18:tt_data=0x08;break;
     case 0x08:tt_data=0x08;break;
     case 0x14:tt_data=0x04;break;
     case 0x04:tt_data=0x04;break;
     case 0x02:tt_data=0x02;break;
     case 0x12:tt_data=0x02;break;
     default:tt_data=0x04;break;
     }
     
  }
  else
  {
      switch(system_wind&0x0f)
     {
     case 0x00:tt_data=0x00;break;
     case 0x01:tt_data=0x01;break;
     case 0x02:tt_data=0x02;break;     
     case 0x04:tt_data=0x04;break;    
     case 0x08:tt_data=0x08;break;
     default:tt_data=0x04;break;
     }
  }

     
      /*
      if(wind_mode==1)
      {
        tt_data|=0x10;
        tt_data&=0x1f;
      }
      else if(wind_board_enable)
        */
      if(wind_board_enable)
      {
        if(wind_board_status)
        {
          tt_data|=0x10;
          tt_data&=0x1f;
        }
        else
        {
          tt_data&=0xef;
                  
          switch(wind_board_angle)
          {
          case 1:tt_data&=0x1f;break;
          case 2:tt_data&=0x3f;tt_data|=0x20;break;
          case 3:tt_data&=0x5f;tt_data|=0x40;break;
          case 4:tt_data&=0x7f;tt_data|=0x60;break;
          case 5:tt_data&=0x9f;tt_data|=0x80;break;
          case 6:tt_data&=0xbf;tt_data|=0xa0;break;
          case 7:tt_data&=0xdf;tt_data|=0xc0;break;
          default:tt_data&=0x1f;break;
          }
        }

      }
      else
      {
        tt_data&=0x0f;
      }

       if(hh_flag)tt_data=0;
      
      
      return tt_data;
}
/********************************************************************************
********************************************************************************/
unsigned char tx_line_12(void)
{
  unsigned char tt_data=0;
  
      tt_data=0;
      
      if((try_run_flag)||(try_run_set))
      {
        if(try_run_status==0)tt_data=0x00;
        else if(system_mode==0x08)tt_data=0x1e;
        else
          tt_data=try_run_fre;
      }
      else
      {
        if(system_mode==0)tt_data=0xff;
        else if(system_mode==0x44)tt_data=dry_set;
        else
        tt_data=tempset;
      }
      
      return tt_data;
}
/********************************************************************************
********************************************************************************/
unsigned char tx_line_13(void)
{
  unsigned char tt_data=0;
  
      tt_data=0;
        
     switch(machine_type)
      {
      case 1:tt_data|=0x10;break;
      case 2:tt_data|=0x20;break;
      case 3:tt_data|=0x30;break;
      default:break;
      }

      //if(system_mode==0x28)tt_data|=0x40;
      if((try_run_flag)||(try_run_set))tt_data|=0x80;
      if(err_reset_flag)tt_data|=0x01;
      if(filter_reset)tt_data|=0x02;
      
      return tt_data;
}
/********************************************************************************
********************************************************************************/
unsigned char tx_line_14(void)
{
  unsigned char tt_data=0;

     tt_data=temp_ad_value;//temp_value;
      
      return tt_data;
}
/********************************************************************************
********************************************************************************/
unsigned char tx_line_15(void)
{
  unsigned char tt_data=0;

       tt_data=0;
   
    
     if(machine_type1==4)
      {
        switch(system_mode)
        {
        case 0x84:tt_data=0x00;break;
        case 0xa4:tt_data=0x01;break;
        case 0xc4:tt_data=0x02;break;
        default:tt_data=0x00;break;
        }
      }
      if(compress_preheat_reset) tt_data|=0x10;

      
      return tt_data;
}
/********************************************************************************
********************************************************************************/
unsigned char tx_line_16(void)
{
  unsigned char tt_data=0;

       tt_data=0;
   
    
   //if(wind_enable>1)
   //{
     switch(system_wind&0x1f)
     { 
     case 0x18:tt_data|=0x40;break;
     case 0x14:tt_data|=0x40;break;
     case 0x12:tt_data|=0x40;break;
     default:break;
     }
   //}
                
      tt_data|=0x80;//默认有0.5代  line_g25_flag
      tt_data|=0x01;

      
      return tt_data;
}
/********************************************************************************
********************************************************************************/
unsigned char tx_line_17(void)
{
  unsigned char tt_data=0;

       tt_data=0;
   
     switch(wind_enable)
      {
       case 0:
       case 1:tx_data[17]=0;break;
       case 2:
       case 3:tt_data|=0xc0;break;
       case 4:
       case 5:tt_data|=0x40;break;
      }
      
      return tt_data;
}
/********************************************************************************
********************************************************************************/
unsigned char tx_line_23(void)
{
 unsigned char tt_data=0;

   tt_data=0;
    
      if(wind_board_set==0)//导风板分别设定时，下面的设定才有效 统一设定时，下面的设定为0
      {
        switch(winddir_enable)
        {
        case 0:
            break;
        case 1:tt_data|=0x10;
            break;
        case 2:tt_data|=0xf0;
            break;
        case 3:tt_data|=0x30;
            break;
        case 4:tt_data|=0x70;
            break;  
        case 5:tt_data|=0x30;
            break;    
        }
        
        //if(wind_board_set)tx_data[17]|=0x08;
        if((wind_board_set_3s)&&(wind_board_set==0))tt_data|=0x08;
      }
    

      
      return tt_data;
}
/********************************************************************************
********************************************************************************/
unsigned char tx_line_24(void)
{
 unsigned char tt_data=0,i=0,j=0,i1=0,j1=0;

  
       tt_data=0;
       //19字节
       /*
         if(wind_mode==1)
          {
            i=1;j=1;
          }
         else if(wind_board_set)//统一设定时，都与第1个导风板一致
           */
          if(wind_board_set)//统一设定时，都与第1个导风板一致  
         {
           i=wind_board_status;j=wind_board_status;
           i1=wind_board_angle;j1=wind_board_angle;
         }
          else
          {
            i=wind_board_status;j=wind_horizontal_status;
            if(a3d_wind_use)
            {
               i1=wind_board_angle;j1=wind_horizontal_angle;
            }
            else 
            {
              i1=wind_board_angle;j1=wind_horizontal_angle;
            }               
          }
          
          

      if(i)
      {
           tt_data|=0x01;
      }
      else
      {
           tt_data|=(i1-1)<<1;
      }

      if(j)
      {
           tt_data|=0x10;
      }
      else
      {
           tt_data|=(j1-1)<<5;
      }
      
   
      
      return tt_data;
}
/********************************************************************************
********************************************************************************/
unsigned char tx_line_25(void)
{
 unsigned char tt_data=0,i=0,j=0,i1=0,j1=0;

       tt_data=0;
       
         //20字节
       /*
      if(wind_mode==1)
      {
            i=1;j=1;
      }
       else if(wind_board_set)//统一设定时，都与第1个导风板一致
         */
       if(wind_board_set)//统一设定时，都与第1个导风板一致
       {
           i=wind_board_status;j=wind_board_status;
           i1=wind_board_angle;j1=wind_board_angle;
       }
      else
      {
        i=wind_board2_status;j=wind_horizontal2_status;
        i1=wind_board2_angle;j1=wind_horizontal2_angle;
      }


      if(i)
      {
          tt_data|=0x01;
      }
      else
      {
           tt_data|=(i1-1)<<1;
      }

      if(j)
      {
           tt_data|=0x10;
      }
      else
      {
           tt_data|=(j1-1)<<5;
      }
   
      
      return tt_data;
}
/********************************************************************************
********************************************************************************/
void line_control_tx_deal(void)
{
  unsigned char i=0;
  
               
      tx_data[0]=0x41;
      tx_data[1]=0x00;
      tx_data[2]=0x1e;
      tx_data[3]=0x02;

      if(line_init_flag==0)
      {
        tx_data[4]=line_control_dress;
        tx_data[5]=0x01;
        if(line_control_dress==0x01)
        {
          tx_data[6]=0x02;tx_data[8]=0x02;
        }
        else
        {
          tx_data[6]=0x01;tx_data[8]=0x01;
        }
        tx_data[7]=0x01;
      }
      else
      {
        tx_data[4]=0x01;
        tx_data[5]=0xff;
        tx_data[6]=0xff;
        tx_data[7]=0xff;
        tx_data[8]=0xff;
        
      }
      tx_data[9]=0x01;   
      
      
      tx_data[10]=tx_line_10();
      tx_data[11]=tx_line_11();
      tx_data[12]=tx_line_12();
      tx_data[13]=tx_line_13();
      tx_data[14]=tx_line_14();
      tx_data[15]=0x00;
      tx_data[16]=tx_line_16();
      tx_data[17]=tx_line_17();
      
      
      tx_data[18]=0;
      tx_data[19]=0;
      tx_data[20]=0;
      tx_data[21]=0;
      tx_data[22]=0;
      tx_data[23]=0;
      tx_data[24]=0;
      
   
      
      
      

      if(option_set_flag&0x01)
      {
        tx_data[25]|=0x01;
        
      }
      else if(line_06_sent)tx_data[25]|=0xde;

      if(option_set_flag==0x02)
      {
        if(option_all_set[2]&0x20)//温度固定b6为1时
        {
          tx_data[26]=tempset+0x80;
        }
        else tx_data[26]=0x80;
      }
      else  tx_data[26]=0;


      tx_data[27]=0;
      if(center_onoff_flag&0x01)tx_data[27]|=0x10;
      else if(center_onoff_flag&0x02)tx_data[27]|=0x80;

      if(por_06_flag)tx_data[28]=0;
      else
      tx_data[28]=0xff;
      
      
      

      tx_total=tx_data[2];
      xor_data=0;
      for(i=1;i<tx_total-1;i++)
       {
         xor_data^=tx_data[i];
       }
      tx_data[tx_total-1]=xor_data;
}
/********************************************************************************
********************************************************************************/
void line_control_tx_new_deal(void)
{
  unsigned char i=0;

      tx_data[0]=0x41;
      tx_data[1]=0x00;
      tx_data[2]=0x18;
      
      tx_data[3]=0x02;
      if(initialize_flag)
      tx_data[4]=line_control_dress;
      else
       tx_data[4]=0x01;


      tx_data[5]=0x01;
      if(line_control_dress==0x01)
      {
        tx_data[6]=0x02;tx_data[8]=0x02;
      }
      else
      {
        tx_data[6]=0x01;tx_data[8]=0x01;
      }
      tx_data[7]=0x01;
      tx_data[9]=0xb1;
      
      
      tx_data[10]=tx_line_10();
      tx_data[11]=tx_line_11();
      tx_data[12]=tx_line_12();
      tx_data[13]=tx_line_13();
      tx_data[14]=tx_line_14();
      tx_data[15]=tx_line_15();
      tx_data[16]=tx_line_16();
      tx_data[17]=tx_line_17();
      
      
      
      
      
      
      if(option_set_flag&0x01)
      {
        tx_data[18]|=0x01;
        
      }
      else if(line_06_sent)tx_data[18]|=0xde;

      if(option_set_flag==0x02)
      {
        if(option_all_set[2]&0x20)//温度固定b6为1时
        {
          tx_data[19]=tempset+0x80;
        }
        else tx_data[19]=0x80;
      }
      else  tx_data[19]=0;


      tx_data[20]=0;
      if(center_onoff_flag&0x01)tx_data[20]|=0x10;
      else if(center_onoff_flag&0x02)tx_data[20]|=0x80;

      if(por_06_flag)tx_data[21]=0;
      else
      tx_data[21]=0xff;
      

      tx_total=tx_data[2];
      xor_data=0;
      for(i=1;i<tx_total-1;i++)
       {
         xor_data^=tx_data[i];
       }
      tx_data[tx_total-1]=xor_data;
}
/********************************************************************************
********************************************************************************/
void line_control_tx_new1_deal(void)
{
   unsigned char i=0;
   
      tx_data[0]=0x41;
      tx_data[1]=0x00;
      tx_data[2]=0x20;
      tx_data[3]=0x02;

      if(initialize_flag)
      tx_data[4]=line_control_dress;
      else
       tx_data[4]=0x01;


      tx_data[5]=0x01;
      if(line_control_dress==0x01)
      {
        tx_data[6]=0x02;tx_data[8]=0x02;
      }
      else
      {
        tx_data[6]=0x01;tx_data[8]=0x01;
      }
      tx_data[7]=0x01;
      tx_data[9]=0xa1;

      tx_data[10]=tx_line_10();
      tx_data[11]=tx_line_11();
      tx_data[12]=tx_line_12();
      tx_data[13]=tx_line_13();
      tx_data[14]=tx_line_14();
      tx_data[15]=tx_line_15();
      tx_data[16]=tx_line_16();
      tx_data[17]=tx_line_17();
      
 
      
      tx_data[18]=0;
      if(option_set_flag&0x01)
      {
        tx_data[18]|=0x01; 
      }
      else if(line_06_sent)tx_data[18]|=0xde;

      if(option_set_flag==0x02)
      {
        if(option_all_set[2]&0x20)//温度固定b6为1时
        {
          tx_data[19]=tempset+0x80;
        }
        else tx_data[19]=0x80;
      }
      else  tx_data[19]=0;


      tx_data[20]=0;
      if(center_onoff_flag&0x01)tx_data[20]|=0x10;
      else if(center_onoff_flag&0x02)tx_data[20]|=0x80;

      if(por_06_flag)tx_data[21]=0;
      else
      tx_data[21]=0xff;
      
      
      
      tx_data[22]=0;
      //tx_data[23]=tx_line_23();
      //tx_data[24]=tx_line_24();
      //tx_data[25]=tx_line_25();
      tx_data[23]=0;
      tx_data[24]=0xff;
      tx_data[25]=0xff;
      
       tx_data[26]=0;
       tx_data[27]=tx_data_21();
      
       tx_data[28]=0;
       tx_data[29]=0;
       tx_data[30]=0;
      
      
      tx_total=tx_data[2];
      xor_data=0;
      for(i=1;i<tx_total-1;i++)
       {
         xor_data^=tx_data[i];
       }
      tx_data[tx_total-1]=xor_data;
  
}
/********************************************************************************
********************************************************************************/
void extendbase_line_control_tx_new_deal(void)
{
   unsigned char i=0;

      tx_data[0]=0x41;
      tx_data[1]=0x00;
      tx_data[2]=0x20;
      tx_data[3]=0x02;
      tx_data[4]=line_control_dress;


      if(master_flag)
      {
      tx_data[5]=0x01;
      tx_data[6]=0x01;
      tx_data[7]=0x01;
      tx_data[8]=0x01;
      }
      else
      {
        tx_data[5]=0x01;
      tx_data[6]=0x02;
      tx_data[7]=0x01;
      tx_data[8]=0x02;
      }

      tx_data[9]=0xb2;


      tx_data[10]=0;
      tx_data[11]=0;
      tx_data[12]=0;
      tx_data[13]=0;
      tx_data[14]=0;
      tx_data[15]=0;
      tx_data[16]=option_f1;

      tx_data[17]=0;
      tx_data[18]=0;
      tx_data[19]=0;
      tx_data[20]=option_all_set_new[5];
      tx_data[21]=option_all_set_new[6];
      tx_data[22]=option_all_set_new[7];
      tx_data[23]=option_all_set_new[8];
      tx_data[24]=0;
      tx_data[25]=0;
      tx_data[26]=0;
      tx_data[27]=0;
      tx_data[28]=0;
      tx_data[29]=0;
      tx_data[30]=0;
      /////////////////
      tx_total=tx_data[2];

      xor_data=0;
      for(i=1;i<tx_total-1;i++)
       {
         xor_data^=tx_data[i];
       }
      tx_data[tx_total-1]=xor_data;
}
/********************************************************************************
********************************************************************************/
void extendbase_line_control_tx_new1_deal(void)
{
  unsigned char i=0;

      tx_data[0]=0x41;
      tx_data[1]=0x00;
      tx_data[2]=0x2c;
      tx_data[3]=0x02;
      tx_data[4]=line_control_dress;


      if(master_flag)
      {
      tx_data[5]=0x01;
      tx_data[6]=0x01;
      tx_data[7]=0x01;
      tx_data[8]=0x01;
      }
      else
      {
      tx_data[5]=0x01;
      tx_data[6]=0x02;
      tx_data[7]=0x01;
      tx_data[8]=0x02;
      }

      tx_data[9]=0xa2;


      tx_data[10]=0;
      tx_data[11]=0;
      tx_data[12]=0;
      tx_data[13]=0;
      tx_data[14]=0;
      tx_data[15]=0;
      tx_data[16]=option_f1;

      tx_data[17]=0;
      tx_data[18]=0;
      tx_data[19]=0;
      tx_data[20]=option_all_set_new[5];
      tx_data[21]=option_all_set_new[6];
      tx_data[22]=option_all_set_new[7];
      tx_data[23]=option_all_set_new[8];
      tx_data[24]=0;
      tx_data[25]=0;
      tx_data[26]=0;
      tx_data[27]=0;
      tx_data[28]=0;
      tx_data[29]=0;
      
      
      tx_data[30]=option_all_set_new1[0];
      tx_data[31]=option_all_set_new1[1];
      tx_data[32]=option_all_set_new1[2];
      tx_data[33]=option_all_set_new1[3];
      tx_data[34]=option_all_set_new1[4];
      tx_data[35]=option_all_set_new1[5];
      
      tx_data[36]=0;
      tx_data[37]=0;
      tx_data[38]=0;
      tx_data[39]=0;
      tx_data[40]=0;
      tx_data[41]=0;
      tx_data[42]=0;
      
      
      
      /////////////////
      tx_total=tx_data[2];

      xor_data=0;
      for(i=1;i<tx_total-1;i++)
       {
         xor_data^=tx_data[i];
       }
      tx_data[tx_total-1]=xor_data;
}
/********************************************************************************
********************************************************************************/
void g25_line_control_tx_deal(void)
{
   unsigned char i=0;


      tx_data[0]=0x41;
      tx_data[1]=0x00;
      tx_data[2]=0x30;
      
      tx_data[3]=0x02;
      
      if(initialize_flag)
      tx_data[4]=line_control_dress;
      else
       tx_data[4]=0x01;


      tx_data[5]=0x01;
      if(line_control_dress==0x01)
      {
        tx_data[6]=0x02;tx_data[8]=0x02;
      }
      else
      {
        tx_data[6]=0x01;tx_data[8]=0x01;
      }
      tx_data[7]=0x01;
      tx_data[9]=0xbd;

       tx_data[10]=((save_flag<<2)&0x0c)|(sleep_flag&0x03); 
       if(mute_flag)tx_data[10]|=0x10;//静音
       //0-无效、1-风吹人、2-风避人、3-无人、4-风吹人和房间无人运行模式同时、5-风避人和房间无人运行模式同时       
       i=0;
       if(human_used)
       {
        switch(human_flag)
        { 
        case 0:i=0;break;
        case 1:i=1;break;
        case 2:i=2;break;  
        case 3:i=4;break;  
        case 4:i=5;break;  
        case 5:i=6;break;  
        default:i=0;break;
        }        
       }        
       else 
       {
        i&=0xf8;
       }
       tx_data[11]=i;
       
       
       tx_data[12]=tx_line_18();
       tx_data[13]=tx_line_19();
      


       //15字节
      //wind_mode 0--普通 1--3D 2--超远
       tx_data[14]=0;
      if(wind_mode==1)tx_data[14]|=0x01;
      else if(wind_mode==2)tx_data[14]|=0x02;
      if(mute_flag)tx_data[14]|=0x04;//静音
      if(heat_flag)tx_data[14]|=0x08;//辅热
      if(health_flag)tx_data[14]|=0x20;//健康
      
      if(half_set)tx_data[14]|=0x40;
      
       //16字节
      tx_data[15]=0x00;
      //17字节
      tx_data[16]=0x00;
      
      //18字节
      tx_data[17]=0;
      if(forest_wind_flag) tx_data[17]|=0x02;
      if(self_clean_flag) tx_data[17]|=0x04;
      
      

       for(i=18;i<47;i++)
       {
         tx_data[i]=0;
       }


      tx_total=tx_data[2];
      xor_data=0;
      for(i=1;i<tx_total-1;i++)
       {
         xor_data^=tx_data[i];
       }
      tx_data[tx_total-1]=xor_data;
}
/********************************************************************************
********************************************************************************/
void g25_extenddata_line_tx_deal(void)
{
   unsigned char i=0;

	tx_data[0]=0x41;
      tx_data[1]=0x00;
      tx_data[2]=0x30;
      
      tx_data[3]=0x02;
     if(initialize_flag)
      tx_data[4]=line_control_dress;
      else
       tx_data[4]=0x01;


      tx_data[5]=0x01;
      if(line_control_dress==0x01)
      {
        tx_data[6]=0x02;tx_data[8]=0x02;
      }
      else
      {
        tx_data[6]=0x01;tx_data[8]=0x01;
      }
      tx_data[7]=0x01;      
      tx_data[9]=0xbe;
      
      
      
       
       tt_nuber=tt_nuber_dress();


      tx_data[10]=option_data_new[tt_nuber][10];
      
      tx_data[11]=option_data_new[tt_nuber][11];;
         if(option_data[tt_nuber][3]&0x80)tx_data[11]|=0x80;


      for(i=12;i<47;i++)
       {
         tx_data[i]=0;
       }

      if(option_hh)tx_data[12]|=0x01;


     tx_total=tx_data[2];

      xor_data=0;
      for(i=1;i<tx_total-1;i++)
       {
         xor_data^=tx_data[i];
       }
      tx_data[tx_total-1]=xor_data;
}
/********************************************************************************
********************************************************************************/
void line_dresschange_request_tx_deal(void)
{
    unsigned char i=0;
 

      tx_data[0]=0x41;
      tx_data[1]=0xf1;
      tx_data[2]=0x0c;
      tx_data[3]=0x02;
      tx_data[4]=line_control_dress;
      
     if(master_flag)
      {
      tx_data[5]=0x01;
      tx_data[6]=0x02;    
      tx_data[7]=0x01;
      tx_data[8]=0x02;     
      }
      else
      {
      tx_data[5]=0x01;
      tx_data[6]=0x01;      
      tx_data[7]=0x01;
      tx_data[8]=0x01;       
      }
      tx_data[9]=0x01;

      if(master_flag)
      tx_data[10]=0x01;
      else
        tx_data[10]=0x02;


      tx_total=tx_data[2];



      xor_data=0;
      for(i=1;i<tx_total-1;i++)
       {
         xor_data^=tx_data[i];
       }
      tx_data[tx_total-1]=xor_data;
}
/********************************************************************************
********************************************************************************/
void line_dresschange_finish_tx_deal(void)
{
    unsigned char i=0;

      tx_data[0]=0x41;
      tx_data[1]=0xe2;
      tx_data[2]=0x0a;
      tx_data[3]=0x02;
      tx_data[4]=line_control_dress;
      
      if(master_flag)
      {
      tx_data[5]=0x01;
      tx_data[6]=0x01;
      tx_data[7]=0x01;
      tx_data[8]=0x01;
      }
      else
      {
      tx_data[5]=0x01;
      tx_data[6]=0x02;
      tx_data[7]=0x01;
      tx_data[8]=0x02;
      }
      


      tx_total=tx_data[2];

      xor_data=0;
      for(i=1;i<tx_total-1;i++)
       {
         xor_data^=tx_data[i];
       }
      tx_data[tx_total-1]=xor_data;
}
/********************************************************************************
********************************************************************************/
void line_dresschange_confirm_tx_deal(void)
{
  unsigned char i=0;

      tx_data[0]=0x41;
      tx_data[1]=0xf2;
      tx_data[2]=0x0a;
      tx_data[3]=0x02;
      tx_data[4]=line_control_dress;
      
      if(master_flag)
      {
      tx_data[5]=0x01;
      tx_data[6]=0x01;
      tx_data[7]=0x01;
      tx_data[8]=0x01;
      }
      else
      {
      tx_data[5]=0x01;
      tx_data[6]=0x02;
      tx_data[7]=0x01;
      tx_data[8]=0x02;
      }


      tx_total=tx_data[2];

      xor_data=0;
      for(i=1;i<tx_total-1;i++)
       {
         xor_data^=tx_data[i];
       }
      tx_data[tx_total-1]=xor_data;
}
/********************************************************************************
********************************************************************************/
void line_group_message_tx_deal(void)
{
    unsigned char i=0,j=0;

      tx_data[0]=0x41;
      tx_data[1]=0x00;
      tx_data[2]=0x2b;
      tx_data[3]=0x02;
      tx_data[4]=0x01;
      tx_data[5]=0x01;
      tx_data[6]=0x02;
      tx_data[7]=0x01;
      tx_data[8]=0x02;
      
     if(line_protocol&0x02)tx_data[9]=0xa4;
     else if(line_protocol&0x01)tx_data[9]=0xb4;
     else
      tx_data[9]=0x04;

      for(i=10;i<tx_data[2];i++)
      {
        tx_data[i]=0;
      }

      for(j=0;j<total_machine;j++)
      {

        if((basedata[j][2]>8)&&(basedata[j][2]<=16))
          tx_data[basedata[j][1]*2+9]|=(0x01<<(basedata[j][2]-9));
        else if(basedata[j][2]<=8)
         tx_data[basedata[j][1]*2+8]|=(0x01<<(basedata[j][2]-1));
      }


      tx_total=tx_data[2];
      xor_data=0;
      for(i=1;i<tx_total-1;i++)
       {
         xor_data^=tx_data[i];
       }
      tx_data[tx_total-1]=xor_data;
}
/********************************************************************************
********************************************************************************/
void line_group_message_extend_tx_deal(void)
{
   unsigned char i=0,j=0;


      tx_data[0]=0x41;
      tx_data[1]=0x00;
      tx_data[2]=0x2b;
      tx_data[3]=0x02;
      tx_data[4]=0x01;
      tx_data[5]=0x01;
      tx_data[6]=0x02;
      tx_data[7]=0x01;
      tx_data[8]=0x02;
      tx_data[9]=0x0b;

      for(i=10;i<tx_data[2];i++)
      {
        tx_data[i]=0;
      }

      for(j=0;j<total_machine;j++)
      {
        if((basedata[j][2]>24)&&(basedata[j][2]<=32))
        tx_data[basedata[j][1]*2+9]|=0x01<<(basedata[j][2]-25);
        else if((basedata[j][2]>16)&&(basedata[j][2]<=24))
         tx_data[basedata[j][1]*2+8]|=0x01<<(basedata[j][2]-17);
      }




      tx_total=tx_data[2];
      xor_data=0;
      for(i=1;i<tx_total-1;i++)
       {
         xor_data^=tx_data[i];
       }
      tx_data[tx_total-1]=xor_data;
}
/********************************************************************************
********************************************************************************/
void a3d_wind_tx_deal(void)
{
   unsigned char i=0,j=0,k=0,i1=0,j1=0,k1=0;

      tx_data[0]=0x71;
      tx_data[1]=0x00;
      tx_data[2]=0x21;
      tx_data[3]=0x02;
      tx_data[4]=line_control_dress;
      tx_data[5]=0x01;//tx_system_nuber1;
      tx_data[6]=0x01;//tx_dress_nuber1;
      tx_data[7]=0x01;//tx_system_nuber2;
      tx_data[8]=0x01;//tx_dress_nuber2;

      tx_data[9]=0xb1;

        i=system_mode&0x1f;
       tx_data[10]=(system_mode&0x1f)<<2;

      if(system_status)tx_data[10]|=0x01;
      else tx_data[10]&=0xfe;

     // if(defrost_flag)tx_data[10]|=0x02;
     // else tx_data[10]&=0xfd;



      tx_data[11]=system_wind&0x0f;
      if(system_wind&0x10)tx_data[11]|=0x80;

      tx_data[11]&=0x9f;
      switch(basedata[0][17]>>5)
      {
      case 0x00:
                break;
      case 0x01:tx_data[11]|=0x20;
                break;
      case 0x02:tx_data[11]|=0x40;
                break;
      case 0x03:tx_data[11]|=0x60;
                break;

      }


       tx_data[12]=((save_flag<<2)&0x0c)|(sleep_flag&0x03);
      
      //0-无效、1-风吹人、2-风避人、3-无人、4-风吹人和房间无人运行模式同时、5-风避人和房间无人运行模式同时
       
       switch(human_flag)
       {
       case 0:tx_data[13]=0x00;break;
       case 1:tx_data[13]=0x01;break;
       case 2:tx_data[13]=0x02;break;
       case 3:tx_data[13]=0x04;break;
       case 4:tx_data[13]=0x05;break;
       case 5:tx_data[13]=0x06;break;
       }
      

        switch(wind_mode)
        {
        case 0:               
                 i=wind_board_status;  j=wind_board2_status;k=wind_horizontal_status;
                 i1=8-wind_board_angle; j1=8-wind_board2_angle; k1=wind_horizontal_angle;
                  break;
        case 1:  i=1;  j=1;k=1;
                 i1=wind_board_angle;  j1=wind_board2_angle;k1=wind_horizontal_angle;

                  break;
        case 2:  i=0;  j=0;k=0;
                 i1=5;  j1=7;k1=4;
                  break;

        }



        tx_data[14]&=0xf0;
      if(i)
      {
           tx_data[14]|=0x01;
      }
      else
      {
           tx_data[14]|=(i1-1)<<1;

      }

      tx_data[14]&=0x0f;
      if(j)
      {
           tx_data[14]|=0x10;
      }
      else
      {
           tx_data[14]|=(j1-1)<<5;
      }

      tx_data[15]&=0xf0;
      if(k)
      {
           tx_data[15]|=0x01;
      }
      else
      {
           tx_data[15]|=(k1-1)<<1;
      }

      tx_data[15]&=0x0f;

      tx_data[16]=0;
      //wind_mode 0--普通 1--3D 2--超远
      if(wind_mode==1)tx_data[16]|=0x01;
      else if(wind_mode==2)tx_data[16]|=0x02;

      if(system_mode==0x44) tx_data[16]|=0x10;
      
      if(half_set)tx_data[16]|=0x40;

      tx_data[17]=tempset;
      tx_data[18]=dry_set;


      tx_data[19]=basedata[0][24];//室内温度
      tx_data[20]=basedata[0][25];//室内湿度

      if(err_now_flag)
      {
       // if(ee_err_flag)
       //   tx_data[21]=0xee;
       // else
        tx_data[21]=option_data[0][10];
      }
      else
      {
         tx_data[21]=0;
      }

      tx_data[22]=basedata[0][19];//对应2G协议室内机至线控器第32字节
      tx_data[23]=0;//备用

      tx_data[24]=option_data[0][0];//机能选择1
      tx_data[25]=option_data_new[0][3];//机能选择9
      tx_data[26]=option_data_new[0][4];//机能选择10
      tx_data[27]=option_data_new[0][8];//机能选择14

      tx_data[28]=0;//机能选择16
      tx_data[29]=0;//备用
      tx_data[30]=0;//备用

      tx_data[31]=0;
      if(a3d_wind_led_flag)  tx_data[31]|=0x01;



      tx_total=tx_data[2];

      xor_data=0;
      for(i=1;i<tx_total-1;i++)
       {
         xor_data^=tx_data[i];
       }
      tx_data[tx_total-1]=xor_data;
}
/********************************************************************************
********************************************************************************/
void save_option_tx_deal(void)
{
  unsigned char i=0;

      tx_data[0]=0x21;
      tx_data[1]=0x00;
      tx_data[2]=0x11;
      tx_data[3]=0x02;
      tx_data[4]=line_control_dress;
      tx_data[5]=tx_system_nuber1;
      tx_data[6]=tx_dress_nuber1;
      tx_data[7]=tx_system_nuber2;
      tx_data[8]=tx_dress_nuber2;

      tx_data[9]=0x90;

       

      



      tx_total=tx_data[2];

      xor_data=0;
      for(i=1;i<tx_total-1;i++)
       {
         xor_data^=tx_data[i];
       }
      tx_data[tx_total-1]=xor_data;
}
/********************************************************************************
********************************************************************************/


/********************************************************************************
********************************************************************************/
void system_tx_check(void)
{

  unsigned char i=0;
  
//  if(system_tx_10ms==0)return;
//  system_tx_10ms=0;
  
  
  
//  option_restore_tx_check();
  
  
  
  if((initialize_flag==0)||(self_check_flag))return;

  
  
  if(tx_change_flag)
  {
  
    tx_time_1s_cnt++;
    if(tx_time_1s_cnt>=100)    
    {
        tx_time_1s_cnt=0;
        tx_change_flag=0;
        tx_change_tx_flag=1;
        
        tx_time_systemon_cnt=0;
        tx_systemon_flag=0;
        
       
   
        if((wifi_tx_flag==0)&&(wifi_connect_ok))
        {
             cloud_servers_cmd=2;
             did0_data=16;
             did0_lenth=97-16;
             wifi_tx_flag=3;
        }
     
    }
  }
  
  

  tx_time_systemon_cnt++;
  if(tx_time_systemon_cnt>=3000)
  {
    tx_time_systemon_cnt=0;
    tx_systemon_flag=1;
    err_4s_cnt=0;
	printf("ok9");
  } 

  

  
  if(tx_data_flag[0].data||tx_data_flag[1].data||tx_data_flag[2].data||tx_data_flag[3].data) return;
  
  //浠庡埆鐨勭嚎鎺у櫒鍙戞潵鐨勫懡浠わ紝褰撴湰韬负浜叉椂锛屽彧缁欏鍐呮満鍙戦�佸熀鏈牸寮忥紝褰撲负瀛愭椂锛屽厛缁欎翰绾挎帶鍣ㄥ彂閫佸熀鏈ā寮?
  if(line_change_flag)
  {
    line_change_flag=0;
    tx_systemon_flag=1;
    /*
    if(master_flag)
    {
      tx_change_tx_flag=1;
    }
    else 
    {
      tx_systemon_flag=1;   
    }
    */
  }
  
  
  if(tx_change_tx_flag)
  {
    tx_change_tx_flag=0;
    if(line_control_double)
    {
      line_control_tx=1;
    }
    else
    {
      timing_tx_flag=1;
      already_tx_nuber=0;
      tx_system_nuber1=basedata[already_tx_nuber][1];tx_dress_nuber1=basedata[already_tx_nuber][2];
      tx_system_nuber2=basedata[already_tx_nuber][1];tx_dress_nuber2=basedata[already_tx_nuber][2];
    }
	printf("ok8");
  }
  else if(tx_systemon_flag)
  {
    tx_systemon_flag=0;
    timing_tx_flag=1;
    already_tx_nuber=0;
    tx_system_nuber1=basedata[already_tx_nuber][1];tx_dress_nuber1=basedata[already_tx_nuber][2];
    tx_system_nuber2=basedata[already_tx_nuber][1];tx_dress_nuber2=basedata[already_tx_nuber][2];
  }
  else if(err_now_flag)
  {
    err_4s_cnt++;
    if(err_4s_cnt>=400)
    {
      err_4s_cnt=0;
      /////////////////////////
      //鏈夐敊璇彂鐢熺殑瀹ゅ唴鏈烘瘡4绉掑彂閫佷竴娆″熀鏈牸寮?
      i=0;
      
        do
         {
           err_4s_nuber++;
           if(err_4s_nuber>=total_machine)
           {
             err_4s_nuber=0;  
             if(a3d_wind_err)break;
           }
           i++;
           if(i>(total_machine*2))break;
         } 
         while((basedata[err_4s_nuber][4]&0x01)==0);     
         timing_tx_flag=1;
	 already_tx_nuber=total_machine;
         tx_system_nuber1=basedata[err_4s_nuber][1];tx_dress_nuber1=basedata[err_4s_nuber][2];
         tx_system_nuber2=basedata[err_4s_nuber][1];tx_dress_nuber2=basedata[err_4s_nuber][2]; 

      //////////////////////
    }
  }
  
  
  
}
/********************************************************************************
********************************************************************************/

//===============================================================================
//trigger to send data out for normal 
void homebus_api_trigger()
{
	tx_change_flag=1;
	tx_time_1s_cnt=0;
}

void homebus_api_power_on()
{
	system_status=1;
	homebus_api_trigger();
}


void homebus_api_power_off()
{
	system_status=0;
	
	homebus_api_trigger();
}

