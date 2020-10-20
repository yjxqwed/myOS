#ifndef __GLOBAL_H__
#define __GLOBAL_H__

// file for some global macros

// page directory base address
#define PD_BASE_ADDR   0x01100000
// global descriptor table base address
#define GDT_BASE_ADDR  0x01101000
// interrupt descriptor table base address
#define IDT_BASE_ADDR  0x01101200
// tss0 base address (task state segment base address)
#define TSS0_BASE_ADDR 0x0


#endif