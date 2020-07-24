/*
*  (C) Copyright 2014-2016 Shenzhen South Silicon Valley microelectronics co.,limited
*
*  All Rights Reserved
*/


#include <log.h>
#include <rtos.h>
#include <porting.h>
#include "cli.h"
#include "ssv_timer.h"

#if (CLI_ENABLE==1)

/* Command Line: */
static char	*sgArgV[CLI_ARG_SIZE];
static ssv_type_u32 	sgArgC;
static ssv_type_u32 	sgCurPos = 0;
static ssv_type_bool	s_interactive = true;

/* cli: */
char gCmdBuffer[CLI_BUFFER_SIZE+1];



#if (CLI_HISTORY_ENABLE==1)

char gCmdHistoryBuffer[CLI_HISTORY_NUM][CLI_BUFFER_SIZE+1];
signed char gCmdHistoryIdx;
signed char gCmdHistoryCnt;

#endif//CLI_HISTORY_ENABLE


/* Command Table: */
extern CLICmds gCliCmdTable[];
extern ssv_type_u8 batch_mode;

//----------------------------------------

#if (CLI_HISTORY_ENABLE==1)

static void Cli_EraseCmdInScreen()
{
	ssv_type_u32 i;
	for(i= 0;i < (ssv6xxx_strlen(gCmdBuffer)+ssv6xxx_strlen(CLI_PROMPT));i++)
	{
		hal_putchar(0x08);
		hal_putchar(0x20);
		hal_putchar(0x08);
	}
	FFLUSH(stdout);
}

static void Cli_PrintCmdInScreen()
{
	ssv_type_u32 i;
	LOG_PRINTF("%s", CLI_PROMPT);
	FFLUSH(stdout);
	for(i= 0;i<ssv6xxx_strlen(gCmdBuffer);i++)
	{
		 hal_putchar(gCmdBuffer[i]);
	}
    FFLUSH(stdout);
}



static ssv_type_u8 Cli_GetPrevHistoryIdx()
{
	return (gCmdHistoryIdx<=0) ? 0 : (gCmdHistoryIdx-1);
}


static ssv_type_u8 Cli_GetNextHistoryIdx()
{
	signed char CmdIdx = gCmdHistoryIdx;

	CmdIdx++;

	if(CmdIdx >= CLI_HISTORY_NUM || CmdIdx > gCmdHistoryCnt)
		CmdIdx--;


	return CmdIdx;
}



static ssv_inline void Cli_StoreCmdBufToHistory(ssv_type_u8 history)
{
	ssv_type_u32 len = ssv6xxx_strlen((const char*)gCmdBuffer);
	ssv6xxx_memcpy((ssv_type_u8*)&gCmdHistoryBuffer[history][0], (ssv_type_u8*)gCmdBuffer, len);
	gCmdHistoryBuffer[history][len]=0x00;
}


static ssv_inline void Cli_RestoreHistoryToCmdBuf(ssv_type_u8 history)
{
	ssv_type_u32 len = ssv6xxx_strlen((const char*)&gCmdHistoryBuffer[history]);
	ssv6xxx_memcpy((ssv_type_u8*)gCmdBuffer, (ssv_type_u8*)&gCmdHistoryBuffer[history][0], len);
	gCmdBuffer[len]= 0x00;
	sgCurPos = len;
}




static void Cli_MovetoPrevHistoryCmdBuf()
{
	ssv_type_u8 CmdIdx = gCmdHistoryIdx;
	ssv_type_u8 NewCmdIdx = Cli_GetPrevHistoryIdx();

	if(CmdIdx == NewCmdIdx)
		return;

	Cli_EraseCmdInScreen();
	Cli_StoreCmdBufToHistory(CmdIdx);
	Cli_RestoreHistoryToCmdBuf(NewCmdIdx);
	Cli_PrintCmdInScreen();
	gCmdHistoryIdx = NewCmdIdx;
}






static void Cli_MovetoNextHistoryCmdBuf()
{
	ssv_type_u8 CmdIdx = gCmdHistoryIdx;
		ssv_type_u8 NewCmdIdx = Cli_GetNextHistoryIdx();

		if(CmdIdx == NewCmdIdx)
			return;

		Cli_EraseCmdInScreen();
		Cli_StoreCmdBufToHistory(CmdIdx);
		Cli_RestoreHistoryToCmdBuf(NewCmdIdx);
		Cli_PrintCmdInScreen();
		gCmdHistoryIdx = NewCmdIdx;
}



static void Cli_RecordInHistoryCmdBuf()
{
	ssv_type_u32 i = CLI_HISTORY_NUM-2;
	ssv_type_u32 len;

	if(sgCurPos)
	{
		//shift history log
		for(i =  CLI_HISTORY_NUM-2; i>0 ; i--)
		{
			len = ssv6xxx_strlen((const char*)&gCmdHistoryBuffer[i]);
			OS_MemCPY((ssv_type_u8*)&gCmdHistoryBuffer[i+1][0], (ssv_type_u8*)&gCmdHistoryBuffer[i][0], len);
			gCmdHistoryBuffer[i+1][len] = 0x00;
		}


		//copy bud to index 1
		len = ssv6xxx_strlen((const char*)&gCmdBuffer);
		OS_MemCPY((ssv_type_u8*)&gCmdHistoryBuffer[1][0], (ssv_type_u8*)&gCmdBuffer, len);
		gCmdHistoryBuffer[1][len] = 0x00;


		//Record how much history we record
		gCmdHistoryCnt++;
		if(gCmdHistoryCnt>=CLI_HISTORY_NUM)
			gCmdHistoryCnt = CLI_HISTORY_NUM-1;
	}

	//Reset buf
	gCmdHistoryBuffer[0][0]=0x00;
	gCmdHistoryIdx = 0;


}








#endif

//----------------------------------------

static void _CliCmdUsage( void )
{
    CLICmds *CmdPtr;

    LOG_PRINTF("\r\nUsage:\r\n");
    for( CmdPtr=gCliCmdTable; CmdPtr->Cmd; CmdPtr ++ )
    {
        LOG_PRINTF("%-20s\t\t%s\r\n", CmdPtr->Cmd, CmdPtr->CmdUsage);
    }
    LOG_PRINTF("\r\n%s", CLI_PROMPT);
    FFLUSH(stdout);
}



ssv_type_s32 Cli_RunCmd(char *CmdBuffer)
{
    CLICmds *CmdPtr;
    ssv_type_u8 ch;
    char *pch;

    for( sgArgC=0,ch=0, pch=CmdBuffer; (*pch!=0x00)&&(sgArgC<CLI_ARG_SIZE); pch++ )
    {
        if ( (ch==0) && (*pch!=' ') && (*pch!=0x0a) && (*pch!=0x0d) )
        {
            ch = 1;
            sgArgV[sgArgC] = pch;
        }

        if ( (ch==1) && ((*pch==' ')||(*pch=='\t')||(*pch==0x0a)||(*pch==0x0d)) )
        {
            *pch = 0x00;
            ch = 0;
            sgArgC ++;
        }
    }

    if(sgArgC==CLI_ARG_SIZE)
    {
        LOG_PRINTF("Total nummber of arg are over %d \r\n",CLI_ARG_SIZE);
        return 0;
    }
    
    if ( ch == 1)
    {
        sgArgC ++;
    }
    else if ( sgArgC > 0 )
    {
//        *(pch-1) = ' ';
    }

    if (sgArgC == 0)
    {
        return 0;
    }
    
    /* Dispatch command */
    for( CmdPtr=gCliCmdTable; CmdPtr->Cmd; CmdPtr ++ )
    {
        if ( !ssv6xxx_strcmp(sgArgV[0], CmdPtr->Cmd) )
        {
            CmdPtr->CmdHandler( sgArgC, sgArgV );
            break;
        }
    }
    if (NULL == CmdPtr->Cmd)
        return -1;
    return 0;
}



struct task_info_st g_cli_task_info[] = {
    { "cli_task_",   (OsMsgQ)0, 2, OS_CLI_PRIO, CLI_TASK_STACK_SIZE, (void *)1, Cli_Task},
//    { "net_task",       (OsMsgQ)1, OS_TASK_PRIO2, 64, NULL,      net_app_task },
};

void Cli_Init(ssv_type_s32 argc, char *argv[])
{
	ssv_type_u32		i;

    OsMsgQ *msgq = NULL;
    ssv_type_s32 qsize = 0;

    ssv6xxx_memset((void *)sgArgV, 0, sizeof(ssv_type_u8 *)*5);
    gCmdBuffer[0] 	= 0x00;
    sgCurPos 		= 0;
    sgArgC 			= 0;
	s_interactive 	= true;

#if (CLI_HISTORY_ENABLE==1)
    {
        int i;
        gCmdHistoryIdx = 0;
        gCmdHistoryCnt = 0;

        for(i=0;i<CLI_HISTORY_NUM; i++)
        gCmdHistoryBuffer[i][0]=0x00;       
    }
        
#endif
    
    ssv6xxx_memset((void *)gCmdBuffer, 0, CLI_BUFFER_SIZE+1);

	/**
	* If any argument is specified, then we do not enter interaction
	* mode. Instead, batch mode is applied to run batch command.
	*/
#if 0
	if (argc > 1)
	{
		for(i=1, gCmdBuffer[0]=0x00; i<argc; i++)
		{
			strcat(gCmdBuffer, argv[i]);
			if (i < (argc-1))
				strcat(gCmdBuffer, " ");
		}
		s_interactive = false;
	}
#endif

    msgq = &g_cli_task_info[0].qevt;
    qsize = (ssv_type_s32)g_cli_task_info[0].qlength;
    if (OS_MsgQCreate(msgq, qsize) != OS_SUCCESS)
    {
        LOG_PRINTF("OS_MsgQCreate faild\r\n");
        return;
    }

    for(i=0; i<sizeof(g_cli_task_info)/sizeof(struct task_info_st); i++) {
        OS_TaskCreate(g_cli_task_info[i].task_func,
        g_cli_task_info[i].task_name,
        g_cli_task_info[i].stack_size<<4,
        NULL,
        g_cli_task_info[i].prio,
        NULL);
    }

}


extern int At_Parser (char *buff, int len);

void Cli_Start(void)
{
//    CLICmds *CmdPtr;
    ssv_type_u8 ch;
//    char *pch;

    if (s_interactive == false)
    {
	   if (Cli_RunCmd(gCmdBuffer) < 0)
	   {
			LOG_PRINTF("\nCommand not found!!\r\n");
	   }
       else
	   {
			LOG_PRINTF("\r\n");
	   }
       FFLUSH(stdout);
	   gCmdBuffer[0] = 0x00;
    }

#ifdef __SSV_UNIX_SIM__
    if ( !kbhit() )
    {
        return;
    }
#endif
    switch ( (ch=hal_getchar()) )
    {
        case 0x00: /* Special Key, read again for real data */
            ch = hal_getchar();
            break;

#ifndef __SSV_UNIX_SIM__
		//Windows mode
		case 0x08: /* Backspace */
#else
		//Linux mode
		case 0x7f:
        case 0x08:
		ch = hal_getchar();//Get extra data
#endif


            if ( 0 < sgCurPos )
            {
                hal_putchar(0x08);
                hal_putchar(0x20);
                hal_putchar(0x08);
                FFLUSH(stdout);
                sgCurPos --;
                gCmdBuffer[sgCurPos] = 0x00;
            }
            break;

// Changed __linux__ to __SSV_UNIX_SIM__ for CYGWIN compatibility
#ifdef __SSV_UNIX_SIM__
        case 0x0a: /* Enter */
#else
        case 0x0d: /* Enter */
#endif
        //invoke_command:

#if (CLI_HISTORY_ENABLE==1)

			Cli_RecordInHistoryCmdBuf();

#endif

            if (sgCurPos > 0) {
                LOG_PRINTF("\r\n");
                if (Cli_RunCmd(gCmdBuffer) < 0)
				{				    
                    if(At_Parser (gCmdBuffer, ssv6xxx_strlen(gCmdBuffer))!= 0)
                        LOG_PRINTF("\nCommand not found!!\r\n");
				}
                else
				{
					LOG_PRINTF("\r\n");
				}
                FFLUSH(stdout);
            }
#if 0

            for( sgArgC=0,ch=0, pch=gCmdBuffer; (*pch!=0x00)&&(sgArgC<CLI_ARG_SIZE); pch++ )
            {
                if ( (ch==0) && (*pch!=' ') )
                {
                    ch = 1;
                    sgArgV[sgArgC] = pch;
                }

                if ( (ch==1) && (*pch==' ') )
                {
                    *pch = 0x00;
                    ch = 0;
                    sgArgC ++;
                }
            }
            if ( ch == 1)
            {
                sgArgC ++;
            }
            else if ( sgArgC > 0 )
            {
                *(pch-1) = ' ';
            }

            if ( sgArgC > 0 )
            {
                /* Dispatch command */
                for( CmdPtr=gCliCmdTable; CmdPtr->Cmd; CmdPtr ++ )
                {
                    if ( !ssv6xxx_strcmp(sgArgV[0], CmdPtr->Cmd) )
                    {
                        log_printf("\n");
                        FFLUSH(stdout);
                        CmdPtr->CmdHandler( sgArgC, sgArgV );
                        break;
                    }
                }
                if ( (NULL==CmdPtr->Cmd) && (0!=sgCurPos) )
                {
                    log_printf("\nCommand not found!!\n");
                    FFLUSH(stdout);
                }
            }
#endif
            LOG_PRINTF("\r\n%s", CLI_PROMPT);
            FFLUSH(stdout);

            gCmdBuffer[0] = 0x00;
            sgCurPos = 0;


            break;


#ifdef _WIN32_
	//Windows platform
		case 0xe0://special key
			{
				ch=hal_getchar();

#if (CLI_HISTORY_ENABLE==1)
				if(0x48 == ch)//up arrow key
				{
					Cli_MovetoNextHistoryCmdBuf();
				}
				else if(0x50 == ch)//down arrow key
				{

					Cli_MovetoPrevHistoryCmdBuf();
				}
				else
				{

				}

#endif//#if (CLI_HISTORY_ENABLE==1)

			}
			break;



#else//__SSV_UNIX_SIM__
		//Linux platform

		case 0x1b:
		{




#if (CLI_HISTORY_ENABLE==1)
			ch=hal_getchar();
			if(0x5b==ch)
			{
				ch=hal_getchar();

				if(0x41==ch){//Up arrow key

					Cli_MovetoNextHistoryCmdBuf();
				}
				else if(0x42==ch){//down arrow key

					Cli_MovetoPrevHistoryCmdBuf();
				}
				else if(0x43==ch){//right arrow key




				}
				else if(0x44==ch){//left arrow key



				}
				else
				{;}


			}
#endif//#if (CLI_HISTORY_ENABLE==1)


		}
			break;

#endif//__SSV_UNIX_SIM__

        case '~': /* for help */
            hal_putchar(ch);
            FFLUSH(stdout);
            _CliCmdUsage();
            break;

        default: /* other characters */


			if(ch>=0x20&&ch<=0x7e)
			{
				if ( (CLI_BUFFER_SIZE-1) > sgCurPos )
	            {
	                gCmdBuffer[sgCurPos++] = ch;
	                gCmdBuffer[sgCurPos] = 0x00;
	                hal_putchar(ch);
	                FFLUSH(stdout);
	            }
			}
            break;
    }
}



extern void _cmd_wifi_ps(void);
extern ssv_type_u32 g_cli_ps_lp;
extern ssv_type_bool g_cli_joining;
extern void cli_ps_t_handler(void* data1, void* data2);

/**
 *  CLI (Command Line Interface) Task:
 *
 *  Create CLI Task for console mode debugging.
 */
void Cli_Task( void *args )
{
    ssv_type_s32 res = 0;
    MsgEvent *msg_evt = NULL;
    
    //LOG_PRINTF("\r\n<<Start Command Line>>\r\n\r\nPress \'?\'  for help......\r\n");
    LOG_PRINTF("\r\n%s", CLI_PROMPT);
    FFLUSH(stdout);

    if (gCmdBuffer[0] != 0x00) {
        Cli_Start();
        return;//exit(0);
    }

    while(1)
    {
        res = msg_evt_fetch_timeout(CLI_MBX, &msg_evt, 30);
        if (res != OS_SUCCESS)
        {
            Cli_Start();
            //OS_MsDelay(50);
    		//taskYIELD();
        }
        else
        {
            switch(msg_evt->MsgType)
            {
                case MEVT_HOST_TIMER:
                    os_timer_expired((void *)msg_evt);
                    break;
                                
                case MEVT_HOST_CMD:
                    switch (msg_evt->MsgData)
                    {
                        case SOC_EVT_PS_WAKENED:
                        {
                            LOG_PRINTF("cli get ps wakened.join=%d\r\n",g_cli_joining);
                            os_cancel_timer(cli_ps_t_handler,(ssv_type_u32)1,(ssv_type_u32)NULL);
                            os_cancel_timer(cli_ps_t_handler,(ssv_type_u32)2,(ssv_type_u32)NULL);
                            if(g_cli_joining == true)
                            {
                                if(g_cli_ps_lp > 0)
                                    os_create_timer(3000,cli_ps_t_handler,(void*)1,NULL, (void*)CLI_MBX); // sleep after 3 sec
                                if(g_cli_ps_lp == 2)
                                    os_create_timer(8000,cli_ps_t_handler,(void*)2,NULL, (void*)CLI_MBX); // wakeup after 8 sec
                            }
                        }
                    }
                    break;
                default:
                    LOG_PRINTF("cli get unknow evt\r\n");
            }
            msg_evt_free(msg_evt);
        }
    }
}
#endif

