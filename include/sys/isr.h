// isr.h/.c are for the (i)nterrupt (s)ervice (r)outines

#ifndef __ISR_H__
#define __ISR_H__

#include <common/types.h>

// CPU exceptions. More info: https://wiki.osdev.org/Exceptions
// #define DIVBYZERO 0x0
// #define DEBUG 0x1
// #define NMASKABLEINT 0x2
// #define BREAKPOINT 0x3
// #define OVERFLOW 0x4
// #define BDRANGEXCEEDED 0x5
// #define INVALIDOP 0x6

#define INT_PIT  0x20
#define INT_KB   0x21
#define INT_ATA0 0x2E
#define INT_ATA1 0x2F

struct InterruptStack {
    // == pushed the seg regs last ==
    uint32_t gs, fs, es, ds;
    // == pushed by 'pushad' ==
    uint32_t edi, esi, ebp;
    // unused, popad will ignore this
    uint32_t esp_dummy;
    uint32_t ebx, edx, ecx, eax;
    // == interrupt number, error code ==
    uint32_t int_no, err_code;
    // == pushed by the processor automatically ==
    uint32_t eip, cs, eflags;
    // these 2 regs will be pushed when a privilege level change happens
    uint32_t esp, ss;
};

void setISRs();

typedef struct InterruptStack isrp_t;
typedef struct InterruptStack istk_t;

typedef void *(*interrupt_handler_t)(isrp_t *p);

// register handler for interrupt
void register_handler(uint32_t int_no, interrupt_handler_t handler);
#endif
