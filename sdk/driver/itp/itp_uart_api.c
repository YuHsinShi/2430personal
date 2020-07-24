#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#ifndef _WIN32
#include <unistd.h>
#endif

#ifdef _WIN32
#include <windows.h>

#define UART_BUFFER_SIZE	512
#define COMNUM_LIMIT			4

typedef struct HCOM_OBJ
{
	HANDLE hCom;
	int ComNum;
} hcom_obj;

static hcom_obj ghCom[4];

#endif

int ComInit(int ComNum, int Baudrate)
{
#ifdef _WIN32
	HANDLE *hCom = NULL;
	char ComName[16];
	COMMTIMEOUTS timeout;
	DCB dcb = { 0 };
	int i = 0;

	if (ComNum == 0) {
		printf("ComPort should start from 1.\n");
		return -1;
	}

	if (ComNum > 9) {
		sprintf(ComName, "\\\\.\\COM%d", ComNum);
	}
	else {
		sprintf(ComName, "COM%d", ComNum);
	}

	printf("ComInit start, ComName: %s\n", ComName);

	while (i < COMNUM_LIMIT) {
		if (ghCom[i].ComNum == 0) {
			hCom = &ghCom[i].hCom;
			ghCom[i].ComNum = ComNum;
			break;
		}
		i++;
	}

	if (i > COMNUM_LIMIT) {
		printf("Exceed ComLimit(4)\n");
		return -1;
	}

	*hCom = CreateFileA(ComName, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (*hCom == INVALID_HANDLE_VALUE) {
		printf("Cannot open Com\n");
		return -1;
	}

	if (!SetupComm(*hCom, UART_BUFFER_SIZE, UART_BUFFER_SIZE)) {
		//Set read and write buffer size is 512 bytes.
		printf("Cannot set in/out buffer size\n");
		CloseHandle(*hCom);
		return -1;
	}

	//Set up timemout for read and write
	timeout.ReadIntervalTimeout = 10;
	timeout.ReadTotalTimeoutMultiplier = 0;
	timeout.ReadTotalTimeoutConstant = 500;
	timeout.WriteTotalTimeoutMultiplier = 0;
	timeout.WriteTotalTimeoutConstant = 500;

	if (!SetCommTimeouts(*hCom, &timeout)) {
		printf("Cannot set timeout\n");
		CloseHandle(*hCom);
		return -1;
	}

	if (!GetCommState(*hCom, &dcb)) {
		printf("Cannot get dcb: %d\n", GetLastError());
		CloseHandle(*hCom);
		return -1;
	}

	//Port Configuration
	dcb.BaudRate = Baudrate;
	dcb.ByteSize = 8;
	dcb.Parity = NOPARITY;
	dcb.StopBits = ONESTOPBIT;

	if (!SetCommState(*hCom, &dcb)) {
		printf("Cannot set dcb: %d\n", GetLastError());
		CloseHandle(*hCom);
		return -1;
	}

	//PurgeComm(*hCom, PURGE_RXCLEAR | PURGE_TXCLEAR);  //Flush TX/RX buffer

	printf("ComInit OK, ComNum=%d Baudrate=%d\n", ComNum, Baudrate);

#endif
	return 0;
}

void ComExit(int ComNum)
{
#ifdef _WIN32
	int i = 0;

	printf("ComExit\n");
	while (i < COMNUM_LIMIT) {
		if (ComNum == ghCom[i].ComNum) {
			CloseHandle(ghCom[i].hCom);
			ghCom[i].ComNum = 0;
			break;
		}
		i++;
	}
#endif
}

//Uart Read
int UartReceive(int ComNum, char *read_str, int len)
{
	int ret = 0;
#ifdef _WIN32
	HANDLE *hCom = NULL;
	DWORD read_byte = 0;
	int i = 0;

	while (i < COMNUM_LIMIT) {
		if (ComNum == ghCom[i].ComNum) {
			hCom = &ghCom[i].hCom;
			break;
		}
		i++;
	}

	if (i > COMNUM_LIMIT || ComNum == 0) {
		printf("Cannot find ComPort\n");
		return 0;
	}

	if (!ReadFile(*hCom, read_str, len, &read_byte, NULL)) {
		printf("Read Error\n");
		return 0;
	}
#if 1
    //if use read_bype as ret, must notice to add extra-handle for read_str
    //because maybe read_str is empty, read_bype > 0.
    ret = read_byte;
#else
	//should not return read_byte, becasue read_byte cannot restore immediately
    //NOTICE: strlen cannot parse "00"
	ret = strlen(read_str);
#endif
#else
	ret = read(ComNum, read_str, len);
#endif
	return ret;
}

//Uart Write
int UartSend(int ComNum, char *write_str, int len)
{
	int ret = 0;
#ifdef _WIN32
	HANDLE *hCom = NULL;
	DWORD written_byte = 0;
	int i = 0;

	while (i < COMNUM_LIMIT) {
		if (ComNum == ghCom[i].ComNum) {
			hCom = &ghCom[i].hCom;
			break;
		}
		i++;
	}

	if (i > COMNUM_LIMIT || ComNum == 0) {
		printf("Cannot find ComPort\n");
		return 0;
	}

	if (!WriteFile(*hCom, write_str, len, &written_byte, NULL)) {
		printf("Write Error\n");
		return 0;
	}

	ret = written_byte;
#else
	ret = write(ComNum, write_str, len);
#endif
	return ret;
}
