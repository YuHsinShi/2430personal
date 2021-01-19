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
#define KEYWORD_BOOT_BIN	    "boot-bin"
#define KEYWORD_TARGET_TEST	    "target-test"

#define KEYWORD_GPIO_SET	    "gpio-set"
#define KEYWORD_GPIO_CLEAR	    "gpio-clear"
#define KEYWORD_GPIO_GET	    "gpio-get"


static portBASE_TYPE prvTaskRebootTargetCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{

  int i;
	( void ) pcCommandString;
	configASSERT( pcWriteBuffer );

	( void ) xWriteBufferLen;

uint8_t indata[16]={0};
uint32_t group ;
uint32_t value ;

#define PIN_GPIO 		20

group = ithGpioGet_GpioInputAddress(PIN_GPIO);//
value=read_slave_register(group,indata);//packet3


for(int i=0;i<9;i++){
printf("0x%x ",indata[i]);}
printf("\n");





if(	value & (0x1 << (PIN_GPIO & 0x1F)) ){ 	
	printf("tartget GPIO %d is HIGH \n",PIN_GPIO );}
else{
	printf("tartget GPIO %d is LOW \n",PIN_GPIO );}


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


static portBASE_TYPEprvTargetStatusGet( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
char tmp_str[128]={0}; 
int ret;
ret =target_auto_detect();
if(ret>0)
{
	ret=get_ite_bootcfg();
	snprintf( tmp_str, 128, "TARGET %x found ",ret );

	if(0b11==ret)//11
	{
		snprintf( tmp_str, 128, "%s COOPERATIVE MODE",tmp_str);
	}
	else if(0b10==ret)//10
	{
		snprintf( tmp_str, 128, "%s NAND BOOT MODE",tmp_str);
	}
	else if(0b01==ret)//
	{
	
	 snprintf( tmp_str, 128, "%s NOR BOOT MODE",tmp_str);
	}
	else if (0b00==ret)
	{
	
		snprintf( tmp_str, 128, "%s SD BOOT MODE",tmp_str);
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








static const xCommandLineInput xTaskRebootTargetCommand =
{
	( const char * const ) "reset",
	( const char * const ) "reset: Reboot target device\r\n",
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
	( const char * const ) "Test if Target is connected \r\n",
	portBASE_TYPEprvTargetStatusGet,
	1
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





//target_io_write(PIN_GPIO,1);

void cliTargetSystemInit(void)
{
    FreeRTOS_CLIRegisterCommand( &xTaskRebootTargetCommand );
	
    FreeRTOS_CLIRegisterCommand( &xReadRegisterCommand );

    FreeRTOS_CLIRegisterCommand( &xWriteRegisterCommand );

	
    FreeRTOS_CLIRegisterCommand( &xTargetStatusGet );


    FreeRTOS_CLIRegisterCommand( &xTargetGpioSet );
		
    FreeRTOS_CLIRegisterCommand( &xTargetGpioClear );
    FreeRTOS_CLIRegisterCommand( &xTargetGpioGet );
	
}
