/*
 * Copyright (c) 2012 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * HAL H/W decompress functions.
 *
 * @author Joseph Chang
 * @version 1.0
 */
#include <stdio.h>
#include "ite/ith.h"
#include "ith_cfg.h"
//#include "stdlib.h"
//#include "string.h"

//#define ENABLE_DCPS_CMDQ
//=============================================================================
//                              macro defination
//=============================================================================
#define DCPS_IRQ_ENABLE
#define ENABLE_DCPS_LOG

#define ITH_DCPS_BASE                    (ITH_DPU_BASE + 0x100)

#define DECOMPRESS_SW_ERR_DONEINDEX_OVER 0x00000001
#define DECOMPRESS_HW_FAIL               0x00000004

#define DCPS_INTR_MASK                   0x00000001
#define DCPS_INTR_ENABLE                 0x00000001
#define DCPS_INTR_DISABLE                0x00000000

#define DCPS_INTR_CLEAR_MASK             0x00000002
#define DCPS_INTR_ENCLEAR                0x00000002
#define DCPS_INTR_DISCLEAR               0x00000000

#define DCPS_DPCLK_UPD                   11
#define DCPS_DPCLK_EN                    19
#define DCPS_M14CLK_EN                   21
#define DCPS_W19CLK_EN                   23
//=============================================================================
//                              LOG definition
//=============================================================================
#ifdef  ENABLE_DCPS_LOG
    #define ITH_DCPS_LOG_INFO            LOG_INFO
    #define ITH_DCPS_LOG_WARNING         LOG_INFO
    #define ITH_DCPS_LOG_DEBUG           LOG_INFO
#else
    #define ITH_DCPS_LOG_INFO            LOG_INFO
    #define ITH_DCPS_LOG_WARNING         LOG_WARN
    #define ITH_DCPS_LOG_DEBUG           LOG_DBG
#endif

#define ITH_DCPS_LOG_ERROR               LOG_ERR
#define ITH_DCPS_LOG_END                 LOG_END

//#define ITH_DCPS_LOG_ERROR        printf(
//#define ITH_DCPS_LOG_END      );

//=============================================================================
//                              Decompress Register definition
//=============================================================================
#define ITH_DCPS_REG_DCR0     (ITH_DCPS_BASE + 0x00)
#define ITH_DCPS_REG_DCR1     (ITH_DCPS_BASE + 0x04)
#define ITH_DCPS_REG_SAR      (ITH_DCPS_BASE + 0x08)
#define ITH_DCPS_REG_DAR      (ITH_DCPS_BASE + 0x0C)
#define ITH_DCPS_REG_SRC_SIZE (ITH_DCPS_BASE + 0x10)
#define ITH_DCPS_REG_DST_SIZE (ITH_DCPS_BASE + 0x14)
#define ITH_DCPS_REG_INTCT    (ITH_DCPS_BASE + 0x18)
#define ITH_DCPS_REG_DSR      (ITH_DCPS_BASE + 0x1C)
#define ITH_DCPS_REG_DDCR     (ITH_DCPS_BASE + 0x20)

#define ITH_DPCLK_CLOCK       (0xD800002C)
//=============================================================================
//                              TYPE definition
//=============================================================================
#ifdef  ENABLE_DCPS_TYPE
    #define DCPS_BOOL         bool
    #define DCPS_UINT8        uint8_t
    #define DCPS_UINT16       uint16_t
    #define DCPS_UINT32       uint32_t
#else
    #define DCPS_BOOL         bool
    #define DCPS_UINT8        unsigned char
    #define DCPS_UINT16       unsigned int
    #define DCPS_UINT32       unsigned long
#endif

#define DCPS_RESULT           DCPS_BOOL
/*****************************************************************/
//globol variable
/*****************************************************************/

static uint32_t g_DcpsSetIndex;
static uint32_t g_DcpsDoneIndex;
static uint32_t g_CmdQueRunIndex;
static uint32_t g_CmdQueDoneIndex;
static uint32_t g_DcpsStatus;
static uint32_t g_CmdCntIsZero;

static uint32_t g_IsrCounter;       //for temp
static uint32_t g_SwCmdQueDone;     //for temp

static uint32_t DcpsRegs[(ITH_DCPS_REG_DDCR - ITH_DCPS_REG_DCR0 + 4) / 4];

/*****************************************************************/
//function protocol type
/*****************************************************************/

#if defined(DCPS_IRQ_ENABLE)
void dcps_isr(void *data);
#endif

static DCPS_RESULT Decompress(ITH_DCPS_INFO *DecompressInfo);
/*****************************************************************/
//function implement
/*****************************************************************/
void ithDcpsEnIntr(void)
{
    ithWriteRegMaskA(ITH_DCPS_REG_INTCT, DCPS_INTR_ENABLE, DCPS_INTR_MASK); /** enable DCPS interrupt **/
}

void ithDcpsDisIntr(void)
{
    ithWriteRegMaskA(ITH_DCPS_REG_INTCT, DCPS_INTR_DISABLE, DCPS_INTR_MASK); /** disable DCPS interrupt **/
}

void ithDcpsInit(ITH_DCPS_INFO *DcpsInfo)
{
    //ITH_DCPS_LOG_INFO "DecompressInitial()!!\n" ITH_DCPS_LOG_END

    //initial Decompress register
    g_DcpsSetIndex    = 0;
    g_DcpsDoneIndex   = 0;
    g_CmdQueRunIndex  = 0;
    g_CmdQueDoneIndex = 0;
    g_DcpsStatus      = 0;
    g_CmdCntIsZero    = 0;

    g_IsrCounter      = 0;  //for test
    g_SwCmdQueDone    = 0;  //for test

    //#ifdef    ENABLE_PATCH_DCPS_AHB0_PRIORITY_ISSUE
    //ithPrintf("~ SET AHB0 Burst size ~\n");
    ithWriteRegMaskA(ITH_AHB0_BASE + 0x88, 0x00001F00, 0x0000FF00);     //adjust the AHB burst length
    //#endif

    //reset decompress engine
    if (!DcpsInfo->IsEnableComQ)
    {
#if 1   //for IT970 or IT9860
        ithWriteRegMaskA(0xD800002C, 0xC0000000, 0xC0000000);
        ithWriteRegMaskA(0xD800002C, 0x00000000, 0xC0000000);
#else
        ithWriteRegA(ITH_DCPS_REG_DCR1, 0x01);
#endif
    }
    DcpsInfo->TotalCmdqCount = ithReadRegA(ITH_DCPS_REG_DDCR);

    //ITH_DCPS_LOG_INFO "[mmpDpuTerminate] Leave\n" ITH_DCPS_LOG_END
}

bool ithDcpsFire(ITH_DCPS_INFO *DcpsInfo)
{
    bool result = true;

    //ITH_DCPS_LOG_INFO "[ithDcpsFire] Enter\n" ITH_DCPS_LOG_END

    ithWriteRegMaskA(0xD0900118, 0x00010200, 0x00ffff00);   //???

#ifdef  ENABLE_DCPS_CMDQ
    if (DcpsInfo->IsEnableComQ)
    {
        if (DcpsInfo->DcpsMode == 0)
        {
            uint32_t *cmdqAddr              = NULL;
            uint32_t *myAddr                = NULL;
            uint32_t ITH_CMDQ_BASE_LO_REG_A = 0xB0600000;
            /*
                ithPrintVram8((uint8_t*)DcpsInfo->srcbuf, 64);
                {
                    int i;
                    printf("show CMDQ reg: base:%x\n",ITH_CMDQ_BASE_LO_REG_A);
                    for(i=0; i<0x10; i++)
                    {
                        printf("%08X ",ithReadRegA(ITH_CMDQ_BASE_LO_REG_A + i*4));
                        if( (i&3)==3 )  printf("\n");
                    }
                }
                printf("\n");
             */
            ithCmdQLock(ITH_CMDQ0_OFFSET);
            cmdqAddr = ithCmdQWaitSize(ITH_CMDQ_SINGLE_CMD_SIZE * 5, ITH_CMDQ0_OFFSET);
            myAddr   = cmdqAddr;

            ITH_CMDQ_SINGLE_CMD(cmdqAddr, ITH_DCPS_REG_SAR,      DcpsInfo->srcbuf);
            ITH_CMDQ_SINGLE_CMD(cmdqAddr, ITH_DCPS_REG_DAR,      DcpsInfo->dstbuf);
            ITH_CMDQ_SINGLE_CMD(cmdqAddr, ITH_DCPS_REG_DST_SIZE, DcpsInfo->dstLen);
            ITH_CMDQ_SINGLE_CMD(cmdqAddr, ITH_DCPS_REG_SRC_SIZE, DcpsInfo->srcLen);
            ITH_CMDQ_SINGLE_CMD(cmdqAddr, ITH_DCPS_REG_DCR0,     0x000000A2);

            ithCmdQFlush(cmdqAddr, ITH_CMDQ0_OFFSET);
            ithCmdQUnlock(ITH_CMDQ0_OFFSET);
            //printf("RUN UCL cmdQ fire\n");
            //ITH_DCPS_LOG_ERROR "CmdQueCnt=%d\n",CmdQueCnt ITH_DCPS_LOG_END
        }

        if (DcpsInfo->DcpsMode)
        {
            uint32_t *cmdqAddr = NULL;
            uint32_t *myAddr   = NULL;

            ithCmdQLock(ITH_CMDQ0_OFFSET);
            cmdqAddr = ithCmdQWaitSize(ITH_CMDQ_SINGLE_CMD_SIZE * 5, ITH_CMDQ0_OFFSET);
            myAddr   = cmdqAddr;

            ITH_CMDQ_SINGLE_CMD(cmdqAddr, ITH_DCPS_REG_SAR,      DcpsInfo->srcbuf);
            ITH_CMDQ_SINGLE_CMD(cmdqAddr, ITH_DCPS_REG_DAR,      DcpsInfo->dstbuf);
            ITH_CMDQ_SINGLE_CMD(cmdqAddr, ITH_DCPS_REG_DST_SIZE, DcpsInfo->dstLen);
            ITH_CMDQ_SINGLE_CMD(cmdqAddr, ITH_DCPS_REG_SRC_SIZE, DcpsInfo->srcLen);

            if (DcpsInfo->DcpsMode == 1)
            {
                ithWriteRegA(ITH_DCPS_REG_DCR0, 0x0000002A);
                printf("RUN BRFLZ compress cmdQ fire\n");
            }
            else if (DcpsInfo->DcpsMode == 2)
            {
                if (DcpsInfo->LzCpsBytePerPxl == 1)
                {
                    ithWriteRegA(ITH_DCPS_REG_DCR0, 0x0000012E);         //1-bit brflz format
                }
                else if (DcpsInfo->LzCpsBytePerPxl == 2)
                {
                    ithWriteRegA(ITH_DCPS_REG_DCR0, 0x0000022E);         //2-bit brflz format
                }
                else
                {
                    ithWriteRegA(ITH_DCPS_REG_DCR0, 0x0000042E);         //4-bit brflz format
                }
                printf("RUN BRFLZ decompress cmdQ fire\n");
            }
            else
            {
                printf("CMDQ ERROR\n");
            }

            ithCmdQFlush(cmdqAddr, ITH_CMDQ0_OFFSET);
            ithCmdQUnlock(ITH_CMDQ0_OFFSET);
            //printf("RUN BRFLZ cmdQ fire\n");
        }
    }
    else
    {
#endif
    ithWriteRegA(ITH_DCPS_REG_SAR,      (uint32_t)DcpsInfo->srcbuf); //source start address in byte
    ithWriteRegA(ITH_DCPS_REG_DAR,      (uint32_t)DcpsInfo->dstbuf); //distination start address

    ithWriteRegA(ITH_DCPS_REG_SRC_SIZE, DcpsInfo->srcLen);           //source size in byte
    ithWriteRegA(ITH_DCPS_REG_DST_SIZE, DcpsInfo->dstLen);           //decompress desitination size in byte

    if (DcpsInfo->DcpsMode == 2)
    {
        uint32_t reg32;
        reg32 = (DcpsInfo->LzCpsBytePerPxl << 8) | 0x0000002E;
        ithWriteRegA(ITH_DCPS_REG_DCR0, reg32);             //brflz format

        //printf("DCPS MODE:BRFLZ\n");
    }
    else if (DcpsInfo->DcpsMode == 1)
    {
        //printf("DCPS MODE:BRFLZ\n");
        ithWriteRegA(ITH_DCPS_REG_DCR0, 0x0000002A);            //fire DPU with LZ 0x2E 1010 --> 1110
    }
    else
    {
        //printf("DCPS MODE:UCL\n");
        ithWriteRegA(ITH_DCPS_REG_DCR0, 0x00000022);            //fire DPU UCL
    }

#ifdef  ENABLE_DCPS_CMDQ
}

#endif
    //ITH_DCPS_LOG_INFO "[ithDcpsFire] Leave\n" ITH_DCPS_LOG_END

    return result;
}

void ithDcpsGetStatus(ITH_DCPS_INFO *DcpsInfo)
{
    //return the decompress status
    if (g_CmdCntIsZero)
    {
        g_DcpsStatus |= 0x10000000;
    }
    else
    {
        g_DcpsStatus &= ~0x10000000;
    }

    if (g_CmdQueDoneIndex == (g_CmdQueRunIndex - 1))
    {
        uint32_t QueCnt = 0;
        QueCnt = ithReadRegA(ITH_DCPS_REG_DDCR);
        if (QueCnt == g_CmdQueRunIndex)
        {
            ITH_DCPS_LOG_ERROR "error, ISR not done!!\n" ITH_DCPS_LOG_END
            g_CmdQueDoneIndex++;
            g_DcpsDoneIndex++;
        }
    }
    DcpsInfo->RegDcpsStatus  = ithReadRegA(ITH_DCPS_REG_DSR);
    DcpsInfo->RegDcpsCmdqCnt = ithReadRegA(ITH_DCPS_REG_DDCR);
}

void ithDcpsGetCmdqCount(ITH_DCPS_INFO *DcpsInfo)
{
    DcpsInfo->TotalCmdqCount = ithReadRegA(ITH_DCPS_REG_DDCR);
}

void ithDcpsWait(ITH_DCPS_INFO *DcpsInfo)
{
    uint32_t Reg32, cnt = 0;
    do
    {
        Reg32 = ithReadRegA(ITH_DCPS_REG_DSR);
        cnt++;

        if (cnt >= 100000)
        {
            printf("DCPS time out \n");
            break;
        }
    } while ((Reg32 & 0x00000001) != 0x00000001);

    if (cnt >= 100000) DcpsInfo->RegDcpsStatus = Reg32 | 0x80000000; //time out flag
    else DcpsInfo->RegDcpsStatus = Reg32;
}

void ithDcpsGetDoneLen(uint32_t *DcpsDoneLen)
{
    *DcpsDoneLen = ithReadRegA(ITH_DCPS_REG_DST_SIZE);
}

void ithDcpsClearIntr(void)
{
    ithWriteRegMaskA(ITH_DCPS_REG_INTCT, DCPS_INTR_ENCLEAR,  DCPS_INTR_CLEAR_MASK); /** enable clear DCPS interrupt **/
    ithWriteRegMaskA(ITH_DCPS_REG_INTCT, DCPS_INTR_DISCLEAR, DCPS_INTR_CLEAR_MASK); /** disable clear DCPS interrupt **/
}

void ithDcpsExit(void)
{
    //ITH_DCPS_LOG_INFO "[ithDcpsClose] Enter\n" ITH_DCPS_LOG_END

    g_DcpsSetIndex    = 0;
    g_DcpsDoneIndex   = 0;
    g_CmdQueRunIndex  = 0;
    g_CmdQueDoneIndex = 0;
    g_DcpsStatus      = 0;

    //ITH_DCPS_LOG_INFO "[ithDcpsClose] Leave\n" ITH_DCPS_LOG_END

    //return true;
}

void ithDcpsSuspend(void)
{
    unsigned int Reg32, TomeOutCnt = 0;
    int          i;

    //wait DCPS idle
    do
    {
        Reg32 = ithReadRegA(ITH_DCPS_REG_DSR);
        if (TomeOutCnt++ >= 10000)
        {
            ithPrintf("[dcps][warning]: TimeOutCnt,reg=%x\n", Reg32);
            TomeOutCnt = 0;
        }
    } while ((Reg32 & 0x00000002) == 0x00000002);

    //backup DCPS registers
    for (i = ITH_DCPS_REG_DCR0; i < ITH_DCPS_REG_DDCR; i += 4)
    {
        switch (i)
        {
        case ITH_DCPS_REG_INTCT:
            DcpsRegs[(i - ITH_DCPS_REG_DCR0) / 4] = ithReadRegA(i);
            ithPrintf("suspend, backup dpu reg,[%x,%08x][%x,%x,%x]\n", ITH_DCPS_REG_INTCT, DcpsRegs[(i - ITH_DCPS_REG_DCR0) / 4], i, (i - ITH_DCPS_REG_DCR0) / 4, ITH_DCPS_REG_DCR0);
            break;

        default:
            // don't need to backup
            break;
        }
    }
}

void ithDcpsResume(void)
{
    int i;

    //restore DCPS registers
    for (i = ITH_DCPS_REG_DCR0; i < ITH_DCPS_REG_DDCR; i += 4)
    {
        switch (i)
        {
        case ITH_DCPS_REG_INTCT:
            ithWriteRegA(i, DcpsRegs[(i - ITH_DCPS_REG_DCR0) / 4]);
            break;

        default:
            // don't need to backup
            break;
        }
    }
}

void ithDcpsResetEngine(void)
{
    ithWriteRegMaskA(0xD800002C, 0xC0000000, 0xC0000000);
    ithWriteRegMaskA(0xD800002C, 0x00000000, 0xC0000000);
}

void ithDcpsEnableClock(void)
{
    ithSetRegBitA(ITH_DPCLK_CLOCK, DCPS_DPCLK_EN);
    ithSetRegBitA(ITH_DPCLK_CLOCK, DCPS_M14CLK_EN);
    ithSetRegBitA(ITH_DPCLK_CLOCK, DCPS_W19CLK_EN);
}

void ithDcpsDisableClock(void)
{
    ithClearRegBitA(ITH_DPCLK_CLOCK, DCPS_DPCLK_EN);
    ithClearRegBitA(ITH_DPCLK_CLOCK, DCPS_M14CLK_EN);
    ithClearRegBitA(ITH_DPCLK_CLOCK, DCPS_W19CLK_EN);
}