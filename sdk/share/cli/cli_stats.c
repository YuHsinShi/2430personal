/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * CLI statistics related functions.
 *
 * @author Jim Tan
 * @version 1.0
 */
#include "cli_cfg.h"
#include "openrtos/FreeRTOS.h"
#include "openrtos/task.h"
#include "openrtos/FreeRTOS_CLI.h"
#include <string.h>

/* Callbacks to handle the command line commands defined by the xTaskStats and
xRunTimeStats command definitions respectively.  These functions are not
necessarily reentrant!  They must be used from one task only - or at least by
only one task at a time. */

static portBASE_TYPE prvTaskStatsCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
const int8_t *const pcHeader = ( int8_t * ) "Task          State  Priority  Stack	#\r\n************************************************\r\n";

	( void ) pcCommandString;
	configASSERT( pcWriteBuffer );

	/* This function assumes the buffer length is adequate. */
	( void ) xWriteBufferLen;

	/* Generate a table of task stats. */
	strcpy( ( char * ) pcWriteBuffer, ( char * ) pcHeader );
	vTaskList( pcWriteBuffer + strlen( ( char * ) pcHeader ) );

	/* There is no more data to return after this single string, so return
	pdFALSE. */
	return pdFALSE;
}
/*-----------------------------------------------------------*/

static portBASE_TYPE prvRunTimeStatsCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
const int8_t * const pcHeader = ( int8_t * ) "Task            Abs Time      % Time\r\n****************************************\r\n";

	( void ) pcCommandString;
	configASSERT( pcWriteBuffer );

	/* This function assumes the buffer length is adequate. */
	( void ) xWriteBufferLen;

	/* Generate a table of task stats. */
	strcpy( ( char * ) pcWriteBuffer, ( char * ) pcHeader );
	vTaskGetRunTimeStats( pcWriteBuffer + strlen( ( char * ) pcHeader ) );

	/* There is no more data to return after this single string, so return
	pdFALSE. */
	return pdFALSE;
}
/*-----------------------------------------------------------*/

/* Structure that defines the "task-stats" command line command. */
static const xCommandLineInput xTaskStatsCommand =
{
	( const char * const ) "task-stats",
	( const char * const ) "task-stats: Displays a table showing the state of each FreeRTOS task\r\n",
	prvTaskStatsCommand,
	0
};

/* Structure that defines the "run-time-stats" command line command. */
static const xCommandLineInput xRunTimeStatsCommand =
{
	( const char * const ) "run-time-stats",
	( const char * const ) "run-time-stats: Displays a table showing how much processing time each FreeRTOS task has used\r\n",
	prvRunTimeStatsCommand,
	0
};
/*-----------------------------------------------------------*/

void cliStatsInit(void)
{
    /* Register two command line commands to show task stats and run time stats
	  respectively. */
    FreeRTOS_CLIRegisterCommand( &xTaskStatsCommand );
    FreeRTOS_CLIRegisterCommand( &xRunTimeStatsCommand );
}
