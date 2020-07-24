 /** @file
 * OLED SH11XX driver.
 *
 *
 */
#include <stdarg.h>

#include "alt_cpu/oledCtrl/oledCtrl.h"
#include "alt_cpu/alt_cpu_utility.h"

#define ENDIAN_SWAP16(x) \
        (((x & 0x00FF) << 8) | ((x & 0xFF00) >> 8))

#define ENDIAN_SWAP32(x) \
        (((x & 0x000000FF) << 24) | \
        ((x & 0x0000FF00) <<  8) | \
        ((x & 0x00FF0000) >>  8) | \
        ((x & 0xFF000000) >> 24))

#define CMD_DISPLAY_OFF   0xAE
#define CMD_DISPLAY_ON    0xAF

#define CMD_SET_PAGE  0xB0
#define CMD_SET_COLUMN_UPPER  0x10
#define CMD_SET_COLUMN_LOWER  0x00

#define LCD_CMD        0x00
#define LCD_DTA        0x01

#define WAIT_TICK 50//64

typedef struct
{
    uint32_t  state;
    uint32_t  gpioCSN;
    uint32_t  colOffset;
    uint32_t maxRow;
    uint32_t maxCol;
    uint32_t addrIndex;
    uint8_t  *buffAddrA;
    uint8_t  *buffAddrB;
    uint8_t  *buffAddrC;
} OLED_CTRL_HANDLE;

static OLED_CTRL_HANDLE gtpOledCtrlHandle[OLED_COUNT] = {0};
static uint8_t *gtpOledCtrlState = (uint8_t *) OLED_STATE_ADDRESS;

#if ((CFG_CHIP_FAMILY == 970) || (CFG_CHIP_FAMILY == 9860))
#define LCD_COMMAND_REG         0xD00000F0
#else
#define LCD_COMMAND_REG         0x121A
#endif

static void oledWait(uint32_t tick)
{
    int i = 0;
    for(i = 0; i < tick; i++)
        asm("");
}

#if ((CFG_CHIP_FAMILY == 970) || (CFG_CHIP_FAMILY == 9860))
static void oledWrite(uint8_t dta, uint8_t typ, uint32_t csn)
{
    setGpioValue(csn, 0);
    // oledWait(WAIT_TICK);
    if(typ == LCD_DTA)
        ithWriteRegMaskA(LCD_COMMAND_REG, 0xB000+dta, 0xFFFF);
    else
        ithWriteRegMaskA(LCD_COMMAND_REG, 0xA000+dta, 0xFFFF);
    oledWait(WAIT_TICK);
    setGpioValue(csn, 1);
    // oledWait(WAIT_TICK);
}
#else
static void oledWrite(uint8_t dta, uint8_t typ, uint32_t csn)
{
    setGpioValue(csn, 0);
    // oledWait(WAIT_TICK);
    if(typ == LCD_DTA)
        ithWriteRegH(LCD_COMMAND_REG, 0xB000+dta);
    else
        ithWriteRegH(LCD_COMMAND_REG, 0xA000+dta);
    oledWait(WAIT_TICK);
    setGpioValue(csn, 1);
    // oledWait(WAIT_TICK);
}
#endif

static void oledSetAddr(uint32_t pag, uint32_t colOffset, uint32_t csn)
{
    oledWrite(CMD_SET_PAGE, LCD_CMD, csn);
    oledWrite(pag, LCD_CMD, csn);
    oledWrite(CMD_SET_COLUMN_LOWER | (colOffset&0x0f), LCD_CMD, csn);
    oledWrite((CMD_SET_COLUMN_UPPER | (colOffset>>4)), LCD_CMD, csn);
}

static void oledCtrlProcessInitCmd(void)
{
    OLED_CTRL_INIT* ptInitData = (OLED_CTRL_INIT*) CMD_DATA_BUFFER_OFFSET;
    uint32_t oledId = ENDIAN_SWAP32(ptInitData->oledId);
    OLED_CTRL_HANDLE *ptOledCtrlHandle = 0;

    if (oledId >= OLED0 && oledId < OLED_COUNT)
    {
        ptOledCtrlHandle = &gtpOledCtrlHandle[oledId];
        ptOledCtrlHandle->gpioCSN   = ENDIAN_SWAP32(ptInitData->gpioCSN);
        ptOledCtrlHandle->colOffset = ENDIAN_SWAP32(ptInitData->colOffset);
        ptOledCtrlHandle->maxRow    = ENDIAN_SWAP32(ptInitData->maxRow);
        ptOledCtrlHandle->maxCol    = ENDIAN_SWAP32(ptInitData->maxCol);
        ptOledCtrlHandle->buffAddrA = (uint8_t *) (ENDIAN_SWAP32(ptInitData->buffAddrA));
        ptOledCtrlHandle->buffAddrB = (uint8_t *) (ENDIAN_SWAP32(ptInitData->buffAddrB));
        ptOledCtrlHandle->buffAddrC = (uint8_t *) (ENDIAN_SWAP32(ptInitData->buffAddrC));

        // set CSN
        setGpioMode(ptOledCtrlHandle->gpioCSN, ITH_GPIO_MODE0);
        setGpioDir(ptOledCtrlHandle->gpioCSN, 0); // GPIO_OUTPUT_MODE
        setGpioValue(ptOledCtrlHandle->gpioCSN, 1);

        ptOledCtrlHandle->state = OLED_CTRL_STATE_INIT;
        *gtpOledCtrlState = OLED_CTRL_STATE_INIT;
    }
}

static void oledCtrlProcessFlipCmd(void)
{
    OLED_CTRL_FLIP* ptInitData = (OLED_CTRL_FLIP*) CMD_DATA_BUFFER_OFFSET;
    uint32_t oledId = ENDIAN_SWAP32(ptInitData->oledId);
    OLED_CTRL_HANDLE *ptOledCtrlHandle = 0;

    if (oledId >= OLED0 && oledId < OLED_COUNT)
    {
        ptOledCtrlHandle = &gtpOledCtrlHandle[oledId];
        ptOledCtrlHandle->addrIndex = ENDIAN_SWAP32(ptInitData->addrIndex);
        ptOledCtrlHandle->state = OLED_CTRL_STATE_WRITE;
        *gtpOledCtrlState = OLED_CTRL_STATE_WRITE;
    }
}

static void oledCtrlProcessDispModeCmd(void)
{
    OLED_CTRL_DISPMODE* ptInitData = (OLED_CTRL_DISPMODE*) CMD_DATA_BUFFER_OFFSET;
    uint32_t oledId = ENDIAN_SWAP32(ptInitData->oledId);
    uint32_t dispMode = ENDIAN_SWAP32(ptInitData->dispMode);
    OLED_CTRL_HANDLE *ptOledCtrlHandle = 0;

    if (oledId >= OLED0 && oledId < OLED_COUNT)
    {
        ptOledCtrlHandle = &gtpOledCtrlHandle[oledId];
        if(dispMode == DISP_ON) {
            //Display ON
            oledWrite(CMD_DISPLAY_ON, LCD_CMD, ptOledCtrlHandle->gpioCSN);
        } else {
            //Display OFF
            oledWrite(CMD_DISPLAY_OFF, LCD_CMD, ptOledCtrlHandle->gpioCSN);
        }
    }
}

static void oledCtrlFlushScreen(void)
{
    uint32_t oledId = OLED0;
    uint32_t idleCnt = 0;
    OLED_CTRL_HANDLE *ptOledCtrlHandle = 0;
    for (oledId = OLED0; oledId < OLED_COUNT; oledId++)
    {
        ptOledCtrlHandle = &gtpOledCtrlHandle[oledId];
        switch (ptOledCtrlHandle->state)
        {
            case OLED_CTRL_STATE_IDEL:
                idleCnt++;
            break;
            case OLED_CTRL_STATE_INIT:
            {
                //clear screen
                uint32_t row, col = 0;
                for(row = 0; row < ptOledCtrlHandle->maxRow; row++)
                {
                    oledSetAddr(row, ptOledCtrlHandle->colOffset, ptOledCtrlHandle->gpioCSN);
                    for(col = 0; col < ptOledCtrlHandle->maxCol; col++)
                    {
                        oledWrite(0, LCD_DTA, ptOledCtrlHandle->gpioCSN);
                    }
                }
                //Display ON
                oledWrite(CMD_DISPLAY_ON, LCD_CMD, ptOledCtrlHandle->gpioCSN);
                ptOledCtrlHandle->state = OLED_CTRL_STATE_IDEL;
                idleCnt++;
            }
            break;
            case OLED_CTRL_STATE_WRITE:
            {
                int row, col, piexl = 0;
                uint8_t* buffAddr[3] = {ptOledCtrlHandle->buffAddrA, ptOledCtrlHandle->buffAddrB, ptOledCtrlHandle->buffAddrC};
                for(row = 0; row < ptOledCtrlHandle->maxRow; row++)
                {
                    oledSetAddr(row, ptOledCtrlHandle->colOffset, ptOledCtrlHandle->gpioCSN);
                    for(col = 0; col < ptOledCtrlHandle->maxCol; col++)
                    {
                        oledWrite(buffAddr[ptOledCtrlHandle->addrIndex][piexl], LCD_DTA, ptOledCtrlHandle->gpioCSN);
                        piexl++;
                    }
                }
                ptOledCtrlHandle->state = OLED_CTRL_STATE_IDEL;
                idleCnt++;
            }
            break;
            default:
            break;
        }
    }
    if(idleCnt == OLED_COUNT && *gtpOledCtrlState != OLED_CTRL_STATE_IDEL) *gtpOledCtrlState = OLED_CTRL_STATE_IDEL;
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
                    oledCtrlProcessInitCmd();
                    break;
                case FLIP_CMD_ID:
                    oledCtrlProcessFlipCmd();
                    break;
                case DISPMODE_CMD_ID:
                    oledCtrlProcessDispModeCmd();
                    break;
                default:
                    break;
            }
            ALT_CPU_COMMAND_REG_WRITE(RESPONSE_CMD_REG, (uint16_t) inputCmd);
        }
        oledCtrlFlushScreen();
    }
}
