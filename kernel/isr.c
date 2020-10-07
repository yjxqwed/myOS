#include "idt.h"
#include "isr.h"
#include "screen.h"
#include "system.h"
#include "debug.h"
#include "kb.h"
#include "string.h"

// cpu exception
extern void isr0();
extern void isr1();
extern void isr2();
extern void isr3();
extern void isr4();
extern void isr5();
extern void isr6();
extern void isr7();
extern void isr8();
extern void isr9();
extern void isr10();
extern void isr11();
extern void isr12();
extern void isr13();
extern void isr14();
extern void isr15();
extern void isr16();
extern void isr17();
extern void isr18();
extern void isr19();
extern void isr20();
extern void isr21();
extern void isr22();
extern void isr23();
extern void isr24();
extern void isr25();
extern void isr26();
extern void isr27();
extern void isr28();
extern void isr29();
extern void isr30();
extern void isr31();

// irq 0-15
extern void isr32();
extern void isr33();
extern void isr34();
extern void isr35();
extern void isr36();
extern void isr37();
extern void isr38();
extern void isr39();
extern void isr40();
extern void isr41();
extern void isr42();
extern void isr43();
extern void isr44();
extern void isr45();
extern void isr46();
extern void isr47();

#define SETINTDES(x) setInterruptDescriptor(&(_idt[x]), (uint32_t)isr##x, 0x10, 0x8e);

/* Normally, IRQs 0 to 7 are mapped to entries 8 to 15. This
*  is a problem in protected mode, because IDT entry 8 is a
*  Double Fault! Without remapping, every time IRQ0 fires,
*  you get a Double Fault Exception, which is NOT actually
*  what's happening. We send commands to the Programmable
*  Interrupt Controller (PICs - also called the 8259's) in
*  order to make IRQ0 to 15 be remapped to IDT entries 32 to
*  47 */
void irq_remap(void) {
    outportb(PIC_M_CTL, 0x11);
    outportb(PIC_S_CTL, 0x11);
    outportb(PIC_M_CTLMASK, 0x20);
    outportb(PIC_S_CTLMASK, 0x28);
    outportb(PIC_M_CTLMASK, 0x04);
    outportb(PIC_S_CTLMASK, 0x02);
    outportb(PIC_M_CTLMASK, 0x01);
    outportb(PIC_S_CTLMASK, 0x01);
    outportb(PIC_M_CTLMASK, 0x0);
    outportb(PIC_S_CTLMASK, 0x0);

    // disable all interrupts
    outportb(PIC_M_CTLMASK, 0b11111111);
}

extern Gate* _idt;
void setISRs() {
    SETINTDES(0);
    SETINTDES(1);
    SETINTDES(2);
    SETINTDES(3);
    SETINTDES(4);
    SETINTDES(5);
    SETINTDES(6);
    SETINTDES(7);
    SETINTDES(8);
    SETINTDES(9);
    SETINTDES(10);
    SETINTDES(11);
    SETINTDES(12);
    SETINTDES(13);
    SETINTDES(14);
    SETINTDES(15);
    SETINTDES(16);
    SETINTDES(17);
    SETINTDES(18);
    SETINTDES(19);
    SETINTDES(20);
    SETINTDES(21);
    SETINTDES(22);
    SETINTDES(23);
    SETINTDES(24);
    SETINTDES(25);
    SETINTDES(26);
    SETINTDES(27);
    SETINTDES(28);
    SETINTDES(29);
    SETINTDES(30);
    SETINTDES(31);

    // set irqs
    irq_remap();
    SETINTDES(32);
    SETINTDES(33);
    SETINTDES(34);
    SETINTDES(35);
    SETINTDES(36);
    SETINTDES(37);
    SETINTDES(38);
    SETINTDES(39);
    SETINTDES(40);
    SETINTDES(41);
    SETINTDES(42);
    SETINTDES(43);
    SETINTDES(44);
    SETINTDES(45);
    SETINTDES(46);
    SETINTDES(47);
}

char* cpu_execption_msgs[] = {
    "0: Divide By Zero Exception",
    "1: Debug Execption",
    "2: Non Maskable Interrupt Execption",
    "3: Breakpoint Execption",
    "4: Into Detected Overflow Exception",
    "5: Out of Bounds Exception",
    "6: Invalid Opcode Exception",
    "7: No Coprocessor Exception",
    "8: Double Fault Exception",
    "9: Coprocessor Segment Overrun Exception",
    "10: Bad TSS Exception",
    "11: Segment Not Present Exception",
    "12: Stack Fault Exception",
    "13: General Protection Fault Exception",
    "14: Page Fault Exception",
    "15: Unknown Interrupt Exception",
    "16: Coprocessor Fault Exception",
    "17: Alignment Check Exception (486+)",
    "18: Machine Check Exception (Pentium/586+)",
    "19: Reserved",
    "20: Reserved",
    "21: Reserved",
    "22: Reserved",
    "23: Reserved",
    "24: Reserved",
    "25: Reserved",
    "26: Reserved",
    "27: Reserved",
    "28: Reserved",
    "29: Reserved",
    "30: Reserved",
    "31: Reserved",
};

void cpu_exception_handler(isrp_t *p) {
    uint32_t err_code = p->err_code;
    uint32_t int_no = p->int_no;
    printf(cpu_execption_msgs[int_no]);
    // printf(" eip=");
    // char out[UINT32LEN];
    // printf(uitosh(p->eip, out));
    // printf(" errco=");
    // printf(uitosh(p->err_code, out));
    printISRParam(p);
    printf(" System Halted.\n");
    while(1);
}

void interrupt_request_handler(isrp_t *p) {
    uint32_t irq_no = p->int_no - 32;
    // printISRParam(p);
    // printf("IRQ recvd!");
    switch(irq_no) {
    case 0: {
        // debugMagicBreakpoint();
        // static uint32_t count = 0;
        // count++;
        // if (count % 10000000 == 0) {
        //     printf("0");
        //     count = 0;
        // }
        // printf("0");
        break;
    } case 1: {
        // debugMagicBreakpoint();
        // uint8_t scancode = inportb(0x60);
        // printf("1");
        kb_handler(p);
        break;
    } default: {
        printf("default");
    }
    }
    // if (irq_no >= 8) {
    //     outportb(0xA0, 0x20);
    // }
    outportb(0x20, 0x20);
}

void interrupt_handler(isrp_t *p) {
    // printISRParam(p);
    uint32_t err_code = p->err_code;
    uint32_t int_no = p->int_no;
    if (int_no <= 31) {
        cpu_exception_handler(p);
    } else if (int_no <= 47) {
        interrupt_request_handler(p);
    } else {
        printf("Unknown interrupt!\n");
    }
}

