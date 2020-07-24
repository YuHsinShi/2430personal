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

#if defined (CFG_UART0_ENABLE) && defined(CFG_DBG_UART0)
#define UART_PORT ITP_DEVICE_UART0
#elif defined (CFG_UART1_ENABLE) && defined(CFG_DBG_UART1)
#define UART_PORT ITP_DEVICE_UART1
#elif defined (CFG_UART2_ENABLE) && defined(CFG_DBG_UART2)
#define UART_PORT ITP_DEVICE_UART2
#elif defined (CFG_UART3_ENABLE) && defined(CFG_DBG_UART3)
#define UART_PORT ITP_DEVICE_UART3
#endif

extern void cliExit(void);
static uint8_t* packageBuffer;
static int packageSize;
#ifdef CFG_NET_TFTP
char tftppara[128];
#endif

static portBASE_TYPE prvTaskUartBootCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString)
{
	cliExit();
	return pdFALSE;
}

static portBASE_TYPE prvTaskUartUplaodCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString)
{
	static portBASE_TYPE xReady = pdFALSE;
	char *pcParameter1;
	portBASE_TYPE xParameter1StringLength;
	int result;

	if (xReady == pdFALSE)
	{
		pcParameter1 = (char *)FreeRTOS_CLIGetParameter(pcCommandString, 1, &xParameter1StringLength);

		pcParameter1[xParameter1StringLength] = 0x00;

		if (xParameter1StringLength == 0)
		{
			snprintf((char *)pcWriteBuffer, xWriteBufferLen, "%s\r\n", "ERROR: Invalid parameters");
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
			snprintf((char *)pcWriteBuffer, xWriteBufferLen, "%s\r\n", "ERROR: Out of memory");
			goto error;
		}

		snprintf((char *)pcWriteBuffer, xWriteBufferLen, "%s\r\n", "OK");
		xReady = pdTRUE;
		return pdTRUE;
	}
	else
	{
		uint32_t readsize, remainsize, bufpos, up_count;

		remainsize = packageSize;
		bufpos = 0;
		up_count = 0;

		do
		{
			readsize = read(UART_PORT, &packageBuffer[bufpos], 1024);

			remainsize -= readsize;
			bufpos += readsize;

			if (bufpos > up_count)
			{
				up_count+=10000;
				printf("uploaded: %d, remain: %d\n", bufpos, remainsize);
			}

		} while (remainsize > 0);
		putchar('\n');

		snprintf((char *)pcWriteBuffer, xWriteBufferLen, "%s\r\n", "OK");

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

static portBASE_TYPE prvTaskUartUpgradeCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString)
{
	(void)pcCommandString;
	configASSERT(pcWriteBuffer);
	(void)xWriteBufferLen;

	if (packageBuffer)
	{
		int ret;
		ITCArrayStream arrayStream;
		itcArrayStreamOpen(&arrayStream, packageBuffer, packageSize);

		if (ugCheckCrc(&arrayStream.stream, NULL))
		{
			snprintf((char *)pcWriteBuffer, xWriteBufferLen, "%s\r\n", "ERROR: Check CRC fail");
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
			snprintf((char *)pcWriteBuffer, xWriteBufferLen, "%s: %d\r\n", "ERROR: Upgrade failed", ret);
			goto error;
		}
	}
	else
	{
		snprintf((char *)pcWriteBuffer, xWriteBufferLen, "%s\r\n", "ERROR: No uploaded package");
		goto error;
	}

	snprintf((char *)pcWriteBuffer, xWriteBufferLen, "%s\r\n", "OK");

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

#ifdef CFG_NET_TFTP
static portBASE_TYPE prvTaskUartTftpUpgradeCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString)
{
	char *pcParameter1;
	ITCStream* upgradeFile = NULL;
	portBASE_TYPE xParameter1StringLength;

	(void)pcCommandString;
	configASSERT(pcWriteBuffer);
	(void)xWriteBufferLen;

	pcParameter1 = (char *)FreeRTOS_CLIGetParameter(pcCommandString, 1, &xParameter1StringLength);

	pcParameter1[xParameter1StringLength] = 0x00;

	strcpy(tftppara, pcParameter1);

	upgradeFile = OpenRecoveryPackage();

	if (upgradeFile)
	{
		int ret;

		if (ugCheckCrc(upgradeFile, NULL))
		{
			snprintf((char *)pcWriteBuffer, xWriteBufferLen, "%s\r\n", "ERROR: Check CRC fail");
			goto error;
		}

		ret = ugUpgradePackage(upgradeFile);

#if defined(CFG_NOR_ENABLE) && CFG_NOR_CACHE_SIZE > 0
		LOG_INFO "Flushing NOR cache...\n" LOG_END
			ioctl(ITP_DEVICE_NOR, ITP_IOCTL_FLUSH, NULL);
#endif

#ifdef CFG_NAND_ENABLE
		ioctl(ITP_DEVICE_NAND, ITP_IOCTL_FLUSH, ITP_NAND_FTL_MODE);
#endif

		if (ret)
		{
			snprintf((char *)pcWriteBuffer, xWriteBufferLen, "%s: %d\r\n", "ERROR: Upgrade failed", ret);
			goto error;
		}
	}
	else
	{
		snprintf((char *)pcWriteBuffer, xWriteBufferLen, "%s\r\n", "ERROR: No uploaded package");
		goto error;
	}

	snprintf((char *)pcWriteBuffer, xWriteBufferLen, "%s\r\n", "OK");

	free(upgradeFile);
	upgradeFile = NULL;
	packageSize = 0;
	return pdFALSE;

error:
	LOG_ERR "%s\n", pcWriteBuffer LOG_END

	if (upgradeFile)
	{
		free(upgradeFile);
		upgradeFile = NULL;
	}
	packageSize = 0;

	return pdFALSE;
}
#endif

static const xCommandLineInput xTaskUartBootCommand =
{
	(const char * const) "uart_boot",
	(const char * const) "uart_boot: Close cli task and start boot image.\r\n",
	prvTaskUartBootCommand,
	0
};

static const xCommandLineInput xTaskUartUploadCommand =
{
	(const char * const) "uart_upload",
	(const char * const) "uart_upload <size>: Upload package file with specified size. UART only.\r\n",
	prvTaskUartUplaodCommand,
	1
};

static const xCommandLineInput xTaskUartUpgradeCommand =
{
	(const char * const) "uart_upgrade",
	(const char * const) "uart_upgrade: Upgrade uploaded package file\r\n",
	prvTaskUartUpgradeCommand,
	0
};

#ifdef CFG_NET_TFTP
static const xCommandLineInput xTaskUartTftpUpgradeCommand =
{
	(const char * const) "uart_tftp_upgrade",
	(const char * const) "uart_tftp_upgrade: Download pkg from tftp serve and upgrade system.\r\n",
	prvTaskUartTftpUpgradeCommand,
	1
};
#endif

void cliUartUpgradeInit(void)
{
	FreeRTOS_CLIRegisterCommand(&xTaskUartBootCommand);
	FreeRTOS_CLIRegisterCommand(&xTaskUartUploadCommand);
	FreeRTOS_CLIRegisterCommand(&xTaskUartUpgradeCommand);
#ifdef CFG_NET_TFTP
	FreeRTOS_CLIRegisterCommand(&xTaskUartTftpUpgradeCommand);
#endif
}
