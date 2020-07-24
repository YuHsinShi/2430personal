/** @file
 * ARMLite device library.
 *
 * @author Kevin Chen
 * @version 1.0
 * @date 2019/10/01
 * @copyright ITE Tech. Inc. All Rights Reserved.
 */
/** @defgroup ARMLite Utility API declaration
 *  @{
 */
#ifndef ARMLITE_UTILITY_API_H
#define ARMLITE_UTILITY_API_H

#ifdef __cplusplus
extern "C" {
#endif

//GPIO
void setGpioDir(unsigned long gpioPin, unsigned long bIn);

void setGpioMode(unsigned long gpioPin, unsigned long mode);

unsigned long getGpioValue(unsigned long gpioPin, unsigned long bIn);

void setGpioValue(unsigned long gpioPin, unsigned long bHigh);

//TickTimer

#ifdef __cplusplus
}
#endif

#endif // ARMLITE_UTILITY_API_H
/** @} */ // end of ARMLITE_UTILITY_API_H
