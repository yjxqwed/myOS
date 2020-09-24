#include "idt.h"
#include "isr.h"
#include "screen.h"

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

#define SETINTDES(x) setInterruptDescriptor(&(_idt[x]), (uint32_t)isr##x, 0x10, 0x8e);

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
}

char* execption_msgs[] = {
    "0: Divide By Zero Exception",
    "1: Debug Execption",
    "2: Non Maskable Interrupt Execption",
    "3: Breakpoint Execption",
    "4: Into Detected Overflow Exception",
    "5:  Out of Bounds Exception",
    "6:  Invalid Opcode Exception",
    "7:  No Coprocessor Exception",
    "8:  Double Fault Exception",
    "9:  Coprocessor Segment Overrun Exception",
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

void interrupt_handler(isrp_t* p) {
    printf("SYSTEM HALT.");
    while(1);
}