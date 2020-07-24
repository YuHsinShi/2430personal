
#ifndef ITE_USBD_NCM_H
#define ITE_USBD_NCM_H

#ifdef __cplusplus
extern "C" {
#endif


int iteUsbdNcmRegister(void);
int iteUsbdNcmUnRegister(void);

int iteUsbdNcmOpen(uint8_t* mac_addr, void(*func)(void *arg, void *packet, int len), void* arg);
int iteUsbdNcmStop(void);
int iteUsbdNcmSend(void* packet, uint32_t len);
int iteUsbdNcmGetLink(void);
int iteUsbdNcmSetRxMode(int flags);

#ifdef __cplusplus
}
#endif

#endif // ITE_USBD_NCM_H
