/*
 * Copyright (c) 2007 ITE Technology Corp. All Rights Reserved.
 */
/** @file
 * ITE RISC header File
 *
 */

//=============================================================================
//                              Include Files
//=============================================================================
#include "ite/ith.h"
#include "ite/itp.h"
#include "stdlib.h"
#include "string.h"
#include "ite/ite_risc.h"
#include "stdio.h"
//=============================================================================
//                              Structure Definition
//=============================================================================

//=============================================================================
//                              Enumeration Type Definition
//=============================================================================

//=============================================================================
//                              Constant Definition
//=============================================================================
#define ARM_LITE_BASE_ADDR	   		0xD8400000
#define ARM_LITE_CTRL_REG      		(ARM_LITE_BASE_ADDR + 0x0)
#define ARM_LITE_FIRE_BIT_OFFSET  	0
#define ARM_LITE_STALL_BIT_OFFSET  	8

#define ARM_LITE_RESET_REG	   		0xD800000C
#define ARM_LITE_RESET_BIT_OFFSET  	29

#define ARM_LITE_PC					(ARM_LITE_BASE_ADDR + 0xC)
#define ARM_LITE_INST_ADDR          (ARM_LITE_BASE_ADDR + 0x20)
#define ARM_LITE_REMAP_ADDR         (ARM_LITE_BASE_ADDR + 0x24)

//ALT CPU is running in SRAM
#define RISC_BASE_ADDR				0xB0200000
#define RISC_REMAP_ADDR				(RISC_BASE_ADDR + 0x74)
#define ALT_CPU_IMG_ADDRESS      	0xA0000000
#define ALT_CPU_CTRL_REG			(RISC_BASE_ADDR + 0x6C)
#define ALT_CPU_FIRE_BIT_OFFSET		(0)
#define ALT_CPU_STALL_BIT_OFFSET	(3)

#define ALT_CPU_RESET_REG	   		 0xD8000050
#define ALT_CPU_RESET_BIT_OFFSET  	 27
#define ALT_CPU_EN_CLK_BIT_OFFSET  	 9
#define ALT_CPU_EN_DG_CLK_BIT_OFFSET 8
#define ALP_CPU_PC					 (RISC_BASE_ADDR + 0x60)

static uint8_t gRiscBuffer[RISC1_IMAGE_SIZE + AUDIO_MESSAGE_SIZE + SHARE_MEM1_SIZE + SHARE_MEM2_SIZE] __attribute__ ((aligned(32)));
static bool    gbInited = false;

//=============================================================================
//                              Function Declaration
//=============================================================================
int
iteRiscInit(
    void)
{
    if (gbInited == false)
    {
        uint32_t startupAddr = (uint32_t)gRiscBuffer;
		uint8_t* testPtr = (uint8_t*)(startupAddr);
        memset(gRiscBuffer, 0x0, sizeof(gRiscBuffer));
        ithWriteRegA(ARM_LITE_REMAP_ADDR, startupAddr);
        ithWriteRegA(ARM_LITE_INST_ADDR, startupAddr);
        ithWriteRegA(RISC_REMAP_ADDR, startupAddr);
        //ithWriteRegA(RISC_REMAP_ADDR, 0);
        gbInited = true;
    }
}
    
int
iteRiscTerminate(
    void)
{
}

void
iteRiscWakeup(
	void)
{
}


uint8_t*
iteRiscGetTargetMemAddress(
    int loadTarget)
{
    switch (loadTarget)
    {
        case RISC1_IMAGE_MEM_TARGET:
            return gRiscBuffer;
        case RISC2_IMAGE_MEM_TARGET:
            return (uint8_t*) ALT_CPU_IMG_ADDRESS;
        case AUDIO_MESSAGE_MEM_TARGET:
            return &gRiscBuffer[RISC1_IMAGE_SIZE];
        case SHARE_MEM1_TARGET:
            return &gRiscBuffer[RISC1_IMAGE_SIZE + AUDIO_MESSAGE_SIZE];
        case SHARE_MEM2_TARGET:
            return &gRiscBuffer[RISC1_IMAGE_SIZE + AUDIO_MESSAGE_SIZE + SHARE_MEM1_SIZE];
        default:
            return 0;
    }
}

int
iteRiscLoadData(
    int             loadTarget,
    uint8_t*        pData,
    int             dataSize)
{
    switch (loadTarget)
    {
        case RISC1_IMAGE_MEM_TARGET:
            if (dataSize > RISC1_IMAGE_SIZE)
            {
                return INVALID_LOAD_SIZE;
            }
            break;
        case RISC2_IMAGE_MEM_TARGET:
            if (dataSize > RISC2_IMAGE_SIZE)
            {
                return INVALID_LOAD_SIZE;
            }
            break;
        case AUDIO_MESSAGE_MEM_TARGET:
            if (dataSize > AUDIO_MESSAGE_SIZE)
            {
                return INVALID_LOAD_SIZE;
            }
            break;
        case SHARE_MEM1_TARGET:
            if (dataSize > SHARE_MEM1_SIZE)
            {
                return INVALID_LOAD_SIZE;
            }
            break;
        case SHARE_MEM2_TARGET:
            if (dataSize > SHARE_MEM2_SIZE)
            {
                return INVALID_LOAD_SIZE;
            }
            break;
        default:
            return INVALID_MEM_TARGET;
    }
	
	memcpy(iteRiscGetTargetMemAddress(loadTarget), pData, dataSize);

#ifdef CFG_CPU_WB
    if (loadTarget != RISC2_IMAGE_MEM_TARGET)
    {
        ithFlushDCacheRange((void*)iteRiscGetTargetMemAddress(loadTarget), dataSize);
        ithFlushMemBuffer();
    }
#endif

    return ITE_RISC_OK_RESULT;
}

void
iteRiscFireCpu(
    int             targetCpu)
{
    int i;
    switch(targetCpu)
    {
        case RISC1_CPU:
        {        
			ithWriteRegMaskA(ARM_LITE_CTRL_REG, (1 << ARM_LITE_FIRE_BIT_OFFSET) | (0 << ARM_LITE_STALL_BIT_OFFSET), (1 << ARM_LITE_FIRE_BIT_OFFSET) | (1 << ARM_LITE_STALL_BIT_OFFSET));
			break;
        }
		case RISC2_CPU:
		{
			ithWriteRegMaskA(ALT_CPU_CTRL_REG, (1 << ALT_CPU_FIRE_BIT_OFFSET) | (0 << ALT_CPU_STALL_BIT_OFFSET), (1 << ALT_CPU_FIRE_BIT_OFFSET) | (1 << ALT_CPU_STALL_BIT_OFFSET));
			break;
		}
        default:
            return;
    }
}

void
iteRiscResetCpu(
    int             targetCpu)
{
    int i;
    switch(targetCpu)
    {
        case RISC1_CPU:
        {
            ithWriteRegMaskA(ARM_LITE_CTRL_REG, (1 << ARM_LITE_STALL_BIT_OFFSET), (1 << ARM_LITE_STALL_BIT_OFFSET));
            for (i = 0; i < 10; i++)
                asm ("");
            ithWriteRegMaskA(ARM_LITE_RESET_REG, (1 << ARM_LITE_RESET_BIT_OFFSET), (1 << ARM_LITE_RESET_BIT_OFFSET));
            for (i = 0; i < 10; i++)
                asm ("");
            ithWriteRegMaskA(ARM_LITE_RESET_REG, (0 << ARM_LITE_RESET_BIT_OFFSET), (1 << ARM_LITE_RESET_BIT_OFFSET));
            break;
        }
		case RISC2_CPU:
        {
            ithWriteRegMaskA(ALT_CPU_CTRL_REG, (1 << ALT_CPU_STALL_BIT_OFFSET), (1 << ALT_CPU_STALL_BIT_OFFSET));
			
            for (i = 0; i < 10; i++)
			{
                asm ("");
			}
            ithWriteRegMaskA(ALT_CPU_RESET_REG, (1 << ALT_CPU_RESET_BIT_OFFSET) | (0 << ALT_CPU_EN_CLK_BIT_OFFSET) | (0 << ALT_CPU_EN_DG_CLK_BIT_OFFSET), (1 << ALT_CPU_RESET_BIT_OFFSET) | (1 << ALT_CPU_EN_CLK_BIT_OFFSET) | (1 << ALT_CPU_EN_DG_CLK_BIT_OFFSET));
            for (i = 0; i < 10; i++)
                asm ("");
            ithWriteRegMaskA(ALT_CPU_RESET_REG, (0 << ALT_CPU_RESET_BIT_OFFSET) | (1 << ALT_CPU_EN_CLK_BIT_OFFSET) | (1 << ALT_CPU_EN_DG_CLK_BIT_OFFSET), (1 << ALT_CPU_RESET_BIT_OFFSET) | (1 << ALT_CPU_EN_CLK_BIT_OFFSET) | (1 << ALT_CPU_EN_DG_CLK_BIT_OFFSET));
            break;
        }
        default:
            return;
    }
}

uint32_t
iteRiscGetProgramCounter(
    int         targetCpu)
{
    uint32_t pc = 0;
    switch(targetCpu)
    {
        case RISC1_CPU:
        {
            pc = ithReadRegA(ARM_LITE_PC);
            break;
        }
        case RISC2_CPU:
        {
            pc = ithReadRegA(ALP_CPU_PC);
            break;
        }
        default:
            return 0;
    }
    return pc;
}
