#include <sys/syscall.h>
#include <sys/isr.h>
#include <kprintf.h>
#include <device/screen.h>
#include <thread/thread.h>
#include <common/types.h>
#include <common/debug.h>
#include <thread/thread.h>

static int sys_write(const char *str);
static void *sys_sbrk(intptr_t __delta);
static int sys_brk(uintptr_t __addr);
static void *sys_sleep(uint32_t ms);

typedef void * (*syscall_handler0_t)(void);
typedef void * (*syscall_handler1_t)(void *arg1);
typedef void * (*syscall_handler2_t)(void *arg1, void *arg2);
typedef void * (*syscall_handler3_t)(void *arg1, void *arg2, void *arg3);

static void *handlers[NUM_SYSCALL];

static void *syscall_handler(isrp_t *p) {
    if (p->eax < SYSCALL_ARG1) {
        return ((syscall_handler0_t)(handlers[p->eax]))();
    } else if (p->eax < SYSCALL_ARG2) {
        return ((syscall_handler1_t)(handlers[p->eax]))(p->ebx);
    } else if (p->eax < SYSCALL_ARG3) {
        return ((syscall_handler2_t)(handlers[p->eax]))(p->ebp, p->ecx);
    } else {
        return ((syscall_handler3_t)(handlers[p->eax]))(p->ebp, p->ecx, p->edx);
    }
}

void syscall_init() {
    handlers[SYSCALL_WRITE] = sys_write;
    handlers[SYSCALL_SBRK] = sys_sbrk;
    handlers[SYSCALL_SLEEP] = sys_sleep;
    register_handler(0x80, syscall_handler);
}

static int sys_write(const char *str) {
    scrn_puts_safe(str, BLACK, WHITE);
    return 0;
}

static void *sys_sbrk(intptr_t __delta) {
    vmm_t *vmm = get_current_thread()->vmm;
    if (vmm == NULL) {
        return -1;
    }
    if (__delta == 0) {
        return vmm->heap_top;
    } else {
        return (void *)-1;
    }
}

static void *sys_sleep(uint32_t ms) {
    thread_msleep(ms);
    return NULL;
}