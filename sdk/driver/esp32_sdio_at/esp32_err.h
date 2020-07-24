#ifndef	ESP32_ERROR_H
#define	ESP32_ERROR_H

#ifdef __cplusplus
extern "C" {
#endif


#define ERR_ESP32_BASE	0x000E0000

#define ERR_ESP32_NO_INIT          			(ERR_ESP32_BASE + 0x01)
#define ERR_ESP32_INVALID_FUNC      			(ERR_ESP32_BASE + 0x02)
#define ERR_ESP32_ZERO_LEN        			(ERR_ESP32_BASE + 0x03)
#define ERR_ESP32_WAIT_TX_BUF_TIMEOUT		(ERR_ESP32_BASE + 0x04)
#define ERR_ESP32_SDIO_TX_FAIL          		(ERR_ESP32_BASE + 0x05)
#define ERR_ESP32_WAIT_ACK_TIMEOUT 		    (ERR_ESP32_BASE + 0x06)
#define ERR_ESP32_NOT_CLAIMED    		    (ERR_ESP32_BASE + 0x07)
#define ERR_ESP32_DATA_SEND_FAIL      	    (ERR_ESP32_BASE + 0x08)
#define ERR_ESP32_GET_INTR_RAW_FAIL     	    (ERR_ESP32_BASE + 0x09)
#define ERR_ESP32_GET_INTR_STS_FAIL     	    (ERR_ESP32_BASE + 0x0A)
#define ERR_ESP32_GET_PKT_LEN_FAIL     	    (ERR_ESP32_BASE + 0x0B)
#define ERR_ESP32_READ_RX_DATA_FAIL     	    (ERR_ESP32_BASE + 0x0C)
#define ERR_ESP32_IN_USED     				(ERR_ESP32_BASE + 0x0D)

#ifdef __cplusplus
}
#endif

#endif
