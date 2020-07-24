
#ifndef ITE_ECM_H
#define ITE_ECM_H

#ifdef __cplusplus
extern "C" {
#endif


int iteEcmRegister(void);

int iteEcmOpen(uint8_t* mac_addr, void(*func)(void *arg, void *packet, int len), void* arg);

int iteEcmStop(void);

int iteEcmSend(void* packet, uint32_t len);

int iteEcmGetLink(void);

int iteEcmSetRxMode(int flags);

#ifdef __cplusplus
}
#endif

#endif // ITE_ECM_H
