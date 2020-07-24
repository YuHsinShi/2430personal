/*
 * SW_SPI.h
 *
 */

#ifndef ATA_API_H_
#define ATA_API_H_

void ATA6570_Init(uint8_t port, uint8_t datarate);
uint8_t ATA6570_GetDeviceMode(uint8_t port);
void ATA6570_SleepMode(uint8_t port);
void ATA6570_Wakeup(uint8_t port);

#endif /* SW_SPI_H_ */