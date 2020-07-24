#include <sys/ioctl.h>
#include <assert.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include "ite/itp.h"
#include "iic/mmp_iic.h"

#define IIC_BUFFER_LEN	1024 * 1024
static uint8_t	IicMasterWriteBuffer[IIC_BUFFER_LEN] = {0};
static uint8_t	IicSlaveReceviceBuffer[IIC_BUFFER_LEN] = {0};
static uint8_t	buf[256] = {0};
static	int       gMasterDev = 0;
static	int       gSlaveDev = 0;

static void _TestMasterReadSlave_IicReceiveCallback(uint8_t* recvBuffer, uint32_t recvBufferSize)
{
    uint32_t i = 0;
	for(i = 0; i < recvBufferSize; i++)
	{
		printf("IIC Slave receive write cmd(0x%02X)\n", recvBuffer[i]);
		if(recvBuffer[i] != IicMasterWriteBuffer[i])
		{
			printf("IIC slave receive cmd compare error!!!!!!!!\n");
			while(1);
		}
	}
}

static int _TestMasterReadSlave_IicWriteCallback(IIC_DEVICE dev)
{
	static uint32_t ii = 0;
	
	//uint8_t		buf[256] = {0};
	uint32_t	i = 0;
	bool		result = false;

	printf("IIC Slave output...\n");

	for(i = 0; i < 256; i++)
	{
		buf[i] = 'a' + i + ii;
		//printf("0x%02x,", buf[i]);
	}
	printf("\n");
	result = mmpIicSlaveFiFoWrite(IIC_PORT_3, buf, 256);
	assert(result == true);
	ii++;
}

void
TestMasterReadSlave()
{
	bool iicResult = false;
	ITPI2cInfo evt;
    uint8_t		cmd = 0xFE;
	uint8_t		recvBuffer[256] = {0};
	uint32_t	i;

	iicResult = mmpIicSetSlaveModeCallback(IIC_PORT_3, 0x77, _TestMasterReadSlave_IicReceiveCallback, _TestMasterReadSlave_IicWriteCallback);
	mmpIicSetSlaveMode(IIC_PORT_3, 0x77);
	assert(iicResult == true);

    IicMasterWriteBuffer[0] = 0xAD;
    IicMasterWriteBuffer[1] = 0xB8;
	IicMasterWriteBuffer[2] = 0xB7;
	IicMasterWriteBuffer[3] = 0xB6;
	IicMasterWriteBuffer[4] = 0xB5;
    
    evt.slaveAddress   = 0x77;
	evt.cmdBuffer      = IicMasterWriteBuffer;
	evt.cmdBufferSize  = 5;
	evt.dataBuffer     = recvBuffer;
	evt.dataBufferSize = 256;
        
	while(1)
	{
		uint32_t	result = 0;

        result = write(gMasterDev, &evt, 1);
		usleep(100000);
	    
	    result = read(gMasterDev, &evt, 1);
		//result = mmpIicReceiveData(IIC_PORT_0, IIC_MASTER_MODE, 0x77, &cmd, 1, recvBuffer, 256);

		if(result != 0)
		{
			printf("master read slave failed!!!\n");
			break;
		}
		printf("Master received:\n");
		for (i = 0; i < 256; i++)
		{
			printf("0x%02X ", recvBuffer[i]);
			if(buf[i] != recvBuffer[i])
			{
				printf("data compare error(0x%x!=0x%x)!!!\n", buf[i], recvBuffer[i]);
				while(1);
			}
			else if(i == 255)
				printf("IIC master read slave data compare ok, test success\n");
		}
		//break;
	}
}

void* TestFunc(void* arg)
{
    //itpInit();
    IIC_OP_MODE iic_port2_mode = IIC_MASTER_MODE;
	IIC_OP_MODE iic_port3_mode = IIC_SLAVE_MODE;

	// init i2c2 device
#ifdef CFG_I2C2_ENABLE
    itpRegisterDevice(ITP_DEVICE_I2C2, &itpDeviceI2c2);
	gMasterDev = open(":i2c2", 0);
    ioctl(ITP_DEVICE_I2C2, ITP_IOCTL_INIT, (void*)iic_port2_mode);
#endif

    // init i2c3 device
#ifdef CFG_I2C3_ENABLE
    itpRegisterDevice(ITP_DEVICE_I2C3, &itpDeviceI2c3);
	gSlaveDev = open(":i2c3", 0);
    ioctl(ITP_DEVICE_I2C3, ITP_IOCTL_INIT, (void*)iic_port3_mode);
#endif
	TestMasterReadSlave();

	return NULL;
}
