
#define __CRT0__

#define ZeroBSS
//#define FillSTACK

#define AXI_REMAP      0xd9000130

#define MEM_BASE       0x00000000
#define ROM_BASE       0x80000000
#define SRAM_BASE      0xa0000000
#define AHB_BASE       0xb0000000
#define AXI_BASE       0xc0000000
#define AXI2_BASE      0xc8000000
#define APB_BASE       0xd0000000

#define TLB_MEM_BASE   (MEM_BASE>>18)
#define TLB_MEM_SIZE   0x040                // 64MB
#define TLB_ROM_BASE   (ROM_BASE>>18)
#define TLB_ROM_SIZE   0x001                // 1M
#define TLB_SRAM_BASE  (SRAM_BASE>>18)
#define TLB_SRAM_SIZE  0x001                // 1M
#define TLB_AHB_BASE   (AHB_BASE>>18)
#define TLB_AHB_SIZE   0x020                // 32MB
#define TLB_AXI_BASE   (AXI_BASE>>18)
#define TLB_AXI_SIZE   0x001                // 1M
#define TLB_AXI2_BASE  (AXI2_BASE>>18)
#define TLB_AXI2_SIZE  0x001                // 1M
#define TLB_APB_BASE   (APB_BASE>>18)
#define TLB_APB_SIZE   0x090                // 144M

/* Standard definitions of Mode bits and Interrupt (I & F) flags in PSRs */
    .equ    A_BIT,          0x100      /* when A bit is set, ABORT is disabled */
    .equ    I_BIT,          0x080      /* when I bit is set, IRQ is disabled */
    .equ    F_BIT,          0x040      /* when F bit is set, FIQ is disabled */

    .equ    USR_MODE,       0x10
    .equ    FIQ_MODE,       0x11
    .equ    IRQ_MODE,       0x12
    .equ    SVC_MODE,       0x13
    .equ    ABT_MODE,       0x17
    .equ    UND_MODE,       0x1B
    .equ    SYS_MODE,       0x1F

/* constant to pre-fill the stack */
    .equ    STACK_FILL,     0xaaaaaaaa

    .extern codec_start
    .extern _stack
    .extern _tlb_base

    .arm
    .align 0
    .code 32

###########################################################################
## Macro
###########################################################################

#define ENTRY(symbol) \
        .global symbol ; \
symbol:

#define FUNC_BEGIN(symbol) \
        .global symbol; \
        .type symbol, %function ; \
symbol:

#define FUNC_END(symbol) \
        .size   symbol, .-symbol

.macro save_user_regs
        stmfd   sp!, {r0-r12, lr}
        mrs     r0, spsr
        stmfd   sp!, {r0}
.endm

.macro restore_user_regs
        ldmfd   sp!, {r0}
        msr     spsr, r0
        ldmfd   sp!, {r0-r12, lr}
        subs    pc, lr, #4
.endm

###########################################################################
## Begin of RAM
###########################################################################
        .section .reset, "ax"
        .org    0x000
ENTRY(_start)
        b       fLib_Start
        ldr     pc, fLib_UndefVector
        ldr     pc, fLib_SWIVector
        ldr     pc, fLib_PreAbtVector
        ldr     pc, fLib_DataAbtVector
        ldr     pc, fLib_ReservdVector
        ldr     pc, fLib_IRQVector
        ldr     pc, fLib_FIQVector

###########################################################################
## Remap
###########################################################################
        .org    32
FUNC_BEGIN(__remap)
        ldr     r1, .L2
        mov     r0, #1
        str     r0, [r1]

        nop                  // 8 delay slots to remap memory.
        nop                  // Fetch+instruction+decode+register+shift+execute+memory+write_back pipeline statge.
        nop
        nop
        nop
        nop
        nop
        b       _start
1:      b       1b           // never here
.L2:
        .word   AXI_REMAP
FUNC_END(__remap)

        .org    0x54
fLib_Reset:          .word   fLib_Start
fLib_UndefVector:    .word   fLib_TrapUndef
fLib_SWIVector:      .word   fLib_TrapSWI
fLib_PreAbtVector:   .word   fLib_PrefetchAbort
fLib_DataAbtVector:  .word   fLib_DataAbort
fLib_ReservdVector:  .word   fLib_ResetStop
fLib_IRQVector:      .word   fLib_DummyIRQ
fLib_FIQVector:      .word   fLib_DummyFIQ

########################################
# Pointer of Print Buffer
########################################
        .org    0xcc
        .global __dbgmsg_buf_ptr
__dbgmsg_buf_ptr:
        .long   0
        .global __dbgmsg_buf_len
__dbgmsg_buf_len:
        .long   0
        .global __dbgmsg_idx_ptr
__dbgmsg_idx_ptr:
        .long   0

###########################################################################
## Start program
###########################################################################
        .org    0x100
FUNC_BEGIN(fLib_Start)
        bl      start_procedure

        # SHOULD NOT REACH HERE
1:      b       1b      // forever loop
        nop
        nop
        nop
        nop
FUNC_END(fLib_Start)

###########################################################################
## UndefVector
###########################################################################
        .org    0x200
FUNC_BEGIN(fLib_TrapUndef)

        msr     CPSR_c,#(UND_MODE | I_BIT | F_BIT)

        #bl      exit

        # SHOULD NOT REACH HERE
1:      b       1b      // forever loop
        nop
        nop
        nop
        nop
FUNC_END(fLib_TrapUndef)

###########################################################################
## SWIVector
###########################################################################
        .org    0x300
FUNC_BEGIN(fLib_TrapSWI)

        msr     CPSR_c,#(SVC_MODE | I_BIT | F_BIT)

        # SHOULD NOT REACH HERE
1:      b       1b      // forever loop
        nop
        nop
        nop
        nop
FUNC_END(fLib_TrapSWI)

###########################################################################
## PreAbtVector
###########################################################################
        .org    0x400
FUNC_BEGIN(fLib_PrefetchAbort)

        msr     CPSR_c,#(ABT_MODE | I_BIT | F_BIT)

        // Read the CR5 fault status regsiters to r0
        mrc     p15,0,r0,c5,c0,0

        #bl      exit

        # SHOULD NOT REACH HERE
1:      b       1b      // forever loop
        nop
        nop
        nop
        nop
FUNC_END(fLib_PrefetchAbort)

###########################################################################
## DataAbtVector
###########################################################################
        .org    0x500
FUNC_BEGIN(fLib_DataAbort)

        msr     CPSR_c,#(ABT_MODE | I_BIT | F_BIT)

        // Read the CR5 fault status regsiters to r0
        mrc     p15,0,r0,c5,c0,0

        #bl      exit

        # SHOULD NOT REACH HERE
1:      b       1b      // forever loop
        nop
        nop
        nop
        nop
FUNC_END(fLib_DataAbort)

###########################################################################
## ReservdVector
###########################################################################
        .org    0x600
FUNC_BEGIN(fLib_ResetStop)

        # SHOULD NOT REACH HERE
1:      b       1b      // forever loop
        nop
        nop
        nop
        nop
FUNC_END(fLib_ResetStop)

###########################################################################
## IRQVector
###########################################################################
        .org    0x700
FUNC_BEGIN(fLib_DummyIRQ)

        # SHOULD NOT REACH HERE
1:      b       1b      // forever loop
        nop
        nop
        nop
        nop
FUNC_END(fLib_DummyIRQ)

###########################################################################
## FIQVector
###########################################################################
        .org    0x800
FUNC_BEGIN(fLib_DummyFIQ)

        # SHOULD NOT REACH HERE
1:      b       1b      // forever loop
        nop
        nop
        nop
        nop
FUNC_END(fLib_DummyFIQ)

###########################################################################
## Start Procedure
###########################################################################
        .org    0x1000
        .section .text

ENTRY(start_procedure)
        /* reset registerr0~r15 =0 */
        mov r0 , #0
        mov r1 , #0
        mov r2 , #0
        mov r3 , #0
        mov r4 , #0
        mov r5 , #0
        mov r6 , #0
        mov r7 , #0
        mov r8 , #0
        mov r9 , #0
        mov r10, #0
        mov r11, #0
        mov r12, #0
        mov r13, #0
        mov r14, #0

        /*
         * set the cpu to SVC32 mode
         */
        mrs r0,cpsr
        bic r0,r0,#0x1f
        orr r0,r0,#0xd3
        msr cpsr,r0

        /*
         * setup mmu
         */
        ldr     r1, =_tlb_base
        mcr     p15,0,r1,c2,c0,0        /* set TLB_BASE_ADDR */

        /*
         * Reset co-processor
         */
        mcr     p15,0,r1,c7,c14,0   /* clean&invalidate D-Cache all */
        mcr     p15,0,r1,c7,c10,4   /* drain write buffer */
        mcr     p15,0,r1,c7,c5,0    /* invalidate I-Cache all */
        mcr     p15,0,r1,c7,c5,6    /* invalidate BTB All */
        mcr     p15,0,r1,c8,c7,0    /* invalidate v1 TLB */

        /*
         * For AXI
         */
        //mov     r5, #0xa0000000
        //ldr     r1, [r5, #0x88]
        //orr     r1, r1, #2
        //str     r1, [r5, #0x88]

#if 0
        mov r5, #0
        mov r2, #0
        orr r2, r2, #0x0c10
#ifdef CFG_CPU_WB
        orr r2, r2, #0x000e             /* set domain=4'b0000, c=1'b1, b=1'b0 */
#else
        orr r2, r2, #0x000a             /* set domain=4'b0000, c=1'b1, b=1'b0 */
#endif
1:      lsl r6, r5, #2                  /* set TLB_MEM */
        lsl r7, r5, #20
        add r5, r5, #1
        add r6, r6, #TLB_MEM_BASE
        add r7, r7, #MEM_BASE
        orr r3, r1, r6
        orr r4, r2, r7
        cmp r5, #TLB_MEM_SIZE
        str r4, [r3]
        bne 1b

        mov r5, #0                      /* set domain=4'b0000, c=1'b1, b=1'b0 */
1:      lsl r6, r5, #2                  /* set TLB_ROM */
        lsl r7, r5, #20
        add r5, r5, #1
        add r6, r6, #TLB_ROM_BASE
        add r7, r7, #ROM_BASE
        orr r3, r1, r6
        orr r4, r2, r7
        cmp r5, #TLB_ROM_SIZE
        str r4, [r3]
        bne 1b

        mov r5, #0
1:      lsl r6, r5, #2                  /* set TLB_SRAM */
        lsl r7, r5, #20
        add r5, r5, #1
        add r6, r6, #TLB_SRAM_BASE
        add r7, r7, #SRAM_BASE
        orr r3, r1, r6
        orr r4, r2, r7
        cmp r5, #TLB_SRAM_SIZE
        str r4, [r3]
        bne 1b

        mov r5, #0
        bic r2, r2, #0x0000000c         /* set domain=4'b0000, clean c=1'b0, b=1'b0 */
1:      lsl r6, r5, #2                  /* set TLB_AHB */
        lsl r7, r5, #20
        add r5, r5, #1
        add r6, r6, #TLB_AHB_BASE
        add r7, r7, #AHB_BASE
        orr r3, r1, r6
        orr r4, r2, r7
        cmp r5, #TLB_AHB_SIZE
        str r4, [r3]
        bne 1b

        mov r5, #0
        bic r2, r2, #0x0000000c         /* set domain=4'b0000, clean c=1'b0, b=1'b0 */
1:      lsl r6, r5, #2                  /* set TLB_AXI */
        lsl r7, r5, #20
        add r5, r5, #1
        add r6, r6, #TLB_AXI_BASE
        add r7, r7, #AXI_BASE
        orr r3, r1, r6
        orr r4, r2, r7
        cmp r5, #TLB_AXI_SIZE
        str r4, [r3]
        bne 1b

        mov r5, #0
        orr r2, r2, #0x00000008         /* set domain=4'b0000, clean c=1'b1, b=1'b0 */
1:      lsl r6, r5, #2                  /* set TLB_AXI2 */
        lsl r7, r5, #20
        add r5, r5, #1
        add r6, r6, #TLB_AXI2_BASE
        add r7, r7, #AXI2_BASE
        orr r3, r1, r6
        orr r4, r2, r7
        cmp r5, #TLB_AXI2_SIZE
        str r4, [r3]
        bne 1b
//        mov r5, #0
//        mov     r8, #0x00000008
//        str     r8, [r2]                /* set domain=4'b0000, c=1'b1, b=1'b0 */
//1:      lsl r6, r5, #2                  /* set TLB_AXI2 */
//        lsl r7, r5, #20
//        add r5, r5, #1
//        add r6, r6, #TLB_AXI2_BASE
//        add r7, r7, #AXI2_BASE
//        orr r3, r1, r6
//        orr r4, r2, r7
//        cmp r5, #TLB_AXI2_SIZE
//        str r4, [r3]
//        bne 1b

        mov r5, #0
        bic r2, r2, #0x0000000c         /* set domain=4'b0000, clean c=1'b0, b=1'b0 */
1:      lsl r6, r5, #2                  /* set TLB_APB */
        lsl r7, r5, #20
        add r5, r5, #1
        add r6, r6, #TLB_APB_BASE
        add r7, r7, #APB_BASE
        orr r3, r1, r6
        orr r4, r2, r7
        cmp r5, #TLB_APB_SIZE
        str r4, [r3]
        bne 1b
#endif

        /*
         * setup domain access
         */
        ldr     r0, =0xffffffff
        mcr     p15,0,r0,c3,c0,0        /* set all domain can access */

        /*
         * setup peripheral port
         */
        mov     r1, #AHB_BASE          /* set peripheral port base address size */
        orr     r1, r1, #0x16           /* set peripheral port size =512MB */
        mcr     p15,0,r1,c15,c2,4

        /*
         * setup CR1-0 Configuration Register
         */
        mov     r0, #0
        mrc     p15,0,r2,c1,c0,0
        bic     r2,r2,#0x00002B00   /* clear bits 13, 11, 9:8 (--V- B-RS) */
        bic     r2,r2,#0x0000008F   /* clear bits 7, 3:0 (B--- WCAM) */

        orr     r2,r2,#0x00000001   /* set bit  0 (M) MMU */
        orr     r2,r2,#0x00000002   /* set bit  1 (A) Align */
        orr     r2,r2,#0x00001000   /* set bit 12 (I) I-Cache */
        orr     r2,r2,#0x00000004   /* set bit  2 (D) D-Cache */
        orr     r2,r2,#0x00000008   /* set bit  3 (W) Write-Buffer */
        orr     r2,r2,#0x00000800   /* set bit 11 (B) Branch Prediction */
        mcr     p15,0,r2,c1,c0,0        /* Enable MMU */
        mcr     p15,0,r0,c7,c5,4        /* PrefetchFlush instruction */

        @Turn off IRQ/FIQ bit of CPU
        mrs     r0,cpsr
        orr     r0,r0,#(I_BIT|F_BIT)
        msr     cpsr_c,r0

#ifdef ZeroBSS
        /* Clear the .bss section (zero init) */
        ldr     r1,=_bss_start
        ldr     r2,=_bss_end
        mov     r3,#0
1:
        cmp     r1,r2
        str r3,[r1], #4
        //stmltia r1!,{r3}
        blt     1b
#endif

#ifdef FillSTACK
        /* Fill the .stack section */
        ldr     r1,=_stack_start
        ldr     r2,=_stack_end
        ldr     r3,=STACK_FILL
1:
        cmp     r1,r2
        str r3,[r1], #4
        //stmltia r1!,{r3}
        blt     1b
#endif

        /*
         *  initial stack
        */
        ldr     r0, =_stack
        bl      InitStacks

        bl      codec_start

FUNC_BEGIN(exit)
ExitFunction:
        mov     r0, #0
        mcr     p15,0,r0,c7,c10,0      // clean DCache All
        mcr     p15,0,r0,c7,c10,4      // SYNC (drain write buffer)
        mcr     p15,0,r0,c7,c0,4       // execute the idle instruction but it only executes when it is not user mode
        nop
        nop
        nop
        nop
        nop
1:
        b       1b
FUNC_END(exit)
###########################################################################
## InitStacks
#  'Routine' called from boot-up to set all stacks.
#  Although we have no stack yet, r14 contains return address. r0 contains
#  top of memory (do not corrupt!) and all stacks are relative to this value.
###########################################################################
InitStacks:
        mov     r6, r14         // !!! Protect the return address !!!

        /* Initialize stack pointers for all ARM modes */
        msr     CPSR_c,#(IRQ_MODE | I_BIT | F_BIT)
        ldr     sp,=_irq_stack                /* set the IRQ stack pointer */

        msr     CPSR_c,#(FIQ_MODE | I_BIT | F_BIT)
        ldr     sp,=_fiq_stack                /* set the FIQ stack pointer */

        msr     CPSR_c,#(SVC_MODE | I_BIT | F_BIT)
        ldr     sp,=_svc_stack                /* set the SVC stack pointer */

        msr     CPSR_c,#(ABT_MODE | I_BIT | F_BIT)
        ldr     sp,=_abt_stack                /* set the ABT stack pointer */

        msr     CPSR_c,#(UND_MODE | I_BIT | F_BIT)
        ldr     sp,=_und_stack                /* set the UND stack pointer */

        msr     CPSR_c,#(SYS_MODE | I_BIT | F_BIT)
        ldr     sp,=_sys_stack                /* set the SYS stack pointer */

        /* switch to supervisor mode */
        msr     CPSR_c,#(SVC_MODE | I_BIT | F_BIT)

        /* Just return back to boot up code */
        mov     pc,r6

itpSuspendRestore:
        /*
         * Invalidate ICache All
         */
        mov     r1, #0
        mcr     p15, 0, r1, c7, c5, 0

        /*
         * disable MMU
         */
        mrc     p15,0,r0,c1,c0,0
        bic     r0, r0, #0x00000001     /* set bit  0 (M) MMU */
        mcr     p15,0,r0,c1,c0,0        /* Disable MMU */
        nop                             /* 4 delay slots to disable mmu */
        nop
        nop
        nop

        /*
         * remap setting & jump to memory
         */
        mov     r2, pc
        add     r2, r2, #0x18
        sub     r2, r2, #0x80000000
        mov     r1, #0xd0000000
        orr     r1, r1, #0x88
        mov     r0, #1
        str     r0, [r1]
        mov     pc, r2

        mov     r1, #0xc0000000
        orr     r1, r1, #0x00200000
        mov     r2, #0x1600
        orr     r2, r2, #0x00a4
        mov     r0, #0x0011
        strh    r0, [r1, r2]

        /*
         * set the cpu to SVC32 mode and Turn off IRQ/FIQ bit of CPU
         */
        mrs     r0, cpsr
        bic     r0, r0, #0x1f
        orr     r0, r0, #0xd3
        msr     cpsr, r0

        mov     r1, #0xc0000000
        orr     r1, r1, #0x00200000
        mov     r2, #0x1600
        orr     r2, r2, #0x00a4
        mov     r0, #0x0022
        strh    r0, [r1, r2]

        /*
         * setup mmu
         */
        mcr     p15, 0, r0, c2, c0, 0 /* setTLB__BASE_ADDR */
1:      b       1b
