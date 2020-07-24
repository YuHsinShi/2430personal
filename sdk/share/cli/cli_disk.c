/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * CLI disk related functions.
 *
 * @author Jim Tan
 * @version 1.0
 */
#include "cli_cfg.h"
#include "openrtos/FreeRTOS.h"
#include "openrtos/semphr.h"
#include "openrtos/FreeRTOS_CLI.h"
#include <sys/dirent.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>

/*-----------------------------------------------------------*/
/* These objects are large, so not stored on the stack.  They are used by
the functions that implement command line commands - and as only one command
line command can be executing at any one time - there is no need to protect
access to the variables using a mutex or other mutual exclusion method. */
static FILE *pxCommandLineFile1, *pxCommandLineFile2;

/* A buffer used to both create content to write to disk, and read content back
from a disk.  Note there is no mutual exclusion on this buffer, so it must not
be accessed outside of the task. */
static uint8_t cRAMBuffer[ 2000 ];

/* The RAM buffer is used by two tasks, so protect it using a mutex. */
static SemaphoreHandle_t xRamBufferMutex = NULL;

/*-----------------------------------------------------------*/
/*
 * Implements the file system "dir" command, accessible through a command
 * console.  See the definition of the xDirCommand command line input structure
 * below.  This function is not necessarily reentrant.  It must not be used by
 * more than one task at a time.
 */
static portBASE_TYPE prvDirCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
static portBASE_TYPE xDirectoryOpen = pdFALSE;
static DIR* xDirectory;
static struct dirent* pxFileInfo;
portBASE_TYPE xReturn = pdFALSE;

	/* This assumes pcWriteBuffer is long enough. */
	( void ) pcCommandString;

	if( xDirectoryOpen == pdFALSE )
	{
		/* This is the first time the function has been called this run of the
		dir command.  Ensure the directory is open. */

		if( ( xDirectory = opendir( getcwd( cRAMBuffer, sizeof( cRAMBuffer ) ) ) ) != NULL )
		{
			xDirectoryOpen = pdTRUE;
		}
		else
		{
			xDirectoryOpen = pdFALSE;
		}
	}

	if( xDirectoryOpen == pdTRUE )
	{
		/* Read the next file. */
		if( ( pxFileInfo = readdir( xDirectory ) ) != NULL )
		{
			if( pxFileInfo->d_name[ 0 ] != '\0' )
			{
			    struct stat sbuf;
			    getcwd( cRAMBuffer, sizeof( cRAMBuffer ) );
			    strcat( cRAMBuffer, pxFileInfo->d_name );
			    stat( cRAMBuffer, &sbuf );

				/* There is at least one more file name to return. */
				snprintf( ( char * ) pcWriteBuffer, xWriteBufferLen, "%s\t\t%d\r\n", pxFileInfo->d_name, sbuf.st_size );
				xReturn = pdTRUE;

			}
		}
		else
		{
			closedir( xDirectory );

			/* There are no more file names to return.   Reset the read
			pointer ready for the next time this directory is read. */
			xReturn = pdFALSE;
			xDirectoryOpen = pdFALSE;
			pcWriteBuffer[ 0 ] = 0x00;
		}
	}

	return xReturn;
}
/*-----------------------------------------------------------*/
/*
 * Implements the file system "del" command, accessible through a command
 * console.  See the definition of the xDelCommand command line input structure
 * below.  This function is not necessarily reentrant.  It must not be used by
 * more than one task at a time.
 */
static portBASE_TYPE prvDelCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
char *pcParameter;
portBASE_TYPE xParameterStringLength;
const unsigned portBASE_TYPE uxFirstParameter = 1U;

	/* This assumes pcWriteBuffer is long enough. */
	( void ) xWriteBufferLen;

	/* Obtain the name of the file being deleted. */
	pcParameter = ( char * ) FreeRTOS_CLIGetParameter( pcCommandString, uxFirstParameter, &xParameterStringLength );

	/* Terminate the parameter string. */
	pcParameter[ xParameterStringLength ] = 0x00;

	if( unlink( ( const char * ) pcParameter ) != 0 )
	{
		snprintf( ( char * ) pcWriteBuffer, xWriteBufferLen, "Could not delete %s\r\n\r\n", pcParameter );
	}

	/* There is only ever one string to return. */
	return pdFALSE;
}
/*-----------------------------------------------------------*/
/*
 * Implements the file system "copy" command, accessible through a command
 * console.  See the definition of the xCopyCommand command line input
 * structure below.  This function is not necessarily reentrant.  It must not
 * be used by more than one task at a time.
 */
static portBASE_TYPE prvCopyCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
char *pcParameter1, *pcParameter2;
portBASE_TYPE xParameter1StringLength, xParameter2StringLength, xFinished = pdFALSE;
const unsigned portBASE_TYPE uxFirstParameter = 1U, uxSecondParameter = 2U;
unsigned int xBytesRead, xBytesWritten;
const TickType_t xMaxDelay = 500UL / portTICK_PERIOD_MS;

	( void ) xWriteBufferLen;

	/* Obtain the name of the source file, and the length of its name. */
	pcParameter1 = ( char * ) FreeRTOS_CLIGetParameter( pcCommandString, uxFirstParameter, &xParameter1StringLength );

	/* Obtain the name of the destination file, and the length of its name. */
	pcParameter2 = ( char * ) FreeRTOS_CLIGetParameter( pcCommandString, uxSecondParameter, &xParameter2StringLength );

	/* Terminate both file names. */
	pcParameter1[ xParameter1StringLength ] = 0x00;
	pcParameter2[ xParameter2StringLength ] = 0x00;

	/* Open the source file. */
	if( ( pxCommandLineFile1 = fopen( ( const char * ) pcParameter1, "rb" ) ) != NULL )
	{
		/* Open the destination file. */
		if( ( pxCommandLineFile2 = fopen( ( const char * ) pcParameter2, "wb" ) ) != NULL )
		{
			while( xFinished == pdFALSE )
			{
				/* About to use the RAM buffer, ensure this task has exclusive access
				to it while it is in use. */
				if( xSemaphoreTake( xRamBufferMutex, xMaxDelay ) == pdPASS )
				{
					if( ( xBytesRead = fread( cRAMBuffer, 1, sizeof( cRAMBuffer ),  pxCommandLineFile1 ) ) != EOF )
					{
						if( xBytesRead != 0U )
						{
							if( ( xBytesWritten = fwrite( cRAMBuffer, 1, xBytesRead, pxCommandLineFile2 ) ) == xBytesRead )
							{
								if( xBytesWritten < xBytesRead )
								{
									snprintf( ( char * ) pcWriteBuffer, xWriteBufferLen, "Error writing to %s, disk full?\r\n\r\n", pcParameter2 );
									xFinished = pdTRUE;
								}
							}
							else
							{
								snprintf( ( char * ) pcWriteBuffer, xWriteBufferLen, "Error during copy\r\n\r\n" );
								xFinished = pdTRUE;
							}
						}
						else
						{
							/* EOF. */
							xFinished = pdTRUE;
						}
					}
					else
					{
						snprintf( ( char * ) pcWriteBuffer, xWriteBufferLen, "Error during copy\r\n\r\n" );
						xFinished = pdTRUE;
					}

					/* Must give the mutex back! */
					xSemaphoreGive( xRamBufferMutex );
				}
			}

			/* Close both files. */
			fclose( pxCommandLineFile1 );
			fclose( pxCommandLineFile2 );
		}
		else
		{
			snprintf( ( char * ) pcWriteBuffer, xWriteBufferLen, "Could not open or create %s\r\n\r\n", pcParameter2 );

			/* Close the source file. */
			fclose( pxCommandLineFile1 );
		}
	}
	else
	{
		snprintf( ( char * ) pcWriteBuffer, xWriteBufferLen, "Could not open %s\r\n\r\n", pcParameter1 );
	}

	return pdFALSE;
}
/*-----------------------------------------------------------*/

/* Structure that defines the "dir" command line command. */
static const xCommandLineInput xDirCommand =
{
	( const char * const ) "dir",
	( const char * const ) "dir: Displays the name and size of each file in the root directory\r\n",
	prvDirCommand,
	0
};

/* Structure that defines the "del" command line command. */
static const xCommandLineInput xDelCommand =
{
	( const char * const ) "del",
	( const char * const ) "del <filename>: Deletes <filename> from the disk\r\n",
	prvDelCommand,
	1
};

/* Structure that defines the "del" command line command. */
static const xCommandLineInput xCopyCommand =
{
	( const char * const ) "copy",
	( const char * const ) "copy <filename1> <filename2>: Copies <filename1> to <filename2>, creating or overwriting <filename2>\r\n",
	prvCopyCommand,
	2
};
/*-----------------------------------------------------------*/

void cliDiskInit(void)
{
	/* Register the file commands with the command interpreter. */
	FreeRTOS_CLIRegisterCommand( &xDirCommand );
	FreeRTOS_CLIRegisterCommand( &xDelCommand );
	FreeRTOS_CLIRegisterCommand( &xCopyCommand );

	/* Create the mutex that protects the shared RAM buffer. */
	xRamBufferMutex = xSemaphoreCreateMutex();
	configASSERT( xRamBufferMutex );
}
