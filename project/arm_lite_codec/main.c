#include <stdio.h>
#include "codecs.h"

extern unsigned char risc1_start_addr[];
extern unsigned char risc1_end_addr[];
extern unsigned char risc1_bss_start[];
extern unsigned char risc1_bss_end[];

int codec_start(void) __attribute__ ( ( naked ) );
extern int codec_info();
extern void ithInvalidateDCache(void);

struct _codec_header __header __attribute__ ((section (".codecs_header"))) = {
    CODEC_MAGIC, TARGET_ID, CODEC_API_VERSION,
    (unsigned char*)risc1_start_addr, (unsigned char*)risc1_end_addr,
    codec_start, 0xffffffff, codec_info
};

void MMIO_Write(unsigned long addr, unsigned short data)
{
    *(volatile unsigned long *) (addr) = data;
}

unsigned int MMIO_Read(unsigned long addr)
{
    return *(volatile unsigned int *) (addr);
}

void dc_invalidate(void)
{
#ifndef CFG_CPU_WB
    ithInvalidateDCache();
#endif
}

int codec_start(void)
{
    // Clear BSS section of CODEC. Be carefull, it will clear all of
    // global variable which un-initialized or initialize with zero's
    // variable.
    #if 1
    {
        int *ptr = (int*)risc1_bss_start;
    do {
        *ptr++ = 0;
    } while((int)ptr <= (int)risc1_bss_end);
    }
    #endif

    //MMIO_Write(AUDIO_DECODER_START_FALG, 1);       

    main();

    // Never return by codec main functions.
    //taskSOFTWARE_BREAKPOINT();
    while(1) ;

    return 0;
}


// dummy __main function, do not remove it.
void __main(void) { /* dummy */ }
// void free(void) { /* dummy */ }