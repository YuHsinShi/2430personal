#include <sys/ioctl.h>
#include <assert.h>
#include <mqueue.h>
#include <string.h>
#include <unistd.h>
#include "ite/itp.h"
#include "ctrlboard.h"
#include "scene.h"

#define EXT_MAX_QUEUE_SIZE      8
#define MAX_OUTDATA_SIZE        64

#define UartHeader     		0xFF
#define UartTotalLength     16
#define UartPayloadLength   13

// Example for UART payload reference
#define UartPayload0   		0x00
#define UartPayload1   		0x01
#define UartPayload2   		0x02
#define UartPayload3   		0x03
#define UartPayload4   		0x04
#define UartPayload5   		0x05
#define UartPayload6   		0x06
#define UartPayload7   		0x07
#define UartPayload8   		0x08
#define UartPayload9   		0x09
#define UartPayload10  		0x0A
#define UartPayload11  		0x0B
#define UartPayload12  		0x0C
#define UartCheckSum		0x4E  // CheckSum = payload0 + payload1 + ...... payload12

#ifdef _WIN32
#define WIN32_COM           5
#endif

typedef enum GET_UART_COMMAND_STATE_TAG
{
    GET_HEADER,
    GET_LENGTH,
    GET_PAYLOAD,
    GET_CHECKSUM,
} GET_UART_COMMAND_STATE;

static mqd_t extInQueue = -1;
static mqd_t extOutQueue = -1;
static pthread_t extTask;
static volatile bool extQuit;
static unsigned char inDataBuf[EXTERNAL_BUFFER_SIZE];
static unsigned char cmdBuf[EXTERNAL_BUFFER_SIZE];
static unsigned int cmdPos = 0;
static GET_UART_COMMAND_STATE gState = GET_HEADER;
static unsigned int payloadCount = 0;
static unsigned int checkSum = 0;







#if 1 //defined(TEST_INTR_MODE)
#include "uart/interface/rs485.h"

#define TEST_PORT       ITP_DEVICE_RS485_1
#define TEST_ITH_PORT	ITH_UART1
#define TEST_DEVICE     itpDeviceRS485_1
#define TEST_BAUDRATE   CFG_UART1_BAUDRATE
#define TEST_GPIO_RX    CFG_GPIO_UART1_RX
#define TEST_GPIO_TX    CFG_GPIO_UART1_TX
#define TEST_GPIO_EN    CFG_GPIO_RS485_1_TX_ENABLE 


static sem_t RS485SemIntr;

static void RS485Callback(void* arg1, uint32_t arg2)
{
	uint8_t getstr[256];

	sem_post(&RS485SemIntr);

}

#endif
unsigned char get_data_crc(char* data_buf,int data_len)
{

	int i;
	unsigned char crc=0;
	int sum=0;
	int tmp;
	for (i=0;i<data_len;i++)
	{
		sum+=data_buf[i];
	}

		tmp = 0 - sum;
		crc =(unsigned char)(tmp);

	//printf("tmp=0x%x\n",tmp);
	//printf("crc=0x%x\n",crc);

	return crc;

}

int gen_ap_mode_set_ok_cmd(char* buf)
{
	unsigned char crc;

	//HEADER:  TAG:( 4 BYTES)
	*buf=0x2f;
	*(buf+1)=0xb4;
	*(buf+2)=0x9D;
	*(buf+3)=0xAC;

	//HEADER:  LEN:( 2 BYTES)
	*(buf+4)=0x0;
	*(buf+5)=8;

	//HEADER:  SRC:( 1 BYTE)
	*(buf+6)=3;

	//DATA: TAG (1 BYTE)
	*(buf+7)=0x93;

	//DATA: CMD (1 BYTE)
	*(buf+8)=0x01;

	//DATA: DATA LENGTH (4 BYTE)
	*(buf+9)=0;
	*(buf+10)=0;
	*(buf+11)=0;
	*(buf+12)=8;


if(1) // ap mode is on
	*(buf+13)=0x01;
else // ap mode is off
	*(buf+13)=0x00;


	crc=get_data_crc(buf+13,1); //data byte crc 

	//DATA CRC
	*(buf+14)=crc;

return 15;

}


int gen_mac_cmd(char* buf)
{

	char mac_array[6];
	unsigned char crc;

	WifiMgr_Get_MAC_Address(mac_array);
	printf("MAC is %s",mac_array);//this is string not hex

	//HEADER:  TAG:( 4 BYTES)
	*buf=0x2f;
	*(buf+1)=0xb4;
	*(buf+2)=0x9D;
	*(buf+3)=0xAC;

	//HEADER:  LEN:( 2 BYTES)
	*(buf+4)=0x0;
	*(buf+5)=13;

	//HEADER:  SRC:( 1 BYTE)
	*(buf+6)=3;

	//DATA: TAG (1 BYTE)
	*(buf+7)=0x93;

	//DATA: CMD (1 BYTE)
	*(buf+8)=0x0;

	//DATA: DATA LENGTH (4 BYTE)
	*(buf+9)=0;
	*(buf+10)=0;
	*(buf+11)=0;
	*(buf+12)=13;

	//DATA: 6 BYTE , set local MAC
	//value was stored in string type
	*(buf+13)=mac_array[0]; 
	*(buf+14)=mac_array[1];
	*(buf+15)=mac_array[2];
	*(buf+16)=mac_array[3];
	*(buf+17)=mac_array[4];
	*(buf+18)=mac_array[5];


	crc=get_data_crc(buf+13,6); //data byte crc 


	//DATA CRC
	*(buf+19)=crc;

return 20;
}

static void* ExternalTask(void* arg)
{

	char recvbuf[256];
	
	char reply_buf[256];
	int len = 0;
	int replylen;

   	printf("Start RS485 Interrupt mode test !\n");

	RS485_OBJ *pRs485Info = (RS485_OBJ*)malloc(sizeof(RS485_OBJ));
	pRs485Info->port = TEST_ITH_PORT;
	pRs485Info->parity = 0;
	pRs485Info->txPin = TEST_GPIO_TX;
	pRs485Info->rxPin = TEST_GPIO_RX;
	pRs485Info->enPin = TEST_GPIO_EN;
	pRs485Info->baud = TEST_BAUDRATE;
	pRs485Info->timeout = 0;
	pRs485Info->mode = UART_INTR_MODE;
	pRs485Info->forDbg = false;

	itpRegisterDevice(TEST_PORT, &TEST_DEVICE);

	ioctl(TEST_PORT, ITP_IOCTL_INIT, (void*)pRs485Info);
	ioctl(TEST_PORT, ITP_IOCTL_REG_RS485_CB, (void*)RS485Callback);
	//ioctl(TEST_PORT, ITP_IOCTL_REG_RS485_DEFER_CB , (void*)RS485Callback);

	sem_init(&RS485SemIntr, 0, 0);


	while (!extQuit)
	{	
		sem_wait(&RS485SemIntr);

		{
			len = read(TEST_PORT, recvbuf , 256);
			printf("(RS485) len = %d,\n", len,recvbuf[0],recvbuf[1],recvbuf[2],recvbuf[3]);
			// HEADER 	TAG  LEN  DAT
			//	4 2 1    1    4    n
			if(recvbuf[7]==0x93 &&recvbuf[8]==0x00)
			{
					printf("(RS485)GET MAC CMD\n");
				//get mac cmd
				replylen= gen_mac_cmd(reply_buf);
				write(TEST_PORT, reply_buf , replylen);
			}
			
			if(recvbuf[7]==0x93 &&recvbuf[8]==0x01)
			{
					printf("(RS485)SET AP MODE ON\n");
				//get mac cmd
				replylen= gen_mac_cmd(reply_buf);
				write(TEST_PORT, reply_buf , replylen);
			}

		}

				
	}
    mq_close(extInQueue);
	mq_close(extOutQueue);
    extInQueue = -1;
	extOutQueue = -1;

    return NULL;
}

void ExternalInit(void)
{
    struct mq_attr qattr;

    qattr.mq_flags = 0;
    qattr.mq_maxmsg = EXT_MAX_QUEUE_SIZE;
    qattr.mq_msgsize = sizeof(ExternalEvent);

    extInQueue = mq_open("external_in", O_CREAT | O_NONBLOCK, 0644, &qattr);
    assert(extInQueue != -1);

    extOutQueue = mq_open("external_out", O_CREAT | O_NONBLOCK, 0644, &qattr);
    assert(extOutQueue != -1);

    extQuit = false;

#if defined(CFG_UART0_ENABLE) && !defined(CFG_DBG_UART0)
#ifdef _WIN32
	if(ComInit(WIN32_COM, CFG_UART0_BAUDRATE)) {
		//ComInit Error
		mq_close(extInQueue);
		mq_close(extOutQueue);
		extInQueue = -1;
		extOutQueue = -1;
		return;
	}
#else
	itpRegisterDevice(ITP_DEVICE_UART0, &itpDeviceUart0);
    ioctl(ITP_DEVICE_UART0, ITP_IOCTL_INIT, NULL);
#endif
#endif

    pthread_create(&extTask, NULL, ExternalTask, NULL);
}

void ExternalExit(void)
{
    extQuit = true;
    pthread_join(extTask, NULL);
}

int ExternalReceive(ExternalEvent* ev)
{
    assert(ev);

    if (extQuit)
        return 0;

    if (mq_receive(extInQueue, (char*)ev, sizeof(ExternalEvent), 0) > 0)
        return 1;

    return 0;
}

int ExternalSend(ExternalEvent* ev)
{
    assert(ev);

    if (extQuit)
        return -1;

    return mq_send(extOutQueue, (char*)ev, sizeof(ExternalEvent), 0);
}
