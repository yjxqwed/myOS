#include <sys/idt.h>
#include <arch/x86.h>
#include <sys/isr.h>
#include <string.h>
#include <common/debug.h>



// Gate* _idt = (Gate*)IDT_BASE_ADDR;
Gate _idt[IDT_SIZE];
idt_ptr_t _ip;

void setInterruptDescriptor(
    Gate* gate, uint32_t offset, uint16_t selector, uint8_t attr
) {
    gate->offset_1 = offset & 0xffff;
    gate->offset_2 = (offset >> 16) & 0xffff;
    gate->selector = selector;
    gate->zero = 0x00;
    gate->type_attr = attr;
}

extern void flushIDT();

void setInterruptDescriptorTable() {
    // clear all gates
    memset(_idt, 0, IDT_SIZE * sizeof(Gate));

    // Add interrupt descriptor here
    setISRs();
    // Add interrupt descriptor above

    _ip.base_ = (uint32_t)_idt;
    _ip.size_ = IDT_SIZE * sizeof(Gate) - 1;
    flushIDT();
}