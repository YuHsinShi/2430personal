#if 1  //porting variable tmp
unsigned char remote_bit_num;
unsigned char remote_byte_num;
unsigned char remote_data;
unsigned char remote_flag;
unsigned char remote_cnt;
unsigned char remote_repeat_start;
unsigned char remote_repeat_time;
unsigned char remote_nuber;
unsigned char receive_end_flag;
unsigned char easy_mode;
unsigned char option_l2;
unsigned char option_hh;
unsigned char initialize_flag;
unsigned char PPG_type;
unsigned char PPG_time;
unsigned char a3d_wind_led_time_cnt;
unsigned char a3d_wind_led_flag;
unsigned char set_status;
unsigned char center_control_mode;
unsigned char por_06_flag;
unsigned char center_control_flag;
unsigned char self_clean_flag;
unsigned char self_clean_flash_cnt;
unsigned char self_clean_flash_flag;
unsigned char high_temp_disinfect_flag;
unsigned char high_temp_disinfect_flash_cnt;
unsigned char high_temp_disinfect_flash_flag;
unsigned char try_run_set;
unsigned char try_run_set_write;
unsigned char try_run_flag;
unsigned char pre_on_flag;
unsigned char pre_on_flag_write;
unsigned char pre_off_flag;
unsigned char pre_off_flag_write;
unsigned char pre_off_time;
unsigned char pre_off_time_write;
unsigned char pre_off_minu;
unsigned char pre_off_time_b;
unsigned char pre_off_time_b_write;
unsigned char system_mode_write;
unsigned char center_control_wind;
unsigned char save_flag;
unsigned char save_used;
unsigned char save_flash_cnt;
unsigned char save_flash_flag;
unsigned char mute_flag;
unsigned char mute_used;
unsigned char mute_flash_cnt;
unsigned char forest_wind_flag;
unsigned char forest_wind_used;
unsigned char forest_wind_flash_cnt;
unsigned char forest_wind_flash_flag;
unsigned char option_fa;
unsigned char system_wind_canntchange;
unsigned char wind_enable;
unsigned char system_wind;
unsigned char wind_auto_flag;
unsigned char system_wind_write;
unsigned char wind_ignore_check;
unsigned char option_l1;
unsigned char TEMP_SETING;
unsigned char settime_cnt;
unsigned char center_control_tempset;
unsigned char option_f9;
unsigned char tempset_canntchange;
unsigned char fre_machine;
unsigned char try_run_status;
unsigned char try_run_fre;
unsigned char fre_table;
unsigned char try_run_set_cnt1;
unsigned char try_run_set_cnt2;
unsigned char dry_set;
unsigned char dry_set_write;
unsigned char tempset;
unsigned char tempset_max;
unsigned char tempset_min;
unsigned char tempset_write;
unsigned char half_set;
unsigned char half_set_write;
unsigned char clear_eeprom_flag;


unsigned char try_run_flag_write;
unsigned char center_onoff_flag;
unsigned char center_useon_flag;
unsigned char center_control_onoff;
unsigned char system_status;
//unsigned char system_off_deal;
unsigned char lock_flag;
unsigned char option_f8;
unsigned char lock_flash_cnt;
unsigned char lock_flash_flag;
unsigned char system_mode_canntchange;
unsigned char tx_time_1s_cnt;
unsigned char tx_change_flag;
unsigned char system_mode;
unsigned char machine_type1;
unsigned char mode_type;
unsigned char mute_flash_flag;
unsigned char wind_ignore_check;
unsigned char clear_eeprom_flag_write;
unsigned char option_ff;
unsigned char point_set_status;
unsigned char server_set_status;
unsigned char err_now_flag;
unsigned char option_preoff;
unsigned char pre_on_time_b;
unsigned char pre_on_minu;
unsigned char pre_on_time;
unsigned char pre_on_time_write;
unsigned char pre_on_time_b_write;
unsigned char sleep_flag;
unsigned char sleep_flag_write;
unsigned char winddir_enable;
unsigned char no_action_flag;
unsigned char center_control_windboard;
unsigned char option_fb;
unsigned char wind_board_status;
unsigned char wind_board_status_write;
unsigned char wind_board_changing;
unsigned char wind_board_angle;
unsigned char a3d_wind_use;
unsigned char wind_board_angle_write;
unsigned char wind_horizontal_angle;
unsigned char wind_horizontal_angle_write;
unsigned char wind_horizontal_status;
unsigned char wind_horizontal_status_write;
unsigned char wind_board2_angle;
unsigned char wind_board2_angle_write;
unsigned char wind_board2_status;
unsigned char wind_board2_status_write;
unsigned char wind_horizontal2_angle;
unsigned char wind_horizontal2_angle_write;
unsigned char wind_horizontal2_status;
unsigned char wind_horizontal2_status_write;
unsigned char wind_mode;
unsigned char wind_mode_write;
unsigned char sleep_used;


unsigned char wind_ignore_check;
unsigned char mute_flag_write;
unsigned char save_flag_write;
unsigned char forest_wind_flag_write;
unsigned char heat_flag;
unsigned char heat_flag_write;
unsigned char heat_used;
unsigned char health_used;
unsigned char health_flag;
unsigned char health_flag_write;
unsigned char a3d_wind_err;
unsigned char err_reset_flag;
unsigned char total_machine;
unsigned char line_err_reset_flag;
unsigned char filter_flag;
unsigned char filter_reset;
unsigned char human_used;
unsigned char human_flag;
unsigned char human_flag_write;
#endif

unsigned char remote_byte_buf[128];

void remote_receive_program(int remote_pulse_time)
{
	int temp0;
ithPrintf("%d-",remote_pulse_time);

	temp0 = remote_pulse_time;
	remote_pulse_time = 0;
ithPrintf("%d",remote_cnt);
	if ((temp0 <= 5) || (temp0>220))//5 220
	{
				ithPrintf("R");

		remote_bit_num = 0;
		remote_byte_num = 0;
		remote_data = 0;
		remote_flag &= 0xfe;
		remote_cnt = 0;
	}
	else if ((temp0<80) && (temp0>30))//80~30
	{
		ithPrintf("A");

		remote_bit_num = 0;
		remote_byte_num = 0;
		remote_data = 0;
		remote_cnt = 0;
		remote_flag |= 0x01;
		if (remote_repeat_start)
			remote_repeat_time = 0;
	}
	else if (remote_flag & 0x01) // 6~30 && 80~220
	{
		ithPrintf("B");

		remote_bit_num++;
		remote_data >>= 1;

		if ((temp0<12) && (temp0>6))
		{
			remote_data &= 0x7f;
		}
		if ((temp0<21) && (temp0 >= 13))
		{
			remote_data |= 0x80;
		}

		if (remote_bit_num == 8)
		{
			remote_bit_num = 0;

			remote_byte_buf[remote_cnt] = remote_data;

			if (remote_cnt<27)
				remote_cnt++;

			switch (remote_byte_buf[7] & 0x0f)
			{
			case 1:remote_nuber = 13; break;
			case 2:remote_nuber = 15; break;
			case 3:remote_nuber = 17; break;
			case 4:remote_nuber = 19; break;
			case 5:remote_nuber = 21; break;
			case 6:remote_nuber = 23; break;
			case 8:remote_nuber = 27; break;
			default:remote_nuber = 13; break;
			}

			if ((remote_cnt >= 13) && (remote_cnt >= remote_nuber))
			{
				receive_end_flag = 0x01; remote_repeat_start = 0x01;
			}
		}
	}
	else
	{
		remote_bit_num = 0;
		remote_byte_num = 0;
		remote_data = 0;
		remote_flag &= 0xfe;
		remote_cnt = 0;
	}
}

void remote_cmd_buffer_in(unsigned char* cmd,int size)
{
	memcpy(remote_byte_buf,cmd, size);

}
void remote_deal_program(void)
{
	unsigned char i, j, remote_d1, remote_d2, enable_flag;
	unsigned int remote_t1, remote_t2;



		if (easy_mode)return;
		if (option_l2>1)return;


		if (option_l2 == 1)
		{
			if (remote_byte_buf[11] & 0x80)
			{
				remote_byte_buf[11] &= 0x7f;
				remote_byte_buf[12] |= 0x80;
			}
			else
			{
				remote_byte_buf[12] &= 0x7f;
				remote_byte_buf[11] |= 0x80;
			}
			//remote_byte_buf[11]&=0x7f;
			//remote_byte_buf[12]|=0x80;

		}

		if (remote_byte_buf[11] & 0x80)return;

		if ((remote_byte_buf[0] == 0x01) && (remote_byte_buf[1] == 0x10) && (option_hh == 0))enable_flag = 1;
		else if ((remote_byte_buf[0] == 0x02) && (remote_byte_buf[1] == 0x20) && (option_hh))enable_flag = 1;
		else enable_flag = 0;

		if ((enable_flag) && (remote_byte_buf[2] == 0x00) &&
			(remote_byte_buf[3] == 0x40) && (remote_byte_buf[4] == 0xbf) && (remote_byte_buf[5] == 0xff) &&
			(remote_byte_buf[6] == 0x00) && (remote_byte_buf[9] == 0x89) && (remote_byte_buf[10] == 0x76) && (initialize_flag))
		{
			PPG_type = 1;
			PPG_time = 0;

			a3d_wind_led_time_cnt = 0;
			a3d_wind_led_flag = 1;

			set_status = 0x00;
			//tx_time_1s_cnt=0;
			//tx_change_flag=1;


			////////////////////////
			//?????
			if ((remote_byte_buf[11] == 0x0a) && (remote_byte_buf[12] == 0xf5))
			{
				if ((center_control_mode) || (por_06_flag))
				{
					center_control_flag = 1;

				}
				else if (self_clean_flag)
				{
					self_clean_flash_cnt = 0;
					self_clean_flash_flag = 1;
				}
				else if (high_temp_disinfect_flag)
				{
					high_temp_disinfect_flash_cnt = 0;
					high_temp_disinfect_flash_flag = 1;
				}
				else
				{
					system_on_deal();//???? ???????????
					try_run_set = 0;
					try_run_set_write = 1;
					try_run_flag |= 0x01;
					try_run_flag_write = 1;
					pre_on_flag = 0;
					pre_on_flag_write = 1;
					pre_off_flag = 1;
					pre_off_flag_write = 1;
					pre_off_time = 4;
					pre_off_time_write = 1;
					pre_off_minu = 0;
					pre_off_time_b = pre_off_time;
					pre_off_time_b_write = 1;
				}
			}

			////////////////////////
			//????
			if ((remote_byte_buf[7] == 0xe6) && (remote_byte_buf[8] == 0x19) &&
				(remote_byte_buf[11] == 0x01) && (remote_byte_buf[12] == 0xfe) &&
				(remote_byte_buf[21] == 0x90) && (remote_byte_buf[22] == 0x6f)
				)
			{
				if (center_onoff_flag & 0x01)center_useon_flag = 1;
				else center_useon_flag = 0;

				system_on_deal();//???? ???????????
			}
			///////////////////////
			//????
			if ((remote_byte_buf[7] == 0xe6) && (remote_byte_buf[8] == 0x19) &&
				(remote_byte_buf[11] == 0x01) && (remote_byte_buf[12] == 0xfe) &&
				(remote_byte_buf[21] == 0xa0) && (remote_byte_buf[22] == 0x5f)
				)
			{

				if ((center_control_onoff) && (system_status))
					center_onoff_flag = 0x01;


				system_off_deal();


			}
			///////////////////////
			//?? ????
			if ((remote_byte_buf[7] == 0xe6) && (remote_byte_buf[8] == 0x19) &&
				(remote_byte_buf[11] == 0x01) && (remote_byte_buf[12] == 0xfe))
			{
				remote_d1 = remote_byte_buf[17]; remote_d2 = remote_byte_buf[18]; i = 1;
			}
			else if ((remote_byte_buf[7] == 0xe5) && (remote_byte_buf[8] == 0x1a) &&
				(remote_byte_buf[11] == 0x04) && (remote_byte_buf[12] == 0xfb))
			{
				remote_d1 = remote_byte_buf[17]; remote_d2 = remote_byte_buf[18]; i = 1;
			}
			else if ((remote_byte_buf[7] == 0xe3) && (remote_byte_buf[8] == 0x1c) &&
				(remote_byte_buf[11] == 0x05) && (remote_byte_buf[12] == 0xfa))
			{
				remote_d1 = remote_byte_buf[15]; remote_d2 = remote_byte_buf[16]; i = 1;
			}
			else if ((remote_byte_buf[7] == 0xe8) && (remote_byte_buf[8] == 0x17) &&
				(remote_byte_buf[11] == 0x0b) && (remote_byte_buf[12] == 0xf4))
			{
				remote_d1 = remote_byte_buf[23]; remote_d2 = remote_byte_buf[24]; i = 1;
			}
			else if ((remote_byte_buf[11] == 0x0a) && (remote_byte_buf[12] == 0xf5))
			{
				remote_d1 = ((remote_byte_buf[17] << 4) & 0xF0) | (((remote_byte_buf[17] >> 4) & 0x0F));
				remote_d2 = ((remote_byte_buf[18] << 4) & 0xF0) | (((remote_byte_buf[18] >> 4) & 0x0F));
				i = 2;
			}
			/*
			else if((remote_byte_buf[11]==0x71)&&(remote_byte_buf[12]==0x8e)&&(system_status))
			{
			remote_d1=remote_byte_buf[17];remote_d2=remote_byte_buf[18];i=1;
			}
			else if((remote_byte_buf[11]==0x73)&&(remote_byte_buf[12]==0x8c)&&(system_status))
			{
			remote_d1=remote_byte_buf[17];remote_d2=remote_byte_buf[18];i=1;
			}
			else if((remote_byte_buf[11]==0x74)&&(remote_byte_buf[12]==0x8b)&&(system_status))
			{
			remote_d1=remote_byte_buf[17];remote_d2=remote_byte_buf[18];i=1;
			}
			*/
			else
			{
				i = 0;
			}

			if ((i) && (remote_d1 + remote_d2 == 0xff))
			{
				//////////////////////////////////////////
				if ((center_control_mode) || (por_06_flag))
				{
					center_control_flag = 1;
				}
				else if (self_clean_flag)
				{
					self_clean_flash_cnt = 0;
					self_clean_flash_flag = 1;
				}
				else if (high_temp_disinfect_flag)
				{
					high_temp_disinfect_flash_cnt = 0;
					high_temp_disinfect_flash_flag = 1;
				}
				else if ((lock_flag) && (option_f8))
				{
					lock_flash_cnt = 0;
					lock_flash_flag = 1;
				}

				else if (system_mode_canntchange)
				{

				}
				else
				{
					tx_time_1s_cnt = 0;
					tx_change_flag = 1;

					j = system_mode;

					if (machine_type1 == 2)
					{
						system_mode = 0x08;
					}
					else
					{
						switch (remote_d1 & 0x0f)
						{
						case 0x02:system_mode = 0x08; break;
						case 0x04:
							if (machine_type1 == 4)system_mode = 0x84;
							else system_mode = 0x04;
							break;
						case 0x06:system_mode = 0x02; break;
						case 0x08:if (mode_type & 0x02)system_mode = 0x10; break;
						case 0x0b:if ((mode_type & 0x01) && (try_run_flag == 0) && (try_run_set == 0))system_mode = 0x01; break;
						case 0x0c:if (machine_type1 == 4)system_mode = 0xa4;
								  else system_mode = 0x04;
								  break;
						case 0x0d:if (machine_type1 == 4)system_mode = 0xc4;
								  else system_mode = 0x04;
								  break;
						}
					}


					////////////////////////
					//????????????
					if ((machine_type1 == 3) && (system_mode == 0x04))
						system_mode = 0x44;
					///////////////////////////////
					//law //mode_ignore_check();


					if (j != system_mode)fuction_clear1_deal();

					system_mode_write = 1;


				}


				//////////////////////////////////////////
				if ((center_control_wind) || (por_06_flag))
				{
					center_control_flag = 1;
				}
				else if (self_clean_flag)
				{
					self_clean_flash_cnt = 0;
					self_clean_flash_flag = 1;
				}
				else if (high_temp_disinfect_flag)
				{
					high_temp_disinfect_flash_cnt = 0;
					high_temp_disinfect_flash_flag = 1;
				}
				else if ((save_flag) && (save_used))
				{
					save_flash_cnt = 0;
					save_flash_flag = 1;
				}
				else if ((mute_flag) && (mute_used))
				{
					mute_flash_cnt = 0;
					mute_flash_flag = 1;
				}
				else if ((forest_wind_flag) && (forest_wind_used))
				{
					forest_wind_flash_cnt = 0;
					forest_wind_flash_flag = 1;
				}
				else if ((lock_flag) && (option_fa))

				{
					lock_flash_cnt = 0;
					lock_flash_flag = 1;
				}

				else if (system_wind_canntchange)
				{

				}
				else if (i == 2)
				{

				}
				else
				{
					tx_time_1s_cnt = 0;
					tx_change_flag = 1;

					if (wind_enable >= 4)
					{

						switch ((remote_d1 >> 4) & 0x0f)
						{
						case 0x09:system_wind = 0x18; break;
						case 0x01:system_wind = 0x08; break;
						case 0x0a:system_wind = 0x14; break;
						case 0x02:system_wind = 0x04; break;
						case 0x04:system_wind = 0x02; break;
						case 0x0c:system_wind = 0x12; break;
						case 0x08:if (wind_auto_flag)system_wind = 0x01; break;
						}
					}
					else if ((wind_enable >= 2) && (wind_enable <= 3))
					{
						switch ((remote_d1 >> 4) & 0x0f)
						{
						case 0x09:system_wind = 0x08; break;//
						case 0x0a:system_wind = 0x04; break;//


						case 0x01:system_wind = 0x08; break;
						case 0x02:system_wind = 0x04; break;
						case 0x04:system_wind = 0x02; break;
						case 0x0c:system_wind = 0x12; break;
						case 0x08:if (wind_auto_flag)system_wind = 0x01; break;
						}
					}
					else
					{
						switch ((remote_d1 >> 4) & 0x0f)
						{
						case 0x09:system_wind = 0x08; break;//
						case 0x0a:system_wind = 0x04; break;//
						case 0x0c:system_wind = 0x02; break;//

						case 1:system_wind = 0x08; break;
						case 2:system_wind = 0x04; break;
						case 4:system_wind = 0x02; break;
						case 8:if (wind_auto_flag)system_wind = 0x01; break;
						}
					}
					system_wind_write = 1;

					//LAW// wind_ignore_check();

				}
				//////////////////////////////////////////


			}
			///////////////////////
			//?Ú…???
			if ((remote_byte_buf[7] == 0xe6) && (remote_byte_buf[8] == 0x19) &&
				(remote_byte_buf[11] == 0x01) && (remote_byte_buf[12] == 0xfe))
			{
				remote_d1 = remote_byte_buf[15]; remote_d2 = remote_byte_buf[16]; i = 1;
			}
			else if ((remote_byte_buf[7] == 0xe5) && (remote_byte_buf[8] == 0x1a) &&
				(remote_byte_buf[11] == 0x04) && (remote_byte_buf[12] == 0xfb))
			{
				remote_d1 = remote_byte_buf[15]; remote_d2 = remote_byte_buf[16]; i = 1;
			}
			else if ((remote_byte_buf[7] == 0xe8) && (remote_byte_buf[8] == 0x17) &&
				(remote_byte_buf[11] == 0x0b) && (remote_byte_buf[12] == 0xf4))
			{
				remote_d1 = remote_byte_buf[15]; remote_d2 = remote_byte_buf[16]; i = 1;
			}
			else if ((remote_byte_buf[7] == 0xe3) && (remote_byte_buf[8] == 0x1c) &&
				(remote_byte_buf[11] == 0x09) && (remote_byte_buf[12] == 0xf6))
			{
				remote_d1 = remote_byte_buf[15]; remote_d2 = remote_byte_buf[16]; i = 1;
				if (option_l1)
				{
					set_status = TEMP_SETING;
					settime_cnt = 0;
				}
			}
			else if ((remote_byte_buf[7] == 0xe3) && (remote_byte_buf[8] == 0x1c) &&
				(remote_byte_buf[11] == 0x08) && (remote_byte_buf[12] == 0xf7))
			{
				remote_d1 = remote_byte_buf[15]; remote_d2 = remote_byte_buf[16]; i = 1;
				if (option_l1)
				{
					set_status = TEMP_SETING;
					settime_cnt = 0;
				}
			}
			else if ((remote_byte_buf[11] == 0x0a) && (remote_byte_buf[12] == 0xf5))
			{
				remote_d1 = remote_byte_buf[13]; remote_d2 = remote_byte_buf[14]; i = 1;
			}
			else if ((remote_byte_buf[11] == 0x71) && (remote_byte_buf[12] == 0x8e) && (system_status))
			{
				remote_d1 = remote_byte_buf[15]; remote_d2 = remote_byte_buf[16]; i = 1;
			}
			else if ((remote_byte_buf[11] == 0x73) && (remote_byte_buf[12] == 0x8c) && (system_status))
			{
				remote_d1 = remote_byte_buf[15]; remote_d2 = remote_byte_buf[16]; i = 1;
			}
			else if ((remote_byte_buf[11] == 0x74) && (remote_byte_buf[12] == 0x8b) && (system_status))
			{
				remote_d1 = remote_byte_buf[15]; remote_d2 = remote_byte_buf[16]; i = 1;
			}
			else
			{
				i = 0;
			}

			if ((i) && (remote_d1 + remote_d2 == 0xff))
			{
				//////////////////////////////////////////
				j = 17 + (remote_d1 - 21) / 2;
				if ((center_control_tempset) || (por_06_flag))
				{
					center_control_flag = 1;
				}
				else if (self_clean_flag)
				{
					self_clean_flash_cnt = 0;
					self_clean_flash_flag = 1;
				}
				else if (high_temp_disinfect_flag)
				{
					high_temp_disinfect_flash_cnt = 0;
					high_temp_disinfect_flash_flag = 1;
				}
				else if ((lock_flag) && (option_f9))
				{
					lock_flash_cnt = 0;
					lock_flash_flag = 1;
				}

				else if ((tempset_canntchange) || (machine_type1 == 2))
				{

				}
				else if ((save_flag) && (save_used))
				{
					save_flash_cnt = 0;
					save_flash_flag = 1;
				}
				else  if ((try_run_flag) || (try_run_set))
				{
					if ((fre_machine) && (system_mode != 0x08))
					{
						tx_change_flag = 1;
						try_run_status = 1;

					//LAW//	if ((j >= 17) && (j <= 30))try_run_fre = fre_table[j - 17];

					}

					if (try_run_set)
					{
						try_run_set_cnt1 = 0;
						try_run_set_cnt2 = 0;
					}
				}
				else if (system_mode == 0x44)
				{
					if (j <= 19)dry_set = 35;
					else if (j>30)dry_set = 90;
					else dry_set = (j - 19) * 5 + 35;

					dry_set_write = 1;
					tx_time_1s_cnt = 0;
					tx_change_flag = 1;
				}
				else
				{
					if (j != tempset)
					{
						tempset = j;
						if (tempset>tempset_max)tempset = tempset_max;
						if (tempset<tempset_min)tempset = tempset_min;
						tempset_write = 1;
						tx_time_1s_cnt = 0;
						tx_change_flag = 1;


					}

					if (half_set)
					{
						half_set = 0;
						half_set_write = 1;
					}


					if (clear_eeprom_flag)
					{
						clear_eeprom_flag = 0;
						clear_eeprom_flag_write = 1;
					}
				}
				//////////////////////////////////////////
			}
			///////////////////////
			//???????
			i = 0;
			if ((remote_byte_buf[7] == 0xe8) && (remote_byte_buf[8] == 0x17) &&
				(remote_byte_buf[11] == 0x0b) && (remote_byte_buf[12] == 0xf4) &&
				(remote_byte_buf[17] + remote_byte_buf[18] == 0xff) &&
				(remote_byte_buf[19] + remote_byte_buf[20] == 0xff) &&
				(remote_byte_buf[21] + remote_byte_buf[22] == 0xff))

			{
				remote_t1 = (((unsigned int)remote_byte_buf[19] & 0x0f) << 8) + remote_byte_buf[17];
				remote_t2 = ((unsigned int)remote_byte_buf[21] << 4) + (remote_byte_buf[19] >> 4);
				i = 1;
			}
			else if ((remote_byte_buf[11] == 0x0a) && (remote_byte_buf[12] == 0xf5))
			{
				remote_t1 = (((unsigned int)remote_byte_buf[17] & 0x0f) << 8) + remote_byte_buf[15];
				remote_t2 = 0;
				i = 1;
			}

			if (i)
			{
				if ((lock_flag) && (option_ff))
				{
					lock_flash_cnt = 0;
					lock_flash_flag = 1;
				}
				else if (self_clean_flag)
				{
					self_clean_flash_cnt = 0;
					self_clean_flash_flag = 1;
				}
				else if (high_temp_disinfect_flag)
				{
					high_temp_disinfect_flash_cnt = 0;
					high_temp_disinfect_flash_flag = 1;
				}
				else if ((remote_t1) && (remote_t2 == 0))//?????
				{
					if ((point_set_status == 0) && (server_set_status == 0) && (err_now_flag == 0) && (option_preoff == 0))
					{
						pre_off_time_b = remote_t1 / 30;
						//pre_off_minu=0;
						if (remote_t1 % 30)
						{
							pre_off_time_b += 1;
							pre_off_minu = 30 - remote_t1 % 30;
						}
						else
						{
							pre_off_minu = 0;
						}



						pre_off_time = pre_off_time_b;
						pre_off_time_write = 1;
						pre_off_time_b_write = 1;
						pre_off_flag = 1;
						pre_off_flag_write = 1;
					}
				}
				else if ((remote_t2) && (remote_t1 == 0))//?????
				{

					if ((point_set_status == 0) && (server_set_status == 0))
					{
						pre_on_time_b = remote_t2 / 30;
						//pre_on_minu=0;
						if (remote_t2 % 30)
						{
							pre_on_time_b += 1;
							pre_on_minu = 30 - remote_t2 % 30;
						}
						else
						{
							pre_on_minu = 0;
						}



						pre_on_time = pre_on_time_b;
						pre_on_time_write = 1;
						pre_on_time_b_write = 1;
						pre_on_flag = 1;
						pre_on_flag_write = 1;
					}
				}
			}



			///////////////////////
			//??????
			if ((remote_byte_buf[7] == 0xe2) && (remote_byte_buf[8] == 0x1d) &&
				(remote_byte_buf[11] == 0x0d) && (remote_byte_buf[12] == 0xf2))
			{
				/////////////////////////
				if (pre_on_flag)
				{
					pre_on_flag = 0;
					pre_on_flag_write = 1;
				}
				if (pre_off_flag)
				{
					if ((try_run_flag == 0) && (err_now_flag == 0) && (option_preoff == 0))
					{
						pre_off_flag = 0;
						pre_off_flag_write = 1;
						if (sleep_flag)
						{
							sleep_flag = 0;
							sleep_flag_write = 1;
							tx_change_flag = 1;
							tx_time_1s_cnt = 0;
						}
					}
				}
				/////////////////////////
			}
			///////////////////////
			//????
			if ((remote_byte_buf[7] == 0xe3) && (remote_byte_buf[8] == 0x1c) &&
				(remote_byte_buf[11] == 0x0e) && (remote_byte_buf[12] == 0xf1))
			{
				remote_d1 = remote_byte_buf[15]; remote_d2 = remote_byte_buf[16]; i = 1;
			}
			else
			{
				i = 0;
			}

			/////////////////////////
			if ((i) && (remote_d1 + remote_d2 == 0xff))
			{

				if (winddir_enable == 0)
				{
					no_action_flag = 1;
				}
				else if ((center_control_windboard) || (por_06_flag))
				{
					center_control_flag = 1;
				}
				else if (self_clean_flag)
				{
					self_clean_flash_cnt = 0;
					self_clean_flash_flag = 1;
				}
				else if (high_temp_disinfect_flag)
				{
					high_temp_disinfect_flash_cnt = 0;
					high_temp_disinfect_flash_flag = 1;
				}
				else if ((forest_wind_flag) && (forest_wind_used))
				{
					forest_wind_flash_cnt = 0;
					forest_wind_flash_flag = 1;
				}
				else if ((option_fb) && (lock_flag))
				{
					lock_flash_cnt = 0;
					lock_flash_flag = 1;
				}

				else
				{
					// if((remote_byte_buf[15]==0x01)&&(remote_byte_buf[16]==0xfe))
					if (remote_d1 == 0x01)
					{
						wind_board_status = 1;
						wind_board_status_write = 1;
						wind_board_changing = wind_board_angle;
					}
					else
					{

						switch (remote_d1)
						{
						case 0x00:j = 1; break;
						case 0x02:j = 2; break;
						case 0x04:j = 3; break;
						case 0x06:j = 4; break;
						case 0x08:j = 5; break;
						case 0x0a:j = 6; break;
						case 0x0c:j = 7; break;
						default:j = 1; break;
						}

						if (a3d_wind_use)wind_board_angle = 8 - j;//8-
						else wind_board_angle = j;

						wind_board_angle_write = 1;
						wind_board_status = 0;
						wind_board_status_write = 1;
					}

					if (winddir_enable == 2)//?????
					{

						wind_horizontal_angle = wind_board_angle;
						wind_horizontal_angle_write = 1;
						wind_horizontal_status = wind_board_status;
						wind_horizontal_status_write = 1;

						wind_board2_angle = wind_board_angle;
						wind_board2_angle_write = 1;
						wind_board2_status = wind_board_status;
						wind_board2_status_write = 1;

						wind_horizontal2_angle = wind_board_angle;
						wind_horizontal2_angle_write = 1;
						wind_horizontal2_status = wind_board_status;
						wind_horizontal2_status_write = 1;

					}
					else if (winddir_enable == 5)//??????
					{
						wind_horizontal_angle = wind_board_angle;
						wind_horizontal_angle_write = 1;
						wind_horizontal_status = wind_board_status;
						wind_horizontal_status_write = 1;
					}
					else if (winddir_enable == 4)//3D?????
					{
						wind_board2_angle = wind_board_angle;
						wind_board2_angle_write = 1;
						wind_board2_status = wind_board_status;
						wind_board2_status_write = 1;
					}



					if (wind_mode)
					{
						wind_mode = 0;
						wind_mode_write = 1;
					}


					tx_time_1s_cnt = 0;
					tx_change_flag = 1;
				}
			}
			////////////////////////
			//???????
			if ((remote_byte_buf[11] == 0x72) && (remote_byte_buf[12] == 0x8d))
			{
				remote_d1 = remote_byte_buf[15]; remote_d2 = remote_byte_buf[16]; i = 1;
			}
			else if ((remote_byte_buf[11] == 0x74) && (remote_byte_buf[12] == 0x8b))
			{
				remote_d1 = remote_byte_buf[23]; remote_d2 = remote_byte_buf[24]; i = 1;
			}
			else
			{
				i = 0;
			}

			if ((i) && (remote_d1 + remote_d2 == 0xff))
			{

				if (winddir_enable == 0)
				{
					no_action_flag = 1;
				}
				else if ((center_control_windboard) || (por_06_flag))
				{
					center_control_flag = 1;
				}
				else if (self_clean_flag)
				{
					self_clean_flash_cnt = 0;
					self_clean_flash_flag = 1;
				}
				else if (high_temp_disinfect_flag)
				{
					high_temp_disinfect_flash_cnt = 0;
					high_temp_disinfect_flash_flag = 1;
				}
				else if ((forest_wind_flag) && (forest_wind_used))
				{
					forest_wind_flash_cnt = 0;
					forest_wind_flash_flag = 1;
				}
				else if ((option_fb) && (lock_flag))
				{
					lock_flash_cnt = 0;
					lock_flash_flag = 1;
				}

				else
				{



					if ((winddir_enable == 3) || (winddir_enable == 4))//3D ?????
					{



						if (remote_d1 == 0x01)
						{
							wind_horizontal_status = 1;
							wind_horizontal_status_write = 1;

						}
						else
						{

							switch (remote_d1)
							{
							case 0x00:wind_horizontal_angle = 1; break;
							case 0x02:wind_horizontal_angle = 2; break;
							case 0x04:wind_horizontal_angle = 3; break;
							case 0x06:wind_horizontal_angle = 4; break;
							case 0x08:wind_horizontal_angle = 5; break;
							case 0x0a:wind_horizontal_angle = 6; break;
							case 0x0c:wind_horizontal_angle = 7; break;
							}

							wind_horizontal_angle_write = 1;


							wind_horizontal_status = 0;
							wind_horizontal_status_write = 1;
						}



					}

					if ((winddir_enable == 3) || (winddir_enable == 4))
					{
						switch ((remote_d1 >> 4) & 0x0f)
						{
						case 0:wind_mode = 0; break;
						case 2:wind_mode = 1; break;
						case 3:wind_mode = 2; break;
						default:wind_mode = 0; break;
						}
						wind_mode_write = 1;
					}



					tx_time_1s_cnt = 0;
					tx_change_flag = 1;
				}
			}
			////////////////////////
			//???
			if ((remote_byte_buf[11] == 0x71) && (remote_byte_buf[12] == 0x8e) &&
				(remote_byte_buf[21] + remote_byte_buf[22] == 0xff))
			{
				if ((center_control_mode) || (por_06_flag))
				{
					center_control_flag = 1;
				}
				else if (sleep_used == 0)
				{

				}
				else if (self_clean_flag)
				{
					self_clean_flash_cnt = 0;
					self_clean_flash_flag = 1;
				}
				else if (high_temp_disinfect_flag)
				{
					high_temp_disinfect_flash_cnt = 0;
					high_temp_disinfect_flash_flag = 1;
				}
				else if ((system_status) && (try_run_flag == 0) && (system_mode != 0x01))
				{
					if ((remote_byte_buf[21] >> 4) & 0x0f)
					{
						//fuction_clear_deal();
						switch (remote_byte_buf[21] & 0x0f)
						{
						case 2:sleep_flag = 1; break;
						case 4:sleep_flag = 2; break;
						case 6:sleep_flag = 3; break;
						}

						pre_off_flag = 1;
						pre_off_flag_write = 1;
						if (option_preoff) pre_off_time = option_preoff;
						else pre_off_time = 16;
						pre_off_time_write = 1;
						pre_off_time_b = pre_off_time;
						pre_off_time_b_write = 1;

						if (wind_enable>3)system_wind = 0x18;
						else system_wind = 0x08;
						system_wind_write = 1;
						////////////
						if (sleep_flag)
						{
							if (mute_flag)
							{
								mute_flag = 0;
								mute_flag_write = 1;
							}
							if (save_flag)
							{
								save_flag = 0;
								save_flag_write = 1;
							}
							if (forest_wind_flag)
							{
								forest_wind_flag = 0;
								forest_wind_flag_write = 1;
							}
						}
						/////////////////



					}
					else
					{
						sleep_flag = 0;
						if ((try_run_flag == 0) && (try_run_set == 0) && (option_preoff == 0))
						{
							pre_off_flag = 0;
							pre_off_flag_write = 1;
						}

					}
					sleep_flag_write = 1;
					tx_time_1s_cnt = 0;
					tx_change_flag = 1;
				}
			}
			////////////////////////
			//????
			if ((remote_byte_buf[11] == 0x73) && (remote_byte_buf[12] == 0x8c) &&
				(remote_byte_buf[21] + remote_byte_buf[22] == 0xff))
			{
				if ((center_control_mode) || (por_06_flag))
				{
					center_control_flag = 1;
				}
				else if (save_used == 0)
				{

				}
				else if (self_clean_flag)
				{
					self_clean_flash_cnt = 0;
					self_clean_flash_flag = 1;
				}
				else if (high_temp_disinfect_flag)
				{
					high_temp_disinfect_flash_cnt = 0;
					high_temp_disinfect_flash_flag = 1;
				}
				else  if ((system_status) && (try_run_flag == 0) && (system_mode != 0x01))
				{
					//fuction_clear_deal();
					switch ((remote_byte_buf[21] >> 4) & 0x0f)
					{
					case 0:save_flag = 0; break;
					case 2:save_flag = 1; break;

					}
					/////////////////
					if (save_flag)
					{
						if (mute_flag)
						{
							mute_flag = 0;
							mute_flag_write = 1;
						}
						if (sleep_flag)
						{

							sleep_flag = 0;
							sleep_flag_write = 1;

							if ((try_run_flag == 0) && (try_run_set == 0) && (option_preoff == 0))
							{
								if (pre_off_flag)
								{
									pre_off_flag = 0;
									pre_off_flag_write = 1;
								}
							}

						}
						if (heat_flag)
						{
							heat_flag = 0;
							heat_flag_write = 1;
						}
						if (forest_wind_flag)
						{
							forest_wind_flag = 0;
							forest_wind_flag_write = 1;
						}

					}
					/////////////////
					save_flag_write = 1;
					tx_time_1s_cnt = 0;
					tx_change_flag = 1;
				}
			}
			////////////////////////
			//????
			if ((remote_byte_buf[11] == 0x75) && (remote_byte_buf[12] == 0x8a) &&
				(remote_byte_buf[21] + remote_byte_buf[22] == 0xff))
			{
				if ((center_control_mode) || (por_06_flag))
				{
					center_control_flag = 1;
				}
				else if (mute_used == 0)
				{

				}
				else if (self_clean_flag)
				{
					self_clean_flash_cnt = 0;
					self_clean_flash_flag = 1;
				}
				else if (high_temp_disinfect_flag)
				{
					high_temp_disinfect_flash_cnt = 0;
					high_temp_disinfect_flash_flag = 1;
				}
				else  if ((system_status) && (try_run_flag == 0) && (system_mode != 0x01))
				{
					//fuction_clear_deal();
					switch (remote_byte_buf[21] & 0x0f)
					{
					case 0:mute_flag = 0; break;
					case 2:mute_flag = 1; break;

					}
					mute_flag_write = 1;
					tx_time_1s_cnt = 0;
					tx_change_flag = 1;
					//////////////
					if (mute_flag)
					{
						if (save_flag)
						{
							save_flag = 0;
							save_flag_write = 1;
						}
						if (sleep_flag)
						{

							sleep_flag = 0;
							sleep_flag_write = 1;

							if ((try_run_flag == 0) && (try_run_set == 0) && (option_preoff == 0))
							{
								if (pre_off_flag)
								{
									pre_off_flag = 0;
									pre_off_flag_write = 1;
								}
							}

						}
						if (forest_wind_flag)
						{
							forest_wind_flag = 0;
							forest_wind_flag_write = 1;
						}
					}
					////////////

				}
			}
			////////////////////////
			//????
			if ((remote_byte_buf[11] == 0x76) && (remote_byte_buf[12] == 0x89) &&
				(remote_byte_buf[15] + remote_byte_buf[16] == 0xff))
			{
				if ((center_control_mode) || (por_06_flag))
				{
					center_control_flag = 1;
				}
				else if (heat_used == 0)
				{

				}
				else if (self_clean_flag)
				{
					self_clean_flash_cnt = 0;
					self_clean_flash_flag = 1;
				}
				else if (high_temp_disinfect_flag)
				{
					high_temp_disinfect_flash_cnt = 0;
					high_temp_disinfect_flash_flag = 1;
				}
				else  if ((system_status) && (try_run_flag == 0) && (system_mode != 0x01))
				{
					//fuction_clear_deal();
					switch (remote_byte_buf[15] & 0x0f)
					{
					case 0:heat_flag = 0; break;
					case 2:heat_flag = 1; break;

					}
					heat_flag_write = 1;
					tx_time_1s_cnt = 0;
					tx_change_flag = 1;
					if (heat_flag)
					{
						if (save_flag)
						{
							save_flag = 0;
							save_flag_write = 1;
						}
					}

				}
			}
			////////////////////////
			//????
			if ((remote_byte_buf[11] == 0x77) && (remote_byte_buf[12] == 0x88) &&
				(remote_byte_buf[15] + remote_byte_buf[16] == 0xff))
			{
				if ((center_control_mode) || (por_06_flag))
				{
					center_control_flag = 1;
				}
				else if (health_used == 0)
				{

				}
				else if (self_clean_flag)
				{
					self_clean_flash_cnt = 0;
					self_clean_flash_flag = 1;
				}
				else if (high_temp_disinfect_flag)
				{
					high_temp_disinfect_flash_cnt = 0;
					high_temp_disinfect_flash_flag = 1;
				}
				else  if ((system_status) && (try_run_flag == 0) && (system_mode != 0x01))
				{
					//fuction_clear_deal();
					switch (remote_byte_buf[15] & 0x0f)
					{
					case 0:health_flag = 0; break;
					case 2:health_flag = 1; break;

					}
					health_flag_write = 1;
					tx_time_1s_cnt = 0;
					tx_change_flag = 1;
					if (health_flag)
					{
						if (forest_wind_flag)
						{
							forest_wind_flag = 0;
							forest_wind_flag_write = 1;
						}
					}

				}
			}
			////////////////////////
			//??? 
			if ((remote_byte_buf[11] == 0x74) && (remote_byte_buf[12] == 0x8b) &&
				(remote_byte_buf[21] + remote_byte_buf[22] == 0xff))
			{
				if ((center_control_mode) || (por_06_flag))
				{
					center_control_flag = 1;
				}
				else if (human_used == 0)
				{

				}
				else if (self_clean_flag)
				{
					self_clean_flash_cnt = 0;
					self_clean_flash_flag = 1;
				}
				else if (high_temp_disinfect_flag)
				{
					high_temp_disinfect_flash_cnt = 0;
					high_temp_disinfect_flash_flag = 1;
				}
				else  if ((system_status) && (system_mode != 0x01))
				{
					i = 0;
					switch (remote_byte_buf[21] & 0x0f)
					{
					case 2:i = 0x01; break;
					case 4:i = 0x02; break;
					case 6:i = 0x03; break;
					}
					if (remote_byte_buf[21] & 0x20)i |= 0x04;

					human_flag = i;
					human_flag_write = 1;

					tx_time_1s_cnt = 0;
					tx_change_flag = 1;
				}
			}
			////////////////////////
			//3D ???  
			if ((remote_byte_buf[11] == 0x78) && (remote_byte_buf[12] == 0x87))
			{
				remote_d1 = remote_byte_buf[15]; remote_d2 = remote_byte_buf[16]; i = 1;
			}
			else if ((remote_byte_buf[11] == 0x74) && (remote_byte_buf[12] == 0x8b))
			{
				remote_d1 = remote_byte_buf[25]; remote_d2 = remote_byte_buf[26]; i = 1;
			}
			else
			{
				i = 0;
			}


			if ((i) && (remote_d1 + remote_d2 == 0xff))
			{


				if (winddir_enable == 0)
				{
					no_action_flag = 1;
				}
				else if ((center_control_windboard) || (por_06_flag))
				{
					center_control_flag = 1;
				}
				else if (self_clean_flag)
				{
					self_clean_flash_cnt = 0;
					self_clean_flash_flag = 1;
				}
				else if (high_temp_disinfect_flag)
				{
					high_temp_disinfect_flash_cnt = 0;
					high_temp_disinfect_flash_flag = 1;
				}
				else if ((forest_wind_flag) && (forest_wind_used))
				{
					forest_wind_flash_cnt = 0;
					forest_wind_flash_flag = 1;
				}
				else if ((option_fb) && (lock_flag))
				{
					lock_flash_cnt = 0;
					lock_flash_flag = 1;
				}

				else
				{

					switch (remote_d1)
					{
					case 0:wind_mode = 0; break;
					case 2:
						wind_mode = 1;
						break;
					case 4:if (a3d_wind_use)
						wind_mode = 2;
						break;
					}

					wind_mode_write = 1;

					tx_time_1s_cnt = 0;
					tx_change_flag = 1;
				}

			}

			///////////////////////
			//??¦Ë
			if ((remote_byte_buf[7] == 0xe1) && (remote_byte_buf[8] == 0x1e) &&
				(remote_byte_buf[11] == 0x0f) && (remote_byte_buf[12] == 0xf0))
			{
				/////////////////////
				if (err_now_flag)
				{
					a3d_wind_err = 0;
					err_now_flag = 0;
					err_reset_flag = 0xffff >> (16 - total_machine);

					line_err_reset_flag = 1;
					/*
					for(i=0;i<CONST_16;i++)
					{
					basedata[i][4]&=0xfe;
					}
					*/
				}
				else if (filter_flag)filter_reset = 1;
				tx_time_1s_cnt = 0;
				tx_change_flag = 1;
				////////////////////////
			}
			///////////////////////

			if ((center_control_flag) && ((center_onoff_flag & 0x01) == 0))
			{
				PPG_type = 3;
				PPG_time = 0;
			}
			if (tx_change_flag)center_control_flag = 0;
			///////////////////////
		}
	
}

