/*
*  (C) Copyright 2014-2016 Shenzhen South Silicon Valley microelectronics co.,limited
*
*  All Rights Reserved
*/


#ifndef _CLI_H_
#define _CLI_H_


typedef void (*CliCmdFunc) ( ssv_type_s32, char ** );


typedef struct CLICmds_st
{
        const char           *Cmd;
        CliCmdFunc          CmdHandler;
        const char           *CmdUsage;
        
} CLICmds, *PCLICmds;

extern struct task_info_st g_cli_task_info[];
#define CLI_MBX     g_cli_task_info[0].qevt


ssv_type_s32  Cli_RunCmd(char *CmdBuffer);
void Cli_Init(ssv_type_s32 argc, char *argv[]);
void Cli_Start(void);
void Cli_Task( void *args );

#endif /* _CLI_H_ */

