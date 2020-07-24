#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "ite/itp.h"
#include "ite/ite_sd.h"
#include "ite/ite_esp32_sdio_at.h"
#if defined(CFG_MMC_ENABLE)
#include "ite/ite_sdio.h"
#else
#include <linux/mmc/sdio_func.h>
#include <linux/mmc/sdio.h>
#endif
#include "esp32_err.h"


#define ESP_SLAVE_CMD53_END_ADDR    0x1f800

#define TX_BUFFER_MAX   0x1000
#define TX_BUFFER_MASK  0xFFF
#define RX_BYTE_MAX     0x100000
#define RX_BYTE_MASK    0xFFFFF

#define DR_REG_SLCHOST_BASE 0//0x3ff55000
#define HOST_SLC0HOST_TOKEN_RDATA_REG          (DR_REG_SLCHOST_BASE + 0x44)
#define HOST_SLC0HOST_INT_RAW_REG          (DR_REG_SLCHOST_BASE + 0x50)
#define HOST_SLC0HOST_INT_ST_REG          (DR_REG_SLCHOST_BASE + 0x58)
#define HOST_SLC0HOST_INT_CLR_REG          (DR_REG_SLCHOST_BASE + 0xD4)
#define HOST_SLCHOST_PKT_LEN_REG          (DR_REG_SLCHOST_BASE + 0x60)

#define HOST_SLC0_RX_NEW_PACKET_INT_ST  (0x1<<23)


struct esp32_sdio_at {
    struct sdio_func *sdio_func;
    uint16_t buffer_size;
    ///< All data that do not fully fill a buffer is still counted as one buffer. E.g. 10 bytes data costs 2 buffers if the size is 8 bytes per buffer.
    ///< Buffer size of the slave pre-defined between host and slave before communication.
    uint16_t block_size;
    uint32_t tx_sent_buffers;    ///< Counter hold the amount of buffers already sent to ESP32 slave. Should be set to 0 when initialization.
    uint32_t rx_got_bytes;       ///< Counter hold the amount of bytes already received from ESP32 slave. Should be set to 0 when initialization.
    uint32_t init : 1;
    uint32_t ready : 1;
    uint32_t claimed : 1;
    sem_t   sem;
    pthread_mutex_t mutex;
    struct esp32_cmd *cmd;
};

static struct esp32_sdio_at     esp32;
sem_t   esp32_sem;

#define READ_BUFFER_LEN     (4096)
static uint8_t esp_at_recvbuf[READ_BUFFER_LEN + 1] = "";  // this is tmp solution....

static const struct sdio_device_id sdio_ids[] = {
    { SDIO_DEVICE(0x6666, 0x2222), .driver_data = (void *)&esp32 },
   	//{}
};


static void esp32_irq(struct sdio_func *func);

static int esp32_probe(struct sdio_func *func, const struct sdio_device_id *id)
{
    int rc;
    printf("esp32_probe()\n");

    memset((void*)&esp32, 0x0, sizeof(struct esp32_sdio_at));
    esp32.sdio_func = func;
    esp32.buffer_size = 512;
    esp32.block_size = 512;
    func->type = SDIO_ESP32_WROOM_32_AT;
    esp32.init = 1;
    esp32.ready = 0;
    esp32.mutex = PTHREAD_MUTEX_INITIALIZER;
    sem_init(&esp32.sem, 0, 0);
    sdio_set_drvdata(func, &esp32);
    
    sdio_claim_host(func);
    /* enable function & get IO ready */
    rc = sdio_enable_func(func);
    if (rc) 
        printf("esp32: enable function fail! rc = 0x%08X \n", rc);

    sdio_release_host(func);

    return rc;
}

static void esp32_remove(struct sdio_func *func)
{
    int rc;
    struct esp32_sdio_at *pesp32;

    printf("esp32_remove()\n");
    pesp32 = sdio_get_drvdata(func);

    sdio_claim_host(func);
    rc = sdio_release_irq(func);
    if (rc) printf("esp32: release irq fail! \n");
    sdio_release_host(func);

    /* free some resource. */
    memset((void*)pesp32, 0x0, sizeof(struct esp32_sdio_at));
}

static struct sdio_driver esp32_driver = {
    .name = "esp32_sdio_at",
    .probe = esp32_probe,
    .remove = esp32_remove,
    .id_table = sdio_ids,
};

int esp32_sdio_at_register(void)
{
    return sdio_register_driver(&esp32_driver);
}

static int esp32_get_intr(struct sdio_func *func, uint32_t *intr_raw, uint32_t *intr_st)
{
    int rc = 0;

    *intr_raw = sdio_readl(func, HOST_SLC0HOST_INT_RAW_REG, &rc);
    if (rc) {
        rc = ERR_ESP32_GET_INTR_RAW_FAIL;
        goto end;
    }

    *intr_st = sdio_readl(func, HOST_SLC0HOST_INT_ST_REG, &rc);
    if (rc) {
        rc = ERR_ESP32_GET_INTR_STS_FAIL;
        goto end;
    }

end:
    return rc;
}

static int esp32_clear_intr(struct sdio_func *func, uint32_t intr_mask)
{
    int rc;
    sdio_writel(func, intr_mask, HOST_SLC0HOST_INT_CLR_REG, &rc);
    return rc;
}

static int esp32_get_rx_data_size(struct esp32_sdio_at *esp32, uint32_t* rx_size)
{
    int rc;
    uint32_t len;

    len = sdio_readl(esp32->sdio_func, HOST_SLCHOST_PKT_LEN_REG, &rc);
    if (rc) {
        rc = ERR_ESP32_GET_PKT_LEN_FAIL;
        return rc;
    }
    len &= RX_BYTE_MASK;
    len = (len + RX_BYTE_MAX - esp32->rx_got_bytes) % RX_BYTE_MAX;
    *rx_size = len;

    return rc;
}

static void esp32_rx_process(struct esp32_sdio_at *esp32, uint8_t *recvbuf, uint32_t len)
{
    struct esp32_cmd *cmd = esp32->cmd;
    bool sem_done = false;

    if (cmd == NULL) {
        //recvbuf[len] = '\0';
        //printf("%s \n", recvbuf);
        return;
    }

    if (cmd->ipd_cb) {
        //recvbuf[len] = '\0';
        //printf("%s \n", recvbuf);
        uint8_t *start_ptr = strstr(recvbuf, "+IPD");
        if (start_ptr) {
            uint8_t data_len[8] = "";
            uint8_t *end_ptr = strchr(recvbuf, ':');
            start_ptr += 5; // +IPD,
            memcpy(data_len, start_ptr, (end_ptr - start_ptr));
            cmd->ipd_cb(end_ptr + 1, atoi(data_len));
            return;
        }
    }

    /* echo: just print */
    if ((len == cmd->cmd_len) && strncmp(recvbuf, cmd->cmd_buf, len) == 0)
        goto echo_info;

    //printf("cmd->flags = %d \n", cmd->flags);
    /* check ACK: OK or ERROR */
    if (((len == 6) || (len == 9) || (len == 11) || (len == 13)) &&
        (cmd->flags & ESP32_WAIT_ACK)) {

        if (strstr(recvbuf, "OK") != NULL) {
            cmd->status = 0;
            sem_done = true;
        }
        else if (strstr(recvbuf, "ERROR") != NULL) {
            cmd->status = ESP32_STS_ERROR;
            sem_done = true;
        }
        else if (strstr(recvbuf, "SEND OK") != NULL) {

            cmd->status = 0;
            sem_done = true;
        }
        else if (strstr(recvbuf, "SEND FAIL") != NULL) {
            cmd->status = ERR_ESP32_DATA_SEND_FAIL;
            sem_done = true;
        }

        if (sem_done == true)
            goto end;
    }
    /* wait '>' for send data */
    if ((len == 3) && (cmd->flags & ESP32_WAIT_FOR_SEND)) {
        if (strstr(recvbuf, ">") != NULL) {
            cmd->status = 0;
            sem_done = true;
        }

        if (sem_done == true)
            goto end;
    }

    /* copy data to user data buffer */
    if (cmd->flags & ESP32_DATA_READ) {
        struct esp32_data *data = (struct esp32_data *)cmd->data;
            
        if ((data->actual_len + len) > data->length) {
            cmd->status = ESP32_BUF_NOT_ENOUGH;
            len = data->length - data->actual_len;
            printf("esp32: user buffer full!! \n");
        }
        recvbuf[len] = '\0';
        //printf("%s \n", recvbuf);
        //printf("len: %d \n", len);
        if (len) {
            uint32_t copy_len = len - data->header_len;
            memcpy((void*)(data->buf + data->actual_len), (void*)(recvbuf + data->header_len), copy_len);
            data->actual_len += copy_len;
        }
        //printf("actual_len: %d \n", data->actual_len);
        if (cmd->status)
            return;
    }
    else {
        /* others string info */
    echo_info:
        recvbuf[len] = '\0';
        printf("%s \n", recvbuf);
    }

end:
    if (sem_done == true) {
        printf("%s \n", recvbuf);
        sem_post(&esp32->sem);
    }
}

/* already claimed */
static void esp32_irq(struct sdio_func *func)
{
    int rc;
    uint32_t intr_raw, intr_st;
    struct esp32_sdio_at *esp32 = sdio_get_drvdata(func);

    rc = esp32_get_intr(func, &intr_raw, &intr_st);
    if (rc) {
        printf("esp32_get_intr fail 0x%X \n", rc);
        return;
    }
    rc = esp32_clear_intr(func, intr_raw);
    if (rc) {
        printf("esp32_clear_intr fail 0x%X \n", rc);
        return;
    }

//printf("intr_raw: 0x%X ¡Aintr_st: 0x%X\n", intr_raw, intr_st);

    /* get the new packet */
    if (intr_raw & HOST_SLC0_RX_NEW_PACKET_INT_ST) {
        uint32_t len, len_align, start_time;

        start_time = itpGetTickCount();
        for (;;) {
            rc = esp32_get_rx_data_size(esp32, &len);
            if (!rc && len > 0)
                break;
            if (rc) 
                goto end;
            if (itpGetTickDuration(start_time) > 200) {
                printf("esp32: get rx data size timeout \n");
                goto end;
            }
        }
        //printf("esp32: rx data size %d \n", len);

        if (len > READ_BUFFER_LEN) {
            len = READ_BUFFER_LEN;
            printf("rx size(%d) > buffer size(%d) ==> TODO!! \n", len, READ_BUFFER_LEN);
        }
        len_align = (len + 3) & (~3);
        rc = sdio_memcpy_fromio(func, esp_at_recvbuf, ESP_SLAVE_CMD53_END_ADDR - len, len_align);
        if (rc) {
            rc = ERR_ESP32_READ_RX_DATA_FAIL;
            goto end;
        }
        esp32->rx_got_bytes += len;

#if 1
        esp32_rx_process(esp32, esp_at_recvbuf, len);
#else
        printf("%s \n", esp_at_recvbuf);
        memset(esp_at_recvbuf, '\0', sizeof(esp_at_recvbuf));
#endif
    }

end:
    if (rc)
        	printf("esp32_irq() rc:%d (0x%08X) \n", rc, rc);
    return;
}

static int esp32_get_tx_buffer_num(struct esp32_sdio_at *esp32, uint32_t *tx_num)
{
    uint32_t len;
    int rc;

    len = sdio_readl(esp32->sdio_func, HOST_SLC0HOST_TOKEN_RDATA_REG, &rc);
    if (rc)
        return rc;
    len = (len >> 16) & TX_BUFFER_MASK;
    len = (len + TX_BUFFER_MAX - esp32->tx_sent_buffers) % TX_BUFFER_MAX;
    *tx_num = len;
    return rc;
}

int esp32_sdio_at_transfer(uint8_t *buf, int length)
{
    int rc, buffer_used;
    uint32_t num = 0, start_time;
    uint16_t buffer_size, len;
    struct sdio_func *func;

    if (!esp32.init)
        return ERR_ESP32_NO_INIT;
    if (!esp32.sdio_func)
        return ERR_ESP32_INVALID_FUNC;
    if (length == 0)
        return ERR_ESP32_ZERO_LEN;

    func = esp32.sdio_func;
    buffer_size = esp32.buffer_size;
    buffer_used = (length + buffer_size - 1) / buffer_size;

    sdio_claim_host(func);
	//printf("TX: %s \n", buf);

    start_time = itpGetTickCount();

    for (;;) {
        rc = esp32_get_tx_buffer_num(&esp32, &num);
        if (!rc && num * buffer_size >= (uint32_t)length) break;
        if (rc) goto end;
        if (itpGetTickDuration(start_time) > 50) {
            printf(" buffer is not enough: %d, %d required. \n", num, buffer_used);
            rc = ERR_ESP32_WAIT_TX_BUF_TIMEOUT;
            goto end;
        }
        else {
            printf(" buffer is not enough: %d, %d required. Retry....\n", num, buffer_used);
        }
    }

    len = (length + 3) & (~3);
    if (len > ESP_SLAVE_CMD53_END_ADDR)
        printf("[ERR] esp32 length:0x%X > 0x1F800 \n", len);

    rc = sdio_memcpy_toio(func, ESP_SLAVE_CMD53_END_ADDR - length, (void*)buf, len);
    if (rc) {
        rc = ERR_ESP32_SDIO_TX_FAIL;
        goto end;
    }

    esp32.tx_sent_buffers += buffer_used;

end:
    sdio_release_host(func);
    if (rc)
        printf("esp32_sdio_at_transfer() rc = %d (0x%08X)", rc, rc);
    return rc;
}

int esp32_sdio_at_set_ready(void)
{
    int rc;
    struct sdio_func *func = esp32.sdio_func;

	/* enable irq later */
    sdio_claim_host(func);
    rc = sdio_claim_irq(func, esp32_irq);
    if (rc) 
        printf("esp32: claim irq fail! rc = 0x%08X \n", rc);
    sdio_release_host(func);

    esp32.ready = 1;
}

int esp32_sdio_at_is_ready(void)
{
    if (esp32.ready)
        return 1;
    else
        return 0;
}

void esp32_sdio_at_lock(void)
{
    pthread_mutex_lock(&esp32.mutex);
    esp32.claimed = 1;
}

void esp32_sdio_at_unlock(void)
{
    esp32.claimed = 0;
    pthread_mutex_unlock(&esp32.mutex);
}

#define ESP32_TIMEOUT   1500

int esp32_at_cmd(struct esp32_cmd *cmd)
{
    int res = 0;

    if (!esp32.claimed) {
        printf("esp32: not claimed!!! \n");
        res = ERR_ESP32_NOT_CLAIMED;
        goto out;
    }

    if (esp32.cmd) {
        printf("esp32: in used!!! err cmd: %s \n", cmd->cmd_buf);
        res = ERR_ESP32_IN_USED;
        goto out;
    }

    /* for pass-through write data */
    if (cmd->flags == ESP32_DATA_WRITE) {
        res = esp32_sdio_at_transfer(cmd->data, cmd->cmd_len);
        if (res)
            goto out;
    }

    if (cmd->flags & (ESP32_WAIT_ACK | ESP32_WAIT_FOR_SEND)) {
        cmd->cmd_len = strlen(cmd->cmd_buf);
        if (cmd->cmd_len > ESP32_CMD_LEN) {
            printf("esp32: cmd length = %d > ESP32_CMD_LEN(%d) \n", cmd->cmd_len, ESP32_CMD_LEN);
            while (1);
        }
        //printf("cmd_len:%d \n", cmd->cmd_len);

        esp32.cmd = cmd;
        res = esp32_sdio_at_transfer(cmd->cmd_buf, cmd->cmd_len);
        if (res)
            goto out;
        //printf("esp32_at_cmd: cmd->flags = %d \n", cmd->flags);

        res = itpSemWaitTimeout(&esp32.sem, (cmd->timeout ? cmd->timeout : ESP32_TIMEOUT));
        if (res) {
            printf("esp32: %s wait ack timeout\n", cmd->cmd_buf);
            res = ERR_ESP32_WAIT_ACK_TIMEOUT;
            goto out;
        }

        if (cmd->flags & ESP32_DATA_WRITE) {
            struct esp32_data *data = cmd->data;
            cmd->flags &= ~ESP32_WAIT_FOR_SEND;
            cmd->flags |= ESP32_WAIT_ACK;
            res = esp32_sdio_at_transfer(data->buf, data->length);
            if (res)
                goto out;

            res = itpSemWaitTimeout(&esp32.sem, (data->timeout ? data->timeout : ESP32_TIMEOUT));
            if (res) {
                printf("esp32: %s wait data ack timeout\n", cmd->cmd_buf);
                res = ERR_ESP32_WAIT_ACK_TIMEOUT;
                goto out;
            }
        }
    }

    /* for pass-through mode */
    if ((cmd->cmd_len == 3) && (strncmp(cmd->data, "+++", 3) == 0))
        usleep(1000*1000);

out:
    if (!cmd->ipd_cb)
        esp32.cmd = NULL;
    if (res)
        printf("esp32_at_cmd() res = %d (0x%08X) \n", res, res);
    return res;
}

int esp32_at_cmd_ipd_end()
{
    esp32.cmd = NULL;
}

