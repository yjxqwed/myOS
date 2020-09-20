#include "gdt.h"

#define _64MiB 64 * 1024 * 1024

seg_des_t _gdt[5];
gdt_ptr_t _gp;
uint16_t _dss, _css;


void setSegmentDescriptor(
    seg_des_t* sd, uint32_t base, uint32_t limit,
    uint8_t flags,  // high 4 bits of the 6th byte
    uint8_t access) {

    uint8_t* target = (uint8_t*)sd;

    // put base
    target[2] = base & 0xff;
    target[3] = (base >> 8) & 0xff;
    target[4] = (base >> 16) & 0xff;
    target[7] = (base >> 24) & 0xff;

    // put flags
    target[5] = access;

    // put limit
    target[0] = limit & 0xff;
    target[1] = (limit >> 8) & 0xff;

    // put high limit and flags
    target[6] = ((limit >> 4) & 0xf) | (flags << 4);
}

extern void flushGDT();

void setGlobalDescriptorTable() {
    setSegmentDescriptor(&(_gdt[0]), 0, 0, 0xc, 0);  // null
    setSegmentDescriptor(&(_gdt[1]), 0, 0, 0xc, 0);  // unused    
    setSegmentDescriptor(&(_gdt[2]), 0, 0xfffff, 0xc ,0x9a);  // code
    setSegmentDescriptor(&(_gdt[3]), 0, 0xfffff, 0xc, 0x92);  // data
    setSegmentDescriptor(&(_gdt[4]), 0, 0, 0xc, 0);  // tss
    _gp.base_ = (uint32_t)_gdt;
    _gp.size_ = 5 * sizeof(seg_des_t) - 1;
    _dss = 0x18;
    _css = 0x10;
    flushGDT();
}