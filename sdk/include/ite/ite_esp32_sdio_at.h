
#ifndef ITE_ESP32_SDIO_AT_H
#define ITE_ESP32_SDIO_AT_H

#include "ite/ith.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ESP32_CMD_LEN   64

struct esp32_cmd {
    uint8_t cmd_buf[ESP32_CMD_LEN]; /* start address is 4-byte alignment */
    uint32_t flags;
#define ESP32_WAIT_ACK      (0x1 << 0)
#define ESP32_WAIT_FOR_SEND (0x1 << 1)
#define ESP32_DATA_READ     (0x1 << 2)
#define ESP32_DATA_WRITE    (0x1 << 3)

    uint32_t cmd_len; /* it alse be data length set by user for ESP32_DATA_WRITE */
    uint32_t timeout; /* 0: it will be 1500ms if need timeout */
    int status;
    void *data;
    void(*ipd_cb)(uint8_t* addr, uint32_t len);
} __attribute__((aligned(32)));;

#define ESP32_STS_OK             0
#define ESP32_STS_ERROR         -1
#define ESP32_BUF_NOT_ENOUGH    -2

struct esp32_data {
    uint8_t     *buf;
    uint32_t    length;
    uint32_t    actual_len;
    uint32_t    header_len;     /* header size to be removed */
    uint32_t    timeout;        /* 0: default is 1500ms */
};


int esp32_sdio_at_register(void);

int esp32_sdio_at_set_ready(void);

int esp32_sdio_at_is_ready(void);

int esp32_at_cmd(struct esp32_cmd *cmd);

int esp32_at_cmd_ipd_end();

void esp32_sdio_at_lock(void);

void esp32_sdio_at_unlock(void);



#ifdef __cplusplus
}
#endif

#endif /* ITE_ESP32_SDIO_AT_H */
