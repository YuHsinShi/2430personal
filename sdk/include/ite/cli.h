/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * Command Line Interface functions.
 *
 * @author Jim Tan
 * @version 1.0
 */
/** @defgroup cli ITE Command Line Interface Library
 *  @{
 */
#ifndef CLI_H
#define CLI_H

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * Initializes CLI library.
 */
void cliInit(void);

/**
 * Terminates CLI library.
 */
void cliExit(void);

#ifdef __cplusplus
}
#endif

#endif // CLI_H
/** @} */ // end of cli