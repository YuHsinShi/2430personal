#ifndef OBD2_HAL_H
#define OBD2_HAL_H

#include "2517FD/can_api.h"

#define CAN_MAX_DATA_BYTES MAX_DATA_BYTES
#define OBD_MAX_DATA_BYTES 8

/*HAL api*/
void HAL_CANInit(OBD_PROTOCOLS pt);
bool HAL_CANRead(uint8_t* rxb);
void HAL_CANWrite(uint8_t* txb);

#endif
