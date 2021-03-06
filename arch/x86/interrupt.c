#include <arch/x86.h>
#include <common/types.h>

static uint32_t get_eflags() {
    uint32_t eflags;
    __asm__ volatile(
        "pushf\n\t"
        "pop %0"
        : "=a"(eflags)  // output
        :               // input
        :               // clobbered regs
    );
    return eflags;
}

INT_STATUS enable_int() {
    INT_STATUS old_status = get_int_status();
    if (old_status == INTERRUPT_OFF) {
        __asm__ volatile("sti" : : : "cc");
    }
    return old_status;
}

INT_STATUS disable_int() {
    INT_STATUS old_status = get_int_status();
    if (old_status == INTERRUPT_ON) {
        __asm__ volatile("cli" : : : "cc");
    }
    return old_status;
}

INT_STATUS get_int_status() {
    return (get_eflags() & EFLAGS_IF_MASK) ? INTERRUPT_ON : INTERRUPT_OFF;
}

void set_int_status(INT_STATUS status) {
    if (status == INTERRUPT_OFF) {
        disable_int();
    } else {
        enable_int();
    }
}
