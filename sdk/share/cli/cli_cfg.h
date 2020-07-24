/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * CLI configurations.
 *
 * @author Jim Tan
 * @version 1.0
 */
#ifndef CLI_CFG_H
#define CLI_CFG_H

// include paths
#include "ite/ith.h"
#include "ite/itp.h"
#include <stdio.h>
#define PRINTF printf

// Debug definition
#if !defined(NDEBUG) && !defined(DEBUG)
    #define DEBUG
#endif

#ifdef DEBUG
    #define ASSERT(e) ((e) ? (void) 0 : ithAssertFail(#e, __FILE__, __LINE__, __FUNCTION__))
#else
    #define ASSERT(e) ((void) 0)
#endif

/* Log fucntions definition */
#define STRINGIFY(x)    #x
#define TOSTRING(x)     STRINGIFY(x)

#define LOG_PREFIX   __FILE__ ":" TOSTRING(__LINE__) ": "

#ifdef CFG_CLI_ERR
    #define LOG_ERR     PRINTF("ERR:" LOG_PREFIX
#else
    #define LOG_ERR     (void)(1 ? 0 :
#endif

#ifdef CFG_CLI_WARN
    #define LOG_WARN    PRINTF("WARN:" LOG_PREFIX
#else
    #define LOG_WARN    (void)(1 ? 0 :
#endif

#ifdef CFG_CLI_INFO
    #define LOG_INFO    PRINTF("INFO:"
#else
    #define LOG_INFO    (void)(1 ? 0 :
#endif

#ifdef CFG_CLI_DBG
    #define LOG_DBG     PRINTF("DBG:"
#else
    #define LOG_DBG     (void)(1 ? 0 :
#endif

#define LOG_END         );

#endif // CLI_CFG_H
