#include <sys/ioctl.h>
#include <assert.h>
#include <mqueue.h>
#include <string.h>
#include <unistd.h>
#include "ite/itp.h"
#include "project.h"
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
static pthread_t WrTask;

static volatile bool extQuit;

static volatile bool WritingQuit;

static unsigned char inDataBuf[EXTERNAL_BUFFER_SIZE];
static unsigned char cmdBuf[EXTERNAL_BUFFER_SIZE];
static unsigned int cmdPos = 0;
static GET_UART_COMMAND_STATE gState = GET_HEADER;
static unsigned int payloadCount = 0;
static unsigned int checkSum = 0;


#define WRITING_DEBUG 	1
#define HEADER_SHIFT 	5

#if WRITING_DEBUG
#define DBG(fmt, ...)		fprintf(stderr, "[DBG] "fmt, ##__VA_ARGS__);
#else
#define DBG(fmt, ...)
#endif
#define ERR(fmt, ...)		fprintf(stderr, "[DBG] "fmt, ##__VA_ARGS__);


//check if usb existed
//check  space available
//read uart and write uart
//timeout buffer full
static FILE* wrhandle;



struct logwrite_t {
    ITPDeviceType itp_uart_index;
	char folder_path[64];
	char file_name;
	int num; //num of the log 
	int handle; //handle NULL not open or closed
	int byte_counter; //counting byte to END the opened file
//time
};

struct logwrite_t log_writer[6]={0};

void log_writer_get_current_file_handle(char index)
{
	char filepath_tmp[64];
//	snprintf(filepath_tmp,64,"%s%d.log",log_writer[index].folder_path,log_writer[index].num);
	snprintf(filepath_tmp,64,"E:\ch%d-%d.log",index,log_writer[index].num);

	log_writer[index].handle = fopen(filepath_tmp, "w+");
	if (NULL == log_writer[index].handle)
		printf("log_writer_get_current_file_handle can not open %s\n",filepath_tmp);

}

void log_writer_get_next_file_handle(char index)
{

	if(NULL == log_writer[index].handle)
		printf("non exist handle\n");
	else
		fclose(log_writer[index].handle);


	log_writer[index].handle =NULL;


	log_writer_get_current_file_handle(index);

	if(NULL == log_writer[index].handle)
		printf("log_writer_get_next_file_handle is NULL\n");
	else
		log_writer[index].num++;


}

static void* WritingTask(void* arg)
{
	//LogFileInit("E:/log.txt");

while(1)
{
    if(access("E:/", F_OK))
        break ;
	else
		usleep(1000);

}

	printf("WritingTask start\n");

	char wbf[2048];
	int wrlen;
	int i;
	char index;
	while(!WritingQuit)
	{
		if ( (mq_receive(extOutQueue, wbf,0, 0) )> 0)
		{
			memcpy(&wrlen,wbf,4);
/*

			DBG("wri: %d byte..\r\n", wrlen,wbf[HEADER_SHIFT]);
			for(i=0;i<wrlen;i++){
				DBG("0x%x ", wbf[i+HEADER_SHIFT]);
			}
				DBG("\n");

*/

			index= wbf[4];

			DBG("WR %d , %d byte..\r\n",index, wrlen);

			if(NULL !=	log_writer[index].handle)
			{
		        fwrite(&wbf[5], 1, wrlen, 	log_writer[index].handle);
				log_writer[index].byte_counter+=wrlen;
		        fflush(log_writer[index].handle);
				if(log_writer[index].byte_counter > 200*1024)
				{
					DBG("ch %d write finished. \r\n",index);
					log_writer[index].byte_counter=0;
					log_writer_get_next_file_handle(index);
				}
			}
			else
			{				//ERR("file handle not found(ch%d) \n",index);
				log_writer_get_current_file_handle(index);  //handle is not opened when initail , do reopen
			}
		}
		usleep(10000);
	}

	
//	DBG("wri: %d byte finished\r\n", counter);
}




void init_log_writer()
{
	int index;
	
	char filepath_tmp[64];
	
	log_writer[0].itp_uart_index=ITP_DEVICE_UART1;
	log_writer[1].itp_uart_index=ITP_DEVICE_UART2;
	log_writer[2].itp_uart_index=ITP_DEVICE_UART3;
	log_writer[3].itp_uart_index=ITP_DEVICE_UART4;
	log_writer[4].itp_uart_index=ITP_DEVICE_UART5;

	for(index=0;index<5;index++)
	{
	
		log_writer[index].num=1;
/*
		snprintf(log_writer[index].folder_path,64,"E:/ch%d/",index);
		
		

		log_writer[index].handle =NULL;
		
		snprintf(filepath_tmp,64,"%s%d.log",log_writer[index].folder_path,log_writer[index].num);
		
		printf("log_writer_get_next_file_handle %s\n",filepath_tmp);
		
		log_writer[index].handle = fopen(filepath_tmp, "w");

		if(NULL == log_writer[index].handle)
			printf("ERROR CH%d not opend\n");
*/
	}
	

	

}


static void* ExternalTask(void* arg)
{
int i;
int readLen;
char index;

	while(1)
	{
		if(access("E:/", F_OK))
			break ;
		else
			usleep(1000);
	
	}
	
	printf("ExternalTask start\n");


    while (!extQuit)
    {
		for(index=0;index<5;index++)
		{
				memset(inDataBuf, 0, EXTERNAL_BUFFER_SIZE); //four byte is header
				// Read data from UART port

				readLen = read(	log_writer[index].itp_uart_index , &inDataBuf[HEADER_SHIFT], EXTERNAL_BUFFER_SIZE);
				if(readLen > 0)
				{
				
					memcpy(inDataBuf,&readLen,4);
					inDataBuf[4]= index;
					DBG("read ch %d: %d byte..\r\n",index, readLen);
					
					/*
					for(i=0;i<readLen;i++){
					printf("0x%x ", inDataBuf[i+HEADER_SHIFT]);
					}
					printf("\n");
					*/
					mq_send(extOutQueue,inDataBuf,readLen+HEADER_SHIFT, 0);
				}
		}
        usleep(10000);
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
	WritingQuit = false;

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
	init_log_writer();


    pthread_create(&extTask, NULL, ExternalTask, NULL);

    pthread_create(&WrTask, NULL, WritingTask, NULL);


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
