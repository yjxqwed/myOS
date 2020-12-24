#include <sys/syscall.h>
#include <sys/isr.h>
#include <kprintf.h>
#include <device/screen.h>
#include <thread/thread.h>
#include <common/types.h>
#include <common/debug.h>
#include <common/utils.h>
#include <thread/thread.h>
#include <mm/pmem.h>

static int sys_write(const char *str);
// static void *sys_sbrk(intptr_t __delta);
static void *sys_brk(uintptr_t __addr);
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
    handlers[SYSCALL_BRK] = sys_brk;
    // handlers[SYSCALL_SBRK] = sys_sbrk;
    handlers[SYSCALL_SLEEP] = sys_sleep;
    register_handler(0x80, syscall_handler);
}

static int sys_write(const char *str) {
    scrn_puts_safe(str, BLACK, WHITE);
    return 0;
}

static void *sys_brk(uintptr_t __addr) {
    vmm_t *vmm = get_current_thread()->vmm;
    // TODO: need to dev a lock to protect vmm
    ASSERT(vmm != NULL);
    void *retval = NULL;
    if (vmm == NULL) {
        retval = SYSCALL_FAIL;
    } else {
        uintptr_t new_top = ROUND_UP_DIV(__addr, PAGE_SIZE) * PAGE_SIZE;
        if (
            new_top >= vmm->heap_bot &&
            new_top <= (vmm->heap_bot + USER_HEAP_LIMIT)
        ) {
            // only handle good request
            if (new_top < vmm->heap_top) {
                // shrink heap
                for (
                    uint32_t brk = new_top;
                    brk < vmm->heap_top;
                    brk += PAGE_SIZE
                ) {
                    page_unmap(vmm->pgdir, brk);
                }
            } else if (new_top > vmm->heap_top) {
                // expand heap
                ppage_t *zp = get_zpage();
                for (
                    uint32_t brk = vmm->heap_top;
                    brk < new_top;
                    brk += PAGE_SIZE
                ) {
                    page_map(vmm->pgdir, brk, zp, PTE_USER | PTE_READABLE);
                }
            }
            vmm->heap_top = new_top;
        }
        retval = vmm->heap_top;
    }
    return retval;
}

// static void *sys_sbrk(intptr_t __delta) {
//     vmm_t *vmm = get_current_thread()->vmm;
//     if (vmm == NULL) {
//         return SYSCALL_FAIL;
//     }
//     if (__delta == 0) {
//         return vmm->heap_top;
//     } else {
//         return SYSCALL_FAIL;
//     }
// }

static void *sys_sleep(uint32_t ms) {
    thread_msleep(ms);
    return NULL;
}
