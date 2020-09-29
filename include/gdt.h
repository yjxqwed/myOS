// Global Descriptor Table
#ifndef __GDT_H__
#define __GDT_H__

#include "types.h"

// SegmentDestriptor is the entry of GDT
// 8 bytes each
// More info: https://wiki.osdev.org/Global_Descriptor_Table
struct SegmentDescriptor {
    // base - 32bits; limits - 20bits
    uint16_t limit_lo_;  // limit 0:15
    uint16_t base_lo_;   // base 0:15
    uint8_t base_mid_;   // base 16:23
    uint8_t access_;
    // 7: Present bit
    // 5-6:Privilege bits
    // 4: Descriptor type
    // 3: Executabe bit
    // 2: Direction bit/Conforming bit
    // 1: Readable bit/Writable bit
    // 0: Accessed bit
    uint8_t flags_limit_hi_;
    // 7: Granularity bit
    // 6: Size bit
    // 4-5: 0
    // 0-3: limit 16:19
    uint8_t base_hi_;    // base 24:31
} __attribute__((packed));

// Segment Descriptor Layout (64 bits)
// |   7    |     6       |     5     |   4    |   3    |   2    |   1    |   0    |  字节
// |76543210|7 6 5 4 3210 |7 65 4 3210|76543210|76543210|76543210|76543210|76543210|  比特
// |--------|-|-|-|-|---- |-|--|-|----|--------|--------|--------|--------|--------|  占位
// |  BASE  |G|D|0|A|LIMIT|P|D |S|TYPE|<------- BASE 23-0 ------>|<-- LIMIT 15-0 ->|  含义
// |  31-24 | |/| |V|19-16| |P |
//            |B| |L|     | |L |

// Segment Selector Layout (16 bits)
// |   1    |     0    |  字节
// |76543210|76543|2|10|  比特
// |--------------|-|--|  占位
// |    INDEX     |T|R |  含义
// |              |I|P |
// |              | |L |
// TI = 0 GDT; TI = 1 LDT
// RPL: Ring Privilege

// Privileges
// DPL privilege in segment/gate descriptors
// CPL privilege in cs/ss registers
// RPL privilege in descriptor selectors

struct GlobalDescriptorTablePointer {
    uint16_t size_;
    uint32_t base_;
} __attribute__((packed));

typedef struct SegmentDescriptor seg_des_t;
typedef struct GlobalDescriptorTablePointer gdt_ptr_t;

void setGlobalDescriptorTable();

#endif