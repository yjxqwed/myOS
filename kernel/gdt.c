#include "gdt.h"
#include "tss.h"

seg_des_t* _gdt = (seg_des_t*)0x01100000;
gdt_ptr_t _gp;
uint16_t _dss, _css;


void setSegmentDescriptor(
    seg_des_t* sd, uint32_t base, uint32_t limit,
    uint8_t flags,  // high 4 bits of the 6th byte
    uint8_t access
) {
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
    target[6] = ((limit >> 16) & 0xf) | (flags << 4);
}

extern void flushGDT();

extern tss_entry_t *tss_entry_0;

void setGlobalDescriptorTable() {
    setSegmentDescriptor(&(_gdt[0]), 0, 0, 0xc, 0);           // null         0x00
    setSegmentDescriptor(&(_gdt[1]), 0, 0, 0xc, 0);           // unused       0x08
    setSegmentDescriptor(&(_gdt[2]), 0, 0xfffff, 0xc ,0x9a);  // kernel code  0x10
    setSegmentDescriptor(&(_gdt[3]), 0, 0xfffff, 0xc, 0x92);  // kernel data  0x18
    setSegmentDescriptor(&(_gdt[4]), 0, 0xfffff, 0xc, 0xfa);  // user code    0x23
    setSegmentDescriptor(&(_gdt[5]), 0, 0xfffff, 0xc, 0xf2);  // user data    0x2B
    setSegmentDescriptor(
        &(_gdt[6]),
        (uint32_t)tss_entry_0,
        103,
        0b0100,  // gr = 0 (byte); sz = 1 (32 bit pm)
        0x89
    );  // tss 0x33
    // setSegmentDescriptor(&(_gdt[7]), 0, 0xfffff, 0xc, 0xb2);  // test: dpl 1 data 0x38
    _gp.base_ = (uint32_t)_gdt;
    _gp.size_ = 7 * sizeof(seg_des_t) - 1;
    _dss = 0x18;
    _css = 0x10;
    flushGDT();
}
