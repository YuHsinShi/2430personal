#include "codecs_defs.h"

OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")
OUTPUT_ARCH(arm)

ENTRY(_start)
GROUP(-lc -lgcc)
SEARCH_DIR(.)

MEMORY
{
		except : ORIGIN = 0x00000000, LENGTH = 0x1000
        ram    : ORIGIN = 0x1000, LENGTH = 64M - 0x1000
        ahbram : ORIGIN = 0xa0000000, LENGTH = 32K 
}

/* The sizes of the stacks used by the application. NOTE: you need to adjust */
SYS_STACK_SIZE = 64K;
IRQ_STACK_SIZE = 64;
FIQ_STACK_SIZE = 64;
SVC_STACK_SIZE = 64K;
ABT_STACK_SIZE = 0;
UND_STACK_SIZE = 0;
HEAP_SIZE      = 1K;

SECTIONS
{
      .reset :
        {
        *(.reset)
        } > except

  .risc1_header :
  {
    _risc1_start_addr = .;
    risc1_start_addr  = .;
    KEEP(*(.codecs_header))

    // codec stack for audio codec on processor 2
    _risc1_stack = .;
  } > ram

      .boot :
        {
          *(.boot_table)
          *(.boot)
          PROVIDE (__boot_end = .);
        } > ram

      .text :
        {
        _dst_beg = .;
        *(.text .stub .text.* .gnu.linkonce.t.*)
        } > ram

      .data ALIGN(0x4) :
        {
        *(.data .rodata .rodata.* .ctors .dtors .gnu.linkonce.r.* .gcc_except_table)
        /* *mallocr.o(.bss); */
        *(.bss .sbss) 
        _dst_end = .;
        } > ram

  . = ALIGN(32 / 8);
  __bss_start = .;
  _bss_start = .;
  risc1_bss_start = .;

      .bss ALIGN(0x4) :
        {
        _bss_start = .;
        __bss_start = .;
        /* *(.bss .sbss) */
        *(COMMON)
        _bss_end = .;
        __bss_end = .;
        } > ram

  _end = .;
  _bss_end = .;
  risc1_bss_end = .;
  
  PROVIDE (end = .);

      .stack ALIGN(0x10) (NOLOAD) :
        {
        _heap_start = .;
        __heap_start = .;
        end = .;
        . += DEFINED(_sbrk) ? 500K : HEAP_SIZE; /* define the heap size here */
        _heap_end   = .;
        __heap_end   = .;

        _stack_start = . ;

        . += IRQ_STACK_SIZE;
        . = ALIGN (4);
        _irq_stack = . ;

        . += FIQ_STACK_SIZE;
        . = ALIGN (4);
        _fiq_stack = . ;

        . += SVC_STACK_SIZE;
        . = ALIGN (4);
        _svc_stack = . ;

        . += ABT_STACK_SIZE;
        . = ALIGN (4);
        _abt_stack = . ;

        . += UND_STACK_SIZE;
        . = ALIGN (4);
        _und_stack = . ;

        . += SYS_STACK_SIZE;
        . = ALIGN (4);
        _sys_stack = . ;

        _stack = .;
        _stack_end = .;
        . += 4;
        
        _risc1_end_addr = .;
        risc1_end_addr  = .;
        } > ram

      .tlb ALIGN(0x4000) :
       {
         _tlb_base = .;
         __end  = .;
       } > ram
      
      .ahbram :
       {
         _ahbram_base = .;
       } > ahbram  
}
