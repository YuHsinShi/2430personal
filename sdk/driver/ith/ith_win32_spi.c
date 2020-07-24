#include "ith_cfg.h"
#include <stdio.h>
//#include "FTCSPI.h"
#include "ftdi/ftdi.h"
#include "ftdi/ftdispi.h"

#define MAX_FREQ_93LC56B_CLOCK_DIVISOR 3   // equivalent to 1.5MHz

static CRITICAL_SECTION       CriticalSection;
static struct ftdispi_context fsc;
#if 0
static FTC_HANDLE             ftHandle;
static FTC_CHIP_SELECT_PINS   ChipSelectsDisableStates;
static FTC_INPUT_OUTPUT_PINS  HighInputOutputPins;
static FTC_HIGHER_OUTPUT_PINS HighPinsWriteActiveStates;
static FTC_INIT_CONDITION     ReadStartCondition;
static FTC_WAIT_DATA_WRITE    WaitDataWriteComplete;
static FTC_INIT_CONDITION     WriteStartCondition;
#endif

#define MAX_READ_DATA_BYTES_BUFFER_SIZE 65536    // 64k bytes
typedef uint8_t ReadDataByteBuffer[MAX_READ_DATA_BYTES_BUFFER_SIZE];
typedef ReadDataByteBuffer *PReadDataByteBuffer;

#define MAX_WRITE_CONTROL_BYTES_BUFFER_SIZE 256    // 256 bytes
typedef uint8_t WriteControlByteBuffer[MAX_WRITE_CONTROL_BYTES_BUFFER_SIZE];
typedef WriteControlByteBuffer *PWriteControlByteBuffer;

#define MAX_WRITE_DATA_BYTES_BUFFER_SIZE 65536    // 64k bytes
typedef uint8_t WriteDataByteBuffer[MAX_WRITE_DATA_BYTES_BUFFER_SIZE];
typedef WriteDataByteBuffer *PWriteDataByteBuffer;

static uint8_t                vramBuffer[CFG_RAM_SIZE] = {0xCC};
static uint8_t                vramBufferCache[CFG_RAM_SIZE];

static int SpiClose(void)
{

    ftdispi_close(&fsc, 1);
    DeleteCriticalSection(&CriticalSection);

    return 0;
}

int SpiOpen(DWORD dwClockRate)
{

    INT Status = 0;
    INT clkrate = 0;
    INT ret = 0;
    struct ftdi_context             *ftdi;
    struct ftdi_version_info        version;

    clkrate = dwClockRate;


    InitializeCriticalSection(&CriticalSection);

    if ((ftdi = ftdi_new()) == 0)
    {
        printf("ftdi_new failed\n");
        return EXIT_FAILURE;
    }
    version = ftdi_get_library_version();

    if ((ret = ftdi_usb_open(ftdi, 0x0403, 0x6010)) < 0)
    {
        ///printf("unable to open ftdi device: %d (%s)\n", ret, ftdi_get_error_string(ftdi));
        ftdi_free(ftdi);
        return EXIT_FAILURE;
    }
    // Read out FTDIChip-ID of R type chips
    if (ftdi->type == TYPE_R)
    {
        unsigned int chipid;
        printf("ftdi_read_chipid: %d\n", ftdi_read_chipid(ftdi, &chipid));
        //printf("FTDI chipid: %X\n",      chipid);
    }

    if (clkrate > 6)
        clkrate = 6;
    if (clkrate <= 0)
        clkrate = 1;

    if ((Status = ftdispi_open(&fsc, ftdi, INTERFACE_A)) != 0)
        return EXIT_FAILURE;

    if ((Status = ftdispi_setmode(&fsc, 1, 0, 0, 0, 0, 0)) != 0)
        return EXIT_FAILURE;

    if ((Status = ftdispi_setclock(&fsc, clkrate * 1000000)) != 0)
        return EXIT_FAILURE;

    if ((Status = ftdispi_setloopback(&fsc, 0)) != 0)
        return EXIT_FAILURE;

    return(Status);

}

static UINT SpiRead(DWORD wrLen, PWriteControlByteBuffer pwrBuf, DWORD rdLen, PReadDataByteBuffer prdBuf)
{
#if 0
    FTC_STATUS Status                 = FTC_SUCCESS;

    DWORD      dwNumDataBytesReturned = 0;

    Status = SPI_Read(ftHandle, &ReadStartCondition, true, false, wrLen * 8,
                      pwrBuf, wrLen, true, true, rdLen * 8,
                      prdBuf, &dwNumDataBytesReturned,
                      &HighPinsWriteActiveStates);

    return(Status);
#else
    UINT Status = 0;
    EnterCriticalSection(&CriticalSection);
    Status = ftdispi_write_read(&fsc, pwrBuf, wrLen, prdBuf, rdLen, 0);
    LeaveCriticalSection(&CriticalSection);
    return(Status);
#endif
}

static ULONG SpiWrite(DWORD ctrlLen, PWriteControlByteBuffer pCtrlBuf, DWORD dataLen, PWriteDataByteBuffer pDataBuf)
{
#if 0
    FTC_STATUS Status = FTC_SUCCESS;
    BOOL       Mutibytecmd;

    if (dataLen == 0)
    {
        Mutibytecmd = false;
    }
    else
    {
        Mutibytecmd = true;
    }

    Status = SPI_Write(ftHandle, &WriteStartCondition, true, false,
                       ctrlLen * 8, pCtrlBuf, ctrlLen,
                       Mutibytecmd, dataLen * 8, pDataBuf, dataLen, &WaitDataWriteComplete,
                       &HighPinsWriteActiveStates);

    return(Status);
#else
    UINT Status = 0;
    UINT data_size = ctrlLen + dataLen;
    uint8_t *data = malloc(data_size*sizeof(uint8_t));

    EnterCriticalSection(&CriticalSection);
    if (data_size > 0)
    {
        data_size = 0;
        if (ctrlLen > 0 && pCtrlBuf != NULL)
        {
            memcpy(&data[data_size], pCtrlBuf, ctrlLen);
            data_size += ctrlLen;
        }
        if (dataLen > 0 && pDataBuf != NULL)
        {
            memcpy(&data[data_size], pDataBuf, dataLen);
            data_size += dataLen;
        }
        Status = ftdispi_write(&fsc, data, data_size, 0);
    }
    LeaveCriticalSection(&CriticalSection);
    free(data);
#endif
}

#define SECTION_SIZE       (0x10000 - 0x10)
#define MAX_PREREAD_COUNT  (0xF000)

#define MEM_ADDRESS_HI     0x206
#define PREREAD_ADDRESS_LO 0x208
#define PREREAD_ADDRESS_HI 0x20A
#define PREREAD_LENGTH     0x20C
#define PREREAD_FIRE       0x20E

static WriteControlByteBuffer wrBuf;
static ReadDataByteBuffer     rdBuf;
static WriteDataByteBuffer    wrDataBuf;

#ifdef CFG_DEV_TEST
DEFINE_COULD_BE_MOCKED_NOT_VOID_FUNC1(
    uint16_t, ithReadRegH, uint16_t, addr)
#else
uint16_t ithReadRegH(uint16_t addr)
#endif
{
    uint16_t value;
    uint32_t error;
    uint32_t wrLen;
    uint32_t rdLen;

    EnterCriticalSection(&CriticalSection);

    addr    /= 2;
    wrLen    = 3;
    rdLen    = 2;

    wrBuf[0] = 1;
    wrBuf[1] = (uint8_t)(addr & 0x00FF);
    wrBuf[2] = (uint8_t)((addr & 0xFF00) >> 8);

    error    = SpiRead(wrLen, &wrBuf, rdLen, &rdBuf);
    if (error != 0)
    {
        printf("\n%s:%s:Ack Error! Error: 0x%08x\n", __TIME__, __FUNCTION__, error);
        value = 0;
        printf("\nAck Error : 0x%x\n",               rdBuf[0]);
    }
    else
    {
        value = (uint16_t)((rdBuf[1] << 8) + rdBuf[0]);
    }

    LeaveCriticalSection(&CriticalSection);

    return value;
}

#ifdef CFG_DEV_TEST
DEFINE_COULD_BE_MOCKED_VOID_FUNC2(
    ithWriteRegH, uint16_t, addr, uint16_t, data)
#else
void ithWriteRegH(uint16_t addr, uint16_t data)
#endif
{
    uint32_t wrLen;
    uint32_t rdLen;
    uint32_t error;

    EnterCriticalSection(&CriticalSection);

    addr    /= 2;
    wrLen    = 3;
    rdLen    = 2;

    wrBuf[0] = 0;
    wrBuf[1] = (uint8_t)(addr & 0x00FF);
    wrBuf[2] = (uint8_t)((addr & 0xFF00) >> 8);
    rdBuf[0] = (uint8_t)(data & 0x00FF);
    rdBuf[1] = (uint8_t)((data & 0xFF00) >> 8);

    error    = SpiWrite(wrLen, &wrBuf, rdLen, &rdBuf);

    if ((addr == 0x08) && (data == 0x1000))
        goto end;

    if (error != 0)
    {
        printf("\n%s:%s:Ack Error! Error: 0x%08x\n", __TIME__, __FUNCTION__, error);
        printf("\nAck Error : 0x%x\n",               rdBuf[0]);
    }

end:
    LeaveCriticalSection(&CriticalSection);
}

#define SPI_PRE_READ
#define SPI_DELAY 0x4

uint32_t ithReadRegA(uint32_t addr)
{
#if defined(SPI_PRE_READ)
    uint32_t value;
    uint32_t error;
    uint32_t wrLen = 9;
    uint32_t rdLen;
    uint8_t  prereadstaus;

    EnterCriticalSection(&CriticalSection);

    wrBuf[0] = 0x0E;
    wrBuf[1] = (uint8_t)(addr & 0x000000FF);
    wrBuf[2] = (uint8_t)((addr & 0x0000FF00) >> 8);
    wrBuf[3] = (uint8_t)((addr & 0x00FF0000) >> 16);
    wrBuf[4] = (uint8_t)((addr & 0xFF000000) >> 24);
    wrBuf[5] = 4;
    wrBuf[6] = 0;
    wrBuf[7] = 0;
    wrBuf[8] = 0;

    SpiWrite(wrLen, &wrBuf, 0, &rdBuf);

    wrBuf[0] = 0x0F;
    do
    {
        SpiRead(1, &wrBuf, 1, &prereadstaus);
        prereadstaus = (prereadstaus & 0x1f) * 4;
    } while (prereadstaus == 0);

    wrLen     = 5 + 4;
    rdLen     = 4;

    wrBuf[0]  = 0x05;
    wrBuf[0] |= (SPI_DELAY << 4);
    wrBuf[1]  = (uint8_t)(addr & 0x000000FF);
    wrBuf[2]  = (uint8_t)((addr & 0x0000FF00) >> 8);
    wrBuf[3]  = (uint8_t)((addr & 0x00FF0000) >> 16);
    wrBuf[4]  = (uint8_t)((addr & 0xFF000000) >> 24);
    wrBuf[5]  = 0;
    wrBuf[6]  = 0;
    wrBuf[7]  = 0;
    wrBuf[8]  = 0;
    wrBuf[9]  = 0;
    wrBuf[10] = 0;
    wrBuf[11] = 0;
    wrBuf[12] = 0;

    error     = SpiRead(wrLen, &wrBuf, rdLen, &rdBuf);
    if (error != 0)
    {
        printf("\n%s:%s:Ack Error! Error: 0x%08x\n", __TIME__, __FUNCTION__, error);
        value = 0;
        printf("\nAck Error : 0x%x\n",               rdBuf[0]);
    }
    else
    {
        value = (uint32_t)((rdBuf[3] << 24) + (rdBuf[2] << 16) + (rdBuf[1] << 8) + rdBuf[0]);
    }

    LeaveCriticalSection(&CriticalSection);
#else     // #if defined(SPI_PRE_READ)
    uint32_t value;
    uint32_t error;
    uint32_t wrLen = 5 + SPI_DELAY;
    uint32_t rdLen = 4;

    EnterCriticalSection(&CriticalSection);

    wrBuf[0]  = 0x05;
    wrBuf[0] |= (SPI_DELAY << 4);
    wrBuf[1]  = (uint8_t)(addr & 0x000000FF);
    wrBuf[2]  = (uint8_t)((addr & 0x0000FF00) >> 8);
    wrBuf[3]  = (uint8_t)((addr & 0x00FF0000) >> 16);
    wrBuf[4]  = (uint8_t)((addr & 0xFF000000) >> 24);
    wrBuf[5]  = 0;
    wrBuf[6]  = 0;
    wrBuf[7]  = 0;
    wrBuf[8]  = 0;
    wrBuf[9]  = 0;
    wrBuf[10] = 0;
    wrBuf[11] = 0;
    wrBuf[12] = 0;

    error     = SpiRead(wrLen, &wrBuf, rdLen, &rdBuf);
    if (error != 0)
    {
        printf("\n%s:%s:Ack Error! Error: 0x%08x\n", __TIME__, __FUNCTION__, error);
        value = 0;
        printf("\nAck Error : 0x%x\n",               rdBuf[0]);
    }
    else
    {
        value = (uint32_t)((rdBuf[3] << 24) + (rdBuf[2] << 16) + (rdBuf[1] << 8) + rdBuf[0]);
    }

    LeaveCriticalSection(&CriticalSection);
#endif     // #if defined(SPI_PRE_READ)
    return value;
}

void ithWriteRegA(uint32_t addr, uint32_t data)
{
    uint32_t wrLen = 5;
    uint32_t rdLen = 4;
    uint32_t error;

    EnterCriticalSection(&CriticalSection);

    wrBuf[0] = 4;
    wrBuf[1] = (uint8_t)(addr & 0x000000FF);
    wrBuf[2] = (uint8_t)((addr & 0x0000FF00) >> 8);
    wrBuf[3] = (uint8_t)((addr & 0x00FF0000) >> 16);
    wrBuf[4] = (uint8_t)((addr & 0xFF000000) >> 24);
    rdBuf[0] = (uint8_t)(data & 0x000000FF);
    rdBuf[1] = (uint8_t)((data & 0x0000FF00) >> 8);
    rdBuf[2] = (uint8_t)((data & 0x00FF0000) >> 16);
    rdBuf[3] = (uint8_t)((data & 0xFF000000) >> 24);

    error    = SpiWrite(wrLen, &wrBuf, rdLen, &rdBuf);

    if ((addr == 0x08) && (data == 0x1000))
        goto end;

    if (error != 0)
    {
        printf("\n%s:%s:Ack Error! Error: 0x%08x\n", __TIME__, __FUNCTION__, error);
        printf("\nAck Error : 0x%x\n",               rdBuf[0]);
    }

end:
    LeaveCriticalSection(&CriticalSection);
}

uint8_t ithReadReg8(uint32_t addr)
{
    uint8_t  value;
    uint32_t error;
    uint32_t wrLen = 5 + SPI_DELAY;
    uint32_t rdLen = 4;

    EnterCriticalSection(&CriticalSection);

    wrBuf[0]  = 0x05;
    wrBuf[0] |= (SPI_DELAY << 4);
    wrBuf[1]  = (uint8_t)((addr & 0xfffffffc) & 0x000000FF);
    wrBuf[2]  = (uint8_t)(((addr & 0xfffffffc) & 0x0000FF00) >> 8);
    wrBuf[3]  = (uint8_t)(((addr & 0xfffffffc) & 0x00FF0000) >> 16);
    wrBuf[4]  = (uint8_t)(((addr & 0xfffffffc) & 0xFF000000) >> 24);
    wrBuf[5]  = 0;
    wrBuf[6]  = 0;
    wrBuf[7]  = 0;
    wrBuf[8]  = 0;
    wrBuf[9]  = 0;
    wrBuf[10] = 0;
    wrBuf[11] = 0;
    wrBuf[12] = 0;

    error     = SpiRead(wrLen, &wrBuf, rdLen, &rdBuf);
    if (error != 0)
    {
        printf("\n%s:%s:Ack Error! Error: 0x%08x\n", __TIME__, __FUNCTION__, error);
        value = 0;
        printf("\nAck Error : 0x%x\n",               rdBuf[0]);
    }
    else
    {
        switch (addr & 3)
        {
        case 0: value = (uint8_t)rdBuf[0];
            break;
        case 1: value = (uint8_t)rdBuf[1];
            break;
        case 2: value = (uint8_t)rdBuf[2];
            break;
        case 3: value = (uint8_t)rdBuf[3];
            break;
        }
    }

    LeaveCriticalSection(&CriticalSection);

    return value;
}

void ithWriteReg8(uint32_t addr, uint8_t data)
{
    uint32_t wrLen = 5;
    uint32_t rdLen = 1;
    uint32_t error;

    EnterCriticalSection(&CriticalSection);

    wrBuf[0] = 4;
    wrBuf[1] = (uint8_t)(addr & 0x000000FF);
    wrBuf[2] = (uint8_t)((addr & 0x0000FF00) >> 8);
    wrBuf[3] = (uint8_t)((addr & 0x00FF0000) >> 16);
    wrBuf[4] = (uint8_t)((addr & 0xFF000000) >> 24);
    rdBuf[0] = (uint8_t)(data & 0x000000FF);
    rdBuf[1] = 0;
    rdBuf[2] = 0;
    rdBuf[3] = 0;

    error    = SpiWrite(wrLen, &wrBuf, rdLen, &rdBuf);

    if ((addr == 0x08) && (data == 0x1000))
        goto end;

    if (error != 0)
    {
        printf("\n%s:%s:Ack Error! Error: 0x%08x\n", __TIME__, __FUNCTION__, error);
        printf("\nAck Error : 0x%x\n",               rdBuf[0]);
    }

end:
    LeaveCriticalSection(&CriticalSection);
}

uint16_t ithReadReg16(uint32_t addr)
{
    uint16_t value;
    uint32_t error;
    uint32_t wrLen = 5 + SPI_DELAY;
    uint32_t rdLen = 4;

    EnterCriticalSection(&CriticalSection);

    wrBuf[0]  = 0x05;
    wrBuf[0] |= (SPI_DELAY << 4);
    wrBuf[1]  = (uint8_t)((addr & 0xfffffffc) & 0x000000FF);
    wrBuf[2]  = (uint8_t)(((addr & 0xfffffffc) & 0x0000FF00) >> 8);
    wrBuf[3]  = (uint8_t)(((addr & 0xfffffffc) & 0x00FF0000) >> 16);
    wrBuf[4]  = (uint8_t)(((addr & 0xfffffffc) & 0xFF000000) >> 24);
    wrBuf[5]  = 0;
    wrBuf[6]  = 0;
    wrBuf[7]  = 0;
    wrBuf[8]  = 0;
    wrBuf[9]  = 0;
    wrBuf[10] = 0;
    wrBuf[11] = 0;
    wrBuf[12] = 0;

    error     = SpiRead(wrLen, &wrBuf, rdLen, &rdBuf);
    if (error != 0)
    {
        printf("\n%s:%s:Ack Error! Error: 0x%08x\n", __TIME__, __FUNCTION__, error);
        value = 0;
        printf("\nAck Error : 0x%x\n",               rdBuf[0]);
    }
    else
    {
        if (addr & 2)
            value = (uint16_t)((rdBuf[3] << 8) + rdBuf[2]);
        else
            value = (uint16_t)((rdBuf[1] << 8) + rdBuf[0]);
    }

    LeaveCriticalSection(&CriticalSection);

    return value;
}

void ithWriteReg16(uint32_t addr, uint16_t data)
{
    uint32_t wrLen = 5;
    uint32_t rdLen = 2;
    uint32_t error;

    EnterCriticalSection(&CriticalSection);

    wrBuf[0] = 4;
    wrBuf[1] = (uint8_t)(addr & 0x000000FF);
    wrBuf[2] = (uint8_t)((addr & 0x0000FF00) >> 8);
    wrBuf[3] = (uint8_t)((addr & 0x00FF0000) >> 16);
    wrBuf[4] = (uint8_t)((addr & 0xFF000000) >> 24);
    rdBuf[0] = (uint8_t)(data & 0x000000FF);
    rdBuf[1] = (uint8_t)((data & 0x0000FF00) >> 8);
    rdBuf[2] = 0;
    rdBuf[3] = 0;

    error    = SpiWrite(wrLen, &wrBuf, rdLen, &rdBuf);

    if ((addr == 0x08) && (data == 0x1000))
        goto end;

    if (error != 0)
    {
        printf("\n%s:%s:Ack Error! Error: 0x%08x\n", __TIME__, __FUNCTION__, error);
        printf("\nAck Error : 0x%x\n",               rdBuf[0]);
    }

end:
    LeaveCriticalSection(&CriticalSection);
}

static void ReadMemory(uint32_t destAddress, uint32_t srcAddress, uint32_t sizeInByte)
{
    uint8_t  *pdest = (uint8_t *)destAddress;
    uint32_t wrLen;
    int      rdLen;
    uint32_t i;
    uint32_t error;

    #if defined(SPI_PRE_READ)
    uint32_t srcAddresstmp = srcAddress;
    uint8_t  prereadstaus;

    EnterCriticalSection(&CriticalSection);

    i        = 0;
    rdLen    = sizeInByte;
    wrLen    = 9;

    wrBuf[0] = 0x0E;
    wrBuf[1] = (uint8_t)(srcAddress & 0x000000FF);
    wrBuf[2] = (uint8_t)((srcAddress & 0x0000FF00) >> 8);
    wrBuf[3] = (uint8_t)((srcAddress & 0x00FF0000) >> 16);
    wrBuf[4] = (uint8_t)((srcAddress & 0xFF000000) >> 24);
    wrBuf[5] = (uint8_t)(sizeInByte & 0x000000FF);
    wrBuf[6] = (uint8_t)((sizeInByte & 0x0000FF00) >> 8);
    wrBuf[7] = (uint8_t)((sizeInByte & 0x00FF0000) >> 16);
    wrBuf[8] = (uint8_t)((sizeInByte & 0xFF000000) >> 24);

    SpiWrite(wrLen, &wrBuf, 0, &rdBuf);

    wrLen        = 9;
    wrBuf[0]     = 0x0F;
    wrBuf[5]     = 0x00;
    wrBuf[6]     = 0x00;
    wrBuf[7]     = 0x00;
    wrBuf[8]     = 0x00;

    SpiRead(1, &wrBuf, 1, &prereadstaus);
    prereadstaus = (prereadstaus & 0x1f) * 4;

    do
    {
        wrBuf[0] = 0x47;
        wrBuf[1] = (uint8_t)(srcAddresstmp & 0x000000FF);
        wrBuf[2] = (uint8_t)((srcAddresstmp & 0x0000FF00) >> 8);
        wrBuf[3] = (uint8_t)((srcAddresstmp & 0x00FF0000) >> 16);
        wrBuf[4] = (uint8_t)((srcAddresstmp & 0xFF000000) >> 24);

        if (prereadstaus != 0)
        {
            if (rdLen >= prereadstaus)
            {
                error          = SpiRead(wrLen, &wrBuf, prereadstaus, &rdBuf);
                memcpy((void *)&pdest[i], (void *)rdBuf, prereadstaus);
                srcAddresstmp += prereadstaus;
                i             += prereadstaus;
            }
            else
            {
                error = SpiRead(wrLen, &wrBuf, rdLen, &rdBuf);
                memcpy((void *)&pdest[i], (void *)rdBuf, rdLen);
                i    += prereadstaus;
            }
        }

        if (rdLen > 0)
        {
            rdLen = rdLen - prereadstaus;
            if (rdLen > 0)
            {
                wrBuf[0]     = 0x0F;
                error        = SpiRead(1, &wrBuf, 1, &prereadstaus);
                prereadstaus = (prereadstaus & 0x1f) * 4;
            }
            else if (rdLen < 0)
            {
                printf(" ERROR~~~~ rdLen < 0 \n");
                while (1)
                    ;
            }
        }
    } while (rdLen != 0);

    LeaveCriticalSection(&CriticalSection);
    #else // #if defined(SPI_PRE_READ)
    unsigned int preSize = 0;

    EnterCriticalSection(&CriticalSection);

    while (sizeInByte)
    {
        preSize = (sizeInByte > MAX_PREREAD_COUNT)
                  ? MAX_PREREAD_COUNT
                  : sizeInByte;

        wrLen    = 9;
        rdLen    = preSize;

        wrBuf[0] = 0x47;
        wrBuf[1] = (uint8_t)(srcAddress & 0x000000FF);
        wrBuf[2] = (uint8_t)((srcAddress & 0x0000FF00) >> 8);
        wrBuf[3] = (uint8_t)((srcAddress & 0x00FF0000) >> 16);
        wrBuf[4] = (uint8_t)((srcAddress & 0xFF000000) >> 24);
        wrBuf[5] = 0x00;
        wrBuf[6] = 0x00;
        wrBuf[7] = 0x00;
        wrBuf[8] = 0x00;

        error    = SpiRead(wrLen, &wrBuf, rdLen, &rdBuf);

        if (error != 0)
        {
            printf("\n%s:%s:Ack Error! Error: 0x%08x\n", __TIME__, __FUNCTION__, error);
            printf("\nAck Error : 0x%x\n",               rdBuf[0]);
        }

        for (i = 0; i < preSize; i++)
        {
            *(pdest + i) = rdBuf[i];
        }

        sizeInByte -= preSize;
        srcAddress += preSize;
        pdest      += preSize;
    }
    LeaveCriticalSection(&CriticalSection);
    #endif // #if defined(SPI_PRE_READ)
}

static void SecWriteMemory(uint16_t SecNum, uint16_t SecOffset, uint8_t *pDataBuffer, uint32_t DataLen)
{
    uint32_t ulRemainLen;
    uint16_t usSecOffset;
    uint16_t usTmpSecOffset;
    uint32_t usDataLen;
    uint32_t wrLen;
    uint32_t wrDataLen;
    uint32_t i;
    uint8_t  *ptr;
    uint32_t error;

    if (DataLen > 0x10000)
    {
        printf("Top Half: Data length is too large\n");
        return;
    }

    ithWriteRegH(0x206, SecNum);

    ptr         = pDataBuffer;

    ulRemainLen = DataLen;
    usSecOffset = SecOffset;
    usDataLen   = (uint16_t)((ulRemainLen > (uint32_t)(0xFF00 - usSecOffset)) ?
                             (uint32_t)(0xFF00 - usSecOffset) : ulRemainLen);

    if (usDataLen == 0)
    {
        usDataLen = (ulRemainLen) ? ((uint16_t)ulRemainLen) : (0xFF00 - usSecOffset);
        if (usDataLen == 0)
            return;
    }

    wrLen     = 3;
    wrDataLen = usDataLen;

    for (i = 0; i < (usDataLen); i += 2)
    {
        wrDataBuf[i]     = *ptr++;
        wrDataBuf[i + 1] = *ptr++;
    }

    wrBuf[0]       = 0;
    usTmpSecOffset = (usSecOffset / 2);

    wrBuf[1]       = (uint8_t)(usTmpSecOffset & 0x00FF);
    wrBuf[2]       = (uint8_t)((usTmpSecOffset & 0xFF00) >> 8);
    wrBuf[2]      |= 0x80;

    error          = SpiWrite(wrLen, &wrBuf, wrDataLen, &wrDataBuf);
    if (error)
    {
        printf("\n%s:SpiWrite Error! Error: 0x%08x\n", __FUNCTION__, error);
    }

    ulRemainLen -= (uint32_t)usDataLen;

    if (ulRemainLen)
    {
        usDataLen   = (uint16_t)ulRemainLen;
        usSecOffset = 0xFF00;

        wrLen       = 3;
        wrDataLen   = usDataLen;

        if (usDataLen > 0x100)
        {
            printf("Buttom Half: Data Length is too large\n");
        }

        ithWriteRegH(0x206, SecNum);

        for (i = 0; i < (usDataLen); i += 2)
        {
            wrDataBuf[i]     = *ptr++;
            wrDataBuf[i + 1] = *ptr++;
        }

        wrBuf[0]       = 0;
        usTmpSecOffset = (usSecOffset / 2);

        wrBuf[1]       = (uint8_t)(usTmpSecOffset & 0x00FF);
        wrBuf[2]       = (uint8_t)((usTmpSecOffset & 0xFF00) >> 8);
        wrBuf[2]      |= 0x80;

        error          = SpiWrite(wrLen, &wrBuf, wrDataLen, &wrDataBuf);
        if (error)
        {
            printf("\n%s:SpiWrite Error! Error: 0x%08x\n", __FUNCTION__, error);
        }
    }
}

static void WriteMemory(uint32_t destAddress, uint32_t srcAddress, uint32_t sizeInByte)
{
    unsigned int secSize = 0;

    EnterCriticalSection(&CriticalSection);

    while (sizeInByte)
    {
        uint32_t wrLen = 5;

        secSize = (sizeInByte > SECTION_SIZE)
                  ? SECTION_SIZE
                  : sizeInByte;

        wrBuf[0] = 4;
        wrBuf[1] = (uint8_t)(destAddress & 0x000000FF);
        wrBuf[2] = (uint8_t)((destAddress & 0x0000FF00) >> 8);
        wrBuf[3] = (uint8_t)((destAddress & 0x00FF0000) >> 16);
        wrBuf[4] = (uint8_t)((destAddress & 0xFF000000) >> 24);
        wrBuf[5] = 0;
        wrBuf[6] = 0;

        SpiWrite(wrLen, &wrBuf, secSize, (PWriteDataByteBuffer)srcAddress);

        sizeInByte  -= secSize;
        srcAddress  += secSize;
        destAddress += secSize;
    }
    LeaveCriticalSection(&CriticalSection);
}

void *ithMapVram(uint32_t vram_addr, uint32_t size, uint32_t flags)
{
    uint8_t *sys_addr;

    ASSERT(size > 0);
    ASSERT(vram_addr < CFG_RAM_SIZE && (vram_addr + size) <= CFG_RAM_SIZE);

    if (size == 0)
        return NULL;

    if (CFG_RAM_SIZE <= vram_addr || CFG_RAM_SIZE < (vram_addr + size))
        return NULL;

    if (flags & ITH_VRAM_READ)
        ReadMemory((uint32_t)&vramBuffer[vram_addr], vram_addr, size);
    memcpy(&vramBufferCache[vram_addr], &vramBuffer[vram_addr], size);
    sys_addr = &vramBufferCache[vram_addr];

    return sys_addr;
}

void ithUnmapVram(void *sys_addr, uint32_t size)
{
    uint32_t vram_addr;

    ASSERT(size > 0);
    ASSERT(((uint8_t *)vramBufferCache) <= ((uint8_t *)sys_addr));
    ASSERT(((uint8_t *)sys_addr) <= (((uint8_t *)vramBufferCache) + CFG_RAM_SIZE));
    ASSERT((((uint8_t *)sys_addr) + size) <= (((uint8_t *)vramBufferCache) + CFG_RAM_SIZE));

    vram_addr = (uint32_t)(((uint8_t *)sys_addr) - vramBufferCache);

    if (memcmp(&vramBuffer[vram_addr], &vramBufferCache[vram_addr], size) != 0)
    {
        memcpy(&vramBuffer[vram_addr], &vramBufferCache[vram_addr], size);
        WriteMemory(vram_addr, (uint32_t)&vramBuffer[vram_addr], size);
    }
}

void ithFlushDCacheRange(void *sys_addr, uint32_t size)
{
    ithUnmapVram(sys_addr, size);
}

uint32_t ithSysAddr2VramAddr(void *sys_addr)
{
    ASSERT(((uint8_t *)vramBufferCache) <= ((uint8_t *)sys_addr));
    ASSERT(((uint8_t *)sys_addr) <= (((uint8_t *)vramBufferCache) + CFG_RAM_SIZE));

    return (uint32_t)sys_addr - (uint32_t)vramBufferCache;
}

void *ithVramAddr2SysAddr(uint32_t vram_addr)
{
    ASSERT(vram_addr <= CFG_RAM_SIZE);

    return &vramBufferCache[vram_addr];
}