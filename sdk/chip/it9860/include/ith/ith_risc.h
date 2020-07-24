/*
 * Copyright (c) 2007 ITE Technology Corp. All Rights Reserved.
 */
/** @file
 * ITE RISC 970 type definition File
 *
 */
//=============================================================================
//                              Include Files
//=============================================================================

//=============================================================================
//                              Structure Definition
//=============================================================================

//=============================================================================
//                              Enumeration Type Definition
//=============================================================================

//=============================================================================
//                              Constant Definition
//=============================================================================

#define RISC1_IMAGE_MEM_TARGET          0
#define RISC2_IMAGE_MEM_TARGET          1
#define AUDIO_MESSAGE_MEM_TARGET        2
#define SHARE_MEM1_TARGET               3
#define SHARE_MEM2_TARGET               4

#define RISC1_CPU                       0
#define RISC2_CPU                       1

#define ARMLITE_CPU                     RISC1_CPU
#define ARMLITE_CPU_IMAGE_MEM_TARGET    RISC1_IMAGE_MEM_TARGET

#define ALT_CPU                         RISC2_CPU
#define ALT_CPU_IMAGE_MEM_TARGET        RISC2_IMAGE_MEM_TARGET

#define RISC1_IMAGE_SIZE                (600 * 1024)
#define RISC2_IMAGE_SIZE                (32 * 1024)
#define AUDIO_MESSAGE_SIZE              (32 * 1024)
#define SHARE_MEM1_SIZE                 (16 * 1024)
#define SHARE_MEM2_SIZE                 (16 * 1024)