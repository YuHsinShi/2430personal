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

#define HEADER_PACKET_SIZE		4  //4 BYTE
#define HEADER_UART_NUMBER		1  //1 BYTE
#define HEADER_CMD		1  //4 BYTE



#define HEADER_SHIFT 		(HEADER_UART_NUMBER+HEADER_PACKET_SIZE+HEADER_CMD)

#define LOG_TIMESTAMP_LEN		12 //10//[12:34:56] //10 content with /0
#define HEADER_RESERVED 		LOG_TIMESTAMP_LEN+HEADER_SHIFT


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


#define LOG_WRITER_STATUS_IDLE 		0
#define LOG_WRITER_STATUS_BOOTING 	1
#define LOG_WRITER_STATUS_POWER_OFF 	2
#define LOG_WRITER_STATUS_END			3


#define LOG_WRITER_CMD_NORMAL 			0
#define LOG_WRITER_CMD_FAIL_BOOT 		1
#define LOG_WRITER_CMD_OK_BOOT 			2

#define LOG_WRITER_MODE_NORMAL		0
#define LOG_WRITER_MODE_POWER_ONOFF		1


struct logwrite_t {
    ITPDeviceType itp_uart_index;
	int mode;				//if 0: capture mode, 1; booting mode
	char folder_path[64];	//path for writing 
	char file_name;			//the name of latest log
	unsigned int num; 				//num of the log 
	FILE* handle; 			//handle NULL not open or closed
	unsigned int byte_counter; 		//counting byte to END the opened file
	unsigned int last_time; 			//last time to generate system time string

	
	unsigned int alive_flag; // alive flag will increase 
	
	// for PowerOnOff mode
	unsigned int power_gpio;
	unsigned int action_time; 
	unsigned int elapse_time;
	unsigned int status;
	unsigned int booting_count; 
	unsigned int fail_count;
	unsigned int total_count;
//time
};

struct logwrite_t log_writer[6]={0};

unsigned int log_writer_get_alive_count(int i)
{
	return log_writer[i].alive_flag;
}


void log_writer_alive_reset(int i)
{
 	log_writer[i].alive_flag=0;
}

int log_writer_get_number_index(char index)
{
	return log_writer[index].num;
}

void log_writer_get_current_file_handle(char index)
{
	char filepath_tmp[64];
//	snprintf(filepath_tmp,64,"%s%d.log",log_writer[index].folder_path,log_writer[index].num);
	snprintf(filepath_tmp,64,"E:/ch%.1d/%d.log",index+1,log_writer[index].num);
//printf("%s\n",filepath_tmp);
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





void creat_writer_folder()
{
	int i;

	int ret;
	char tmp[8] = { 0 };
	for (i = 0; i<5; i++)
	{
		snprintf(tmp, 8, "E:/ch%1.d", i + 1);
		ret = mkdir(tmp, 0700);
	}
	return;
}
static void* WritingTask(void* arg)
{
	//LogFileInit("E:/log.txt")

	printf("WritingTask start\n");

	char wbf[2048];
	int wrlen;
	int i;
	char index;
	char cmd;



// 
	creat_writer_folder();

	while(!WritingQuit)
	{
		if ( (mq_receive(extOutQueue, wbf,0, 0) )> 0)
		{
			memcpy(&wrlen,wbf,4);
			index= wbf[4];
			cmd= wbf[5];

			//DBG("WR %d , %d/%d byte..\r\n",index, wrlen,log_writer[index].byte_counter );
			
			/*
			
						DBG("wri: %d byte..\r\n", wrlen,wbf[HEADER_SHIFT]);
						for(i=0;i<wrlen;i++){
							DBG("0x%x ", wbf[i+HEADER_SHIFT]);
						}
							DBG("\n");
			
			*/
				if(LOG_WRITER_CMD_FAIL_BOOT==cmd) //jump to next 
				{				
					log_writer_get_next_file_handle(index);
				}
				else if (LOG_WRITER_CMD_OK_BOOT == cmd) //jump to next 
				{
					log_writer_get_next_file_handle(index);
				}				
				else
				{
					if(NULL ==	log_writer[index].handle)
					{
						//ERR("file handle not found(ch%d) \n",index);
						log_writer_get_current_file_handle(index);  //handle is not opened when initail , do reopen				
					}
					else
					{
				        fwrite(&wbf[HEADER_SHIFT], 1, wrlen, 	log_writer[index].handle);
						//TODO: error handle if writing is failed
						
						log_writer[index].byte_counter+=wrlen;
				        fflush(log_writer[index].handle);
						if(log_writer[index].byte_counter > uart[index].fileMaxsize *1024)
						{
							DBG("ch %d write finished. \r\n",index);
							log_writer[index].byte_counter=0;
							log_writer_get_next_file_handle(index);
						}


					}

				}

		}
		else
		{


		}

		
		usleep(10000);
	}

	for(index=0;index<5;index++)
	{
		if(NULL == log_writer[index].handle)
			printf("non exist handle\n");
		else
			fclose(log_writer[index].handle);


		log_writer[index].handle =NULL;

	}	
//	DBG("wri: %d byte finished\r\n", counter);
}

void header_set(char* buf,int readLen,char channel,char cmd)
{
	memcpy(buf,&readLen,4); //BYTE 0-3
	*(buf+4)= channel;		//BYTE 4					
	*(buf+5)= cmd;		//BYTE 5

}


void get_fail_total_stringcount(int channel,char* buffer)
{
	snprintf(buffer, 64, "%d/%d", log_writer[channel - 1].booting_count, log_writer[channel - 1].total_count);
	
}

unsigned int get_fail_count(int channel)
{
	return log_writer[channel - 1].fail_count;
}

unsigned int get_elapsed_time_channel(int channel)
{
	return log_writer[channel - 1].elapse_time;
}


static void* ReadUartToLogTask(void* arg)
{
int i;
int readLen;
char index;
char* pos;
char cmd=0;
	printf("ExternalTask start\n");

    while (!extQuit)
    {
		//power control if this is in booting mode
		
		for(index=0;index<5;index++)
		{
		
			log_writer[index].elapse_time = (SDL_GetTicks() - log_writer[index].action_time)/1000;

			if(LOG_WRITER_MODE_POWER_ONOFF == log_writer[index].mode)
			{
				if(LOG_WRITER_STATUS_IDLE == log_writer[index].status) //do booting
				{
					//power on 			
					ithGpioSetOut(log_writer[index].power_gpio);
					ithGpioSet(log_writer[index].power_gpio);
					
					log_writer[index].status=LOG_WRITER_STATUS_BOOTING; //booting
					log_writer[index].action_time = SDL_GetTicks();
					log_writer[index].alive_flag=0;
					log_writer[index].booting_count++;
					if (log_writer[index].booting_count >= log_writer[index].total_count )
						log_writer[index].status = LOG_WRITER_STATUS_END;

				}
				else if(LOG_WRITER_STATUS_BOOTING == log_writer[index].status)	
				{			
				
					if (log_writer[index].elapse_time >= 30 )
					{
						ithGpioSetOut(log_writer[index].power_gpio);
						ithGpioClear(log_writer[index].power_gpio); //power off 

						//clear rest unread buffer 
						read(log_writer[index].itp_uart_index , &inDataBuf[HEADER_RESERVED], EXTERNAL_BUFFER_SIZE);

						header_set(inDataBuf,0,index,LOG_WRITER_CMD_FAIL_BOOT);//force to end this file handle
						mq_send(extOutQueue, inDataBuf,HEADER_SHIFT, 0);
						
						log_writer[index].fail_count++;
						log_writer[index].status=LOG_WRITER_STATUS_POWER_OFF; //booting
						log_writer[index].action_time = SDL_GetTicks();
					}
				}
				else  if(LOG_WRITER_STATUS_POWER_OFF == log_writer[index].status)
				{
					//power off time 
					if (log_writer[index].elapse_time >= 3 )
					{
						log_writer[index].status=LOG_WRITER_STATUS_IDLE; //exceed the resting time do booting
					}

				}
				else  if (LOG_WRITER_STATUS_END == log_writer[index].status)
				{
					continue;
				}
				
				else
				{

					ithPrintf("Unknow log wiriter state [0x%x]",log_writer[index].status);
				}

			}

		}
			//for log caputure.
		for(index=0;index<5;index++)
		{
				memset(inDataBuf, 0, EXTERNAL_BUFFER_SIZE); 
				// Read data from UART port
				
				readLen = read(	log_writer[index].itp_uart_index , &inDataBuf[HEADER_RESERVED], EXTERNAL_BUFFER_SIZE);
				/*
				if(readLen > 0)
				{
				
					DBG("UART RD: ch %d: %d byte(%d)..(%x,%x)\r\n",index, readLen,readLen,log_writer[index].itp_uart_index,ITP_DEVICE_UART1);
					for(i=0;i<readLen;i++){
					printf("0x%x ", inDataBuf[i+HEADER_RESERVED]);
					}
					printf("\n");
					
					continue;
				}
				*/
				if(readLen > 0)
				{
					log_writer[index].alive_flag++; // uart aive
					
						//check if one second is passsed->record the log
						if (SDL_GetTicks() - log_writer[index].last_time >= 1000)
						{

							if(uart[index].timestamp)
							{					
								unsigned int min, hour, sec;
								unsigned int total;
								total = get_elapsed_total_seconds();
								min = get_min_passed(total);
								hour = get_hour_passed(total);
								sec = get_sec_passed(total);
							
								snprintf(&inDataBuf[HEADER_RESERVED-LOG_TIMESTAMP_LEN],LOG_TIMESTAMP_LEN,"\n[%.2d:%.2d:%.2d]\n",hour,min,sec);
								inDataBuf[HEADER_RESERVED-1]=0xa;//force to change line
//								printf("TIME= %s",&inDataBuf[HEADER_SHIFT];
								pos=&inDataBuf[HEADER_RESERVED-HEADER_SHIFT-LOG_TIMESTAMP_LEN];

								readLen+=LOG_TIMESTAMP_LEN;

								log_writer[index].last_time=SDL_GetTicks();
							}
							else
							{
								pos=&inDataBuf[HEADER_RESERVED-HEADER_SHIFT];

							}
							
						}
						else
						{
							pos=&inDataBuf[HEADER_RESERVED-HEADER_SHIFT];
						}
					/*
					memcpy(pos,&readLen,4); //BYTE 0-3
					*(pos+4)= index;		//BYTE 4					
					*(pos+5)= cmd;		//BYTE 5
					*/

					header_set(pos,readLen,index,cmd);

					//DBG("read ch %d: %d byte(%d)..\r\n",index, readLen,readLen);
						
					 mq_send(extOutQueue, pos,readLen, 0);

					 if (LOG_WRITER_MODE_POWER_ONOFF == log_writer[index].mode)
					 {
						 //searching
						 if (NULL != strstr(pos, "itu loading time"))
						 {
							 printf("BOOTING OK~	\n");
							 ithGpioSetOut(log_writer[index].power_gpio);
							 ithGpioClear(log_writer[index].power_gpio); //power off 
							 log_writer[index].status = LOG_WRITER_STATUS_POWER_OFF; //booting
							 log_writer[index].action_time = SDL_GetTicks();
							 header_set(inDataBuf, 0, index, LOG_WRITER_CMD_OK_BOOT);//force to end this file handle
							 mq_send(extOutQueue, inDataBuf, HEADER_SHIFT, 0);
						 }
					 }

				}
		}
        usleep(5000);
    }

/*
    mq_close(extInQueue);
	mq_close(extOutQueue);
    extInQueue = -1;
	extOutQueue = -1;
*/
    return NULL;
}




void init_log_writer()
{
	int index;
	
	
	log_writer[0].itp_uart_index=ITP_DEVICE_UART1;
	log_writer[1].itp_uart_index=ITP_DEVICE_UART2;
	log_writer[2].itp_uart_index=ITP_DEVICE_UART3;
	log_writer[3].itp_uart_index=ITP_DEVICE_UART4;
	log_writer[4].itp_uart_index=ITP_DEVICE_UART5;



	log_writer[0].power_gpio=27;
	log_writer[1].power_gpio=28;
	log_writer[2].power_gpio=29;
	log_writer[3].power_gpio=30;
	log_writer[4].power_gpio=31;


	for(index=0;index<5;index++)
	{
	
		log_writer[index].num=1;

		log_writer[index].last_time=SDL_GetTicks();
		if(NULL != log_writer[index].handle)
		{
			fclose(log_writer[index].handle);
		}


		log_writer[index].alive_flag	= 0;
		log_writer[index].byte_counter	= 0;		
		log_writer[index].num			= 0;	
		log_writer[index].fail_count	= 0;
		log_writer[index].booting_count = 0;
		log_writer[index].total_count = 100;

	}
	

	

}





void log_writer_normal_mode()
{
int index;
	for(index=0;index<5;index++)
	{
		 log_writer[index].mode = LOG_WRITER_MODE_NORMAL;
	}

}
void log_writer_poweron_mode()
{
int index;
	for(index=0;index<5;index++)
	{
		 log_writer[index].mode = LOG_WRITER_MODE_POWER_ONOFF;
	}

}


void log_writer_stop()
{
printf("log_writer_stop \n");
   extQuit = true;
   WritingQuit=true;
   pthread_join(extTask, NULL);
   pthread_join(WrTask, NULL);


}

void log_writer_start()
{

		init_log_writer();

		set_timecounter_start();//start to cout the elapsed time


	    pthread_create(&extTask, NULL, ReadUartToLogTask, NULL);

	    pthread_create(&WrTask, NULL, WritingTask, NULL);

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
