#ifndef __TSS_H__
#define __TSS_H__

#include <common/types.h>

// TSS is used for x86 hardware multitasking. Since 
// we will use software multitasking, it is unimportant.
struct TaskStateSegment {
    /* pervious TSS (unused) */
    uint32_t prev_tss;
    /* stack pointer for kernel mode */
    uint32_t esp0;
    /* stack segment for kernel mode */
    uint32_t ss0;

    /* everything below here is unusued now.. */
    uint32_t esp1;
    uint32_t ss1;
    uint32_t esp2;
    uint32_t ss2;
    uint32_t cr3;
    uint32_t eip;
    uint32_t eflags;
    uint32_t eax;
    uint32_t ecx;
    uint32_t edx;
    uint32_t ebx;
    uint32_t esp;
    uint32_t ebp;
    uint32_t esi;
    uint32_t edi;
    uint32_t es;
    uint32_t cs;
    uint32_t ss;
    uint32_t ds;
    uint32_t fs;
    uint32_t gs;
    uint32_t ldt;
    uint16_t trap;
    uint16_t iomap_base;
} __attribute__((packed));

typedef struct TaskStateSegment tss_t;

// void setTssEntry0();

void init_tss(tss_t *tss_pointer);

#endif
