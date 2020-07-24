#ifndef ITH_STORAGE_H
#define ITH_STORAGE_H

/** @addtogroup ith ITE Hardware Library
 *  @{
 */
/** @addtogroup ith_storage Storage
 *  @{
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * The pin of card definition.
 */
typedef enum
{
    ITH_CARDPIN_SD0 = 0,    ///< SD0 card pin
    ITH_CARDPIN_MS  = 0,    ///< MS card pin
    ITH_CARDPIN_XD  = 0,    ///< xD card pin
    ITH_CARDPIN_SD1,        ///< SD1 card pin
    ITH_CARDPIN_CF,         ///< CF card pin

    ITH_CARDPIN_MAX
} ITHCardPin;

#define SD_PIN_NUM 10
/**
 * The card configuration definition.
 */
typedef struct
{
    uint8_t cardDetectPins[ITH_CARDPIN_MAX];    ///< Array of card detection GPIO pins
    uint8_t powerEnablePins[ITH_CARDPIN_MAX];   ///< Array of power enable GPIO pins
    uint8_t writeProtectPins[ITH_CARDPIN_MAX];  ///< Array of write protect GPIO pins
    uint8_t sd0Pins[SD_PIN_NUM];
    uint8_t sd1Pins[SD_PIN_NUM];
} ITHCardConfig;

/**
 * Initializes card module.
 *
 * @param cfg The card configuration to set.
 */
void ithCardInit(const ITHCardConfig *cfg);

/**
 * Power on specified card.
 *
 * @param pin The card power on pin to power on.
 */
void ithCardPowerOn(ITHCardPin pin);

/**
 * Power off specified card.
 *
 * @param pin The card power on pin to power off.
 */
void ithCardPowerOff(ITHCardPin pin);

#if defined(CFG_GPIO_SD1_WIFI_POWER_ENABLE) && defined(CFG_NET_WIFI_SDIO_NGPL)
/**
 * Power on NGPL SDIO WIFI card.
 *
 */
void ithWIFICardPowerOn(void);

/**
 * Power off NGPL SDIO WIFI card.
 *
 */
void ithWIFICardPowerOff(void);
#endif

/**
 * Whether card is inserted or not.
 *
 * @param pin The card detect pin to detect.
 */
bool ithCardInserted(ITHCardPin pin);

/**
 * Whether card is locked or not.
 *
 * @param pin The card write protect pin to detect.
 */
bool ithCardLocked(ITHCardPin pin);

typedef enum
{
    ITH_STOR_NAND = 0,
    ITH_STOR_XD   = 1,
    ITH_STOR_SD   = 2,
    ITH_STOR_MS_0 = 3,
    ITH_STOR_MS_1 = 4,
    ITH_STOR_CF   = 5,
    ITH_STOR_NOR  = 6,
    ITH_STOR_SD1  = 7
} ITHStorage;

extern void *ithStorMutex;
/**
 * Select the storagte. It will set the corresponding GPIO.
 *
 * @param storage The storage be selected.
 */
void ithStorageSelect(ITHStorage storage);
/**
 * Unselect the storagte. 
 *
 * @param storage The storage be unselected.
 */
void ithStorageUnSelect(ITHStorage storage);
/**
 * Power reset for the SD card. 
 *
 * @param storage The SD card be selected.
 */
void ithSdPowerReset(ITHStorage storage);
/**
 * Get the SD card operation mode.  (1: 4-bit, 0: 1-bit) 
 *
 * @param storage The SD card be selected.
 */
int ithSd4bitMode(ITHStorage storage);
/**
 * Get the SDIO card operation mode.  (1: 4-bit, 0: 1-bit)
 *
 * @param storage The SD card be selected.
 */
int ithSdio4bitMode(void);
/**
 * Get the SD/SDIO max clock.
 *
 * @param storage The SD card be selected.
 */
int ithSdMaxClk(ITHStorage storage);
/**
 * Get the SD pin share information. (1: no pin share, 0: pin share)
 *
 * @param storage The SD card be selected.
 */
int ithSdNoPinShare(ITHStorage storage);
/**
 * Get the SD delay setting. It's for offset 0x100.
 *
 * @param storage The SD card be selected.
 */
uint32_t ithSdDelay(ITHStorage storage);

#ifdef __cplusplus
}
#endif

#endif // ITH_STORAGE_H

/** @} */ // end of ith_storage
/** @} */ // end of ith
