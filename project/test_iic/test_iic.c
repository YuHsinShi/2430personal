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
/*
	iicResult = mmpIicSetSlaveModeCallback(IIC_PORT_3, 0x77, _TestMasterReadSlave_IicReceiveCallback, _TestMasterReadSlave_IicWriteCallback);
	mmpIicSetSlaveMode(IIC_PORT_3, 0x77);
	assert(iicResult == true);
*/  
    IicMasterWriteBuffer[0] = 0x00;
    /*IicMasterWriteBuffer[1] = 0xB8;
	IicMasterWriteBuffer[2] = 0xB7;
	IicMasterWriteBuffer[3] = 0xB6;
	IicMasterWriteBuffer[4] = 0xB5;
  */

        for (i = 0; i < 128; i++)
		{
			recvBuffer[i]=i;
		}
		
    evt.slaveAddress   = 0xa0>>1;
	evt.cmdBuffer      = IicMasterWriteBuffer;
	evt.cmdBufferSize  = 1;
	evt.dataBuffer     = recvBuffer;
	evt.dataBufferSize = 128;
	//while(1)
	{
		uint32_t	result = 0;

        result = write(gMasterDev, &evt, 1);
		
		usleep(1*1000);	    
		
		for (i = 0; i < 128; i++)
		{
			recvBuffer[i]=0;
		}
	    
		result = read(gMasterDev, &evt, 1);
		//result = mmpIicReceiveData(IIC_PORT_1, IIC_MASTER_MODE, 0xa1, &cmd, 1, recvBuffer, 1);

		if(result != 0)
		{
			printf("master read slave failed!!!\n");
			//break;
		}
		printf("Master received: %d \n",result);
		for (i = 0; i < 128; i++)
		{
			printf("0x%02X ", recvBuffer[i]);
			/*
			if(buf[i] != recvBuffer[i])
			{
				printf("data compare error(0x%x!=0x%x)!!!\n", buf[i], recvBuffer[i]);
				while(1);
			}
			else if(i == 255)
				printf("IIC master read slave data compare ok, test success\n");
			*/
		}
		//break;
	}
}







#define AT24C16_WRITE_ADDR 0xA0
#define AT24C16_READ_ADDR  0xA1

/*
��������:  AT24C16ҳд����
��������:
    addr: �����￪ʼд����
    len : д��೤������
    *p  : ������ݵĻ�����
˵��: AT24C16�ڲ���һ��ҳд������,ҳ��ַ����֮��Ḵλ����ǰҳ����ʼ��ַ��
AT24C16ҳ��������С:  16���ֽ�
��д�����AT24C16��100ҳ�ĵ�3���ֽڽ���д���ݵ�ʱ�򣬲������£�
1��������ʼ�źţ�
2������������ַ0XA6��1010 0110��1010�ǹ̶���ַ��011��ҳ��ַ�ĸ���λ��0��ʾд��������
3�����Ͳ�����ַ0X43��0100 0011��0100��ҳ��ַ�ĵ���λ��0011��ҳ��ַƫ����������100ҳ�ڵĵ������ֽڣ�
4������Ҫд�����ݣ�
5��������ֹ�źš�
������AT24C02,ֱ��д�豸��ַ�����ݵ�ַ��
*/
void AT24C16_PageWrite(uint16_t addr,uint16_t len,uint8_t *buffer)
{
	uint8_t i;
	uint8_t page_addr,page_addr_H,Devicce_Write_Addr,Devicce_Read_Addr;
	page_addr=addr>>4;
	page_addr_H=page_addr>>5;
	Devicce_Write_Addr=AT24C16_WRITE_ADDR + (page_addr_H<<1);
	Devicce_Read_Addr=AT24C16_READ_ADDR+(page_addr_H<<1);
	printf("Devicce_Write_Addr = 0x%X  Devicce_Read_Addr = 0x%X  addr =0x%X \n",Devicce_Write_Addr,Devicce_Read_Addr,addr&0xFF);



#if 1
	uint8_t* tmp;
	uint32_t	result = 0;
	ITPI2cInfo evt;

	tmp=malloc(len+1);
	tmp[0]=(uint8_t)(addr&0xFF);
	memcpy(&tmp[1],buffer,len);
	
    evt.slaveAddress   = Devicce_Write_Addr>>1;
	evt.cmdBuffer      = tmp;
	evt.cmdBufferSize  = len+1;
	evt.dataBuffer     = 0;
	evt.dataBufferSize = 0;
	
	result = write(gMasterDev, &evt, 1);


	free(tmp);
	usleep(10*1000); //�ȴ�д���

#else
	IIC_START();
	IIC_WriteOneByte(Devicce_Write_Addr); //�����豸��ַ
	IIC_GetACK(); //��ȡӦ��
	IIC_WriteOneByte(addr&0xFF);  //���ʹ�����ݵĵ�ַ
	IIC_GetACK(); //��ȡӦ��
	for(i=0;i<len;i++)
	{
		 IIC_WriteOneByte(p[i]);  //����ʵ��Ҫ��ŵ�����
		 IIC_GetACK(); //��ȡӦ��
	}
	IIC_STOP();   //����ֹͣ�ź�
	DelayMs(10);  //�ȴ�д���
#endif	
}



/*
��������: ָ��λ��д��ָ������������
��������:
    addr: �����￪ʼд����
    len : д��೤������
    *p  : ������ݵĻ�����
*/
void AT24C16_WriteData(uint16_t addr,uint16_t len,uint8_t *p)
{
   uint8_t page_remain=16-addr%16; //�õ���ǰҳʣ����ֽ�����
   if(page_remain>=len)
   {
      page_remain=len;
   }
   while(1)
   {
       AT24C16_PageWrite(addr,page_remain,p);
       if(page_remain==len)break;
       addr+=page_remain;
       p+=page_remain;
       len-=page_remain;
       if(len>=16)page_remain=16;
       else page_remain=len;
   }
}


/*
��������: ָ��λ�ö�ȡָ������������
��������:
    addr: �����￪ʼ��ȡ����
    len : ��ȡ�೤������
    *p  : ������ݵĻ�����
��д�����AT24C16��100ҳ�ĵ�3���ֽڽ��ж����ݵ�ʱ�򣬲������£�
1��������ʼ�źţ�
2������������ַ0XA6��1010 0110��1010�ǹ̶���ַ��011��ҳ��ַ�ĸ���λ��0��ʾд��������
3�����Ͳ�����ַ0X43��0100 0011��0100��ҳ��ַ�ĵ���λ��0011��ҳ��ַƫ����������100ҳ�ڵĵ������ֽڣ�
4������Ҫд�����ݣ�
5��������ֹ�źš�
������AT24C02,ֱ��д�豸��ַ�����ݵ�ַ��
*/
void AT24C16_ReadData(uint16_t addr,uint16_t len,uint8_t *p)
{
	uint16_t i;
	uint8_t page_addr,page_addr_H,Devicce_Write_Addr,Devicce_Read_Addr;
	page_addr=addr>>4;
	page_addr_H=page_addr>>5;
	Devicce_Write_Addr=AT24C16_WRITE_ADDR + (page_addr_H<<1);
	Devicce_Read_Addr=AT24C16_READ_ADDR+(page_addr_H<<1);
//	printf("Devicce_Write_Addr = 0x%X \n Devicce_Read_Addr = 0x%X \n addr =0x%X \n",Devicce_Write_Addr,Devicce_Read_Addr,addr&0xFF);
#if 1
	uint32_t	result = 0;
	ITPI2cInfo evt;
	uint8_t data_addr_local;
	data_addr_local=(uint8_t)(addr&0xFF);

    evt.slaveAddress   = Devicce_Write_Addr>>1;
	evt.cmdBuffer      = &data_addr_local;
	evt.cmdBufferSize  = 1;
	evt.dataBuffer     = p;
	evt.dataBufferSize = len;
	
	result = read(gMasterDev, &evt, 1);

#else
	IIC_START();
	IIC_WriteOneByte(Devicce_Write_Addr); //�����豸��ַ(д)
	IIC_GetACK(); //��ȡӦ��
	IIC_WriteOneByte(addr&0xFF); //���ʹ�����ݵĵ�ַ(������ȡ���ݵĵ�ַ)
	IIC_GetACK(); //��ȡӦ��
	IIC_START();
	IIC_WriteOneByte(Devicce_Read_Addr); //�����豸��ַ(��)
	IIC_GetACK(); //��ȡӦ��
 
	for(i=0;i<len;i++)
	{
			p[i]=IIC_ReadOneByte(); //��������
			IIC_SendAck(0);  //���ӻ�����Ӧ��
	}
	IIC_SendAck(1);  //���ӻ����ͷ�Ӧ��
	IIC_STOP();      //ֹͣ�ź�
#endif


} 
 
 

 void Test_eeprom()
{
	#define EEPROM_WRITE_ADD  	4096
	#define DATA_SIZE 			128
	uint8_t writeout[DATA_SIZE];
	uint8_t readback[DATA_SIZE];
	int i;
		for (i = 0; i < DATA_SIZE; i++)
		{
			writeout[i]= (uint8_t)i;
		}
	
	AT24C16_WriteData(EEPROM_WRITE_ADD,DATA_SIZE,writeout);
	usleep(5*1000);	
	AT24C16_ReadData(EEPROM_WRITE_ADD,DATA_SIZE,readback);
	
		for (i = 0; i < DATA_SIZE; i++)
		{
			if(writeout[i] != readback[i])
			{
				printf("MIS MATCHED  %d, wr=0x%x re=0x%x\n",i,writeout[i], readback[i]);
				break;
			}
		}
	printf("===================\n");

	printf("Test_eeprom Success....\n");
	printf("===================\n");

}


void* TestFunc(void* arg)
{
    //itpInit();
    IIC_OP_MODE iic_port2_mode = IIC_MASTER_MODE;
	IIC_OP_MODE iic_port3_mode = IIC_SLAVE_MODE;

	// init i2c2 device
#if 1 //def CFG_I2C1_ENABLE
    itpRegisterDevice(ITP_DEVICE_I2C1, &itpDeviceI2c1);
	gMasterDev = open(":i2c1", 0);
    ioctl(ITP_DEVICE_I2C1, ITP_IOCTL_INIT, (void*)IIC_MASTER_MODE);
#endif

    // init i2c3 device
#ifdef CFG_I2C3_ENABLE
    itpRegisterDevice(ITP_DEVICE_I2C3, &itpDeviceI2c3);
	gSlaveDev = open(":i2c3", 0);
    ioctl(ITP_DEVICE_I2C3, ITP_IOCTL_INIT, (void*)iic_port3_mode);
#endif
	//TestMasterReadSlave();
	Test_eeprom();
	return NULL;
}
