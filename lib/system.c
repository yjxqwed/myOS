#include "system.h"

uint8_t* memset(uint8_t* mem, uint8_t val, uint32_t size) {
    if (mem == NULL) {
        return NULL;
    }
    // TODO: what if mem + size > 4GiB
    for (int i = 0; i < size; i++) {
        mem[i] = val;
    }
    return mem;
}

uint8_t inportb(uint16_t port) {
    uint8_t val;
    __asm__ volatile (
        "inb %0, %1"
        : "=a"(val)   // output
        : "Nd"(port)  // input
        :             // clobbered regs
    );
    return val;
}

void outportb(uint16_t port, uint8_t val) {
    __asm__ volatile (
        "outb %1, %0"
        :             // output
        : "a"(val), 
          "Nd"(port)  // input
        :             // clobbered regs
    );
}
