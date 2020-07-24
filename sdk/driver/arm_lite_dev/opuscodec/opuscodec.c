#include <string.h>
#include <stdlib.h>
#include "arm_lite_dev/opuscodec/opuscodec.h"

#define TEST_DEVICE_LOAD_FROM_H

#ifdef TEST_DEVICE_LOAD_FROM_H //load from include fine
static uint8_t gpOpusCodecImage[] =
{
    #include "opuscodec.hex"
};

#else //loaded from file system
#include <stdio.h>
#define IMAGE_PATH ("A:/opuscodec.rom")
#define HEADER_LEN_OFFSET   12
#endif

static void _loadTestDeviceFirmware(void)
{
#ifdef TEST_DEVICE_LOAD_FROM_H
    iteRiscLoadData(ARMLITE_CPU_IMAGE_MEM_TARGET,gpOpusCodecImage,sizeof(gpOpusCodecImage));
#else
    FILE* f;
    char* buf;
    char buf2[4];
    uint32_t headersize, imagesize = 0;
    int nResult;

    f = fopen(IMAGE_PATH, "rb");
    if (f)
    {
        // get header size
        if (fseek(f, HEADER_LEN_OFFSET, SEEK_SET))
        {
            printf("%s(%d)\n", __FILE__, __LINE__);
            goto end;
        }

        if (fread(&headersize, 1, sizeof (uint32_t), f) != sizeof (uint32_t))
        {
            printf("%s(%d)\n", __FILE__, __LINE__);
            goto end;
        }
        headersize = itpBetoh32(headersize);

        // get image size
        if (fseek(f, headersize - 4, SEEK_SET))
        {
            printf("%s(%d)\n", __FILE__, __LINE__);
            goto end;
        }
        if (fread(&imagesize, 1, sizeof (uint32_t), f) != sizeof (uint32_t))
        {
            printf("%s(%d)\n", __FILE__, __LINE__);
            goto end;
        }
        imagesize = itpBetoh32(imagesize);

        if (fseek(f, headersize, SEEK_SET))
        {
            printf("%s(%d)\n", __FILE__, __LINE__);
            goto end;
        }
    
        if (fread(buf2, 1, 4, f) != 4)
        {
            printf("%s(%d)\n", __FILE__, __LINE__);
            goto end;
        }

        if (strncmp(buf2, "SMAZ", 4) == 0)
        {
            uint32_t contentsize;
    
            // get content size
            if (fseek(f, HEADER_LEN_OFFSET + 4, SEEK_SET))
            {
                printf("%s(%d)\n", __FILE__, __LINE__);
                goto end;
            }
    
            if (fread(&contentsize, 1, sizeof (uint32_t), f) != sizeof (uint32_t))
            {
                printf("%s(%d)\n", __FILE__, __LINE__);
                goto end;
            }
            contentsize = itpBetoh32(contentsize);
    
            // read data
            buf = (char*) memalign(32, ITH_ALIGN_UP(contentsize,32));
            if (!buf)
            {
                printf("%s(%d)\n", __FILE__, __LINE__);
                goto end;
            }

            if (fseek(f, headersize, SEEK_SET))
            {
                printf("%s(%d)\n", __FILE__, __LINE__);
                goto end;
            }
            nResult = fread(buf, 1, contentsize, f);
            if (nResult != contentsize)
            {
                printf("%s(%d)\n", __FILE__, __LINE__);
                goto end;
            }
    
            // hardware decompress
            ioctl(ITP_DEVICE_DECOMPRESS, ITP_IOCTL_INIT, NULL);
            if (write(ITP_DEVICE_DECOMPRESS, buf+8, contentsize) == contentsize)
            {
                read(ITP_DEVICE_DECOMPRESS, iteRiscGetTargetMemAddress(ARMLITE_CPU_IMAGE_MEM_TARGET), imagesize);
            }       
            ioctl(ITP_DEVICE_DECOMPRESS, ITP_IOCTL_EXIT, NULL);
        }
    end:
        if (buf)
        {
            free(buf);
        }
        if (f)
        {
            fclose(f);
        }
    }
    else
    {
        printf("%s(%d)\n", __FILE__, __LINE__);
    }
#endif
}

static void opusCodecProcessCommand(int cmdId)
{
    int i = 0;
    ARMLITE_COMMAND_REG_WRITE(REQUEST_CMD_REG, cmdId);
    while(1)
    {
        if (ARMLITE_COMMAND_REG_READ(RESPONSE_CMD_REG) != cmdId)
            continue;
        else
            break;
    }
    ARMLITE_COMMAND_REG_WRITE(REQUEST_CMD_REG, 0);
    for (i = 0; i < 1024; i++)
    {
        asm("");
    }
    ARMLITE_COMMAND_REG_WRITE(RESPONSE_CMD_REG, 0);
}

static int opusCodecIoctl(int file, unsigned long request, void *ptr, void *info)
{
    uint8_t* pExchangeAddress = (uint8_t*) (iteRiscGetTargetMemAddress(ARMLITE_CPU_IMAGE_MEM_TARGET) + CMD_DATA_BUFFER_OFFSET);
    switch (request)
    {
        case ITP_IOCTL_INIT:
        {
            //Stop CPU
            iteRiscResetCpu(ARMLITE_CPU);

            //Clear Commuication Engine and command buffer
            memset(pExchangeAddress, 0x0, MAX_CMD_DATA_BUFFER_SIZE);
            ARMLITE_COMMAND_REG_WRITE(REQUEST_CMD_REG, 0);
            ARMLITE_COMMAND_REG_WRITE(RESPONSE_CMD_REG, 0);

            //Load Engine First
            _loadTestDeviceFirmware();

            //Fire CPU
            iteRiscFireCpu(ARMLITE_CPU);
            break;
        }
        case ITP_IOCTL_OPUS_CREATE:
        {
            OPUS_INIT_DATA* ptInitData = (OPUS_INIT_DATA*) ptr;
            memcpy(pExchangeAddress, ptInitData, sizeof(OPUS_INIT_DATA));
#ifdef CFG_CPU_WB
            ithFlushDCacheRange((void*)pExchangeAddress, sizeof(OPUS_INIT_DATA));
            ithFlushMemBuffer();
#endif
            opusCodecProcessCommand(OPUS_CREATE);
            
            ithInvalidateDCacheRange(pExchangeAddress, sizeof(OPUS_INIT_DATA));
            memcpy(ptInitData, pExchangeAddress, sizeof(OPUS_INIT_DATA));
            break;
        }
        case ITP_IOCTL_OPUS_ENCODE:
        {
            OPUS_CONTEXT* ptInitData = (OPUS_CONTEXT*) ptr;
            memcpy(pExchangeAddress, ptInitData, sizeof(OPUS_CONTEXT));
#ifdef CFG_CPU_WB
            ithFlushDCacheRange((void*)pExchangeAddress, sizeof(OPUS_CONTEXT));
            ithFlushMemBuffer();
#endif
            opusCodecProcessCommand(OPUS_ENCODE);
            
            ithInvalidateDCacheRange(pExchangeAddress, sizeof(OPUS_CONTEXT));
            memcpy(ptInitData, pExchangeAddress, sizeof(OPUS_CONTEXT));
            break;
        }
        case ITP_IOCTL_OPUS_DECODE:
        {
            OPUS_CONTEXT* ptInitData = (OPUS_CONTEXT*) ptr;
            memcpy(pExchangeAddress, ptInitData, sizeof(OPUS_CONTEXT));
#ifdef CFG_CPU_WB
            ithFlushDCacheRange((void*)pExchangeAddress, sizeof(OPUS_CONTEXT));
            ithFlushMemBuffer();
#endif
            opusCodecProcessCommand(OPUS_DECODE);
            
            ithInvalidateDCacheRange(pExchangeAddress, sizeof(OPUS_CONTEXT));
            memcpy(ptInitData, pExchangeAddress, sizeof(OPUS_CONTEXT));
            break;
        }
        
        default:
            break;
    }
    return 0;
}

const ITPDevice itpDeviceOpusCodec =
{
    ":opusCodec",
    itpOpenDefault,
    itpCloseDefault,
    itpReadDefault,
    itpWriteDefault,
    itpLseekDefault,
    opusCodecIoctl,
    NULL
};
