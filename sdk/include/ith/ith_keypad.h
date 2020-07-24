#ifndef ITH_KEYPAD_H
#define ITH_KEYPAD_H

/** @addtogroup ith ITE Hardware Library
 *  @{
 */
/** @addtogroup ith_keypad Keypad
 *  @{
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initializes keypad module.
 *
 * @param pinCount The count of GPIO pins.
 * @param pinArray The array of GPIO pin numbers.
 */
void ithKeypadInit(unsigned int pinCount, unsigned int *pinArray);

/**
 * Enables keypad module.
 */
void ithKeypadEnable(void);

/**
 * Probes the input of keypad.
 */
int ithKeypadProbe(void);

#ifdef __cplusplus
}
#endif

#endif // ITH_KEYPAD_H
/** @} */ // end of ith_keypad
/** @} */ // end of ith