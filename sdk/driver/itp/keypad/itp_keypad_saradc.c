/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * Castor3 keypad module.
 *
 * @author EL
 * @version 1.0
 */
#include <stdlib.h>
#include <string.h>
#include "../itp_cfg.h"
#include "saradc/saradc.h"

#define WRITEBUFFER_LEN                 4096
#define SARADC_output_set_byte          8
#define SARADC_output_set_valid_byte    6
#define SARADC_output_unit2_byte        3
#define KEY_NUM                         16

typedef enum KEYPAD_KEY_TAG
{
    KEY_1 = 0,     
    KEY_2,  
    KEY_3,  
    KEY_UP, 
    KEY_4,  
    KEY_5,     
    KEY_6,  
    KEY_DOWN,   
    KEY_7,  
    KEY_8,  
    KEY_9,  
    KEY_RING,   
    KEY_STAR,   
    KEY_0,      
    KEY_HASHTAG,        
    KEY_HOME,       
    NOKEY,
    DEFAULT     // default key value
} KEYPAD_KEY;

typedef struct _KEY_INFO {
    KEYPAD_KEY key;
    int count;
} KEY_INFO;

int SAMPLE_COUNT = 10;
uint8_t* writeBuffer = NULL;
int channel = 0;
static KEYPAD_KEY key = NOKEY, pre_key = NOKEY;

void itpKeypadStandby(void)
{
    // Enter Standby mode
    printf("[%s]\n", __FUNCTION__);
    SAMPLE_COUNT = 5;
}

void itpKeypadResume(void)
{
    // Leave out Standby mode
    printf("[%s]\n", __FUNCTION__);
    SAMPLE_COUNT = 10;
}

static int XAINToChannel(uint32_t XAINValue)
{
    int channel = 0;
    
    if(XAINValue & 0x1) {
        channel = 0;
    }
    else if(XAINValue & 0x2) {
        channel = 1;
    }
    else if(XAINValue & 0x4) {
        channel = 2;
    }
    else if(XAINValue & 0x8) {
        channel = 3;
    }

    return channel;
}

int itpKeypadProbe(void)
{
    SARADC_RESULT result = SARADC_SUCCESS;
    uint16_t writeBuffer_len = 512;
    uint16_t calibrationOutput = 0;
    int count = 1;
    int i = 0;
    int tmp_key = 0;
    KEY_INFO Record[10];
    bool flag = false;
    int KeyKing = DEFAULT, CountKing = 0;

    // Init Key_Info struct
    for(i = 0; i < 10; i++) {
        Record[i].key = DEFAULT;
        Record[i].count = 0;
    }

    while(count <= SAMPLE_COUNT) {
        // Get SarADC raw data
        if (result = mmpSARConvert(channel, writeBuffer_len, &calibrationOutput)) {
            printf("mmpSARConvert() error (0x%x) !!\n", result);
            return -1;
        }

        // Key depend on saradc raw data
        if (calibrationOutput > 0xCEC && calibrationOutput < 0xD64)
            tmp_key = KEY_1; //0xd28
        else if (calibrationOutput > 0xDEF && calibrationOutput < 0xE67)
            tmp_key = KEY_2; //0xe27
        else if (calibrationOutput > 0xEB5 && calibrationOutput <= 0xF60)
            tmp_key = KEY_3; //0xf49
        else if (calibrationOutput > 0x887 && calibrationOutput <= 0x988)
            tmp_key = KEY_4; //0x922
        else if (calibrationOutput > 0x988 && calibrationOutput <= 0xA98)
            tmp_key = KEY_5; //0xa21
        else if (calibrationOutput > 0xAE1 && calibrationOutput <= 0xB98)
            tmp_key = KEY_6; //0xb28
        else if (calibrationOutput > 0x4E5 && calibrationOutput <= 0x559)
            tmp_key = KEY_7; //0x517
        else if (calibrationOutput > 0x5DF && calibrationOutput <= 0x66C) 
            tmp_key = KEY_8; //0x5ee
        else if (calibrationOutput > 0x6E9 && calibrationOutput <= 0x76C)
            tmp_key = KEY_9; //0x71f
        else if (calibrationOutput > 0x1C4 && calibrationOutput <= 0x248)
            tmp_key = KEY_0; //0x219
        else if (calibrationOutput > 0xF60 && calibrationOutput <= 0xFBA)
            tmp_key = KEY_UP; //0xf83
        else if (calibrationOutput > 0xB98 && calibrationOutput <= 0xCA5)
            tmp_key = KEY_DOWN; //0xc1f
        else if (calibrationOutput > 0x7F6 && calibrationOutput <= 0x887)
            tmp_key = KEY_RING; //0x81c
        else if (calibrationOutput > 0x3DC && calibrationOutput <= 0x45D)
            tmp_key = KEY_HOME; //0x402
        else if (calibrationOutput > 0x100 && calibrationOutput <= 0x142)
            tmp_key = KEY_STAR; //0x119
        else if (calibrationOutput > 0x2D3 && calibrationOutput <= 0x34F)
            tmp_key = KEY_HASHTAG; //0x315
        else {
            tmp_key = NOKEY;
        }

        //printf("[%s] Key=%d PreKey=%d Calibration=%x\n", __FUNCTION__, tmp_key, pre_key, calibrationOutput);

        // Fill up KEY_INFO struct
        i = 0;
        while(Record[i].key != DEFAULT && i < SAMPLE_COUNT) {
            if(Record[i].key == tmp_key) {
                Record[i].count = Record[i].count + 1;
                flag = true;
            }
            i++;
        }

        // IF KEY_INFO don't have tmp_key member
        if(flag == false) {
            Record[i].key = tmp_key;
            Record[i].count = Record[i].count + 1;
        }

        if(count == SAMPLE_COUNT) {
            // Find the largest count of key
            i = 0;
            while(Record[i].key != DEFAULT && i < SAMPLE_COUNT) {                
                if(Record[i].count >= CountKing) {
                    KeyKing = Record[i].key;
                    CountKing = Record[i].count;
                }                
                i++;
            }

#if 0
            if(KeyKing != NOKEY) {
                printf("KeyKing=%d\n", KeyKing);
                for(i=0; i < 10; i++) {
                    printf("K=%d W=%d\n", Record[i].key, Record[i].count);
                }
            }
#endif

            if (KeyKing != pre_key && KeyKing !=  NOKEY)
            {
                //printf("[%s] Key=%d PreKey=%d Calibration=%x\n", __FUNCTION__, tmp_key, pre_key, calibrationOutput);
                pre_key = KeyKing;
                
                return KeyKing;
            }

            // Re-init Key_Info struct
            for(i = 0; i < 10; i++) {
                Record[i].key = DEFAULT;
                Record[i].count = 0;
            }

            pre_key = KeyKing;
        }

        count++;
        flag = false;
    }

    return -1;
}

void itpKeypadInit(void)
{
    SARADC_RESULT result = SARADC_SUCCESS;
    uint16_t range_min = 0, range_max = 0;

    result = mmpSARInitialize(SARADC_MODE_SRAM_INTR, SARADC_AMPLIFY_1X, SARADC_CLK_DIV_9);
    if (result) {
        printf("mmpSARInitialize() error (0x%x) !!\n", result);
        return;
    }

    channel = XAINToChannel(CFG_SARADC_VALID_XAIN);    
}

int itpKeypadGetMaxLevel(void)
{
    return KEY_NUM;
}
