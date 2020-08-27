#include <sys/ioctl.h>
#include <assert.h>
#include <mqueue.h>
#include <string.h>
#include <unistd.h>
#include "ite/itp.h"
#include "ctrlboard.h"
#include "scene.h"

static pthread_t UpgradeUartTask;

static void* UpgradeFWTask(void* arg)
{
	int readLen = 0, i = 0;
	unsigned char tmp[1024] = {0};
	bool upgradeFlag = false;
	unsigned char Ack[1] = {0};

	printf("%s start\n", __FUNCTION__);

	while (1) {
		readLen = read(UPGRADE_UART_PORT, tmp, 1024);
		if (readLen > 0 && !upgradeFlag)
		{
			for (i = 0; i < readLen; i++)
			{
				if (tmp[i] == UPGRADE_PATTERN)
				{
					upgradeFlag = true;
					break;
				}
			}
			if (upgradeFlag)
			{
				Ack[0] = ACK20;
				write(UPGRADE_UART_PORT, Ack, 1);
				break;
			}
		}
	}

	printf("%s end\n", __FUNCTION__);

	// Reset Uart FIFO
#if (UPGRADE_UART_PORT == ITP_DEVICE_UART0)
	ithUartSetRxTriggerLevel(ITH_UART0_BASE, ITH_UART_TRGL0);
#elif (UPGRADE_UART_PORT == ITP_DEVICE_UART1)
	ithUartSetRxTriggerLevel(ITH_UART1_BASE, ITH_UART_TRGL0);
#endif

	SceneQuit(QUIT_UPGRADE_UART);
}

void UpgradeUartInit(void)
{
    pthread_create(&UpgradeUartTask, NULL, UpgradeFWTask, NULL);
}

void UpgradeUartExit(void)
{
    pthread_join(UpgradeUartTask, NULL);
}

