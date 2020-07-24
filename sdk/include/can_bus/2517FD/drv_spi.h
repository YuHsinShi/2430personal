/*******************************************************************************
  SPI Driver:  Header File

  Company:
    Microchip Technology Inc.

  File Name:
    drv_spi.h

  Summary:
    This header file contains the MCU specific SPI definitions and declarations.

  Description:
    .
 *******************************************************************************/

//DOM-IGNORE-BEGIN
/*******************************************************************************
Copyright (c) 2018 Microchip Technology Inc. and its subsidiaries.

Subject to your compliance with these terms, you may use Microchip software and 
any derivatives exclusively with Microchip products. It is your responsibility 
to comply with third party license terms applicable to your use of third party 
software (including open source software) that may accompany Microchip software.

THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER EXPRESS, 
IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED WARRANTIES 
OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A PARTICULAR PURPOSE.

IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, 
INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND WHATSOEVER 
RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS BEEN ADVISED OF 
THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE FULLEST EXTENT ALLOWED 
BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN ANY WAY RELATED TO 
THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY, THAT YOU HAVE PAID 
DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
 *******************************************************************************/
//DOM-IGNORE-END

#ifndef _DRV_SPI_H
#define _DRV_SPI_H

// Include files
//#include "asf.h"
//#include <stdint.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "ite/ith.h"
#include "ite/itp.h"
#include "ssp/mmp_spi.h"

// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility
extern "C" {
#endif
// DOM-IGNORE-END  

// Index to SPI channel
// Used when multiple MCP25xxFD are connected to the same SPI interface, but with different CS
#define DRV_CANFDSPI_INDEX_0         0
#define DRV_CANFDSPI_INDEX_1         1

// Index to SPI channel
// Used when multiple MCP25xxFD are connected to the same SPI interface, but with different CS
#define SPI_DEFAULT_BUFFER_LENGTH 96

// Code anchor for break points
#define Nop() usleep(200)//@@asm("nop")

//! SPI Initialization

void DRV_SPI_Initialize(uint8_t SpiID);

//! SPI Read/Write Transfer

int8_t DRV_SPI_TransferData(uint8_t spiSlaveDeviceIndex, uint8_t *SpiTxData, uint8_t *SpiRxData, uint16_t spiTransferSize,uint16_t spiReadSize);

#ifdef __cplusplus
}
#endif

#endif	// _DRV_SPI_H
