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

static portBASE_TYPE prvTaskRebootCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
  int i;
	( void ) pcCommandString;
	configASSERT( pcWriteBuffer );

	( void ) xWriteBufferLen;

  LOG_INFO "reboot countdown:\n" LOG_END
  for (i = 3; i > 0; --i)
  {
    LOG_INFO "%d..\n", i LOG_END
    sleep(1);
  }

  exit(0);

	return pdFALSE;
}

static const xCommandLineInput xTaskRebootCommand =
{
	( const char * const ) "reboot",
	( const char * const ) "reboot: Reboot this device\r\n",
	prvTaskRebootCommand,
	0
};

void cliSystemInit(void)
{
    FreeRTOS_CLIRegisterCommand( &xTaskRebootCommand );
}
