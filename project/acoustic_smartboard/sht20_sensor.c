#include "ite/itp.h"
#include "sht20_sensor.h"

#define SHT20_SLAVE_ADDR   0x40
#define SHT20_READ_STATUS  0xe7
#define SHT20_WRITE_STATUS 0xe6
#define SHT20_MS_PRECISION 0x03    //11 bit
#define SHT20_HEATER       0x00    //disable heater
#define SHT20_OTP_RELOAD   0x01    //disable OTP reloading
#define SHT20_TRIGGER_T_MS 0xe3
#define SHT20_ERROR_T_C    99

static int        gMasterDev = 0;
static uint8_t    sendBuffer[2] = {0};
static uint8_t    recvBuffer[3] = {0};
static ITPI2cInfo event = {0};

static void clear_buffer(void)
{
    memset(sendBuffer, 0x0, sizeof(sendBuffer));
    memset(recvBuffer, 0x0, sizeof(recvBuffer));
    memset(&event, 0x0, sizeof(ITPI2cInfo));
}

void SHT20_Init(void)
{
#ifdef CFG_I2C1_ENABLE
    uint32_t result = 0;
    uint8_t  status_temp = 0;

    gMasterDev = open(":i2c1", 0);

    clear_buffer();
    sendBuffer[0]        = SHT20_READ_STATUS;
    event.slaveAddress   = SHT20_SLAVE_ADDR;
	event.cmdBuffer      = sendBuffer;
	event.cmdBufferSize  = 1;
	event.dataBuffer     = recvBuffer;
	event.dataBufferSize = 1;
    if ((result = read(gMasterDev, &event, 1)) != 0)
    {
        printf("SHT20_Init reading status error\n");
        return;
    }
    else
        status_temp = recvBuffer[0];
    printf("SHT20_Init initial status: %x\n", status_temp);

    status_temp = (status_temp & ~(0x1 << 7)) | (SHT20_MS_PRECISION >> 1 & 0x1) << 7;
    status_temp = (status_temp & ~(0x1 << 0)) | (SHT20_MS_PRECISION & 0x1) << 0;
    status_temp = (status_temp & ~(0x1 << 2)) | (SHT20_HEATER & 0x1) << 2;
    status_temp = (status_temp & ~(0x1 << 1)) | (SHT20_OTP_RELOAD & 0x1) << 1;

    clear_buffer();
    sendBuffer[0]        = SHT20_WRITE_STATUS;
    sendBuffer[1]        = status_temp;
    event.slaveAddress   = SHT20_SLAVE_ADDR;
	event.cmdBuffer      = sendBuffer;
	event.cmdBufferSize  = 2;
    if ((result = write(gMasterDev, &event, 1)) != 0)
    {
        printf("SHT20_Init writing status error\n");
        return;
    }

    clear_buffer();
    sendBuffer[0]        = SHT20_READ_STATUS;
    event.slaveAddress   = SHT20_SLAVE_ADDR;
	event.cmdBuffer      = sendBuffer;
	event.cmdBufferSize  = 1;
	event.dataBuffer     = recvBuffer;
	event.dataBufferSize = 1;
    if ((result = read(gMasterDev, &event, 1)) != 0)
    {
        printf("SHT20_Init reading status error2\n");
        return;
    }
    else
        status_temp = recvBuffer[0];
    printf("SHT20_Init current status: %x\n", status_temp);
#endif
}

float SHT20_Detect(void)
{
    float    result = SHT20_ERROR_T_C;
    uint32_t temp_temp = 0;

#ifdef CFG_I2C1_ENABLE
    clear_buffer();
    sendBuffer[0]        = SHT20_TRIGGER_T_MS;
    event.slaveAddress   = SHT20_SLAVE_ADDR;
	event.cmdBuffer      = sendBuffer;
	event.cmdBufferSize  = 1;
	event.dataBuffer     = recvBuffer;
	event.dataBufferSize = 3;
    if (read(gMasterDev, &event, 1) != 0)
    {
        printf("SHT20_Detect reading temperature error\n");
    }
    else
    {
        temp_temp = recvBuffer[1] | recvBuffer[0] << 8;
        result = (float)temp_temp;
        result = result / 65536 * 175.72 - 46.85;
    }

    return result;
#else
    return result;
#endif
}
