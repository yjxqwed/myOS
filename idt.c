#include "idt.h"
#include "system.h"
#include "isr.h"



Gate* _idt = (Gate*)0x01180000;
idt_ptr_t _ip;

void setInterruptDescriptor(
    Gate* gate, uint32_t offset, uint16_t selector, uint8_t attr
) {
    gate->offset_1 = offset & 0xffff;
    gate->offset_2 = (offset >> 4) & 0xffff;
    gate->selector = selector;
    gate->zero = 0x00;
    gate->type_attr = attr;
}

extern void flushIDT();

void setInterruptDescriptorTable() {
    // clear all gates
    memset((uint8_t*)_idt, 0, 256 * sizeof(Gate));
    
    // Add interrupt descriptor here

    setISRs();


    // Add interrupt descriptor above

    _ip.base_ = (uint32_t)_idt;
    _ip.size_ = 256 * sizeof(Gate) - 1;
    flushIDT();
}