/*
 * Copyright (c) 2007 ITE Technology Corp. All Rights Reserved.
 */
/** @file
 * ITE RISC header File
 *
 */

#ifndef ITE_RISC_H
#define ITE_RISC_H

//=============================================================================
//                              Include Files
//=============================================================================
#ifdef __cplusplus
extern "C" {
#endif

#include "ith/ith_risc.h"

//=============================================================================
//                              Structure Definition
//=============================================================================

//=============================================================================
//                              Enumeration Type Definition
//=============================================================================

typedef enum ITE_RISC_ENGINE_TAG
{
    ITE_SW_PERIPHERAL_ENGINE = 1,    /**< SW peripheral engine */
    ITE_RISC_ENGINE_RESERVED
} ITE_RISC_ENGINE;

//=============================================================================
//                              Constant Definition
//=============================================================================

#define ITE_RISC_OK_RESULT              0
#define INVALID_MEM_TARGET              -1
#define INVALID_LOAD_SIZE               -2

//=============================================================================
//                              Function Declaration
//=============================================================================
int
iteRiscInit(
    void);
    
int
iteRiscTerminate(
    void);

void
iteRiscWakeup(
	void);
    
int
iteRiscLoadData(
    int             loadTarget,
    uint8_t*        pData,
    int             dataSize);

void
iteRiscFireCpu(
    int             targetCpu);

void
iteRiscResetCpu(
    int             targetCpu);

uint32_t
iteRiscGetProgramCounter(
    int             targetCpu);

uint8_t*
iteRiscGetTargetMemAddress(
    int             loadTarget);


int32_t
iteRiscOpenEngine(
    ITE_RISC_ENGINE engine_type, uint32_t bootmode);

int32_t
iteRiscTerminateEngine(
    void);
#ifdef __cplusplus
}
#endif

#endif