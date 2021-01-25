/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * CLI system related functions.
 *
 * @author Jim Tan
 * @version 1.0
 */
#include "cli_cfg.h"
#include "openrtos/FreeRTOS.h"
#include "openrtos/task.h"
#include "openrtos/FreeRTOS_CLI.h"
#include <stdlib.h>

#define KEYWORD_READ_REGISTER   "read-register"
#define KEYWORD_WRITE_REGISTER   "write-register"


#define KEYWORD_RAM_SCRIPT	    "ram-script"
#define KEYWORD_BIN_LOAD	    "bin-load"
#define KEYWORD_BIN_BOOT	    "bin-boot"

#define KEYWORD_LOGMODE_ON_OFF	    "log-mode"

#define KEYWORD_TARGET_TEST	    	"target-status"
#define KEYWORD_TARGET_GETMODE	    "target-getmode"
#define KEYWORD_TARGET_GETID	    "target-chipid"
#define KEYWORD_TARGET_BURNROM	    "target-burnrom"

#define KEYWORD_TARGET_REBOOT	    "target-reboot"


#define KEYWORD_GPIO_SET	    "gpio-set"
#define KEYWORD_GPIO_CLEAR	    "gpio-clear"
#define KEYWORD_GPIO_GET	    "gpio-get"


static portBASE_TYPE prvTaskRebootTargetCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{

/*
ithPrintf("Reboot...\n");
ithWatchDogEnable();  ==  //ithSetRegBitA(ITH_WD_BASE + ITH_WD_CR_REG, ITH_WD_EN);
ithWatchDogSetReload(0);  //ithWriteRegA(ITH_WD_BASE + ITH_WD_LOAD_REG, 0);
ithWatchDogRestart(); //  //ithWriteRegA(ITH_WD_BASE + ITH_WD_RESTART_REG, ITH_WD_AUTORELOAD);

*/
ithPrintf("prvTaskRebootTargetCommand...\n");
ithSetRegBitA_target(ITH_WD_BASE + ITH_WD_CR_REG, ITH_WD_EN);
ithWriteRegA_target(ITH_WD_BASE + ITH_WD_LOAD_REG, 0);
ithWriteRegA_target(ITH_WD_BASE + ITH_WD_RESTART_REG, ITH_WD_AUTORELOAD);	
snprintf( ( char * ) pcWriteBuffer, xWriteBufferLen, "OK\r\n");


return pdFALSE;
}





static portBASE_TYPE prvReadRegisterCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{

	printf("prvReadRegisterCommand,%s\n",pcCommandString);
//xWriteBufferLen=1024
unsigned int address;
unsigned int reg_value;

address=atoi(&pcCommandString[strlen(KEYWORD_READ_REGISTER)+1+2]);
if( (0==address) || ('0'!=pcCommandString[strlen(KEYWORD_READ_REGISTER)+1]) 
	|| ('x'!=pcCommandString[strlen(KEYWORD_READ_REGISTER)+2]) ) {
	snprintf(pcWriteBuffer,64, "Please use hex format, ex: 0x12345678 \n");
    return pdFALSE;
}
else{	
//printf("%s\n",&pcCommandString[strlen(KEYWORD_READ_REGISTER)+1]);

	reg_value=read_slave_register_value(address);
	snprintf(pcWriteBuffer,8, "%d",reg_value);
	printf("address=0x%x,%x,%s\n",address,reg_value,pcWriteBuffer);

    return pdFALSE;
}
}

static portBASE_TYPE prvWriteRegisterCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{

    return pdTRUE;
}





static portBASE_TYPE prvGpioSetCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
	char *pcParameter1;
	portBASE_TYPE xParameter1StringLength;
	int gpio_num;
	
	pcParameter1 = ( char * ) FreeRTOS_CLIGetParameter( pcCommandString, 1, &xParameter1StringLength );
	pcParameter1[ xParameter1StringLength ] = 0x00;

    if (strncmp(pcParameter1, "0x", 2) == 0 || strncmp(pcParameter1, "0X", 2) == 0)
    {
      gpio_num = strtol(&pcParameter1[2], NULL, 16);
    }
    else
    {
      gpio_num = strtol(pcParameter1, NULL, 10);
    }

	ithPrintf("TAGET GPIO %d output High \n",gpio_num);

	target_io_write(gpio_num,1);
    snprintf( ( char * ) pcWriteBuffer, xWriteBufferLen, "OK\r\n");

    return pdFALSE;
}

static portBASE_TYPE prvGpioClearCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
	char *pcParameter1;
	portBASE_TYPE xParameter1StringLength;
	int gpio_num;
	
	pcParameter1 = ( char * ) FreeRTOS_CLIGetParameter( pcCommandString, 1, &xParameter1StringLength );
	pcParameter1[ xParameter1StringLength ] = 0x00;

    if (strncmp(pcParameter1, "0x", 2) == 0 || strncmp(pcParameter1, "0X", 2) == 0)
    {
      gpio_num = strtol(&pcParameter1[2], NULL, 16);
    }
    else
    {
      gpio_num = strtol(pcParameter1, NULL, 10);
    }

ithPrintf("TAGET GPIO %d output LOW \n",gpio_num);

	target_io_write(gpio_num,0);
    snprintf( ( char * ) pcWriteBuffer, xWriteBufferLen, "OK\r\n");

    return pdFALSE;
}

static portBASE_TYPE prvGpioGetCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
	char *pcParameter1;
	portBASE_TYPE xParameter1StringLength;
	int gpio_num;
	
	pcParameter1 = ( char * ) FreeRTOS_CLIGetParameter( pcCommandString, 1, &xParameter1StringLength );
	pcParameter1[ xParameter1StringLength ] = 0x00;

    if (strncmp(pcParameter1, "0x", 2) == 0 || strncmp(pcParameter1, "0X", 2) == 0)
    {
      gpio_num = strtol(&pcParameter1[2], NULL, 16);
    }
    else
    {
      gpio_num = strtol(pcParameter1, NULL, 10);
    }


if(1 ==	target_io_read(gpio_num) )
{
    snprintf( ( char * ) pcWriteBuffer, xWriteBufferLen, "GPIO %d is HIGH\r\n",gpio_num );

}
else
{
    snprintf( ( char * ) pcWriteBuffer, xWriteBufferLen, "GPIO %d is LOW\r\n",gpio_num);

}


    return pdFALSE;
}


static portBASE_TYPE prvTargetStatusGet( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
char tmp_str[128]={0}; 
int ret;
ret =target_auto_detect();
if(ret>0)
{
	ret=get_ite_chip_id();
	if(1==ret)
	 	snprintf( tmp_str, 128, "TARGET 9860 found ");
	else if(1==ret) 
		snprintf( tmp_str, 128, "TARGET 970 found ");
	else		
		snprintf( tmp_str, 128, "TARGET unkown? " );
	
	ret=get_ite_bootcfg();

	if(0b11==ret)//11
	{
		snprintf( tmp_str, 128, "%s in COOPERATIVE MODE",tmp_str);
	}
	else if(0b10==ret)//10
	{
		snprintf( tmp_str, 128, "%s in NAND BOOT MODE",tmp_str);
	}
	else if(0b01==ret)//
	{
	
	 snprintf( tmp_str, 128, "%s in NOR BOOT MODE",tmp_str);
	}
	else if (0b00==ret)
	{
	
		snprintf( tmp_str, 128, "%s in SD BOOT MODE",tmp_str);
	}
	else
	{
	
		snprintf( tmp_str, 128, "%s in ?? MODE",tmp_str);
	}



	snprintf( ( char * ) pcWriteBuffer, xWriteBufferLen, "%s\r\n", tmp_str );


	
}
else
{
	snprintf( ( char * ) pcWriteBuffer, xWriteBufferLen, "TARGET not Found! \r\n" );

}



    return pdFALSE;
}


static char* downloadBuffer;
static int downloadSize;
//ret = true if still have some data
portBASE_TYPE  cli_download_process( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
	static portBASE_TYPE xReady = pdFALSE;
	char *pcParameter1;
	portBASE_TYPE xParameter1StringLength;
	int result;
	
	  if( xReady == pdFALSE )
	  {
		pcParameter1 = ( char * ) FreeRTOS_CLIGetParameter( pcCommandString, 1, &xParameter1StringLength );
		
		pcParameter1[ xParameter1StringLength ] = 0x00;
		
		if (xParameter1StringLength == 0)
		{
		  snprintf( ( char * ) pcWriteBuffer, xWriteBufferLen, "%s\r\n", "ERROR: Invalid parameters" );
		  goto error;
		}
		
		if (strncmp(pcParameter1, "0x", 2) == 0 || strncmp(pcParameter1, "0X", 2) == 0)
		{
		  downloadSize = strtol(&pcParameter1[2], NULL, 16);
		}
		else
		{
		  downloadSize = strtol(pcParameter1, NULL, 10);
		}
		LOG_DBG "%s: size=%d\n", pcParameter1, downloadSize LOG_END	  
		
		downloadBuffer = malloc(downloadSize);
		if (downloadBuffer == NULL)
		{
		  snprintf( ( char * ) pcWriteBuffer, xWriteBufferLen, "%s\r\n", "ERROR: Out of memory" );
		  goto error;		 
		}
	
		snprintf( ( char * ) pcWriteBuffer, xWriteBufferLen, "%s\r\n", "OK" );
		xReady = pdTRUE;
		return pdTRUE;
	  }
	  else
	  {
		uint32_t readsize, remainsize, bufpos;
	
		remainsize = downloadSize;
		bufpos = 0;
	
		do
		{
			if (ioctl(ITP_DEVICE_USBDACM, ITP_IOCTL_IS_CONNECTED, NULL) == 0)
			{
			  snprintf( ( char * ) pcWriteBuffer, xWriteBufferLen, "%s\r\n", "ERROR" );
			  goto error;
			}
			  
			readsize = read(ITP_DEVICE_USBDACM, &downloadBuffer[bufpos], remainsize);
	
			LOG_DBG "readsize=%u remainsize=%u\n", readsize, remainsize LOG_END
	
			remainsize -= readsize;
			bufpos += readsize;
	
			putchar('.');
			fflush(stdout);
			
		} while (remainsize > 0);
		putchar('\n');
	
		snprintf( ( char * ) pcWriteBuffer, xWriteBufferLen, "%s\r\n", "OK" );
		
		xReady = pdFALSE;
		return pdFALSE;  
	  }
	  
	error:
	  LOG_ERR "%s\n", pcWriteBuffer LOG_END
			
	  if (downloadBuffer)
	  {
		free(downloadBuffer);
		downloadBuffer = NULL;
	  }
	  downloadSize = 0;
		
	  xReady = pdFALSE;
	  return pdFALSE;  

}


#define script_path "A:/ram.txt"
#define bin_path "A:/boot.bin"

static portBASE_TYPE  prvRamScriptDownload( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
	portBASE_TYPE ret;

	ret=cli_download_process(pcWriteBuffer,xWriteBufferLen, pcCommandString );
	if(pdFALSE==ret)
	{
	
		printf("\n prvRamScriptDownload path %s,%d\n",script_path,downloadSize);
		//write file to SD card
		FILE* fp;
		fp=fopen(script_path,"wb");
		if(NULL == fp)
		{
		printf("\n open fail path\n");

		}
		
		if(NULL != fp && downloadSize!=0)
		{
			fwrite(downloadBuffer,1,downloadSize,fp);
			fclose(fp);
			if (downloadBuffer)
			{
			
			  printf("\nfree ubffer\n");
			  free(downloadBuffer);
			  downloadBuffer = NULL;
			}
			downloadSize = 0;

		}

	}

	return ret;
}

static portBASE_TYPE  prvBinDownload( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
	portBASE_TYPE ret;

	ret=cli_download_process(pcWriteBuffer,xWriteBufferLen, pcCommandString );
	if(pdFALSE==ret)
	{
	 printf("\n prvBinDownload path=%s\n",bin_path);
		//write file to SD card
		FILE* fp;
		fp=fopen(bin_path,"wb");
		if(NULL != fp){

		printf("\n open file fail \n");

		}	
		 

		if(downloadSize!=0)
		{
		
			fwrite(downloadBuffer,1,downloadSize,fp);
			fclose(fp);

		}

		if (downloadBuffer)
		{
		  free(downloadBuffer);
		  downloadBuffer = NULL;
		}
		downloadSize = 0;

	}

	return ret;
}

static portBASE_TYPE  prvBinBoot( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
//	portBASE_TYPE ret;

//("E:/IT9860_360Mhz_DDR2_360Mhz.txt");
//("E:/burner_portable.bin");


target_do_booting(script_path,bin_path);
//change log to 
//snprintf( ( char * ) pcWriteBuffer, xWriteBufferLen, "%s\r\n", "" );

	return pdFALSE;
}



static portBASE_TYPE  prvLogModeOnOff( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
	portBASE_TYPE ret;
	char *pcParameter1;
	portBASE_TYPE xParameter1StringLength;
	
	pcParameter1 = ( char * ) FreeRTOS_CLIGetParameter( pcCommandString, 1, &xParameter1StringLength );
	
	if(pcParameter1[0] == 'y')
	{
		log_mode_set_on_off(1);
	}
	else
	{
		log_mode_set_on_off(0);

	}
	snprintf( ( char * ) pcWriteBuffer, xWriteBufferLen, "%s\r\n", "OK" );

	return pdFALSE;
}


static portBASE_TYPE  prvBurnRom( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
	portBASE_TYPE ret;
	
	snprintf( ( char * ) pcWriteBuffer, xWriteBufferLen, "%s\r\n", "OK" );
	burn_process_start();

	return pdFALSE;
}

//=====================================================================================================================

static const xCommandLineInput xTaskRebootTargetCommand =
{
	( const char * const ) KEYWORD_TARGET_REBOOT,
	( const char * const ) "target-reboot: Reboot target device\r\n",
	prvTaskRebootTargetCommand,
	0
};



static const xCommandLineInput xReadRegisterCommand =
{
	( const char * const ) KEYWORD_READ_REGISTER,
	( const char * const ) "read-register <hex address>  32 bit address, ex. read-register 0xd8000000 \r\n",
	prvReadRegisterCommand,
	1
};
static const xCommandLineInput xWriteRegisterCommand =
{
	( const char * const ) KEYWORD_WRITE_REGISTER,
	( const char * const ) "write-register <hex address>  32 bit address, ex. write-register 0xd8000000 \r\n",
	prvWriteRegisterCommand,
	1
};


static const xCommandLineInput xTargetStatusGet =
{
	( const char * const ) KEYWORD_TARGET_TEST,
	( const char * const ) "Check target status \r\n",
	prvTargetStatusGet,
	0
};

static const xCommandLineInput xTargetGpioSet =
{
	( const char * const ) KEYWORD_GPIO_SET,
	( const char * const ) "Set taget GPIO, EX: set-gpio num \r\n",
	prvGpioSetCommand,
	1
};

static const xCommandLineInput xTargetGpioClear =
{
	( const char * const ) KEYWORD_GPIO_CLEAR,
	( const char * const ) "Clear taget GPIO, EX: clear-gpio num \r\n",
	prvGpioClearCommand,
	1
};

static const xCommandLineInput xTargetGpioGet =
{
	( const char * const ) KEYWORD_GPIO_GET,
	( const char * const ) "Get taget GPIO, EX: get-gpio num \r\n",
	prvGpioGetCommand,
	1
};


static const xCommandLineInput xTargeRamScriptDownload =
{
	( const char * const ) KEYWORD_RAM_SCRIPT,
	( const char * const ) "script upload  \r\n",
	prvRamScriptDownload,
	1
};



static const xCommandLineInput xTargeBinDownload =
{
	( const char * const ) KEYWORD_BIN_LOAD,
	( const char * const ) "bin upload \r\n",
	prvBinDownload,
	1
};

static const xCommandLineInput xTargeBinBoot =
{
	( const char * const ) KEYWORD_BIN_BOOT,
	( const char * const ) "bin boot \r\n",
	prvBinBoot,
	0
};




static const xCommandLineInput xTargeBurnRom =
{
	( const char * const ) KEYWORD_TARGET_BURNROM,
	( const char * const ) "burn rom \r\n",
	prvBurnRom,
	0
};



static const xCommandLineInput xTargeLogModeOnOff =
{
	( const char * const ) KEYWORD_LOGMODE_ON_OFF,
	( const char * const ) "log mode on = 1 or off =0 \r\n",
	prvLogModeOnOff,
	1
};

//target_io_write(PIN_GPIO,1);

void cliTargetSystemInit(void)
{
    FreeRTOS_CLIRegisterCommand( &xTaskRebootTargetCommand );


//REGISTER ADDRESS	
    FreeRTOS_CLIRegisterCommand( &xReadRegisterCommand );
    FreeRTOS_CLIRegisterCommand( &xWriteRegisterCommand );

	
    FreeRTOS_CLIRegisterCommand( &xTargetStatusGet );


//GPIO modued
    FreeRTOS_CLIRegisterCommand( &xTargetGpioSet );		
    FreeRTOS_CLIRegisterCommand( &xTargetGpioClear );
    FreeRTOS_CLIRegisterCommand( &xTargetGpioGet );
	
//script module
    FreeRTOS_CLIRegisterCommand( &xTargeRamScriptDownload );
    FreeRTOS_CLIRegisterCommand( &xTargeBinDownload );
    FreeRTOS_CLIRegisterCommand( &xTargeBinBoot );
//
    FreeRTOS_CLIRegisterCommand( &xTargeBurnRom );
	
    FreeRTOS_CLIRegisterCommand( &xTargeLogModeOnOff );

	
}
