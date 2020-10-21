#ifndef __GLOBAL_H__
#define __GLOBAL_H__

// file for some global macros


// global descriptor table base address
#define GDT_BASE_ADDR  0x00700000
// tss base address (task state segment base address)
#define TSS_BASE_ADDR  0x00700100
// interrupt descriptor table base address
#define IDT_BASE_ADDR  0x00700300
// page directory base address
#define PD_BASE_ADDR   0x00800000

// kernel stack is 4 MiB starting from 0x00cf ffff
#define K_STACK_TOP    0x00cfffff
#define K_STACK_SIZE   4 * 1024 * 1024


#endif