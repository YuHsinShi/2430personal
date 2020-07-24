/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * CLI upgrade related functions.
 *
 * @author Jim Tan
 * @version 1.0
 */
#include "cli_cfg.h"
#include "openrtos/FreeRTOS.h"
#include "openrtos/task.h"
#include "openrtos/FreeRTOS_CLI.h"
#include "ite/itc.h"
#include "ite/ug.h"
#include <sys/ioctl.h>
#include <stdlib.h>
#include <string.h>

static uint8_t* packageBuffer;
static int packageSize;

static portBASE_TYPE prvTaskUploadCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
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
      packageSize = strtol(&pcParameter1[2], NULL, 16);
    }
    else
    {
      packageSize = strtol(pcParameter1, NULL, 10);
    }
    LOG_DBG "%s: size=%d\n", pcParameter1, packageSize LOG_END    
    
    packageBuffer = malloc(packageSize);
    if (packageBuffer == NULL)
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

    remainsize = packageSize;
    bufpos = 0;

    do
    {
        if (ioctl(ITP_DEVICE_USBDACM, ITP_IOCTL_IS_CONNECTED, NULL) == 0)
        {
          snprintf( ( char * ) pcWriteBuffer, xWriteBufferLen, "%s\r\n", "ERROR" );
          goto error;
        }
          
       	readsize = read(ITP_DEVICE_USBDACM, &packageBuffer[bufpos], remainsize);

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
        
  if (packageBuffer)
  {
    free(packageBuffer);
    packageBuffer = NULL;
  }
  packageSize = 0;
    
  xReady = pdFALSE;
  return pdFALSE;  
}

static portBASE_TYPE prvTaskUpgradeCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
	( void ) pcCommandString;
	configASSERT( pcWriteBuffer );
	( void ) xWriteBufferLen;

  if (packageBuffer)
  {
      int ret;
      ITCArrayStream arrayStream;    
      itcArrayStreamOpen(&arrayStream, packageBuffer, packageSize);

      if (ugCheckCrc(&arrayStream.stream, NULL))
      {
          snprintf( ( char * ) pcWriteBuffer, xWriteBufferLen, "%s\r\n", "ERROR: Check CRC fail" );
          goto error;
      }
      
      ret = ugUpgradePackage(&arrayStream.stream);
      
  #if defined(CFG_NOR_ENABLE) && CFG_NOR_CACHE_SIZE > 0
      LOG_INFO "Flushing NOR cache...\n" LOG_END
      ioctl(ITP_DEVICE_NOR, ITP_IOCTL_FLUSH, NULL);
  #endif

	#ifdef CFG_NAND_ENABLE
	    ioctl(ITP_DEVICE_NAND, ITP_IOCTL_FLUSH, ITP_NAND_FTL_MODE);
	#endif

      if (ret)
      {
          snprintf( ( char * ) pcWriteBuffer, xWriteBufferLen, "%s: %d\r\n", "ERROR: Upgrade failed", ret );
          goto error;
      }
  }
  else
  {
      snprintf( ( char * ) pcWriteBuffer, xWriteBufferLen, "%s\r\n", "ERROR: No uploaded package" );
      goto error;
  }

  snprintf( ( char * ) pcWriteBuffer, xWriteBufferLen, "%s\r\n", "OK" );
  
  free(packageBuffer);
  packageBuffer = NULL;
  packageSize = 0;
  return pdFALSE;  

error:
  LOG_ERR "%s\n", pcWriteBuffer LOG_END
        
  if (packageBuffer)
  {
    free(packageBuffer);
    packageBuffer = NULL;
  }
  packageSize = 0;

  return pdFALSE;  
}

static const xCommandLineInput xTaskUploadCommand =
{
	( const char * const ) "upload",
	( const char * const ) "upload <size>: Upload package file with specified size. USB ACM only.\r\n",
	prvTaskUploadCommand,
	1
};

static const xCommandLineInput xTaskUpgradeCommand =
{
	( const char * const ) "upgrade",
	( const char * const ) "upgrade: Upgrade uploaded package file\r\n",
	prvTaskUpgradeCommand,
	0
};

void cliUpgradeInit(void)
{
    FreeRTOS_CLIRegisterCommand( &xTaskUploadCommand );
    FreeRTOS_CLIRegisterCommand( &xTaskUpgradeCommand );
}
