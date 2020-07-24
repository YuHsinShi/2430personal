/** @file
 * ITE IoT bus declaration.
 *
 * @author Steven Hsiao
 * @version 1.0
 * @date 2016
 * @copyright ITE Tech. Inc. All Rights Reserved.
 */
#ifndef IOT_BUS_H
#define IOT_BUS_H

#ifdef __cplusplus
extern "C" {
#endif

int IotBusWrite(uint8_t* txBuffer, int txSize);

int IotBusRead(uint8_t* rxBuffer, int rxSize);

int IotBusInit(void);

#ifdef __cplusplus
}
#endif

#endif /* AIRCONDITIONER_H */
/** @} */ // end of airconditioner