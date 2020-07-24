/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * CLI rawdata related functions.
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
#include <string.h>
#include <unistd.h>

static portBASE_TYPE prvTaskWriteRawDataCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
static portBASE_TYPE xReady = pdFALSE;
static int fd = -1;
static uint32_t position, filesize, blocksize, gapsize, alignsize, bufsize, pos, pos2;
static char* buf = NULL;
char *pcParameter1, *pcParameter2, *pcParameter3;
portBASE_TYPE xParameter1StringLength, xParameter2StringLength, xParameter3StringLength;

  if( xReady == pdFALSE )
  {
  	pcParameter1 = ( char * ) FreeRTOS_CLIGetParameter( pcCommandString, 1, &xParameter1StringLength );
  	pcParameter2 = ( char * ) FreeRTOS_CLIGetParameter( pcCommandString, 2, &xParameter2StringLength );
  	pcParameter3 = ( char * ) FreeRTOS_CLIGetParameter( pcCommandString, 3, &xParameter3StringLength );
  	
  	pcParameter1[ xParameter1StringLength ] = 0x00;
  	pcParameter2[ xParameter2StringLength ] = 0x00;
    pcParameter3[ xParameter3StringLength ] = 0x00;
    
    if (xParameter1StringLength == 0 || xParameter2StringLength == 0 || xParameter3StringLength == 0)
    {
      snprintf( ( char * ) pcWriteBuffer, xWriteBufferLen, "%s\r\n", "ERROR: Invalid parameters" );
      goto error;
    }
    
    if (strcmp(pcParameter1, "nand") == 0)
    {
      fd = open(":nand", O_RDWR, 0);
    }
    else if (strcmp(pcParameter1, "nor") == 0)
    {
      fd = open(":nor", O_RDWR, 0);
    }
    else if (strcmp(pcParameter1, "sd0") == 0)
    {
      fd = open(":sd0", O_RDWR, 0);
    }
    else if (strcmp(pcParameter1, "sd1") == 0)
    {
      fd = open(":sd1", O_RDWR, 0);
    }
    else
    {
      snprintf( ( char * ) pcWriteBuffer, xWriteBufferLen, "%s\r\n", "ERROR: Unknown storage" );
      goto error;
    }

    if (fd == -1)
    {
      snprintf( ( char * ) pcWriteBuffer, xWriteBufferLen, "%s\r\n", "ERROR: Open storage fail" );
      goto error;
    }

    if (ioctl(fd, ITP_IOCTL_GET_BLOCK_SIZE, &blocksize))
    {
      snprintf( ( char * ) pcWriteBuffer, xWriteBufferLen, "%s\r\n", "ERROR: Get block size fail" );
      goto error;
    }

    if (ioctl(fd, ITP_IOCTL_GET_GAP_SIZE, &gapsize))
    {
      snprintf( ( char * ) pcWriteBuffer, xWriteBufferLen, "%s\r\n", "ERROR: Get gap size fail" );
      goto error;
    }
    LOG_DBG "blocksize=%d gapsize=%d\n", blocksize, gapsize LOG_END
    
    if (strncmp(pcParameter2, "0x", 2) == 0 || strncmp(pcParameter2, "0X", 2) == 0)
    {
      position = strtol(&pcParameter2[2], NULL, 16);
    }
    else
    {
      position = strtol(pcParameter2, NULL, 10);
    }

    if (strncmp(pcParameter3, "0x", 2) == 0 || strncmp(pcParameter3, "0X", 2) == 0)
    {
      filesize = strtol(&pcParameter3[2], NULL, 16);
    }
    else
    {
      filesize = strtol(pcParameter3, NULL, 10);
    }
    LOG_DBG "%s: position=0x%X size=%u\n", pcParameter1, position, filesize LOG_END

    alignsize = ITH_ALIGN_UP(filesize, blocksize + gapsize);
    bufsize = CFG_CLI_BUF_SIZE < alignsize ? CFG_CLI_BUF_SIZE : alignsize;
    buf = malloc(bufsize);
    if (!buf)
    {
      snprintf( ( char * ) pcWriteBuffer, xWriteBufferLen, "%s: %u\r\n", "ERROR: Out of memory", bufsize );
      goto error;
    }

   	pos = position / (blocksize + gapsize);
    alignsize = position % (blocksize + gapsize);
    	
    if (alignsize > 0)
    {
        LOG_WARN "Position 0x%X and block+gap size 0x%X are not aligned; align to 0x%X\n", position, (blocksize + gapsize), (pos * (blocksize + gapsize)) LOG_END
    }

    pos2 = lseek(fd, pos, SEEK_SET);
    if (pos2 != pos)
    {
      snprintf( ( char * ) pcWriteBuffer, xWriteBufferLen, "%s\r\n", "ERROR: lseek fail" );
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

      alignsize = ITH_ALIGN_UP(readsize, blocksize) / blocksize;
      size2 = write(fd, buf, alignsize);
      if (size2 != alignsize)
      {
        snprintf( ( char * ) pcWriteBuffer, xWriteBufferLen, "%s: %u != %u\r\n", "ERROR: Cannot write data", size2, alignsize );
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
       
    close(fd);
    fd = -1;
    
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

  if (fd != -1)
  {
    close(fd);
    fd = -1;    
  }
    
  xReady = pdFALSE;
  return pdFALSE;  
}
/*-----------------------------------------------------------*/

static portBASE_TYPE prvReadRawDataCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
static portBASE_TYPE xReady = pdFALSE;
static int fd = -1;
static uint32_t position, filesize, blocksize, gapsize, alignsize, bufsize, pos, pos2;
static char* buf = NULL;
char *pcParameter1, *pcParameter2, *pcParameter3;
portBASE_TYPE xParameter1StringLength, xParameter2StringLength, xParameter3StringLength;

  if( xReady == pdFALSE )
  {
  	pcParameter1 = ( char * ) FreeRTOS_CLIGetParameter( pcCommandString, 1, &xParameter1StringLength );
  	pcParameter2 = ( char * ) FreeRTOS_CLIGetParameter( pcCommandString, 2, &xParameter2StringLength );
  	pcParameter3 = ( char * ) FreeRTOS_CLIGetParameter( pcCommandString, 3, &xParameter3StringLength );
  	
  	pcParameter1[ xParameter1StringLength ] = 0x00;
  	pcParameter2[ xParameter2StringLength ] = 0x00;
    pcParameter3[ xParameter3StringLength ] = 0x00;
    
    if (xParameter1StringLength == 0 || xParameter2StringLength == 0 || xParameter3StringLength == 0)
    {
      snprintf( ( char * ) pcWriteBuffer, xWriteBufferLen, "%s\r\n", "ERROR: Invalid parameters" );
      goto error;
    }
    
    if (strcmp(pcParameter1, "nand") == 0)
    {
      fd = open(":nand", O_RDWR, 0);
    }
    else if (strcmp(pcParameter1, "nor") == 0)
    {
      fd = open(":nor", O_RDWR, 0);
    }
    else if (strcmp(pcParameter1, "sd0") == 0)
    {
      fd = open(":sd0", O_RDWR, 0);
    }
    else if (strcmp(pcParameter1, "sd1") == 0)
    {
      fd = open(":sd1", O_RDWR, 0);
    }
    else
    {
      snprintf( ( char * ) pcWriteBuffer, xWriteBufferLen, "%s\r\n", "ERROR: Unknown storage" );
      goto error;
    }

    if (fd == -1)
    {
      snprintf( ( char * ) pcWriteBuffer, xWriteBufferLen, "%s\r\n", "ERROR: Open storage fail" );
      goto error;
    }

    if (ioctl(fd, ITP_IOCTL_GET_BLOCK_SIZE, &blocksize))
    {
      snprintf( ( char * ) pcWriteBuffer, xWriteBufferLen, "%s\r\n", "ERROR: Get block size fail" );
      goto error;
    }

    if (ioctl(fd, ITP_IOCTL_GET_GAP_SIZE, &gapsize))
    {
      snprintf( ( char * ) pcWriteBuffer, xWriteBufferLen, "%s\r\n", "ERROR: Get gap size fail" );
      goto error;
    }
    LOG_DBG "blocksize=%d gapsize=%d\n", blocksize, gapsize LOG_END
    
    if (strncmp(pcParameter2, "0x", 2) == 0 || strncmp(pcParameter2, "0X", 2) == 0)
    {
      position = strtol(&pcParameter2[2], NULL, 16);
    }
    else
    {
      position = strtol(pcParameter2, NULL, 10);
    }

    if (strncmp(pcParameter3, "0x", 2) == 0 || strncmp(pcParameter3, "0X", 2) == 0)
    {
      filesize = strtol(&pcParameter3[2], NULL, 16);
    }
    else
    {
      filesize = strtol(pcParameter3, NULL, 10);
    }
    LOG_DBG "%s: position=0x%X size=%u\n", pcParameter1, position, filesize LOG_END

    alignsize = ITH_ALIGN_UP(filesize, blocksize + gapsize);
    bufsize = CFG_CLI_BUF_SIZE < alignsize ? CFG_CLI_BUF_SIZE : alignsize;
    buf = malloc(bufsize);
    if (!buf)
    {
      snprintf( ( char * ) pcWriteBuffer, xWriteBufferLen, "%s: %u\r\n", "ERROR: Out of memory", bufsize );
      goto error;
    }

   	pos = position / (blocksize + gapsize);
    alignsize = position % (blocksize + gapsize);
    	
    if (alignsize > 0)
    {
        LOG_WARN "Position 0x%X and block+gap size 0x%X are not aligned; align to 0x%X\n", position, (blocksize + gapsize), (pos * (blocksize + gapsize)) LOG_END
    }

    pos2 = lseek(fd, pos, SEEK_SET);
    if (pos2 != pos)
    {
      snprintf( ( char * ) pcWriteBuffer, xWriteBufferLen, "%s\r\n", "ERROR: lseek fail" );
      goto error;
    }

    snprintf( ( char * ) pcWriteBuffer, xWriteBufferLen, "%s\r\n", "OK" );
    xReady = pdTRUE;
    return pdTRUE;
  }
  else
  {
    uint32_t readsize, size2, remainsize, writesize;

    remainsize = filesize;

    do
    {
       	readsize = (remainsize < bufsize) ? remainsize : bufsize;
       	alignsize = ITH_ALIGN_UP(readsize, blocksize) / blocksize;
       	size2 = read(fd, buf, alignsize);
        if (size2 != alignsize)
        {
          LOG_ERR "ERROR: Cannot read data; %u != %u\n", size2, alignsize LOG_END
          pcWriteBuffer[0] = '\0';
          goto error;
        }

        LOG_DBG "write: size=%u\n", readsize LOG_END

        size2 = 0;
        writesize = 0;
        do
        {
          if (ioctl(ITP_DEVICE_USBDACM, ITP_IOCTL_IS_CONNECTED, NULL) == 0)
          {
            LOG_ERR "ERROR: USB disconnected\n" LOG_END
            pcWriteBuffer[0] = '\0';
            goto error;
          }

          writesize = readsize - size2;

          size2 += write(ITP_DEVICE_USBDACM, buf + size2, writesize);
          
          LOG_DBG "readsize=%u size2=%u\n", readsize, size2 LOG_END
          
        } while (size2 < readsize);

        remainsize -= readsize;

        LOG_DBG "remainsize=%u\n", remainsize LOG_END

        putchar('.');
        fflush(stdout);
    }
    while (remainsize > 0);
    putchar('\n');

    pcWriteBuffer[0] = '\0';

    if (filesize % 512 == 0) // workaround linux-cdc-acm.inf cannot receive multiple of 512 bytes bug
    {
      size2 = write(ITP_DEVICE_USBDACM, pcWriteBuffer, 1);
      LOG_DBG "workaround written size: %u\n", size2 LOG_END
    }
    
    free(buf); 
    buf = NULL;
       
    close(fd);
    fd = -1;    
    
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

  if (fd != -1)
  {
    close(fd);
    fd = -1;    
  }
    
  xReady = pdFALSE;
  return pdFALSE;  
}
/*-----------------------------------------------------------*/

/* Structure that defines the "run-time-stats" command line command. */
static const xCommandLineInput xWriteRawDataCommand =
{
	( const char * const ) "write-rawdata",
	( const char * const ) "write-rawdata <nand|nor|sd0|sd1> <position> <size>: Writes data to specified storage with position and size. position must align to block size. USB ACM only.\r\n",
	prvTaskWriteRawDataCommand,
	3
};

/* Structure that defines the "task-stats" command line command. */
static const xCommandLineInput xReadRawDataCommand =
{
	( const char * const ) "read-rawdata",
	( const char * const ) "read-rawdata <nand|nor|sd0|sd1> <position> <size>: Reads data from specified storage with position and size. position must align to block size. USB ACM only.\r\n",
	prvReadRawDataCommand,
	3
};
/*-----------------------------------------------------------*/

void cliRawDataInit(void)
{
    /* Register two command line commands to show task stats and run time stats
	  respectively. */
    FreeRTOS_CLIRegisterCommand( &xWriteRawDataCommand );
    FreeRTOS_CLIRegisterCommand( &xReadRawDataCommand );
}
