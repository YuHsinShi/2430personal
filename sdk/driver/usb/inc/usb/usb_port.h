#ifndef USB_PORT_H
#define USB_PORT_H

#include "openrtos/portmacro.h"


#ifdef __cplusplus
extern "C" {
#endif


#define MMP_TRUE        true
#define MMP_FALSE       false

#define MMP_NULL        NULL
#define MMP_INT         int
#define MMP_INT32       int
#define MMP_UINT32      uint32_t
#define MMP_UINT        uint32_t
#define MMP_UINT16      uint16_t
#define MMP_UINT8       uint8_t
#define MMP_ULONG       uint32_t
#define MMP_BOOL        bool
#define MMP_INLINE      inline


/*
 * for sempahore
 */
sem_t* usb_create_sem(int cnt);
#define MMP_MUTEX                       sem_t*
#define SYS_CreateSemaphore(cnt,c)      usb_create_sem(cnt)  
#define SYS_WaitSemaphore               sem_wait
#define SYS_ReleaseSemaphore            sem_post
#define SYS_DeleteSemaphore(a)			do { sem_destroy(a); free(a); } while(0)

/*
 * for spin lock
 */
typedef struct {
	volatile unsigned int lock;
} _spinlock_t;

#define _spin_lock_init(x)	
#define _spin_lock(x)					ithEnterCritical()
#define _spin_unlock(x)					ithExitCritical()
#define _local_irq_save()					
#define _local_irq_restore()				
#define _spin_lock_irqsave(lock)		do { _local_irq_save();  _spin_lock(lock); } while (0)
#define _spin_unlock_irqrestore(lock)	do { _spin_unlock(lock);  _local_irq_restore(); } while (0)
#define _spin_lock_irq(x)               _spin_lock(x)
#define _spin_unlock_irq(x)             _spin_unlock(x)


#define MMP_Sleep(x)                    usleep(x*1000)


#ifdef __cplusplus
}
#endif

#endif


