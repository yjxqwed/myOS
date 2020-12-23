#include <sys/syscall.h>
#include <sys/isr.h>
#include <kprintf.h>
#include <device/screen.h>
#include <thread/thread.h>
#include <common/types.h>

static void sys_write(const char *str);
static void *sys_sbrk(intptr_t increement);
static int sys_brk(uintptr_t addr);

static void syscall_handler(isrp_t *p) {
    if (p->eax == SYSCALL_WRITE) {
        kprintf(KPL_DEBUG, "write, str=0x%X\n", p->ebx);
        sys_write((const char *)p->ebx);
    }
}

void syscall_init() {
    register_handler(0x80, syscall_handler);
}

static void sys_write(const char *str) {
    scrn_puts_safe(str, BLACK, WHITE);
}