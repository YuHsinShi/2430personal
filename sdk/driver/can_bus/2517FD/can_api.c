#include "can_api.h"

static pthread_mutex_t internal_mutex = PTHREAD_MUTEX_INITIALIZER;

#ifdef APP_USE_RX_INT
bool _RX_INT(uint8_t CANFDSPI_INDEX)
{
#if(CFG_CANBUS1_ENABLE)
    if(CANFDSPI_INDEX)
    {
        if(CFG_CANBUS0_SPI_PORT)
            return (bool)!ithGpioGet(CFG_GPIO_CANBUS_INT1);
        else
            return (bool)!ithGpioGet(CFG_GPIO_CANBUS1_INT1);
    }
    else
    {
        if(CFG_CANBUS0_SPI_PORT == 0)
            return (bool)!ithGpioGet(CFG_GPIO_CANBUS_INT1);
        else
            return (bool)!ithGpioGet(CFG_GPIO_CANBUS1_INT1);
    }
#else
    return (bool)!ithGpioGet(CFG_GPIO_CANBUS_INT1);
#endif
}
#endif
static void _CAN_GPIO_Setting()
{
#if(CFG_CANBUS_ENABLE)
            // Interrupts Gpio Setting
            if(CFG_GPIO_CANBUS_GEN_INT > 0)
            {
                ithGpioSetMode(CFG_GPIO_CANBUS_GEN_INT, ITH_GPIO_MODE0);
                ithGpioSetIn(CFG_GPIO_CANBUS_GEN_INT);
            }
            if(CFG_GPIO_CANBUS_INT0 > 0)
            {
                ithGpioSetMode(CFG_GPIO_CANBUS_INT0, ITH_GPIO_MODE0);
                ithGpioSetIn(CFG_GPIO_CANBUS_INT0);
            }
            
            if(CFG_GPIO_CANBUS_INT1 > 0)
            {
                ithGpioSetMode(CFG_GPIO_CANBUS_INT1, ITH_GPIO_MODE0);
                ithGpioSetIn(CFG_GPIO_CANBUS_INT1);
            }        
    
            if(CFG_GPIO_CANBUS_STBY > 0)
            {
                ithGpioSetMode(CFG_GPIO_CANBUS_STBY, ITH_GPIO_MODE0);
                ithGpioSetOut(CFG_GPIO_CANBUS_STBY);
                ithGpioClear(CFG_GPIO_CANBUS_STBY);
            }
#endif
#if(CFG_CANBUS1_ENABLE)
            // Interrupts Gpio Setting
            if(CFG_GPIO_CANBUS1_GEN_INT > 0)
            {
                ithGpioSetMode(CFG_GPIO_CANBUS1_GEN_INT, ITH_GPIO_MODE0);
                ithGpioSetIn(CFG_GPIO_CANBUS1_GEN_INT);
            }
            if(CFG_GPIO_CANBUS1_INT0 > 0)
            {
                ithGpioSetMode(CFG_GPIO_CANBUS1_INT0, ITH_GPIO_MODE0);
                ithGpioSetIn(CFG_GPIO_CANBUS1_INT0);
            }
            
            if(CFG_GPIO_CANBUS1_INT1 > 0)
            {
                ithGpioSetMode(CFG_GPIO_CANBUS1_INT1, ITH_GPIO_MODE0);
                ithGpioSetIn(CFG_GPIO_CANBUS1_INT1);
            }        
    
            if(CFG_GPIO_CANBUS1_STBY > 0)
            {
                ithGpioSetMode(CFG_GPIO_CANBUS1_STBY, ITH_GPIO_MODE0);
                ithGpioSetOut(CFG_GPIO_CANBUS1_STBY);
                ithGpioClear(CFG_GPIO_CANBUS1_STBY);
            }
#endif

}
void ithCANOpen(uint8_t CANFDSPI_INDEX,CAN_BITTIME_SETUP bitTime, CAN_SYSCLK_SPEED clk)
{
    //Device
    CAN_CONFIG config;
    //TX 
    CAN_TX_FIFO_CONFIG txConfig;
    CAN_TEF_CONFIG     tefConfig;
    //RX
    CAN_RX_FIFO_CONFIG rxConfig;
    REG_CiFLTOBJ fObj;
    REG_CiMASK mObj;
    
    pthread_mutex_lock(&internal_mutex);

    _CAN_GPIO_Setting();

    //OSC on
    DRV_CANFDSPI_OscillatorEnable(CANFDSPI_INDEX);

    // DEVICE RESET
    DRV_CANFDSPI_Reset(CANFDSPI_INDEX);
    
    // Enable ECC and initialize RAM
    DRV_CANFDSPI_EccEnable(CANFDSPI_INDEX);

    DRV_CANFDSPI_RamInit(CANFDSPI_INDEX, 0xff);

    // Configure device
    DRV_CANFDSPI_ConfigureObjectReset(&config);
    config.IsoCrcEnable = 1;
    config.StoreInTEF = 0;

    DRV_CANFDSPI_Configure(CANFDSPI_INDEX, &config);

    // Setup TX FIFO
    DRV_CANFDSPI_TransmitChannelConfigureObjectReset(&txConfig);
    txConfig.FifoSize = 7;
    txConfig.PayLoadSize = CAN_PLSIZE_64;
    txConfig.TxPriority = 1;

    DRV_CANFDSPI_TransmitChannelConfigure(CANFDSPI_INDEX, APP_TX_FIFO, &txConfig);
    
    #ifdef APP_USE_TIME_STAMP
    tefConfig.TimeStampEnable = 0x1;
    DRV_CANFDSPI_TefConfigure(CANFDSPI_INDEX, &tefConfig);
    #endif

    // Setup RX FIFO
    DRV_CANFDSPI_ReceiveChannelConfigureObjectReset(&rxConfig);
    rxConfig.FifoSize = 15;
    rxConfig.PayLoadSize = CAN_PLSIZE_64;
    #ifdef APP_USE_TIME_STAMP
    rxConfig.RxTimeStampEnable = 0x1;
    #endif

    DRV_CANFDSPI_ReceiveChannelConfigure(CANFDSPI_INDEX, APP_RX_FIFO, &rxConfig);

    // Setup RX Filter
    fObj.word = 0x00;
    fObj.bF.SID = 0x01;  //Filter setting for SID
    fObj.bF.EXIDE = 0;
    fObj.bF.EID = 0x00;

    DRV_CANFDSPI_FilterObjectConfigure(CANFDSPI_INDEX, CAN_FILTER0, &fObj.bF);

    // Setup RX Mask
    mObj.word = 0;
    mObj.bF.MSID = 0x0; //SID MASK
    mObj.bF.MIDE = 0;   // 1:Only allow standard IDs 0:allow standard & ex
    mObj.bF.MEID = 0x0;
    DRV_CANFDSPI_FilterMaskConfigure(CANFDSPI_INDEX, CAN_FILTER0, &mObj.bF);

    // Link FIFO and Filter
    DRV_CANFDSPI_FilterToFifoLink(CANFDSPI_INDEX, CAN_FILTER0, APP_RX_FIFO, true);

    // Setup Bit Time
    //DRV_CANFDSPI_BitTimeConfigure(DRV_CANFDSPI_INDEX_0, CAN_500K_2M, CAN_SSP_MODE_AUTO, CAN_SYSCLK_40M);
    DRV_CANFDSPI_BitTimeConfigure(CANFDSPI_INDEX, bitTime, CAN_SSP_MODE_AUTO, clk);

    // Setup Transmit and Receive Interrupts
    DRV_CANFDSPI_GpioModeConfigure(CANFDSPI_INDEX, GPIO_MODE_INT, GPIO_MODE_INT);
	#ifdef APP_USE_TX_INT
    DRV_CANFDSPI_TransmitChannelEventEnable(CANFDSPI_INDEX, APP_TX_FIFO, CAN_TX_FIFO_NOT_FULL_EVENT);
	#endif
    DRV_CANFDSPI_ReceiveChannelEventEnable(CANFDSPI_INDEX, APP_RX_FIFO, CAN_RX_FIFO_NOT_EMPTY_EVENT);
    DRV_CANFDSPI_ModuleEventEnable(CANFDSPI_INDEX, CAN_TX_EVENT | CAN_RX_EVENT);

    #ifdef APP_USE_TIME_STAMP
    //Time stamp setting
    DRV_CANFDSPI_TimeStampDisable(CANFDSPI_INDEX);
    //TBC increments 1 every 1us (SYSCLK 40MHZ  set 39 = 40 cycles count 1)
    DRV_CANFDSPI_TimeStampPrescalerSet(CANFDSPI_INDEX,39);
    DRV_CANFDSPI_TimeStampModeConfigure(CANFDSPI_INDEX,CAN_TS_SOF);
    DRV_CANFDSPI_TimeStampSet(CANFDSPI_INDEX, 0);
    DRV_CANFDSPI_TimeStampEnable(CANFDSPI_INDEX);
    #endif
    // Select Normal Mode
    DRV_CANFDSPI_OperationModeSelect(CANFDSPI_INDEX, CAN_NORMAL_MODE);
    Nop();

    printf("[%d*]CANFDSPI MODE = %d\n",CANFDSPI_INDEX,DRV_CANFDSPI_OperationModeGet(CANFDSPI_INDEX));
    pthread_mutex_unlock(&internal_mutex);

}

bool ithCANRead(uint8_t CANFDSPI_INDEX, CAN_RX_MSGOBJ* rxObj, uint8_t *rxd)
{
    bool received = false;
    uint32_t i = 0;
    CAN_RX_FIFO_EVENT rxFlags;

    Nop();
    Nop();
    pthread_mutex_lock(&internal_mutex);
    // Check if FIFO is not empty
#ifdef APP_USE_RX_INT
    if (_RX_INT(CANFDSPI_INDEX)) {
#else
    DRV_CANFDSPI_ReceiveChannelEventGet(CANFDSPI_INDEX, APP_RX_FIFO, &rxFlags);

    if (rxFlags & CAN_RX_FIFO_NOT_EMPTY_EVENT) {
#endif
        // Get message
        DRV_CANFDSPI_ReceiveMessageGet(CANFDSPI_INDEX, APP_RX_FIFO, rxObj, rxd, MAX_DATA_BYTES);
        received = true;
    }
    pthread_mutex_unlock(&internal_mutex);

    return received;
}

void ithCANWrite(uint8_t CANFDSPI_INDEX, CAN_TX_MSGOBJ* txObj, uint8_t *txd)
{
    uint8_t tec;
    uint8_t rec;
    CAN_ERROR_STATE errorFlags;
    CAN_TX_FIFO_EVENT txFlags;
    //CAN_TX_FIFO_STATUS status;

    pthread_mutex_lock(&internal_mutex);

    uint8_t attempts = MAX_TXQUEUE_ATTEMPTS;

    // Check if FIFO is not full
    do {
        DRV_CANFDSPI_TransmitChannelEventGet(CANFDSPI_INDEX, APP_TX_FIFO, &txFlags);
        if (attempts == 0) {
            Nop();
            Nop();
            DRV_CANFDSPI_ErrorCountStateGet(CANFDSPI_INDEX, &tec, &rec, &errorFlags);
            return;
        }
        attempts--;
    }
    while (!(txFlags & CAN_TX_FIFO_NOT_FULL_EVENT));

    // Load message and transmit
    uint8_t n = DRV_CANFDSPI_DlcToDataBytes(txObj->bF.ctrl.DLC);

    DRV_CANFDSPI_TransmitChannelLoad(CANFDSPI_INDEX, APP_TX_FIFO, txObj, txd, n, true);
#if 0//DEBUG MSG
    DRV_CANFDSPI_ErrorCountStateGet(CANFDSPI_INDEX, &tec, &rec, &errorFlags);
    printf("[%d*]tec = %d, rec = %d, errorFlags = %x\n",CANFDSPI_INDEX, tec, rec, errorFlags);
    //DRV_CANFDSPI_TransmitQueueStatusGet(CANFDSPI_INDEX,&status);
    //printf("[%d*]status = %x\n",CANFDSPI_INDEX,status);
#endif
    pthread_mutex_unlock(&internal_mutex);

}

bool ithCANTestRamAccess(uint8_t CANFDSPI_INDEX)
{
    pthread_mutex_lock(&internal_mutex);
    // Variables
    uint8_t length;
    bool good = false;
    uint32_t i = 0;
    uint8_t ttxd[MAX_DATA_BYTES];
    uint8_t rrxd[MAX_DATA_BYTES];

    Nop();

    // Verify read/write with different access length
    // Note: RAM can only be accessed in multiples of 4 bytes
    for (length = 4; length <= MAX_DATA_BYTES; length += 4) {
        for (i = 0; i < length; i++) {
            ttxd[i] = rand() & 0xff;
            rrxd[i] = 0xff;
        }

        Nop();

        // Write data to RAM
        DRV_CANFDSPI_WriteByteArray(CANFDSPI_INDEX, cRAMADDR_START, ttxd, length);

        // Read data back from RAM
        DRV_CANFDSPI_ReadByteArray(CANFDSPI_INDEX, cRAMADDR_START, rrxd, length);

        // Verify
        good = false;
        for (i = 0; i < length; i++) {
            good = ttxd[i] == rrxd[i];

            if (!good) {
                Nop();
                Nop();

                // Data mismatch
                return false;
            }
        }
    }
    pthread_mutex_unlock(&internal_mutex);

    return true;
}

bool ithCANSleepMode(uint8_t CANFDSPI_INDEX)
{
    CAN_OSC_STATUS status;
    pthread_mutex_lock(&internal_mutex);
    DRV_CANFDSPI_OperationModeSelect(CANFDSPI_INDEX, CAN_SLEEP_MODE);
    Nop();
    DRV_CANFDSPI_OscillatorStatusGet(CANFDSPI_INDEX, &status);

    printf("[*]OSC Ready = %x\n",status.OscReady);

    pthread_mutex_unlock(&internal_mutex);
}

uint32_t ithCANDlcToDataBytes(CAN_DLC dlc)
{
    return DRV_CANFDSPI_DlcToDataBytes(dlc);
}

CAN_DLC ithCANDataBytesToDlc(uint8_t n)
{
    return DRV_CANFDSPI_DataBytesToDlc(n);
}


