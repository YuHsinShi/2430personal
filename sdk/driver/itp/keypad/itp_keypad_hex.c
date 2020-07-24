/*
 * Copyright (c) 2018 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * Hex keypad module.
 * col1  col2  col3  col4
 *  |     |     |     |
 *  o-----o-----o-----o----- raw1 (key1~4)
 *  |     |     |     |
 *  o-----o-----o-----o----- raw2 (key5~8)
 *  |     |     |     |
 *  o-----o-----o-----o----- raw3 (key9~12)
 *  |     |     |     |
 *  o-----o-----o-----o----- raw4 (key13~16)
 *
 * @give 8 gpio pins for connecting with col1~4 & raw1~4
 * @initial 1: set col1~4 as output pin with 1
 * @initial 2: set raw1~4 as input pin (default status is 1 because of pull-up R)
 * @loop scan the status of raw1 ~ raw4 by setting col1 ~ col4 as low
 * @ex:col1 output as 0, if raw1 got 0, then key1 is pressed.
 *
 * @author Jospeh
 * @version 1.0
 */
#include "../itp_cfg.h"

static const unsigned int kpGpioTable[] = { CFG_GPIO_KEYPAD };
static unsigned int *COL_TABLE = NULL;
static unsigned int *RAW_TABLE = NULL;
static uint8_t gKpInitOk = 0;

int itpKeypadProbe(void)
{
    unsigned int i,j;
    int kn = ITH_COUNT_OF(kpGpioTable); 
    int colNum = kn / 2;
    int rawNum = kn / 2; 
    int kpIndex = -1;

    for (i = 0; i < colNum; i++)
    {
        //set GPIO output as low
        ithGpioClear(COL_TABLE[i]);        
        usleep(10);
        
        for (j = 0; j < rawNum; j++)
        {            
            if(ithGpioGet(RAW_TABLE[j])==0)
            {
                //jedge which key is pressed(keymap[i][j])                
                kpIndex = (i * colNum) + j;
                #ifdef EN_DBG_MSG
                printf("kpIndex=%d, v=%d\n",kpIndex,(i * colNum) + j);
                #endif
                break;
            }
        }
        ithGpioSet(COL_TABLE[i]); //set col_N as low
        if(kpIndex != -1) break;
    }
    
    #ifdef EN_DBG_MSG
    if(kpIndex != -1) printf("kpIndex = %d\n",kpIndex);
    #endif
    
    return kpIndex;
}

void itpKeypadInit(void)
{
    unsigned int i;
    int num = ITH_COUNT_OF(kpGpioTable); 

    #ifdef EN_DBG_MSG
    printf("total gpio number is %d\n",num);
    printf("gpio lists:\n");
    for (i = 0; i < num; i++)   printf("%02d,",kpGpioTable[i]);
    printf("\n~ list end ~\n\n");
    #endif

    //transform the column & raw gpio pin table
    COL_TABLE = &kpGpioTable[0];
    RAW_TABLE = &kpGpioTable[(num / 2)];
    
    //init column GPIO pin
    for (i = 0; i < (num / 2); i++)
    {
        ithGpioSet(COL_TABLE[i]);
        ithGpioSetOut(COL_TABLE[i]);
        ithGpioSetMode(COL_TABLE[i], ITH_GPIO_MODE0);      
    }

    //init raw GPIO pin
    for (i = 0; i < (num / 2); i++)
    {
    	//set GPIO pull enable & set pull-up
    	ithGpioCtrlEnable(RAW_TABLE[i], ITH_GPIO_PULL_ENABLE);
    	ithGpioCtrlEnable(RAW_TABLE[i], ITH_GPIO_PULL_UP);
    	
        ithGpioSetIn(RAW_TABLE[i]);    
        ithGpioSetMode(RAW_TABLE[i], ITH_GPIO_MODE0);      
    }        
}

int itpKeypadGetMaxLevel(void)
{
    int num = ITH_COUNT_OF(kpGpioTable);
    return ((num / 2) * (num / 2));
}
