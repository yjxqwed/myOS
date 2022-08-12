#include <arch/x86.h>

uint8_t inportb(uint16_t port) {
    uint8_t val;
    __asm_volatile(
        "inb %0, %1\n\t"
        "nop\n\t"
        "nop"         // introduce some delay
        : "=a"(val)   // output
        : "Nd"(port)  // input
        :             // clobbered regs
    );
    return val;
}

void outportb(uint16_t port, uint8_t val) {
    __asm_volatile(
        "outb %1, %0\n\t"
        "nop\n\t"
        "nop"         // introduce some delay
        :             // output
        : "a"(val), "Nd"(port)  // input
        :             // clobbered regs
    );
}

uint32_t inportd(uint16_t port) {
    uint32_t val;
    __asm_volatile (
        "ind %0, %1\n\t"
        "nop\n\t"
        "nop"         // introduce some delay
        : "=a"(val)   // output
        : "Nd"(port)  // input
        :             // clobbered regs
    );
    return val;
}

void outportd(uint16_t port, uint32_t val) {
    __asm_volatile (
        "outd %1, %0\n\t"
        "nop\n\t"
        "nop"         // introduce some delay
        :             // output
        : "a"(val), "Nd"(port)  // input
        :             // clobbered regs
    );
}

void inportsw(
    uint16_t port, void *buf, uint32_t word_cnt
) {
    __asm_volatile(
        "cld;"
        "rep insw;"
        : "+D"(buf), "+c"(word_cnt)
        : "Nd"(port)
        : "memory", "cc"
    );
}

void outportsw(
    uint16_t port, void *buf, uint32_t word_cnt
) {
    __asm_volatile(
        "cld;"
        "rep outsw;"
        : "+S"(buf), "+c"(word_cnt)
        : "Nd"(port)
        : "cc"
    );
}

void inportsd(
    uint16_t port, void *buf, uint32_t dword_cnt
) {
    __asm_volatile(
        "cld;"
        "rep insd;"
        : "+D"(buf), "+c"(dword_cnt)
        : "Nd"(port)
        : "memory", "cc"
    );
}

void outportsd(
    uint16_t port, void *buf, uint32_t dword_cnt
) {
    __asm_volatile(
        "cld;"
        "rep outsw;"
        : "+S"(buf), "+c"(dword_cnt)
        : "Nd"(port)
        : "cc"
    );
}



void lgdt(void *gp) {
    __asm_volatile (
        "lgdt [%0]"
        :
        : "r"(gp)  // input
        :
    );
}

void lidt(void *ip) {
    __asm_volatile (
        "lidt [%0]"
        :
        : "r"(ip)  // input
        :
    );
}

void lcr0(uint32_t x) {
    __asm_volatile (
        "mov cr0, %0"
        :
        : "r"(x)
        :
    );
}

uint32_t scr0() {
    uint32_t cr0;
    __asm_volatile (
        "mov %0, cr0"
        : "=r"(cr0)
        :
        :
    );
    return cr0;
}

void lcr3(uint32_t x) {
    __asm_volatile (
        "mov cr3, %0"
        :
        : "r"(x)
        :
    );
}

uint32_t scr3() {
    uint32_t cr3;
    __asm_volatile (
        "mov %0, cr3"
        : "=r"(cr3)
        :
        :
    );
    return cr3;
}

uint32_t scr2() {
    uint32_t cr2;
    __asm_volatile (
        "mov %0, cr2"
        : "=r"(cr2)
        :
        :
    );
    return cr2;
}

void invlpg(void *va) {
    __asm_volatile (
        "invlpg [%0]"
        :
        : "r"(va)
        : "memory"
    );
}

void ltr(uint16_t selector_tss) {
    __asm_volatile (
        "ltr %0"
        :
        : "r"(selector_tss)
        :
    );
}

void hlt() {
    __asm_volatile (
        "hlt"
        :
        :
        : "memory"
    );
}