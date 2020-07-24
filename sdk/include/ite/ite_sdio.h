/** @file
 * ITE SDIO Driver API header file.
 *
 * @author Irene Lin
 * @copyright ITE Tech.Inc.All Rights Reserved.
 */

#ifndef ITE_SDIO_H
#define ITE_SDIO_H


#ifdef __cplusplus
extern "C" {
#endif


/** @defgroup driver_sdio SDIO
 *  @{
 */

#define SDIO_ANY_ID (~0)

struct sdio_device_id {
    unsigned char	    class;			/* SDIO standard interface code or SDIO_ANY_ID */
    unsigned short	    vendor;			/* Vendor or SDIO_ANY_ID */
    unsigned short	    device;			/* Device ID or SDIO_ANY_ID */
    void* driver_data;	        /* private data to the driver */
};

struct sd_card;

struct sdio_func {
    struct sd_card *card;
    void        (*irq_handler)(struct sdio_func *); /* IRQ callback */
    unsigned int    num;
    unsigned int    class;
    unsigned int    vendor;
    unsigned int    device;
    unsigned int    max_blksize;    /* maximum block size */
    unsigned int    cur_blksize;    /* current block size */
    unsigned int    enable_timeout;
    unsigned char   tmpbuf[4];
    void            *drv_data;
    struct sdio_driver *drv;
    unsigned int    type;
};

/*
 * SDIO function device driver
 */
struct sdio_driver {
    char *name;
    const struct sdio_device_id *id_table;

    int(*probe)(struct sdio_func *, const struct sdio_device_id *);
    void(*remove)(struct sdio_func *);
};

/**
 * Describe a specific SDIO device
 * @vend: the 16 bit manufacturer code
 * @dev: the 16 bit function id
 */
#define SDIO_DEVICE(vend,dev) \
	.class = SDIO_ANY_ID, \
	.vendor = (vend), .device = (dev)

/**
 * Register a SDIO function driver.
 * @drv: the registerd function driver
 * 
 * @return: 0 if success.
 */
int iteSdioRegisterDriver(struct sdio_driver *drv);

/**
 * Claim a bus for a set of operations.
 * @func: SDIO function that will be accessd
 */
void iteSdioClaimHost(struct sdio_func *func);

/**
 * Release a bus for a certain SDIO function.
 * @func: SDIO function that will be accessd
 */
void iteSdioReleaseHost(struct sdio_func *func);

/**
 *	Claim the IRQ for a SDIO function
 *	@func: SDIO function
 *	@handler: IRQ handler callback
 *
 *	Claim and activate the IRQ for the given SDIO function. 
 */
int iteSdioClaimIrq(struct sdio_func *func, void(*handler)(struct sdio_func *));

/**
 *	Release the IRQ for a SDIO function
 *	@func: SDIO function
 *
 *	Disable and release the IRQ for the given SDIO function.
 */
int iteSdioReleaseIrq(struct sdio_func *func);

/**
 *	Enables a SDIO function
 *	@func: SDIO function to enable
 *
 *	Powers up and activates a SDIO function so that register access is possible.
 */
int iteSdioEnableFunc(struct sdio_func *func);

/**
 *	Disable a SDIO function
 *	@func: SDIO function to disable
 *
 *	Powers down and deactivates a SDIO function. Register access
 *	to this function will fail until the function is reenabled.
 */
int iteSdioDisableFunc(struct sdio_func *func);

/**
 * Set the block size of an SDIO function.
 * @func: SDIO function to change
 * @block_size: new block size
 */
int iteSdioSetBlockSize(struct sdio_func *func, unsigned int block_size);

/**
 *	Read a single byte from a SDIO function
 *	@func: SDIO function to access
 *	@addr: address to read
 *
 *	Reads a single byte from the address space of a given SDIO
 *	function. If there is a problem reading the address, 0xff
 *	is returned.
 */
unsigned char iteSdioRead8(struct sdio_func *func, unsigned int addr, int *err);

/**
 *	Write a single byte to a SDIO function
 *	@func: SDIO function to access
 *	@data: byte to write
 *	@addr: address to write to
 */
void iteSdioWrite8(struct sdio_func *func, unsigned char data, unsigned int addr, int *err);

/**
 *	Read a chunk of memory from a SDIO function
 *	@func: SDIO function to access
 *	@dst: buffer to store the data
 *	@addr: address to begin reading from
 *	@size: number of bytes to read
 *
 *	Return 0 if success.
 */
int iteSdioMemcpyFromIo(struct sdio_func *func, void *dst, unsigned int addr, int size);

/**
 *	Write a chunk of memory to a SDIO function
 *	@func: SDIO function to access
 *	@addr: address to start writing to
 *	@src: buffer that contains the data to write
 *	@size: number of bytes to write
 *
 *	Return 0 if success.
 */
int iteSdioMemcpyToIo(struct sdio_func *func, unsigned int addr, void *src, int size);

/**
*	Read a 16 bit data from a SDIO function
*	@func: SDIO function to access
*	@addr: address to read
*/
unsigned short iteSdioRead16(struct sdio_func *func, unsigned int addr, int *err);

/**
*	Write a 16 bit data to a SDIO function
*	@func: SDIO function to access
*	@data: data to write
*	@addr: address to write to
*/
void iteSdioWrite16(struct sdio_func *func, unsigned short data, unsigned int addr, int *err);

/**
*	Read a 32 bit integer from a SDIO function
*	@func: SDIO function to access
*	@addr: address to read
*	@err: optional status value
*/
unsigned int iteSdioRead32(struct sdio_func *func, unsigned int addr, int *err);

/**
 *	write a 32 bit integer to a SDIO function
 *	@func: SDIO function to access
 *	@data: data to write
 *	@addr: address to write to
 */
void iteSdioWrite32(struct sdio_func *func, unsigned int data, unsigned int addr, int *err);

/**
 *	Read a single byte from SDIO function 0
 *	@func: an SDIO function of the card
 *	@addr: address to read
 *
 *	Reads a single byte from the address space of SDIO function 0.
 */
unsigned char iteSdioF0Read8(struct sdio_func *func, unsigned int addr, int *err);

/**
 *	Write a single byte to SDIO function 0
 *	@func: an SDIO function of the card
 *	@data: byte to write
 *	@addr: address to write to
 *
 *	Writes a single byte to the address space of SDIO function 0.
 */
void iteSdioF0Write8(struct sdio_func *func, unsigned char data, unsigned int addr, int *err);

/**
 *	Read from a FIFO on a SDIO function
 *	@func: SDIO function to access
 *	@buf: buffer to store the data
 *	@addr: address of (single byte) FIFO
 *	@size: number of bytes to read
 *
 *	Reads from the specified FIFO of a given SDIO function.
 */
int iteSdioMemcpyFromFifo(struct sdio_func *func, void *buf, unsigned int addr, int size);

/**
 *	Write to a FIFO of a SDIO function
 *	@func: SDIO function to access
 *	@addr: address of (single byte) FIFO
 *	@src: buffer that contains the data to write
 *	@size: number of bytes to write
 *
 *	Writes to the specified FIFO of a given SDIO function.
 */
int iteSdioMemcpyToFifo(struct sdio_func *func, unsigned int addr, void *src, int size);


#define sdio_get_drvdata(f)	    (f->drv_data)
#define sdio_set_drvdata(f,d)	do { f->drv_data = d; } while(0)

#define sdio_register_driver        iteSdioRegisterDriver
#define sdio_claim_host             iteSdioClaimHost
#define sdio_release_host           iteSdioReleaseHost
#define sdio_claim_irq              iteSdioClaimIrq
#define sdio_release_irq            iteSdioReleaseIrq
#define sdio_enable_func            iteSdioEnableFunc
#define sdio_disable_func           iteSdioDisableFunc
#define sdio_set_block_size         iteSdioSetBlockSize
#define sdio_readb                  iteSdioRead8
#define sdio_writeb                 iteSdioWrite8
#define sdio_memcpy_fromio          iteSdioMemcpyFromIo
#define sdio_memcpy_toio            iteSdioMemcpyToIo
#define sdio_readw                  iteSdioRead16
#define sdio_writew                 iteSdioWrite16  
#define sdio_readl                  iteSdioRead32 
#define sdio_writel                 iteSdioWrite32  
#define sdio_f0_readb               iteSdioF0Read8
#define sdio_f0_writeb              iteSdioF0Write8
#define sdio_readsb                 iteSdioMemcpyFromFifo
#define sdio_writesb                iteSdioMemcpyToFifo

/** @} */ // end of driver_sdio


#ifdef __cplusplus
}
#endif

#endif /* ITE_SDIO_H */
