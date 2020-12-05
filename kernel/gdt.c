#include <sys/gdt.h>
#include <sys/tss.h>
#include <string.h>
#include <arch/x86.h>

// we give gdt 8 entries
#define GDT_SIZE 8

seg_des_t _gdt[GDT_SIZE];
gdt_ptr_t _gp;


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

extern tss_t tss;

void setGlobalDescriptorTable() {
    // zero out _gdt
    memset(_gdt, 0, GDT_SIZE * sizeof(seg_des_t));
    // kernel code  0x10
    setSegmentDescriptor(&(_gdt[2]), 0, 0xfffff, 0xc ,0x9a);
    // kernel data  0x18
    setSegmentDescriptor(&(_gdt[3]), 0, 0xfffff, 0xc, 0x92);
    // tss          0x20
    init_tss(&tss);
    setSegmentDescriptor(
        &(_gdt[4]), (uint32_t)(&tss), sizeof(tss_t) - 1, 0x04, 0x89
    );
    // user code    0x2B
    setSegmentDescriptor(&(_gdt[5]), 0, 0xfffff, 0xc, 0xfa);
    // user data    0x33
    setSegmentDescriptor(&(_gdt[6]), 0, 0xfffff, 0xc, 0xf2);


    _gp.base_ = (uint32_t)_gdt;
    _gp.size_ = GDT_SIZE * sizeof(seg_des_t) - 1;
    lgdt(&_gp);
    extern void flushGDT();
    flushGDT();
    ltr(SELECTOR_TSS);
}
