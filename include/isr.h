// isr.h/.c are for the (i)nterrupt (s)ervice (r)outines

#ifndef __ISR_H__
#define __ISR_H__

#include "types.h"

struct InterruptServiceRoutineParam {
    uint32_t gs, fs, es, ds;      /* pushed the segs last */
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;  /* pushed by 'pusha' */
    uint32_t int_no, err_code;    /* our 'push byte #' and ecodes do this */
    uint32_t eip, cs, eflags, useresp, ss;   /* pushed by the processor automatically */ 
};

void setISRs();

typedef struct InterruptServiceRoutineParam isrp_t;
#endif