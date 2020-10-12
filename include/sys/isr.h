// isr.h/.c are for the (i)nterrupt (s)ervice (r)outines

#ifndef __ISR_H__
#define __ISR_H__

#include <common/types.h>

// 8259A chip ports. More info: https://wiki.osdev.org/8259_PIC
#define PIC_M_CTL     0x20
#define PIC_M_CTLMASK 0x21
#define PIC_S_CTL     0xA0
#define PIC_S_CTLMASK 0xA1

// CPU exceptions. More info: https://wiki.osdev.org/Exceptions
#define DIVBYZERO 0x0
#define DEBUG 0x1
#define NMASKABLEINT 0x2
#define BREAKPOINT 0x3
#define OVERFLOW 0x4
#define BDRANGEXCEEDED 0x5
#define INVALIDOP 0x6

struct InterruptServiceRoutineParam {
    uint32_t gs, fs, es, ds;      /* pushed the segs last */
    uint32_t edi, esi, ebp, kernel_esp, ebx, edx, ecx, eax;  /* pushed by 'pushad' */
    uint32_t int_no, err_code;    /* our 'push byte #' and ecodes do this */
    uint32_t eip, cs, eflags, user_esp, ss;   /* pushed by the processor automatically */ 
};

void setISRs();

typedef struct InterruptServiceRoutineParam isrp_t;
#endif