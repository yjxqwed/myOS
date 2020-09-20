#include "gdt.h"
#include "types.h"
// #include "utils.h"

GlobalDescriptorTablePointer gp;
extern "C" void flushGDT();
uint16_t dss;  // data segment selector
uint16_t css;  // code segment selector


SegmentDescriptor::SegmentDescriptor(
    uint32_t base, uint32_t limit, uint8_t flags) {

    uint8_t* target = (uint8_t*)this;

    if (limit <= 65536) {  // 16-bit
        target[6] = 0x40;  // GR = 0 (1B granularity)
    } else {  // 32-bit
        target[6] = 0xc0;  // GR = 1 (4KiB granularity)
        if (limit & 0xfff != 0xfff) {
            limit = (limit >> 12) - 1;
        } else {
            limit = (limit >> 12);
        }
    }

    // put limit
    target[0] = limit & 0xff;
    target[1] = (limit >> 8) & 0xff;
    target[6] |= (limit >> 16) & 0xf;

    // put base
    target[2] = base & 0xff;
    target[3] = (base >> 8) & 0xff;
    target[4] = (base >> 16) & 0xff;
    target[7] = (base >> 24) & 0xff;

    // put flags
    target[5] = flags;
}

// Get base from the segment descriptor
uint32_t SegmentDescriptor::Base() {
    uint8_t* target = (uint8_t*)this;
    uint32_t base = target[7];
    base = (base << 8) + target[4];
    base = (base << 8) + target[3];
    base = (base << 8) + target[2];
    return base;
}

// Get base from the segment descriptor
uint32_t SegmentDescriptor::Limit() {
    uint8_t* target = (uint8_t*)this;
    uint32_t limit = target[6] & 0xf;
    limit = (limit << 8) + target[1];
    limit = (limit << 8) + target[0];
    if (target[6] & 0xc0 == 0xc0) {  // GR is 1 => 32-bit, shift needed
        limit = (limit << 12) | 0xfff;
    }
    return limit;
}

const uint32_t _64MiB = 64 * 1024 * 1024;

GlobalDescriptorTable::GlobalDescriptorTable():
    null_segment_descriptor_(0, 0, 0), 
    unused_segment_descriptor_(0, 0, 0),
    code_segment_descriptor_(0, _64MiB, 0x9a),
    data_segment_descriptor_(0, _64MiB, 0x92),
    task_state_segment_descriptor_(0, 0, 0) {

    asm("xchg %bx, %bx");
    // uint32_t i[2];
    // i[1] = (uint32_t)this;  // offset
    // i[0] = sizeof(GlobalDescriptorTable) << 16;  // size
    // asm volatile("lgdt (%0)": :"p" (((uint8_t *) i)+2));
    gp.base_ = (uint32_t)this;
    gp.size_ = sizeof(GlobalDescriptorTable) - 1;
    dss = this->DataSegmentSelector();
    css = this->CodeSegmentSelector();
    flushGDT();
}

// Because they are kernel's segments, the selectors are like offsets.
// Refer to gdt.h for more info about segment selector.

uint16_t GlobalDescriptorTable::CodeSegmentSelector() {
    return (uint16_t)((uint8_t*)&code_segment_descriptor_ - (uint8_t*)this);
}

uint16_t GlobalDescriptorTable::DataSegmentSelector() {
    return (uint16_t)((uint8_t*)&data_segment_descriptor_ - (uint8_t*)this);
}