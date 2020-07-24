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

//=============================================================================
//                              Structure Definition
//=============================================================================

//=============================================================================
//                              Enumeration Type Definition
//=============================================================================

//=============================================================================
//                              Constant Definition
//=============================================================================

#define RISC1_MISC_REG         (ITH_RISC_BASE + 0x1C)
#define RISC1_STALL_BIT_OFFSET 1
#define RISC1_FIRE_BIT_OFFSET  0

#define RISC1_RESET_REG        (ITH_HOST_BASE + 0x50)
#define RISC1_RESET_BIT_OFFSET 30
#define RISC1_CLOCK_REG        RISC1_RESET_REG
#define RISC1_CLOCK_BIT_OFFSET 2
#define RISC1_CLOCK_ON_VAL     0x3
#define RISC1_CLOCK_OFF_VAL    0x0
#define RISC1_CLOCK_VAL_MASK   0x3

#define RISC1_PC               (ITH_RISC_BASE + 0x10)

#define RISC2_MISC_REG         (ITH_RISC_BASE + 0x3C)
#define RISC2_STALL_BIT_OFFSET 1
#define RISC2_FIRE_BIT_OFFSET  0

#define RISC2_RESET_REG        (ITH_HOST_BASE + 0x50)
#define RISC2_RESET_BIT_OFFSET 29
#define RISC2_CLOCK_REG        RISC2_RESET_REG
#define RISC2_CLOCK_BIT_OFFSET 4
#define RISC2_CLOCK_ON_VAL     0x3
#define RISC2_CLOCK_OFF_VAL    0x0
#define RISC2_CLOCK_VAL_MASK   0x3

#define RISC2_PC               (ITH_RISC_BASE + 0x30)

#define RISC3_MISC_REG         (ITH_RISC_BASE + 0x6C)
#define RISC3_STALL_BIT_OFFSET 1
#define RISC3_FIRE_BIT_OFFSET  0

#define RISC3_RESET_REG        (ITH_HOST_BASE + 0x50)
#define RISC3_RESET_BIT_OFFSET 27

#define RISC3_CLOCK_REG        RISC3_RESET_REG
#define RISC3_CLOCK_BIT_OFFSET 8
#define RISC3_CLOCK_ON_VAL     0x3
#define RISC3_CLOCK_OFF_VAL    0x0
#define RISC3_CLOCK_VAL_MASK   0x3


#define RISC3_PC               (ITH_RISC_BASE + 0x60)


#define RISC_REMAP_ADDR        (ITH_RISC_BASE + 0x74)

//RISC2 is running in SRAM
#define RISC3_IMG_ADDRESS      0xA0000000      

static uint8_t gRiscBuffer[RISC1_IMAGE_SIZE + RISC2_IMAGE_SIZE + AUDIO_MESSAGE_SIZE + SHARE_MEM1_SIZE + SHARE_MEM2_SIZE] __attribute__ ((aligned(32)));
static bool    gbInited = false;
static uint32_t gRisc1Setting, gRisc2Setting;

static uint8_t gRiscStartupCode[] = {
    #include "riscStartupCode.hex"
};

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
        memset(gRiscBuffer, 0x0, sizeof(gRiscBuffer));
        memcpy(gRiscBuffer, gRiscStartupCode, sizeof(gRiscStartupCode));
#ifdef CFG_CPU_WB
        ithFlushDCacheRange((void*)gRiscBuffer, sizeof(gRiscStartupCode));
        ithFlushMemBuffer();
#endif
        ithWriteRegA(RISC_REMAP_ADDR, startupAddr);

        gbInited = true;
    }
}
    
int
iteRiscTerminate(
    void)
{
	gRisc1Setting = ithReadRegA(RISC1_MISC_REG);
	gRisc2Setting = ithReadRegA(RISC2_MISC_REG);

	ithSetRegBitA(RISC1_MISC_REG, RISC1_STALL_BIT_OFFSET);
	ithSetRegBitA(RISC2_MISC_REG, RISC2_STALL_BIT_OFFSET);
}

void
iteRiscWakeup(
	void)
{
	if(!(gRisc1Setting&0x2)) {
		ithClearRegBitA(RISC1_MISC_REG, RISC1_STALL_BIT_OFFSET);
	}

	if(!(gRisc1Setting&0x2)) {
		ithClearRegBitA(RISC2_MISC_REG, RISC2_STALL_BIT_OFFSET);
	}
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
            return (uint8_t*) &gRiscBuffer[RISC1_IMAGE_SIZE];
        case RISC3_IMAGE_MEM_TARGET:
            return (uint8_t*) RISC3_IMG_ADDRESS;
        case AUDIO_MESSAGE_MEM_TARGET:
            return &gRiscBuffer[RISC1_IMAGE_SIZE + RISC2_IMAGE_SIZE];
        case SHARE_MEM1_TARGET:
            return &gRiscBuffer[RISC1_IMAGE_SIZE + RISC2_IMAGE_SIZE + AUDIO_MESSAGE_SIZE];
        case SHARE_MEM2_TARGET:
            return &gRiscBuffer[RISC1_IMAGE_SIZE + RISC2_IMAGE_SIZE + AUDIO_MESSAGE_SIZE + SHARE_MEM1_SIZE];
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
        case RISC3_IMAGE_MEM_TARGET:
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
    ithFlushDCacheRange((void*)iteRiscGetTargetMemAddress(loadTarget), dataSize);
    ithFlushMemBuffer();
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
            //Turn on Clock
            ithWriteRegMaskA(RISC1_CLOCK_REG, RISC1_CLOCK_ON_VAL << RISC1_CLOCK_BIT_OFFSET, RISC1_CLOCK_VAL_MASK << RISC1_CLOCK_BIT_OFFSET);
        
            ithWriteRegMaskA(RISC1_MISC_REG, ((0 << RISC1_STALL_BIT_OFFSET) | (1 << RISC1_FIRE_BIT_OFFSET)), ((1 << RISC1_STALL_BIT_OFFSET) | (1 << RISC1_FIRE_BIT_OFFSET)));
            for (i = 0; i < 10; i++)
                asm ("");
            ithWriteRegMaskA(RISC1_MISC_REG, (0 << RISC1_FIRE_BIT_OFFSET), (1 << RISC1_FIRE_BIT_OFFSET));  
            break;
        }
        case RISC2_CPU:
        {
            //Turn on Clock
            ithWriteRegMaskA(RISC2_CLOCK_REG, RISC2_CLOCK_ON_VAL << RISC2_CLOCK_BIT_OFFSET, RISC2_CLOCK_VAL_MASK << RISC2_CLOCK_BIT_OFFSET);
            
            ithWriteRegMaskA(RISC2_MISC_REG, ((0 << RISC2_STALL_BIT_OFFSET) | (1 << RISC2_FIRE_BIT_OFFSET)), ((1 << RISC2_STALL_BIT_OFFSET) | (1 << RISC2_FIRE_BIT_OFFSET)));
            for (i = 0; i < 10; i++)
                asm ("");
            ithWriteRegMaskA(RISC2_MISC_REG, (0 << RISC2_FIRE_BIT_OFFSET), (1 << RISC2_FIRE_BIT_OFFSET));  
            break;
        }
        case RISC3_CPU:
        {
            //Turn on Clock
            ithWriteRegMaskA(RISC3_CLOCK_REG, RISC3_CLOCK_ON_VAL << RISC3_CLOCK_BIT_OFFSET, RISC3_CLOCK_VAL_MASK << RISC3_CLOCK_BIT_OFFSET);

            ithWriteRegMaskA(RISC3_MISC_REG, ((0 << RISC3_STALL_BIT_OFFSET) | (1 << RISC3_FIRE_BIT_OFFSET)), ((1 << RISC3_STALL_BIT_OFFSET) | (1 << RISC3_FIRE_BIT_OFFSET)));
            for (i = 0; i < 10; i++)
                asm ("");
            ithWriteRegMaskA(RISC3_MISC_REG, (1 << RISC3_FIRE_BIT_OFFSET), (1 << RISC3_FIRE_BIT_OFFSET));          
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
            ithWriteRegMaskA(RISC1_MISC_REG, ((1 << RISC1_STALL_BIT_OFFSET) | (0 << RISC1_FIRE_BIT_OFFSET)), ((1 << RISC1_STALL_BIT_OFFSET) | (1 << RISC1_FIRE_BIT_OFFSET)));
            // reset risc cpu
            ithWriteRegMaskA(RISC1_RESET_REG, 1 << RISC1_RESET_BIT_OFFSET, 1 << RISC1_RESET_BIT_OFFSET);
            for (i = 0; i < 2048; i++)
                asm ("");
            ithWriteRegMaskA(RISC1_RESET_REG, 0 << RISC1_RESET_BIT_OFFSET, 1 << RISC1_RESET_BIT_OFFSET);
            
            //Turn off Clock
            ithWriteRegMaskA(RISC1_CLOCK_REG, RISC1_CLOCK_OFF_VAL << RISC1_CLOCK_BIT_OFFSET, RISC1_CLOCK_VAL_MASK << RISC1_CLOCK_BIT_OFFSET);
            break;
        }
        case RISC2_CPU:
        {
            ithWriteRegMaskA(RISC2_MISC_REG, ((1 << RISC2_STALL_BIT_OFFSET) | (0 << RISC2_FIRE_BIT_OFFSET)), ((1 << RISC2_STALL_BIT_OFFSET) | (1 << RISC2_FIRE_BIT_OFFSET)));
            // reset risc cpu
            ithWriteRegMaskA(RISC2_RESET_REG, 1 << RISC2_RESET_BIT_OFFSET, 1 << RISC2_RESET_BIT_OFFSET);
            for (i = 0; i < 2048; i++)
                asm ("");
            ithWriteRegMaskA(RISC2_RESET_REG, 0 << RISC2_RESET_BIT_OFFSET, 1 << RISC2_RESET_BIT_OFFSET);

            //Turn off Clock
            ithWriteRegMaskA(RISC2_CLOCK_REG, RISC2_CLOCK_OFF_VAL << RISC2_CLOCK_BIT_OFFSET, RISC2_CLOCK_VAL_MASK << RISC2_CLOCK_BIT_OFFSET);

            break;
        }
        case RISC3_CPU:
        {
            ithWriteRegMaskA(RISC3_MISC_REG, ((1 << RISC3_STALL_BIT_OFFSET) | (0 << RISC3_FIRE_BIT_OFFSET)), ((1 << RISC3_STALL_BIT_OFFSET) | (1 << RISC3_FIRE_BIT_OFFSET)));
            // reset risc cpu
            ithWriteRegMaskA(RISC3_RESET_REG, 1 << RISC3_RESET_BIT_OFFSET, 1 << RISC3_RESET_BIT_OFFSET);
            for (i = 0; i < 2048; i++)
                asm ("");
            ithWriteRegMaskA(RISC3_RESET_REG, 0 << RISC3_RESET_BIT_OFFSET, 1 << RISC3_RESET_BIT_OFFSET);

            //Turn off Clock
            ithWriteRegMaskA(RISC3_CLOCK_REG, RISC3_CLOCK_OFF_VAL << RISC3_CLOCK_BIT_OFFSET, RISC3_CLOCK_VAL_MASK << RISC3_CLOCK_BIT_OFFSET);
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
            pc = ithReadRegA(RISC1_PC);
            break;
        }
        case RISC2_CPU:
        {
            pc = ithReadRegA(RISC2_PC);
            break;
        }
        case RISC3_CPU:
        {
            pc = ithReadRegA(RISC3_PC);
            break;
        }
        default:
            return 0;
    }
    return pc;
}
