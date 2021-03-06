#ifndef __IDT_H__
#define __IDT_H__

#include <common/types.h>

#define IDT_SIZE 256

// IDT entry. More info: https://wiki.osdev.org/IDT
struct InterruptDescriptor {
   uint16_t offset_1; // offset bits 0..15
   uint16_t selector; // a code segment selector in GDT or LDT
   uint8_t zero;      // unused, set to 0
   uint8_t type_attr; // type and attributes, see below
   uint16_t offset_2; // offset bits 16..31
} __attribute__((packed));

// type and attrs
//   7                           0
// +---+---+---+---+---+---+---+---+
// | P |  DPL  | S |    GateType   |
// +---+---+---+---+---+---+---+---+

#define GATE_DPL_0  0x00
#define GATE_DPL_3  0x60
#define GATE_P_1    0x80
// 80386 32-bit interrupt gate
#define GATE_INT_32  0xe
// 80386 32-bit trap gate
#define GATE_TRAP_32 0xf

typedef struct InterruptDescriptor Gate;

struct InterruptDescriptorTablePointer {
    uint16_t size_;
    uint32_t base_;
} __attribute__((packed));

typedef struct InterruptDescriptorTablePointer idt_ptr_t;

void setInterruptDescriptorTable();
void setInterruptDescriptor(
    Gate* gate, uint32_t offset, uint16_t selector, uint8_t attr
);
#endif
