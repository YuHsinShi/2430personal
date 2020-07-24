/*
*  (C) Copyright 2014-2016 Shenzhen South Silicon Valley microelectronics co.,limited
*
*  All Rights Reserved
*/


#ifndef _CLI_CMD_SOC_H_
#define _CLI_CMD_SOC_H_

struct soc_cmd_table_st
{
    ssv_type_u8              cmd_id;
    ssv_type_u8              cmd_len;
    ssv_type_u8              *cmd_buf;
};


void cmd_soc(ssv_type_s32 argc, char *argv[]);
void cmd_read(ssv_type_s32 argc, char *argv[]);
void cmd_write(ssv_type_s32 argc, char *argv[]);
void cmd_write_52(ssv_type_s32 argc, char *argv[]);
void cmd_read_52(ssv_type_s32 argc, char *argv[]);

#endif /* _CLI_CMD_SOC_H_ */

