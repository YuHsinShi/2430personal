#include "obd2.h"
#include "obd2_hal.h"


static pthread_mutex_t hal_internal_mutex = PTHREAD_MUTEX_INITIALIZER;

// Transmit objects
//CAN_TX_MSGOBJ txObj;

// Receive objects
//CAN_RX_MSGOBJ rxObj;

void HAL_CANInit(OBD_PROTOCOLS pt)
{

    pthread_mutex_lock(&hal_internal_mutex);
    #if 0
    
    /* Init SPI driver */
    DRV_SPI_Initialize(DRV_CANFDSPI_INDEX_1);
   
    switch(pt)
    {
        case PRO_CAN_11B_500K:
        case PRO_CAN_29B_500K:
            ithCANOpen(DRV_CANFDSPI_INDEX_1, CAN_500K_1M, CAN_SYSCLK_40M);
            break;
        case PRO_CAN_11B_250K:
        case PRO_CAN_29B_250K:
            ithCANOpen(DRV_CANFDSPI_INDEX_1 ,CAN_250K_1M, CAN_SYSCLK_40M);
            break;
        default:
            printf("Not support this protocol\n");
            break;
    }

    /* Configure transmit message */
    txObj.word[0] = 0;
    txObj.word[1] = 0;
    txObj.bF.ctrl.BRS = 0;             //bit rate switch, 2.0B BRS off
    txObj.bF.ctrl.DLC = CAN_DLC_8;     //Data Length Code 2.0B  = 8
    txObj.bF.ctrl.FDF = 0;             //identifier FD frame
    
    if(pt == PRO_CAN_29B_500K || pt == PRO_CAN_29B_250K)
    {
        /*ID:18 DB 33 f1*/
        txObj.bF.id.SID = 0x636;
        txObj.bF.id.EID = 0x333f1;
        txObj.bF.ctrl.IDE = 0x1;         //identifier Ex 29bit
    }
    else
    {
        txObj.bF.id.SID = 0x7DF;
        txObj.bF.ctrl.IDE = 0x0;         //identifier normal 11bit
    }
    #endif
    pthread_mutex_unlock(&hal_internal_mutex);

    
}

bool HAL_CANRead(uint8_t* rxb)
{
    #if 0

    pthread_mutex_lock(&hal_internal_mutex);

    if(ithCANRead(DRV_CANFDSPI_INDEX_1, &rxObj, rxb))
    {
        pthread_mutex_unlock(&hal_internal_mutex);
        return true;
    }
    else
    {
        pthread_mutex_unlock(&hal_internal_mutex);
        return false;
    }
    #endif
    return false;
    
        
}

void HAL_CANWrite(uint8_t* txb)
{
    #if 0
    pthread_mutex_lock(&hal_internal_mutex);

    ithCANWrite(DRV_CANFDSPI_INDEX_1, &txObj, txb);
    
    pthread_mutex_unlock(&hal_internal_mutex);
    #endif

}
