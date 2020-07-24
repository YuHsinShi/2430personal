/** @file
 * ITE IoT protocol wrapper declaration.
 *
 * @author Steven Hsiao
 * @version 1.0
 * @date 2016
 * @copyright ITE Tech. Inc. All Rights Reserved.
 */
#ifndef PROTOCOL_WRAPPER_H
#define PROTOCOL_WRAPPER_H

#ifdef __cplusplus
extern "C" {
#endif

int IotProtocolWrite(uint32_t id, uint8_t* txBuffer, int txSize);

int IotProtocolRead(uint32_t id, uint8_t* param, int pararmSize, uint8_t* rxBuffer, int rxSize);

#ifdef __cplusplus
}
#endif

#endif /* AIRCONDITIONER_H */
/** @} */ // end of airconditioner