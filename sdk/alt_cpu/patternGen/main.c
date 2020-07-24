#include <stdarg.h>

#include "alt_cpu/patternGen/patternGen.h"
#include "alt_cpu/alt_cpu_utility.h"

#define ENDIAN_SWAP16(x) \
        (((x & 0x00FF) << 8) | ((x & 0xFF00) >> 8))

#define ENDIAN_SWAP32(x) \
        (((x & 0x000000FF) << 24) | \
        ((x & 0x0000FF00) <<  8) | \
        ((x & 0x00FF0000) >>  8) | \
        ((x & 0xFF000000) >> 24))

#define PATTERN_GEN_STATE_NULL               0
#define PATTERN_GEN_STATE_WRITE              1
#define PATTERN_GEN_STATE_WRITE_DELAY        2

typedef struct
{
    uint32_t                bValid;
    uint32_t                bRun;
    uint32_t                patternGenGpio;
    uint32_t                curTime;
    uint32_t                checkTickCount;
    uint32_t                defaultGpioValue; //Once repeat count down to 0, patternGenGpio will set back to default GPIO value.
    uint32_t                timeUnitInUs; //period time unit in microseconds.

    PATTERN_WRITE_DATA      tWriteData[MAX_WRITE_DATA_COUNT];
    uint32_t                validDataCount;
    uint32_t                sendCount;
    uint32_t                writeIndex;

    PATTERN_WRITE_DATA      tTmpWriteData[MAX_WRITE_DATA_COUNT];
    uint32_t                tmpValidDataCount;
    uint32_t                tmpSendCount;
    uint32_t                bInputMode;
    uint32_t                state;
} PATTERN_GEN_HANDLE;

static uint32_t           gTickPerUs = 0;
static PATTERN_GEN_HANDLE gptPatternGenHandle[PATTERN_GEN_COUNT] = { 0 };

static void patternGenProcessInitCmd(void)
{
    PATTERN_GEN_INIT_DATA* ptInitData = (PATTERN_GEN_INIT_DATA*) CMD_DATA_BUFFER_OFFSET;
    uint8_t* pAddr = (uint8_t*)ptInitData;
    uint32_t patternGenId = ENDIAN_SWAP32(ptInitData->patternGenId);
    uint32_t cpuClk = ENDIAN_SWAP32(ptInitData->cpuClock);
    PATTERN_GEN_HANDLE *ptPatternGenHandle = 0;

    if (patternGenId >= PATTERN_GEN0 && patternGenId < PATTERN_GEN_COUNT && cpuClk)
    {
        ptPatternGenHandle = &gptPatternGenHandle[patternGenId];
        gTickPerUs = cpuClk / (1000 * 1000);
        ptPatternGenHandle->patternGenGpio = ENDIAN_SWAP32(ptInitData->patternGenGpio);

        ptPatternGenHandle->timeUnitInUs = ENDIAN_SWAP32(ptInitData->timeUnitInUs);
        if (ptPatternGenHandle->timeUnitInUs == 0)
        {
            ptPatternGenHandle->timeUnitInUs = 1;
        }
        ptPatternGenHandle->defaultGpioValue = ENDIAN_SWAP32(ptInitData->defaultGpioValue);

       
        ptPatternGenHandle->bValid = 1;
        ptPatternGenHandle->bRun = 0;
        ptPatternGenHandle->curTime = 0;
        ptPatternGenHandle->state  = PATTERN_GEN_STATE_NULL;
        setGpioMode(ptPatternGenHandle->patternGenGpio, 0);

        if (ptPatternGenHandle->defaultGpioValue == GPIO_INPUT_MODE)
        {
            setGpioDir(ptPatternGenHandle->patternGenGpio, 1);
            ptPatternGenHandle->bInputMode = 1;
        }
        else
        {
            setGpioDir(ptPatternGenHandle->patternGenGpio, 0);
            ptPatternGenHandle->bInputMode = 0;
            if (ptPatternGenHandle->defaultGpioValue != GPIO_KEEP_LAST_VALUE)
            {
                setGpioValue(ptPatternGenHandle->patternGenGpio, ptPatternGenHandle->defaultGpioValue);
            }
        }
    }
}

static void patternGenProcessWriteCmd(void)
{
    PATTERN_GEN_WRITE_CMD_DATA* ptWriteCmd = (PATTERN_GEN_WRITE_CMD_DATA*) CMD_DATA_BUFFER_OFFSET;
    uint32_t patternGenId = ENDIAN_SWAP32(ptWriteCmd->patternGenId);
    uint32_t sendCount = ENDIAN_SWAP32(ptWriteCmd->sendCount);
    uint32_t validCount = ENDIAN_SWAP32(ptWriteCmd->validDataCount);

    PATTERN_GEN_HANDLE *ptPatternGenHandle = 0;
    int i = 0;
    if (patternGenId >= PATTERN_GEN0 && patternGenId < PATTERN_GEN_COUNT)
    {
        ptPatternGenHandle = &gptPatternGenHandle[patternGenId];
        if (ptPatternGenHandle->bValid && sendCount && validCount && validCount < MAX_WRITE_DATA_COUNT)
        {
            if (ptPatternGenHandle->bRun == 0 || ptPatternGenHandle->sendCount == 0)
            {
                ptPatternGenHandle->sendCount = sendCount;
                ptPatternGenHandle->validDataCount = validCount;
                ptPatternGenHandle->tmpSendCount = 0;
                ptPatternGenHandle->tmpValidDataCount = 0;
                for (i = 0; i < ptPatternGenHandle->validDataCount; i++)
                {
                    ptPatternGenHandle->tWriteData[i].value = ENDIAN_SWAP16(ptWriteCmd->tWriteData[i].value);
                    ptPatternGenHandle->tWriteData[i].period = ENDIAN_SWAP16(ptWriteCmd->tWriteData[i].period);
                }
            }
            else
            {
                ptPatternGenHandle->tmpSendCount = sendCount;
                ptPatternGenHandle->tmpValidDataCount = validCount;
                for (i = 0; i < ptPatternGenHandle->validDataCount; i++)
                {
                    ptPatternGenHandle->tTmpWriteData[i].value = ENDIAN_SWAP16(ptWriteCmd->tWriteData[i].value);
                    ptPatternGenHandle->tTmpWriteData[i].period = ENDIAN_SWAP16(ptWriteCmd->tWriteData[i].period);
                }
            }
            ptPatternGenHandle->bRun = 1;
        }
    }
}

static void patternGenProcessStopCmd(void)
{
    PATTERN_GEN_STOP_CMD_DATA* ptStopCmd = (PATTERN_GEN_STOP_CMD_DATA*) CMD_DATA_BUFFER_OFFSET;
    uint32_t patternGenId = ENDIAN_SWAP32(ptStopCmd->patternGenId);  
    PATTERN_GEN_HANDLE *ptPatternGenHandle = 0;
    
    if (patternGenId >= PATTERN_GEN0 && patternGenId < PATTERN_GEN_COUNT)
    {
        ptPatternGenHandle = &gptPatternGenHandle[patternGenId];
        if (ptPatternGenHandle->bValid && ptPatternGenHandle->bRun)
        {
            ptPatternGenHandle->bRun = 0;
            ptPatternGenHandle->state = PATTERN_GEN_STATE_NULL;
            ptPatternGenHandle->curTime = 0;
            ptPatternGenHandle->writeIndex = 0;
        }
    }
}

static void patternGenProcessUpdateSendCountCmd(void)
{
    PATTERN_GEN_UPDATE_SEND_COUNT_CMD_DATA* ptSendCountCmd = (PATTERN_GEN_UPDATE_SEND_COUNT_CMD_DATA*) CMD_DATA_BUFFER_OFFSET;
    uint32_t patternGenId = ENDIAN_SWAP32(ptSendCountCmd->patternGenId);  
    PATTERN_GEN_HANDLE *ptPatternGenHandle = 0;
    
    if (patternGenId >= PATTERN_GEN0 && patternGenId < PATTERN_GEN_COUNT)
    {
        ptPatternGenHandle = &gptPatternGenHandle[patternGenId];
        if (ptPatternGenHandle->bValid)
        {
            ptPatternGenHandle->bRun = 1;
            ptPatternGenHandle->state = PATTERN_GEN_STATE_NULL;
            ptPatternGenHandle->curTime = 0;
            ptPatternGenHandle->writeIndex = 0;
            ptPatternGenHandle->sendCount = ENDIAN_SWAP32(ptSendCountCmd->sendCount);
        }
    }
}

static void patternGenGeneratePattern(void)
{
    uint32_t patternGenId = 0;
    int i = 0;
    PATTERN_GEN_HANDLE *ptPatternGenHandle = 0;

    for (patternGenId = PATTERN_GEN0; patternGenId < PATTERN_GEN_COUNT; patternGenId++)
    {
        ptPatternGenHandle = &gptPatternGenHandle[patternGenId];
        if (ptPatternGenHandle->bRun && ptPatternGenHandle->sendCount)
        {
            switch (ptPatternGenHandle->state)
            {
                case PATTERN_GEN_STATE_NULL:
                {
                    //Update tmp wrtie data to current write data
                    if (ptPatternGenHandle->tmpSendCount && ptPatternGenHandle->tmpValidDataCount)
                    {
                        for (i = 0; i < ptPatternGenHandle->tmpValidDataCount; i++)
                        {
                            ptPatternGenHandle->tWriteData[i].value = ptPatternGenHandle->tTmpWriteData[i].value;
                            ptPatternGenHandle->tWriteData[i].period = ptPatternGenHandle->tTmpWriteData[i].period;
                        }
                        ptPatternGenHandle->sendCount = ptPatternGenHandle->tmpSendCount;
                        ptPatternGenHandle->validDataCount = ptPatternGenHandle->tmpValidDataCount;
                        ptPatternGenHandle->tmpSendCount = ptPatternGenHandle->tmpValidDataCount = 0;
                    }
                    ptPatternGenHandle->writeIndex = 0;
                    ptPatternGenHandle->state = PATTERN_GEN_STATE_WRITE;
                    //Set to output mode
                    if (ptPatternGenHandle->bInputMode)
                    {
                        setGpioDir(ptPatternGenHandle->patternGenGpio, 0);
                        ptPatternGenHandle->bInputMode = 0;
                    }
                    break;
                }
                case PATTERN_GEN_STATE_WRITE:
                {
                    ptPatternGenHandle->curTime = getCurTimer(0);
                    setGpioValue(ptPatternGenHandle->patternGenGpio, ptPatternGenHandle->tWriteData[ptPatternGenHandle->writeIndex].value);
                    ptPatternGenHandle->checkTickCount = ptPatternGenHandle->tWriteData[ptPatternGenHandle->writeIndex].period * ptPatternGenHandle->timeUnitInUs * gTickPerUs;
                    ptPatternGenHandle->state = PATTERN_GEN_STATE_WRITE_DELAY;
                    break;
                }
                case PATTERN_GEN_STATE_WRITE_DELAY:
                {
                    if (getDuration(0, ptPatternGenHandle->curTime) >= ptPatternGenHandle->checkTickCount)
                    {
                        ptPatternGenHandle->writeIndex++;
                        
                        if (ptPatternGenHandle->writeIndex >= ptPatternGenHandle->validDataCount)
                        {
                            if (ptPatternGenHandle->sendCount != INIFINITE_SEND_COUNT)
                            {
                                ptPatternGenHandle->sendCount--;
                            }

                            if (ptPatternGenHandle->sendCount == 0)
                            {
                                if (ptPatternGenHandle->defaultGpioValue == GPIO_INPUT_MODE)
                                {
                                    setGpioDir(ptPatternGenHandle->patternGenGpio, 1);
                                    ptPatternGenHandle->bInputMode = 1;
                                }
                                else if (ptPatternGenHandle->defaultGpioValue != GPIO_KEEP_LAST_VALUE)
                                {
                                    setGpioValue(ptPatternGenHandle->patternGenGpio, ptPatternGenHandle->defaultGpioValue);
                                }
                            }
                            ptPatternGenHandle->state = PATTERN_GEN_STATE_NULL;
                        }
                        else
                        {
                            ptPatternGenHandle->state = PATTERN_GEN_STATE_WRITE;
                        }
                    }
                    break;
                }
            }
        }
    }
}

int main(int argc, char **argv)
{
    //Set GPIO and Clock Setting
    int inputCmd = 0;

    //Start Timer
    startTimer(0);

    while(1)
    {
        inputCmd = ALT_CPU_COMMAND_REG_READ(REQUEST_CMD_REG);
        if (inputCmd && ALT_CPU_COMMAND_REG_READ(RESPONSE_CMD_REG) == 0)
        {
            switch(inputCmd)
            {
                case INIT_CMD_ID:
                    patternGenProcessInitCmd();
                    break;
                case WRITE_CMD_ID:
                    patternGenProcessWriteCmd();
                    break;
                case STOP_CMD_ID:
                    patternGenProcessStopCmd();
                    break;
                case UPDATE_SEND_COUNT_CMD_ID:
                    patternGenProcessUpdateSendCountCmd();
                    break;
                default:
                    break;
            }
            ALT_CPU_COMMAND_REG_WRITE(RESPONSE_CMD_REG, (uint16_t) inputCmd);
        }
        patternGenGeneratePattern();
    }
}
