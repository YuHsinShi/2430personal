#include <sys/ioctl.h>
#include <assert.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include "ite/ith.h"
#include "ite/itp.h"
#include "ite/ite_sd.h"



#define TEST_SD_NUM     CFG_SD_INDEX

#define SD_DELAY_REG     0xB0900004
#define SD_REG_BASE      ((TEST_SD_NUM == 0) ? ITH_SD0_BASE : ITH_SD1_BASE)
#define IP_DELAY_REG     (SD_REG_BASE + 0x100)

static void scan_window(void);

void* TestFunc(void* arg)
{
    scan_window();
    while (1);
}



static uint8_t data[512*512];

#define K  (1024/512)   // bytes to sector
static uint32_t rSector[10] = { 0 * K, 256 * K, 512 * K, 800 * K, 1024 * K, 1200 * K, 1400 * K, 1800 * K, 2000 * K, 2100 * K };
static uint32_t rSectorCnt[10] = { 1, 2, 4, 8, 16, 32, 64, 128, 256, 512 };

#define DELAY_CNT   (16)
#define IP_DELAY_CNT     3

struct success_info {
    int success;
    uint32_t success_cunt;
};
static struct success_info info[IP_DELAY_CNT][DELAY_CNT];

static int do_test(uint32_t *success_cnt)
{
#define TEST_SIZE       1*1024*1024
    int index, i, loopCnt, rc = 0;
    int success = 1;

    for (index = 0; index < 10; index++)
    {
        loopCnt = TEST_SIZE / (rSectorCnt[index] * 512);

        for (i = 0; i < loopCnt; i++) {
            rc = iteSdReadMultipleSectorEx(TEST_SD_NUM, rSector[index], rSectorCnt[index], data);
            if (rc) {
                success = 0;
                goto end;
            }

            rc = iteSdWriteMultipleSectorEx(TEST_SD_NUM, rSector[index], rSectorCnt[index], data);
            if (rc) {
                success = 0;
                goto end;
            }
            (*success_cnt)++;
        }
    }
end:
    return success;
}

static void scan_window(void)
{
    int delay_idx, ip_delay_idx;
    uint32_t delay_offset;

#if 1//defined(CFG_MMC_ENABLE)
{
    SD_CARD_INFO card_info = { 0 };

    iteSdcInitialize(TEST_SD_NUM, &card_info);
	if ((card_info.type != SD_TYPE_MMC) && (card_info.type != SD_TYPE_SD)) {
        printf("iteSdcInitialize() no available card type! %d \n", card_info.type);
        while (1);
    }
}
#endif
	iteSdInitializeEx(TEST_SD_NUM);

    for (ip_delay_idx = 0; ip_delay_idx < IP_DELAY_CNT; ip_delay_idx++) {
        // ip delay setting
        ithWriteRegMaskA(IP_DELAY_REG, (ip_delay_idx << 8), (0x3F << 8));
        printf("0x%08X = 0x%08X \n", IP_DELAY_REG, ithReadRegA(IP_DELAY_REG));
        for (delay_idx = 0; delay_idx < DELAY_CNT; delay_idx++) {
            // new delay setting
            delay_offset = (TEST_SD_NUM == 0) ? 0 : 4;
            ithWriteRegMaskA(SD_DELAY_REG, (delay_idx << delay_offset), (0xF << delay_offset));
            printf("0x%08X = 0x%08X \n", SD_DELAY_REG, ithReadRegA(SD_DELAY_REG));

            info[ip_delay_idx][delay_idx].success = do_test(&info[ip_delay_idx][delay_idx].success_cunt);
        }
    }
    printf("\n\n  \n\n");

    printf(" new delay:    0    1    2    3    4    5    6    7    8    9    10   11   12   13   14   15 \n");
    for (ip_delay_idx = 0; ip_delay_idx < IP_DELAY_CNT; ip_delay_idx++) {
        printf("ip delay %d: %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d \n", ip_delay_idx,
            info[ip_delay_idx][0].success_cunt,
            info[ip_delay_idx][1].success_cunt,
            info[ip_delay_idx][2].success_cunt,
            info[ip_delay_idx][3].success_cunt,
            info[ip_delay_idx][4].success_cunt,
            info[ip_delay_idx][5].success_cunt,
            info[ip_delay_idx][6].success_cunt,
            info[ip_delay_idx][7].success_cunt,
            info[ip_delay_idx][8].success_cunt,
            info[ip_delay_idx][9].success_cunt,
            info[ip_delay_idx][10].success_cunt,
            info[ip_delay_idx][11].success_cunt,
            info[ip_delay_idx][12].success_cunt,
            info[ip_delay_idx][13].success_cunt,
            info[ip_delay_idx][14].success_cunt,
            info[ip_delay_idx][15].success_cunt
            );
    }

    printf("\n\n Scan Done!!! \n\n");

    return;
}
