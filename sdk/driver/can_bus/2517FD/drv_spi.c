/*******************************************************************************
  SPI Driver:  Implementation

  Company:
    Microchip Technology Inc.

  File Name:
    drv_spi.c

  Summary:
    Implementation of MCU specific SPI functions.

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

// Include files
#include "drv_spi.h"
#include "conf_board.h"
#include "conf_clock.h"

#if 0
/* Chip select. */
#define SPI_CHIP_SEL 0
//#define SPI_CHIP_PCS spi_get_pcs(SPI_CHIP_SEL)

//#define SPI_NCS_PIN SPI0_NPCS1_GPIO

/* Clock polarity. */
#define SPI_CLK_POLARITY 0

/* Clock phase. */
#define SPI_CLK_PHASE 1

/* Delay before SPCK. */
#define SPI_DLYBS 0x00

/* Delay between consecutive transfers. */
#define SPI_DLYBCT 0x00

/* Clock Speed */
#define SPI_BAUDRATE 12500000

/* Local function prototypes */
inline void spi_master_init(void);
inline int8_t spi_master_transfer(uint8_t *SpiTxData, uint8_t *SpiRxData, uint16_t spiTransferSize);
#endif

static int fd;
static int fd_1;


void DRV_SPI_Initialize(uint8_t SpiID)
{
    if(SpiID == 1)
    {
    	itpRegisterDevice(ITP_DEVICE_SPI1, &itpDeviceSpi1);
        ioctl(ITP_DEVICE_SPI1, ITP_IOCTL_INIT, NULL);

    	fd_1 = open(":spi1", O_RDONLY);
    	if (!fd_1)
    		printf("--- open device spi1 fail ---\n");
    	else
    		printf("fd_1 = %d\n", fd_1);
    }
    else
    {
    	itpRegisterDevice(ITP_DEVICE_SPI, &itpDeviceSpi0);
        ioctl(ITP_DEVICE_SPI, ITP_IOCTL_INIT, NULL);

    	fd = open(":spi0", O_RDONLY);
    	if (!fd)
    		printf("--- open device spi0 fail ---\n");
    	else
    		printf("fd = %d\n", fd);
    }

}

int8_t DRV_SPI_TransferData(uint8_t spiSlaveDeviceIndex, uint8_t *SpiTxData, uint8_t *SpiRxData, uint16_t spiTransferSize, uint16_t spiReadSize)
{
    int8_t result = -1;
	ITPSpiInfo SpiInfo = {0};
    uint8_t command  = 0;
    int     SPI_handle = 0;
    command = (SpiTxData[0] & 0xF0) >> 4;
    //printf("command = %x\n", command);
    if(spiSlaveDeviceIndex == 0)
    {
        SPI_handle = fd;
    }
    else
    {
        SPI_handle = fd_1;
    }
    //command : cINSTRUCTION_READ 0x03 ,cINSTRUCTION_READ_CRC 0x0b
    if(command != 0x03 && command != 0x0b)
    {
        
        SpiInfo.readWriteFunc = ITP_SPI_PIO_WRITE;
        SpiInfo.cmdBuffer = SpiTxData;
        SpiInfo.cmdBufferSize = spiTransferSize;
        SpiInfo.dataBuffer = SpiTxData;
        SpiInfo.dataBufferSize = 0;
        result = (int8_t)itpWrite(SPI_handle, (char*)&SpiInfo, 1);
        //printf("Write command ,R(%d)\n", result);
    }
    else
    {
        SpiInfo.readWriteFunc = ITP_SPI_PIO_READ;   
        SpiInfo.cmdBuffer = SpiTxData;
        SpiInfo.cmdBufferSize = spiTransferSize;
        SpiInfo.dataBuffer = SpiRxData;
        SpiInfo.dataBufferSize = spiReadSize;
        
        if(itpRead(SPI_handle, (char*)&SpiInfo, 1) > 0)
            result = 0;

        //printf("Read command ,R(%d)\n",result);
    }
    return result;
	//return spi_master_transfer(SpiTxData, SpiRxData, spiTransferSize);
}
#if 0
void spi_master_init(void)
{
	uint32_t div;
	Nop();

	/* Configure an SPI peripheral. */
	spi_enable_clock(SPI_MASTER_BASE);
	spi_disable(SPI_MASTER_BASE);
	spi_reset(SPI_MASTER_BASE);
	spi_set_lastxfer(SPI_MASTER_BASE);
	spi_set_master_mode(SPI_MASTER_BASE);
	//spi_disable_mode_fault_detect(SPI_MASTER_BASE);
	spi_set_clock_polarity(SPI_MASTER_BASE, SPI_CHIP_SEL, SPI_CLK_POLARITY);
	spi_set_clock_phase(SPI_MASTER_BASE, SPI_CHIP_SEL, SPI_CLK_PHASE);
	spi_set_bits_per_transfer(SPI_MASTER_BASE, SPI_CHIP_SEL, SPI_CSR_BITS_8_BIT);
	
	div = spi_calc_baudrate_div(SPI_BAUDRATE, sysclk_get_peripheral_hz() );

	spi_set_baudrate_div(SPI_MASTER_BASE, SPI_CHIP_SEL, div);
	spi_set_transfer_delay(SPI_MASTER_BASE, SPI_CHIP_SEL, SPI_DLYBS, SPI_DLYBCT);
	
	// Chip select
	//spi_set_peripheral_chip_select_value(SPI_MASTER_BASE, SPI_CHIP_PCS);
	ioport_set_pin_dir(SPI_NCS_PIN, IOPORT_DIR_OUTPUT);
	ioport_set_pin_mode(SPI_NCS_PIN, 0);
	ioport_set_pin_level(SPI_NCS_PIN, true);

	spi_set_fixed_peripheral_select(SPI_MASTER_BASE);

	spi_enable(SPI_MASTER_BASE);
}

//#define USE_SPI_FUNCTIONS

int8_t spi_master_transfer(uint8_t *SpiTxData, uint8_t *SpiRxData, uint16_t spiTransferSize)
{

	uint16_t pos = 0;
	#ifdef USE_SPI_FUNCTIONS
	uint16_t data = 0;
	#endif

	ioport_set_pin_level(SPI_NCS_PIN, false);
	
	while(pos < spiTransferSize)
	{
		// Transmit
		#ifdef USE_SPI_FUNCTIONS
		spi_write(SPI_MASTER_BASE, SpiTxData[pos], 0, 0);
		
		#else
		while (!(SPI_MASTER_BASE->SPI_SR & SPI_SR_TDRE));
		SPI_MASTER_BASE->SPI_TDR = SpiTxData[pos];
		#endif

		ioport_set_pin_level(EXT1_PIN_5, true);

		// Receive
		#ifdef USE_SPI_FUNCTIONS
		spi_read(SPI_MASTER_BASE, &data, 0);
		SpiRxData[pos] = (uint8_t)(data & UINT8_MAX);
		
		#else
		while (!(SPI_MASTER_BASE->SPI_SR & SPI_SR_RDRF));
		SpiRxData[pos] = (uint8_t)SPI_MASTER_BASE->SPI_RDR;
		#endif

		ioport_set_pin_level(EXT1_PIN_5, false);

		pos++;
	}

	ioport_set_pin_level(SPI_NCS_PIN, true);
	
	return 0;
}
#endif

