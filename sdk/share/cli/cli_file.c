/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * CLI file related functions.
 *
 * @author Jim Tan
 * @version 1.0
 */
#include "cli_cfg.h"
#include "openrtos/FreeRTOS.h"
#include "openrtos/task.h"
#include "openrtos/FreeRTOS_CLI.h"
#include <sys/ioctl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "ite/itp.h"

static portBASE_TYPE prvTaskWriteFileCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
static portBASE_TYPE xReady = pdFALSE;
static FILE* f = NULL;
static uint32_t position, filesize, bufsize, pos, pos2;
static char* buf = NULL;
char *pcParameter1, *pcParameter2;
portBASE_TYPE xParameter1StringLength, xParameter2StringLength;

  if( xReady == pdFALSE )
  {
  	pcParameter1 = ( char * ) FreeRTOS_CLIGetParameter( pcCommandString, 1, &xParameter1StringLength );
  	pcParameter2 = ( char * ) FreeRTOS_CLIGetParameter( pcCommandString, 2, &xParameter2StringLength );

  	pcParameter1[ xParameter1StringLength ] = 0x00;
    pcParameter2[ xParameter2StringLength ] = 0x00;

    if (xParameter1StringLength == 0 || xParameter2StringLength == 0)
    {
      snprintf( ( char * ) pcWriteBuffer, xWriteBufferLen, "%s\r\n", "ERROR: Invalid parameters" );
      goto error;
    }

    if (strlen(pcParameter1) > 3)
    {
      f = fopen(pcParameter1, "wb");
    }
    else
    {
      snprintf( ( char * ) pcWriteBuffer, xWriteBufferLen, "%s\r\n", "ERROR: Invalid parameters" );
      goto error;
    }

    if (f == NULL)
    {
      snprintf( ( char * ) pcWriteBuffer, xWriteBufferLen, "%s\r\n", "ERROR: Open file fail" );
      goto error;
    }

    if (strncmp(pcParameter2, "0x", 2) == 0 || strncmp(pcParameter2, "0X", 2) == 0)
    {
      filesize = strtol(&pcParameter2[2], NULL, 16);
    }
    else
    {
      filesize = strtol(pcParameter2, NULL, 10);
    }
    LOG_DBG "%s: size=%u\n", pcParameter1, filesize LOG_END

    bufsize = CFG_CLI_BUF_SIZE < filesize ? CFG_CLI_BUF_SIZE : filesize;
    buf = malloc(bufsize);
    if (!buf)
    {
      snprintf( ( char * ) pcWriteBuffer, xWriteBufferLen, "%s: %u\r\n", "ERROR: Out of memory", bufsize );
      goto error;
    }

    snprintf( ( char * ) pcWriteBuffer, xWriteBufferLen, "%s\r\n", "OK" );
    xReady = pdTRUE;
    return pdTRUE;
  }
  else
  {
    uint32_t readsize, size2, remainsize, readusbsize;

    remainsize = filesize;

    do
    {
     	readsize = (remainsize < bufsize) ? remainsize : bufsize;

     	size2 = 0;
     	do
     	{
        if (ioctl(ITP_DEVICE_USBDACM, ITP_IOCTL_IS_CONNECTED, NULL) == 0)
        {
          snprintf( ( char * ) pcWriteBuffer, xWriteBufferLen, "%s\r\n", "ERROR" );
          goto error;
        }
   	    size2 += read(ITP_DEVICE_USBDACM, buf + size2, readsize - size2);

   	    LOG_DBG "readsize=%u size2=%u\n", readsize, size2 LOG_END

     	} while (size2 < readsize);

      size2 = fwrite(buf, 1, readsize, f);
      if (size2 != readsize)
      {
        snprintf( ( char * ) pcWriteBuffer, xWriteBufferLen, "%s: %u != %u\r\n", "ERROR: Cannot write data", size2, readsize );
        goto error;
      }

      remainsize -= readsize;

      putchar('.');
      fflush(stdout);
    }
    while (remainsize > 0);
    putchar('\n');

    snprintf( ( char * ) pcWriteBuffer, xWriteBufferLen, "%s\r\n", "OK" );

    free(buf);
    buf = NULL;

    fclose(f);
    f = NULL;

#if defined(CFG_NOR_ENABLE) && CFG_NOR_CACHE_SIZE > 0
        ioctl(ITP_DEVICE_NOR, ITP_IOCTL_FLUSH, NULL);
#endif

    xReady = pdFALSE;
    return pdFALSE;
  }

error:
  LOG_ERR "%s\n", pcWriteBuffer LOG_END

  if (buf)
  {
    free(buf);
    buf = NULL;
  }

  if (f != NULL)
  {
    fclose(f);
    f = NULL;
  }

  xReady = pdFALSE;
  return pdFALSE;
}
/*-----------------------------------------------------------*/

/* Structure that defines the "write-file" command line command. */
static const xCommandLineInput xWriteFileCommand =
{
	( const char * const ) "write-file",
	( const char * const ) "write-file <filepath> <size>: Writes file to specified path with size. USB ACM only.\r\n",
	prvTaskWriteFileCommand,
	2
};
/*-----------------------------------------------------------*/

void cliFileInit(void)
{
    /* Register command line commands respectively. */
    FreeRTOS_CLIRegisterCommand( &xWriteFileCommand );
}
