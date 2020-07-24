#include <sys/ioctl.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <malloc.h>
#include "ite/itp.h"
#include "nor/mmp_nor.h"
#include "ssp/mmp_spi.h"

#ifndef _WIN32
#define FUNC_MSG(format, args...)	printf("%s[%d]: "format, __FUNCTION__, __LINE__, ##args)
#else
#define FUNC_MSG(x)	printf(x)
#endif
#define FUNC_ENTRY					printf("%s()[%d] Enter\n", __FUNCTION__, __LINE__)
#define FUNC_LEAVE					printf("%s()[%d] LEAVE\n", __FUNCTION__, __LINE__)

#define SPI_SEL	SPI_0

#define SLAVE_TEST_SIZE 256

uint8_t slave_cmd[SLAVE_TEST_SIZE] = {0};

bool slaveCallbackFunc(uint32_t inData)
{
    static uint32_t callback_index = 0;
    static uint32_t freerun_index = 0;

    if (freerun_index % 2)
    {
        freerun_index = 0;
        return true;
    }
    freerun_index++;

    if ((uint8_t)inData == slave_cmd[callback_index % SLAVE_TEST_SIZE])
    {
        printf("SPI slave receive 0x%08X.\n", (uint8_t)inData);
    }
    else
    {
        printf("SPI slave receive(error) 0x%08X.\n", (uint8_t)inData);
        while(1) sleep(1);
    }

    callback_index++;
    callback_index %= SLAVE_TEST_SIZE;

    return true;
}

void spi_test_master()
{
    mmpSpiInitialize(SPI_0, SPI_OP_MASTR, CPO_0_CPH_0, SPI_CLK_10M);
    mmpSpiInitialize(SPI_1, SPI_OP_SLAVE, CPO_0_CPH_0, SPI_CLK_10M);
    mmpSpiSetSlaveCallbackFunc(SPI_1, slaveCallbackFunc);
    uint32_t i = 0;
    for (i = 0; i < SLAVE_TEST_SIZE; i++)
		slave_cmd[i] = i;
    sleep(1);

    uint32_t send_index = 0;
    uint8_t master_recv_buffer = 0;
    while(1)
    {
        //mmpSpiPioWrite(SPI_0, slave_cmd + send_index, 1, 0, 0, 0);
        master_recv_buffer = 0;
        mmpSpiPioRead(SPI_0, slave_cmd + send_index, 1, &master_recv_buffer, 1, 0);

        if (master_recv_buffer == *(slave_cmd + send_index))
        {
            printf("SPI master receive 0x%08X.\n", master_recv_buffer);
        }
        else
        {
            printf("SPI master receive(error) 0x%08X.\n", master_recv_buffer);
            while(1) sleep(1);
        }

        send_index++;
        send_index %= SLAVE_TEST_SIZE;
        usleep(200000);
    }
}

void nor_test_insanity_write()
{
	uint32_t	norSize = NorGetCapacity(SPI_SEL, SPI_CSN_0);
	uint32_t	writeOffset = 0;
	uint32_t	writeSize = 0;
	uint32_t	i = 0;

	printf("\n========================================\n");
	printf("   %s\n", __FUNCTION__);
	printf("========================================\n");	
	for (writeOffset = 0; writeOffset < norSize; writeOffset++)
	{
		for (writeSize = 1; writeSize < (norSize - writeOffset); writeSize++)
		{
			uint32_t	bufferSize = writeSize;
			uint8_t*	buffer = (uint8_t*)memalign(32, bufferSize);
			uint8_t*	readBackRuffer = (uint8_t*)memalign(32, bufferSize);

			if (buffer)
			{
				for (i = 0; i < bufferSize; i++)
				{
					buffer[i] = (uint8_t)('g' + writeSize);
				}
				printf("\tWrite %d Bytes to address 0x%02X......", writeSize, writeOffset);
				if (NorWrite(SPI_SEL, SPI_CSN_0, writeOffset, buffer, bufferSize))
				{
					printf("fail.\n");
					while(1);
				}
				else
				{
					printf("success.\n");
				}
			}
			else
			{
				FUNC_MSG("Out of memory.\n");
				while(1);
			}

			if (readBackRuffer)
			{
				printf("\tRead %d Bytes from address 0x%02X......", writeSize, writeOffset);
				if (NorRead(SPI_SEL, SPI_CSN_0, writeOffset, readBackRuffer, bufferSize))
				{
					printf("fail.\n");
					while(1);
				}
				else
				{
					printf("success.\n");
				}
			}
			else
			{
				FUNC_MSG("Out of memory.\n");
				while(1);
			}

			printf("\tData comparison......");
			if (memcmp(buffer, readBackRuffer, bufferSize) == 0)
			{
				printf("ok.\n");
			}
			else
			{
				printf("fail.\n");
				for(i=0; i< bufferSize; i++)
					printf("buffer[%d]=0x%x, readBackRuffer[%d]=0x%x\n", i, buffer[i], i, readBackRuffer[i]);
				while(1);
			}
			printf("\n");

			free(buffer);
			free(readBackRuffer);
		}
	}
}

void nor_test_write_without_erase()
{
	uint32_t	i = 0;
	uint32_t	norSize = NorGetCapacity(SPI_SEL, SPI_CSN_0);
	uint32_t	address = 0;
	uint32_t	bufferSize = 256;
	uint8_t*	buffer = NULL;
	uint8_t*	readBackBuffer = NULL;
	bool		result = true;

	printf("\n========================================\n");
	printf("   %s\n", __FUNCTION__);
	printf("========================================\n");

	buffer = (uint8_t*)memalign(32, bufferSize);
	if (buffer == NULL)
	{
		FUNC_MSG("Out of memory.\n");
		return;
	}
	readBackBuffer = (uint8_t*)memalign(32, norSize);
	if (readBackBuffer == NULL)
	{
		FUNC_MSG("Out of memory.\n");
		return;
	}

	for (i = 0; i < bufferSize; i++)
	{
		buffer[i] = 'z' + i;
	}
	printf("\tErase all block.\n");
	if (NorEraseAll(SPI_SEL, SPI_CSN_0))
	{
		FUNC_MSG("NorEraseAll() fail.");
	}
	printf("\tWrite 256 Bytes, from 0 to 0x%08X", norSize - 1);
	address = 0;
	while(address < norSize)
	{
		if (NorWriteWithoutErase(SPI_SEL, SPI_CSN_0, address, buffer, bufferSize))
		{
			FUNC_MSG("NorWriteWithoutErase() fail.\n");
		}
		address += bufferSize;
		if ((address % 65536) == 0)
		{
			printf("."); fflush(stdout);
		}
	}
	printf("success.\n");

	printf("\tRead back to validation...");
	if (NorRead(SPI_SEL, SPI_CSN_0, 0, readBackBuffer, norSize))
	{
		FUNC_MSG("NorRead() fail.\n");
	}
	for (i = 0; i < norSize; i++)
	{
		if (readBackBuffer[i] != ((uint8_t)('z' + i)))
		{
			printf("fail.\n");
			printf("\tByte %d should be 0x%02X, but it was 0x%02X.\n", i, ((uint8_t)('z' + i)), readBackBuffer[i]);
			result = false;
			break;
		}
	}
	if (result == true)
	{
		printf("success.\n");
	}
	free(readBackBuffer);
}

void nor_test_erase_all()
{
	uint32_t	bufferSize = NorGetCapacity(SPI_SEL, SPI_CSN_0);
	uint8_t*	readBackBuffer = (uint8_t*)memalign(32, bufferSize);
	uint32_t	i = 0;
	bool		testResult = true;
	printf("\n========================================\n");
	printf("   %s\n", __FUNCTION__);
	printf("========================================\n");
	printf("\tErase all block.\n");
	if (NorEraseAll(SPI_SEL, SPI_CSN_0))
	{
		FUNC_MSG("NorEraseAll() fail.");
	}
	printf("\tRead data back, all bytes should be 0xFF");
	if (NorRead(SPI_SEL, SPI_CSN_0, 0, readBackBuffer, bufferSize))
	{
		FUNC_MSG("NorRead() fail.");
	}
	for (i = 0; i < bufferSize; i++)
	{
		if (readBackBuffer[i] != 0xFF)
		{
			printf("..........failed. Data[%d] != 0xFF, All bytes should be 0xFF!\n", i);
			testResult = false;
			break;
		}
	}
	if (testResult == true)
	{
		printf("..........success.\n");
	}
	free(readBackBuffer);
}

void nor_write_test3()
{
	const uint32_t TEST_BUFFER_SIZE = 512 * 1024;

	uint8_t*	writeBuffer = (uint8_t*)memalign(32, TEST_BUFFER_SIZE);
	uint8_t*	readBackBuffer = (uint8_t*)memalign(32, TEST_BUFFER_SIZE);
	uint32_t	i = 0;
	uint32_t	writeAddress = 0;
	struct timeval	t1, t2;
	double		elapsedTime;

	printf("\n========================================\n");
	printf("   NOR: Simple Write Test\n");
	printf("========================================\n");
	for (i = 0; i < TEST_BUFFER_SIZE; i++)
	{
		writeBuffer[i] = 'B' + i;
	}

	writeAddress = 0;
	gettimeofday(&t1, NULL);
	if (NorWrite(SPI_SEL, SPI_CSN_0, writeAddress, writeBuffer, TEST_BUFFER_SIZE))
	{
		FUNC_MSG("NorWrite() fail.\n");
	}
	gettimeofday(&t2, NULL);
	elapsedTime = (t2.tv_sec - t1.tv_sec) * 1000.0;			// sec to ms
    elapsedTime += ((t2.tv_usec - t1.tv_usec) / 1000.0);	// us to ms
    printf("\tWrite %.2f MB, take %.2f ms, %.2f MB/S\n", (double)TEST_BUFFER_SIZE/1024.0/1024.0, elapsedTime, (double)TEST_BUFFER_SIZE/1024.0/1024.0/(elapsedTime / 1000));
    gettimeofday(&t1, NULL);
	if (NorRead(SPI_SEL, SPI_CSN_0, writeAddress, readBackBuffer, TEST_BUFFER_SIZE))
	{
		FUNC_MSG("NorRead() fail.\n");
	}
	gettimeofday(&t2, NULL);
	elapsedTime = (t2.tv_sec - t1.tv_sec) * 1000.0;			// sec to ms
    elapsedTime += ((t2.tv_usec - t1.tv_usec) / 1000.0);	// us to ms
    printf("\tRead %.2f MB, take %.2f ms, %.2f MB/S\n", (double)TEST_BUFFER_SIZE/1024.0/1024.0, elapsedTime, (double)TEST_BUFFER_SIZE/1024.0/1024.0/(elapsedTime / 1000));
	if (memcmp(readBackBuffer, writeBuffer, TEST_BUFFER_SIZE) != 0)
	{
		uint32_t printCount = 0;

		printf("\t....................data compare fail.\n");
	}
	else
	{
		printf("\t....................success.\n");
	}
	free(writeBuffer);
	free(readBackBuffer);
}

void nor_write_test2()
{
	const uint32_t TEST_BUFFER_SIZE = 4 * 1024 * 1024;

	uint8_t*	writeBuffer = (uint8_t*)memalign(32, TEST_BUFFER_SIZE);
	uint8_t*	readBackBuffer = (uint8_t*)memalign(32, TEST_BUFFER_SIZE);
	uint32_t	i = 0;
	uint32_t	writeAddress = 0;
	struct timeval	t1, t2;
	double		elapsedTime;

	printf("\n========================================\n");
	printf("   NOR: Performance Test\n");
	printf("========================================\n");
	for (i = 0; i < TEST_BUFFER_SIZE; i++)
	{
		writeBuffer[i] = 'A' + i;
	}

	writeAddress = 0;
	gettimeofday(&t1, NULL);
	if (NorWrite(SPI_SEL, SPI_CSN_0, writeAddress, writeBuffer, TEST_BUFFER_SIZE))
	{
		FUNC_MSG("NorWrite() fail.\n");
	}
	gettimeofday(&t2, NULL);
	elapsedTime = (t2.tv_sec - t1.tv_sec) * 1000.0;			// sec to ms
    elapsedTime += ((t2.tv_usec - t1.tv_usec) / 1000.0);	// us to ms
    printf("\tWrite %.2f MB, take %.2f ms, %.2f MB/S\n", (double)TEST_BUFFER_SIZE/1024.0/1024.0, elapsedTime, (double)TEST_BUFFER_SIZE/1024.0/1024.0/(elapsedTime / 1000));
    gettimeofday(&t1, NULL);
	if (NorRead(SPI_SEL, SPI_CSN_0, writeAddress, readBackBuffer, TEST_BUFFER_SIZE))
	{
		FUNC_MSG("NorRead() fail.\n");
	}
	gettimeofday(&t2, NULL);
	elapsedTime = (t2.tv_sec - t1.tv_sec) * 1000.0;			// sec to ms
    elapsedTime += ((t2.tv_usec - t1.tv_usec) / 1000.0);	// us to ms
    printf("\tRead %.2f MB, take %.2f ms, %.2f MB/S\n", (double)TEST_BUFFER_SIZE/1024.0/1024.0, elapsedTime, (double)TEST_BUFFER_SIZE/1024.0/1024.0/(elapsedTime / 1000));
	if (memcmp(readBackBuffer, writeBuffer, TEST_BUFFER_SIZE) != 0)
	{
		uint32_t printCount = 0;

		printf("\t....................data compare fail.\n");
	}
	else
	{
		printf("\t....................success.\n");
	}
	free(writeBuffer);
	free(readBackBuffer);
}

bool nor_write_test1()
{
    const uint32_t TEST_BUFFER_SIZE = 2097152;//256 * 1024;

	uint8_t*	writeBuffer = (uint8_t*)memalign(32, TEST_BUFFER_SIZE);
	uint8_t*	readBackBuffer = (uint8_t*)memalign(32, TEST_BUFFER_SIZE);
	uint32_t	i = 0;
	uint32_t	writeAddress = 0;
    //uint32_t    addressOffsets[] = {0, 1, 2 + 65535, 3 + 65535 * 2};
    //uint32_t    addressOffsets[] = {0};
    uint32_t    addressOffsets[] = {0, 2097152, 2097152*2, 2097152*3, 2097152*4, 2097152*5, 2097152*6, 2097152*7,
	                               2097152*8, 2097152*9, 2097152*10, 2097152*11, 2097152*12, 2097152*13, 2097152*14, 2097152*15};
	uint32_t	addressOffsetIndex = 0;
	bool        result = true;

	printf("\n========================================\n");
	printf("   NOR: Data Validation\n");
	printf("========================================\n");
	printf("\tInit write buffer\n");
	for (i = 0; i < TEST_BUFFER_SIZE; i++)
	{
		writeBuffer[i] = 'a' + i;
	}
	for (addressOffsetIndex = 0; addressOffsetIndex < (sizeof(addressOffsets) / sizeof(addressOffsets[0])); addressOffsetIndex++)
	{
        printf("=============addressOffsetIndex: %d======================\n", addressOffsetIndex);
		writeAddress = addressOffsets[addressOffsetIndex];
		printf("\tWrite test buffer to address 0x%08X\n", writeAddress);
		if (NorWrite(SPI_SEL, SPI_CSN_0, writeAddress, writeBuffer, TEST_BUFFER_SIZE))
		{
			FUNC_MSG("NorWrite() fail.\n");
			result = false;
			goto end;
		}
		printf("\tRead back for verify");
		memset(readBackBuffer, 0x00, TEST_BUFFER_SIZE);
		if (NorRead(SPI_SEL, SPI_CSN_0, writeAddress, readBackBuffer, TEST_BUFFER_SIZE))
		{
			FUNC_MSG("NorRead() fail.\n");
			result = false;
			goto end;
		}
		if (memcmp(readBackBuffer, writeBuffer, TEST_BUFFER_SIZE) != 0)
		{
			uint32_t printCount = 0;
			printf("....................fail.\n");
			result = false;	
            //goto end;
			for (i = 0; i < TEST_BUFFER_SIZE; i++)
			{
				if (readBackBuffer[i] != writeBuffer[i])
				{
                    printf("readBackBuffer[%d](0x%02X) != writeBuffer[%d](0x%02X)\n", i, readBackBuffer[i], i, writeBuffer[i]);
					printCount++;
				}
				if (printCount >= 32)
				{
									
					break;
				}
			}
		}
		else
		{
			printf("....................success.\n");
		}
	}
	end:
	free(writeBuffer);
	free(readBackBuffer);
	return result;
}

/*
void test_nor()
{
	uint32_t	i = 0;
	uint32_t	j = 0;
	uint32_t	dataSize = 1024 * 1024;
	uint8_t*	data = (uint8_t*)memalign(32, dataSize);
	uint32_t	offsetIndex = 1;

	mmpSpiInitialize(SPI_SEL, SPI_OP_MASTR);
	NorInitial(SPI_SEL, SPI_CSN_0);
	NorRead(SPI_SEL, SPI_CSN_0, offsetIndex, data, dataSize);

	for (i = 0; i < dataSize; i++)
	{
		if (data[i] != (uint8_t)(i + offsetIndex))
		{
			printf("error! data[%d] = 0x%02X, should be 0x%02X", i, data[i], (uint8_t)(i + offsetIndex));
			break;
		}
	}
	printf("\n");

	NorErase(SPI_SEL, SPI_CSN_0, 65537, 1);

	NorRead(SPI_SEL, SPI_CSN_0, 0, data, dataSize);
	j = 0;
	for (i = 0; i < (256 * 1024); i++)
	{
		if (data[i] != 0xFF)
		{
			printf("data[%d] = 0x%02X\n", i, data[i]);
			j++;
			if (j > 32)
			{
				//break;
			}
		}
	}
	printf("\n");
}
*/

void* TestFunc(void* arg)
{
    //itpInit();

    //spi_test_master();

    //mmpSpiInitialize(SPI_SEL, SPI_OP_MASTR);
    NorInitial(SPI_SEL, SPI_CSN_0);

    /*NOR_PROTECT_ITEM ni[3] = {{0x300000, 100},
                              {0x500000, 500},
                              {0x1000000, 1}};

    if (NorSWProtect(SPI_SEL, SPI_CSN_0, sizeof(ni)/sizeof(NOR_PROTECT_ITEM), ni))
        printf("NorSWProtect fail\n");*/

    nor_write_test1();
    nor_write_test2();
    nor_write_test3();
    nor_test_erase_all();
    nor_test_write_without_erase();
    nor_test_insanity_write();

	return 0;
}
