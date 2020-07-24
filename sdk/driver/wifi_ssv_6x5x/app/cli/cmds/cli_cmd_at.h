/*
*  (C) Copyright 2014-2016 Shenzhen South Silicon Valley microelectronics co.,limited
*
*  All Rights Reserved
*/


#ifndef _CLI_CMD_AT_H_
#define _CLI_CMD_AT_H_

#define MAX_CMD_LEN (32)
#define MAX_ARGUMENT 24

typedef struct _stParam
{
	char*	argv[MAX_ARGUMENT];
	int	argc;
} stParam;

typedef int (*PF_handle) (stParam *param);
typedef struct _at_cmd_info
{
	char*		atCmd;
	PF_handle	pfHandle;
    ssv_type_u8 maxargu;
} at_cmd_info;
#define	ATRSP_OK                    "+OK\r\n"
#define	ATRSP_ERROR                 "+ERROR:%d\r\n"
#define	ATRSP_ERROR_STR           	"+ERROR:%s\r\n"

//RF Test AT Command
#define ATCMD_RADIO_CHANNEL			"AT+RF_CHANNEL"
#define ATCMD_RADIO_RF_START			"AT+RF_START"
#define ATCMD_RADIO_RF_RATE			"AT+RF_RATE"
#define ATCMD_RADIO_RF_STOP			"AT+RF_STOP"
#define ATCMD_RADIO_RF_RESET			"AT+RF_RESET"
#define ATCMD_RADIO_RF_COUNT			"AT+RF_COUNT"
#define ATCMD_RADIO_RF_READ_TEMPCS	"AT+RF_READ_TEMPCS"
//#define ATCMD_RADIO_RF_ENABLE_TCSR	"AT+RADIO_RF_ENABLE_TCSR"
#define ATCMD_RADIO_RF_BANDGAIN		"AT+RF_2GBAND_GAIN"
#define ATCMD_RADIO_RF_GAIN			"AT+RF_GAIN"
#define ATCMD_RADIO_RF_RATEGAIN		"AT+RF_RATE_GAIN"
#define ATCMD_RADIO_RF_DUMP			"AT+RF_DUMP"
#define ATCMD_RF_TABLE_RT           "AT+RF_TABLE_RT"
#define ATCMD_RF_TABLE_HT           "AT+RF_TABLE_HT"
#define ATCMD_RF_TABLE_LT           "AT+RF_TABLE_LT"
#define ATCMD_RF_5GTABLE_RT         "AT+RF_5GTABLE_RT"
#define ATCMD_RF_5GTABLE_HT         "AT+RF_5GTABLE_HT"
#define ATCMD_RF_5GTABLE_LT         "AT+RF_5GTABLE_LT"
#define ATCMD_RF_5GTABLE_LT         "AT+RF_5GTABLE_LT"
#define ATCMD_RADIO_RF_READ_TEMPCS  "AT+RF_READ_TEMPCS"
#define ATCMD_RADIO_RF              "AT+RF"


int At_ShowRfCommand (stParam *param);
int At_RadioRFStart(stParam *param);
int At_RadioChannel(stParam *param);
int At_RadioRFRate(stParam *param);
int At_RadioRFStop(stParam *param);
int At_RadioRFReset(stParam *param);
int At_RadioRFCount(stParam *param);
int At_RadioRFDump(stParam *param);
int At_RfTableRT(stParam *param);
int At_RfTableLT(stParam *param);
int At_RfTableHT(stParam * param);
int At_Rf5GTableRT(stParam *param);
int At_Rf5GTableLT(stParam *param);
int At_Rf5GTableHT(stParam * param);
int At_RadioRFReadTempcs(stParam * param);

#endif /* _CLI_CMD_AT_H_ */

