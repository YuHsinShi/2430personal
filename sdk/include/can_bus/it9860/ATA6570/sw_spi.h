/*
 * SW_SPI.h
 *
 */

#ifndef SW_SPI_H_
#define SW_SPI_H_

#include <sys/ioctl.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "ite/ith.h"
#include "ite/itp.h"

//ATA6570 REG LIST

//Device ID:0x74
#define _ATA6570_ADDR          0x74
//ATA6570 REG STA
#define _ATA6570_DMCR          0x01
#define _ATA6570_DMSR          0x03
#define _ATA6570_SECR          0x04
#define _ATA6570_GPM0          0x06
#define _ATA6570_GPM1          0x07
#define _ATA6570_GPM2          0x08
#define _ATA6570_GPM3          0x09
#define _ATA6570_RWPR          0x0A
#define _ATA6570_TRXCR         0x20
#define _ATA6570_TRXSR         0x22
#define _ATA6570_TRXECR        0x23
#define _ATA6570_DRCR          0x26
#define _ATA6570_CIDR0         0x27
#define _ATA6570_CIDR1         0x28
#define _ATA6570_CIDR2         0x29
#define _ATA6570_CIDR3         0x2A
#define _ATA6570_CIDMR0        0x2B
#define _ATA6570_CIDMR1        0x2C
#define _ATA6570_CIDMR2        0x2D
#define _ATA6570_CIDMR3        0x2E
#define _ATA6570_CFCR          0x2F
#define _ATA6570_BFECR         0x32
#define _ATA6570_BFIR          0x33
#define _ATA6570_TRXECR2       0x34
#define _ATA6570_TRXESR2       0x35
#define _ATA6570_WDCR1         0x36
#define _ATA6570_WDCR2         0x37
#define _ATA6570_WDSR          0x38
#define _ATA6570_WDTRIG        0x39
#define _ATA6570_EFCR          0x3A
#define _ATA6570_FECR          0x3B
#define _ATA6570_GLFT          0x67
#define _ATA6570_CDMR0         0x68
#define _ATA6570_CDMR1         0x69
#define _ATA6570_CDMR2         0x6A
#define _ATA6570_CDMR3         0x6B
#define _ATA6570_CDMR4         0x6C
#define _ATA6570_CDMR5         0x6D
#define _ATA6570_CDMR6         0x6E
#define _ATA6570_CDMR7         0x6F
#define _ATA6570_PWKS          0x4B
#define _ATA6570_WKECR         0x4C
#define _ATA6570_GESR          0x60
#define _ATA6570_SESR          0x61
#define _ATA6570_TRXESR        0x63
#define _ATA6570_WKESR         0x64
#define _ATA6570_DIDR          0x7E
#define _ATA6570_FUDI          0x70
#define _ATA6570_FUDO          0x71
#define _ATA6570_FUSEL         0x72
#define _ATA6570_BGCAL         0x73
#define _ATA6570_FRCCAL        0x74
#define _ATA6570_HRCCALL       0x75
#define _ATA6570_HRCCALH       0x76
//ATA6570 REG END
//MODE DEFINE
#define _ATA6570_OPMODE_SLEEP  0x01
#define _ATA6570_OPMODE_STBY   0x04
#define _ATA6570_OPMODE_NORMAL 0x07
//WAKEUP DATA RATE
#define _ATA6570_0050K         0x00
#define _ATA6570_0100K         0x01
#define _ATA6570_0125K         0x02
#define _ATA6570_0250K         0x03
#define _ATA6570_0500K         0x05
#define _ATA6570_1000K         0x07

void  spi_write_ata6570(uint8_t port, uint8_t addr, uint8_t value);
uint8_t spi_read_ata6570(uint8_t port, uint8_t addr);
void spi_init_ata6570(uint8_t port);

#endif /* SW_SPI_H_ */