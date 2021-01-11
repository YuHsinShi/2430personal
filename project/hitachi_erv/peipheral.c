#include <sys/ioctl.h>
#include <assert.h>
#include <stdio.h>
#include "ite/itp.h"
#include "iic/mmp_iic.h"




#if 1 //beeper

static int beeper_initial=0;

#define BEEPER_FREQ 		4000

#define BEEPER_GPIO 		15
#define BEEPER_PWM_INDEX 	6  //TIMER7
#define BEEPER_PWM_MODE 	0



#define RED_LED_PWM_GPIO 		43
#define RED_LED_PWM_INDEX 		0 //TIMER1
#define RED_LED_PWM_MODE 		0

#define GREEN_LED_PWM_GPIO 		42
#define GREEN_LED_PWM_INDEX 	1 //TIMER2
#define GREEN_LED_PWM_MODE 		0


void* beeper_task(void* arg)
{

        ithPwmEnable(BEEPER_PWM_INDEX, BEEPER_GPIO, BEEPER_PWM_MODE);
        usleep(100*1000);
        ithPwmDisable(BEEPER_PWM_INDEX,BEEPER_GPIO);

    return NULL;
}


void green_led_duty(unsigned int duty)
{
	ithPwmSetDutyCycle(GREEN_LED_PWM_INDEX,duty);
	ithPwmEnable(GREEN_LED_PWM_INDEX, GREEN_LED_PWM_GPIO, GREEN_LED_PWM_MODE);

}

void red_led_duty(unsigned int duty)
{
	ithPwmSetDutyCycle(RED_LED_PWM_INDEX,duty);
	ithPwmEnable(RED_LED_PWM_INDEX, RED_LED_PWM_GPIO, RED_LED_PWM_MODE);

}


void beeper_init()
{

//	Test_eeprom();
	green_led_duty(50);
	red_led_duty(50);


    ithPwmInit(BEEPER_PWM_INDEX,BEEPER_FREQ,100);
    ithPwmReset(BEEPER_PWM_INDEX,BEEPER_GPIO,BEEPER_PWM_MODE);
	ithPwmSetDutyCycle(BEEPER_PWM_INDEX,50);
	ithPwmDisable(BEEPER_PWM_INDEX,BEEPER_GPIO);


	beeper_initial=1;

}

void beeper_once()
{
//return;
	if(0== beeper_initial)
		beeper_init();

	 pthread_t beep_tid;
	 pthread_create(&beep_tid, NULL, beeper_task, NULL);
}



void led_init()
{

		ithPwmInit(RED_LED_PWM_INDEX,5000,0);
		ithPwmReset(RED_LED_PWM_INDEX,RED_LED_PWM_GPIO,RED_LED_PWM_MODE);

		ithPwmInit(GREEN_LED_PWM_INDEX,5000,0);
		ithPwmReset(GREEN_LED_PWM_INDEX,GREEN_LED_PWM_GPIO,GREEN_LED_PWM_MODE);		
}

void peripheral_init()
{
	led_init();
	beeper_init();
}

#endif

#if 1

#endif

#define AT24C16_WRITE_ADDR 0xA0
#define AT24C16_READ_ADDR  0xA1


static	int gMasterDev = 0;

void init_eeprom_iic()
{
 
	#ifdef CFG_I2C1_ENABLE
	itpRegisterDevice(ITP_DEVICE_I2C1, &itpDeviceI2c1);
	gMasterDev = open(":i2c1", 0);
	ioctl(ITP_DEVICE_I2C1, ITP_IOCTL_INIT, (void*)IIC_MASTER_MODE);
	#endif
}

/*
函数功能:  AT24C16页写函数
函数参数:
    addr: 从哪里开始写数据
    len : 写入多长的数据
    *p  : 存放数据的缓冲区
说明: AT24C16内部有一个页写缓冲器,页地址超出之后会复位到当前页的起始地址。
AT24C16页缓冲器大小:  16个字节
编写程序对AT24C16第100页的第3个字节进行写数据的时候，步骤如下：
1）发送起始信号；
2）发送器件地址0XA6（1010 0110，1010是固定地址，011是页地址的高三位，0表示写操作）；
3）发送操作地址0X43（0100 0011，0100是页地址的低四位，0011是页地址偏移量，即第100页内的第三个字节，
4）发送要写的数据，
5）发送终止信号。
（对于AT24C02,直接写设备地址和数据地址）
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
	
	if(0 == gMasterDev)
		init_eeprom_iic();


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
	usleep(10*1000); //等待写完成

#else
	IIC_START();
	IIC_WriteOneByte(Devicce_Write_Addr); //发送设备地址
	IIC_GetACK(); //获取应答
	IIC_WriteOneByte(addr&0xFF);  //发送存放数据的地址
	IIC_GetACK(); //获取应答
	for(i=0;i<len;i++)
	{
		 IIC_WriteOneByte(p[i]);  //发送实际要存放的数据
		 IIC_GetACK(); //获取应答
	}
	IIC_STOP();   //发送停止信号
	DelayMs(10);  //等待写完成
#endif	
}



/*
函数功能: 指定位置写入指定数量的数据
函数参数:
    addr: 从哪里开始写数据
    len : 写入多长的数据
    *p  : 存放数据的缓冲区
*/
void eeprom_24C16_WriteData(uint16_t addr,uint16_t len,uint8_t *p)
{
   uint8_t page_remain=16-addr%16; //得到当前页剩余的字节数量
   
	
	if(0 == gMasterDev)
		init_eeprom_iic();   
   
   
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
函数功能: 指定位置读取指定数量的数据
函数参数:
    addr: 从哪里开始读取数据
    len : 读取多长的数据
    *p  : 存放数据的缓冲区
编写程序对AT24C16第100页的第3个字节进行读数据的时候，步骤如下：
1）发送起始信号；
2）发送器件地址0XA6（1010 0110，1010是固定地址，011是页地址的高三位，0表示写操作）；
3）发送操作地址0X43（0100 0011，0100是页地址的低四位，0011是页地址偏移量，即第100页内的第三个字节，
4）发送要写的数据，
5）发送终止信号。
（对于eeprom_24C16_ReadData,直接写设备地址和数据地址）
*/
void eeprom_24C16_ReadData(uint16_t addr,uint16_t len,uint8_t *p)
{
	uint16_t i;
	uint8_t page_addr,page_addr_H,Devicce_Write_Addr,Devicce_Read_Addr;
	
	
	if(0 == gMasterDev)
		init_eeprom_iic();
	
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
	IIC_WriteOneByte(Devicce_Write_Addr); //发送设备地址(写)
	IIC_GetACK(); //获取应答
	IIC_WriteOneByte(addr&0xFF); //发送存放数据的地址(即将读取数据的地址)
	IIC_GetACK(); //获取应答
	IIC_START();
	IIC_WriteOneByte(Devicce_Read_Addr); //发送设备地址(读)
	IIC_GetACK(); //获取应答
 
	for(i=0;i<len;i++)
	{
			p[i]=IIC_ReadOneByte(); //接收数据
			IIC_SendAck(0);  //给从机发送应答
	}
	IIC_SendAck(1);  //给从机发送非应答
	IIC_STOP();      //停止信号
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
	
	eeprom_24C16_WriteData(EEPROM_WRITE_ADD,DATA_SIZE,writeout);
	usleep(5*1000);	
	eeprom_24C16_ReadData(EEPROM_WRITE_ADD,DATA_SIZE,readback);
	
		for (i = 0; i < DATA_SIZE; i++)
		{
			if(writeout[i] != readback[i])
			{

				printf("===================\n");
				printf("MIS MATCHED  %d, wr=0x%x re=0x%x\n",i,writeout[i], readback[i]);	
				printf("Test_eeprom FAIL....\n");
				printf("===================\n");
				
				return;
			}
		}
	printf("===================\n");

	printf("Test_eeprom Success....\n");
	printf("===================\n");

}
