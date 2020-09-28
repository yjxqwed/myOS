#include "system.h"

uint8_t inportb(uint16_t port) {
    uint8_t val;
    __asm__ volatile (
        "inb %0, %1"
        "\n\tnop\n\t nop" // introduce some delay
        : "=a"(val)   // output
        : "Nd"(port)  // input
        :             // clobbered regs
    );
    return val;
}

void outportb(uint16_t port, uint8_t val) {
    __asm__ volatile (
        "outb %1, %0"
        "\n\tnop\n\tnop" // introduce some delay
        :             // output
        : "a"(val), 
          "Nd"(port)  // input
        :             // clobbered regs
    );
}
