#include "ite/ith.h"
#include "ite/itp.h"
#include "iot_bus.h"
#include "stdlib.h"
#include "stdio.h"
#include "ssp/mmp_spi.h"

#define SPI_PORT                    SPI_0
#define MAX_TRANSACTION_SIZE        4096

#define SLAVE_READY_GPIO            22

#define SPIS_CFG_RD_CMD             0x0a
#define SPIS_RD_CMD                 0x81
#define SPIS_CFG_WR_CMD             0x0c
#define SPIS_WR_CMD                 0x0e
#define SPIS_RS_CMD                 0x06
#define SPIS_PWON_CMD               0x04
#define SPIS_PWOFF_CMD              0x02
#define SPIS_CT_CMD                 0x10
#define SPIS_ADDRESS_ID             0x55aa0000

#define SLAVE_POWER_ON_OFFSET       (0)
#define SLAVE_POWER_ON_MASK         (0x1 << SLAVE_POWER_ON_OFFSET)
#define SLAVE_CFG_SUCCESS_OFFSET    (1)
#define SLAVE_CFG_SUCCESS_MASK      (0x1 << SLAVE_CFG_SUCCESS_OFFSET)
#define SLAVE_TXRX_FIFO_RDY_OFFSET  (2)
#define SLAVE_TXRX_FIFO_RDY_MASK    (0x1 << SLAVE_TXRX_FIFO_RDY_OFFSET)
#define SLAVE_RW_DONE_OFFSET        (5)
#define SLAVE_RW_DONE_MASK          (0x1 << SLAVE_RW_DONE_OFFSET)

static void _spiQuerySlaveStatus(uint8_t *status)
{
    uint8_t status_cmd = SPIS_RS_CMD;
    uint8_t status_receive = 0;

    /* Note:
     * The value of receive_length is the valid number of bytes received plus the number of bytes to send.
     * For example, here the valid number of bytes received is 1 byte,
     * and the number of bytes to send also is 1 byte, so the receive_length is 2.
     */
    mmpSpiPioRead(SPI_PORT, &status_cmd, 1, &status_receive, 1, 8);
    *status = status_receive;
    //printf("status: 0x%X\n", *status);
}

static int _waitSlaveStatusDone(uint32_t waitStatus, uint32_t timeoutCount)
{
    uint8_t status;
    uint32_t counter = 0;
    while(1)
    {
        _spiQuerySlaveStatus(&status);
        if (status & waitStatus)
        {
            return 1;
        }
        else
        {
            if (counter++ >= timeoutCount)
            {
                return 0;
            }
            usleep(100);
        }
    }
}

void _waitSlaveReady(void)
{
    while (!ithGpioGet(SLAVE_READY_GPIO))
    {
        usleep(100);
    }
}

void _waitSlaveReady1(void)
{
    while (ithGpioGet(SLAVE_READY_GPIO))
    {
        usleep(100);
    }
}


int IotBusWrite(uint8_t* txBuffer, int txSize)
{
    int bufPos = 0;
    int sendSize = 0;
    int result = 0;
    uint8_t status = 0;

    uint8_t cfg_wr_cmd[9] = { 0 };
    uint8_t cmdWrite = SPIS_WR_CMD;

    _waitSlaveReady();
    //CW
    cfg_wr_cmd[0] = SPIS_CFG_WR_CMD;
    cfg_wr_cmd[1] = SPIS_ADDRESS_ID & 0xff;
    cfg_wr_cmd[2] = (SPIS_ADDRESS_ID >> 8) & 0xff;
    cfg_wr_cmd[3] = (SPIS_ADDRESS_ID >> 16) & 0xff;
    cfg_wr_cmd[4] = (SPIS_ADDRESS_ID >> 24) & 0xff;
    cfg_wr_cmd[5] = (txSize - 1) & 0xff;
    cfg_wr_cmd[6] = ((txSize - 1) >> 8) & 0xff;
    cfg_wr_cmd[7] = ((txSize - 1) >> 16) & 0xff;
    cfg_wr_cmd[8] = ((txSize - 1) >> 24) & 0xff;
send_cw:
    mmpSpiPioWrite(SPI_PORT, cfg_wr_cmd, 9, NULL, 0, 8);

    //printf("%s(%d)\n", __FUNCTION__, __LINE__);
    if (!_waitSlaveStatusDone(SLAVE_TXRX_FIFO_RDY_MASK, 16))
    {
        goto send_cw;
    }
    //printf("%s(%d)\n", __FUNCTION__, __LINE__);
send_wr:
	_waitSlaveReady1();

    if (txSize >= 16)
    {
        result = mmpSpiDmaWrite(SPI_PORT, &cmdWrite, 1, txBuffer, txSize, 8);
    }
    else
    {
        result = mmpSpiPioWrite(SPI_PORT, &cmdWrite, 1, txBuffer, txSize, 8);
    }
    //printf("%s(%d)\n", __FUNCTION__, __LINE__);
    //if (!_waitSlaveStatusDone(SLAVE_RW_DONE_MASK, 16))
    //{
    //    goto send_wr;
    //}
    //printf("%s(%d)\n", __FUNCTION__, __LINE__);


    if (!result)
    {
        return result;
    } 
    return txSize;
}

int IotBusRead(uint8_t* rxBuffer, int rxSize)
{
    int bufPos = 0;
    int sendSize = 0;
    int result = 0;
    uint8_t status = 0;

    uint8_t cfg_rd_cmd[9] = { 0 };
    uint8_t cmdRead = SPIS_RD_CMD;

    _waitSlaveReady();

    //CR
    cfg_rd_cmd[0] = SPIS_CFG_RD_CMD;
    cfg_rd_cmd[1] = SPIS_ADDRESS_ID & 0xff;
    cfg_rd_cmd[2] = (SPIS_ADDRESS_ID >> 8) & 0xff;
    cfg_rd_cmd[3] = (SPIS_ADDRESS_ID >> 16) & 0xff;
    cfg_rd_cmd[4] = (SPIS_ADDRESS_ID >> 24) & 0xff;
    cfg_rd_cmd[5] = (rxSize - 1) & 0xff;
    cfg_rd_cmd[6] = ((rxSize - 1) >> 8) & 0xff;
    cfg_rd_cmd[7] = ((rxSize - 1) >> 16) & 0xff;
    cfg_rd_cmd[8] = ((rxSize - 1) >> 24) & 0xff;
send_cr:
    mmpSpiPioWrite(SPI_PORT, cfg_rd_cmd, 9, NULL, 0, 8);
    //printf("%s(%d)\n", __FUNCTION__, __LINE__);
    if (!_waitSlaveStatusDone(SLAVE_TXRX_FIFO_RDY_MASK, 16))
    {
        goto send_cr;
    }
    //printf("%s(%d)\n", __FUNCTION__, __LINE__);

send_rd:
		_waitSlaveReady1();

    if (sendSize >= 16)
    {
        result = mmpSpiDmaRead(SPI_PORT, &cmdRead, 1, rxBuffer, rxSize, 8);
    }
    else
    {
        result = mmpSpiPioRead(SPI_PORT, &cmdRead, 1, rxBuffer, rxSize, 8);
    }
    //printf("%s(%d)\n", __FUNCTION__, __LINE__);
    //if (!_waitSlaveStatusDone(SLAVE_RW_DONE_MASK, 16))
    //{
    //    goto send_rd;
    //}
    //printf("%s(%d)\n", __FUNCTION__, __LINE__);

    if (!result)
    {
        return result;
    } 

    return rxSize;
}

int IotBusInit(void)
{
    int result = 0;
    uint8_t status = 0;
    uint8_t powerOn = SPIS_PWON_CMD;

    //Slave
    ithGpioSetMode(SLAVE_READY_GPIO, ITH_GPIO_MODE0);
    ithGpioSetIn(SLAVE_READY_GPIO);

    result = mmpSpiInitialize(SPI_PORT, SPI_OP_MASTR, CPO_0_CPH_0, SPI_CLK_20M);

    mmpSpiPioWrite(SPI_PORT, &powerOn, 1, NULL, 0, 8);
    while(1)
    {
        _spiQuerySlaveStatus(&status);
        if (status & SLAVE_POWER_ON_MASK)
        {
            printf("Power On stage is done...\n");
            break;
        }
    }
    return result;
}
