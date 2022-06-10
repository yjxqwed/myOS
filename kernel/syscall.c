#include <sys/syscall.h>
#include <sys/isr.h>
#include <kprintf.h>
#include <device/tty.h>
#include <thread/thread.h>
#include <common/types.h>
#include <common/debug.h>
#include <common/utils.h>
#include <thread/thread.h>
#include <mm/pmem.h>
// #include <fs/myfs/fs.h>
#include <fs/simplefs/simplefs.h>

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
        return ((syscall_handler2_t)(handlers[p->eax]))(p->ebx, p->ecx);
    } else {
        return ((syscall_handler3_t)(handlers[p->eax]))(p->ebx, p->ecx, p->edx);
    }
}

void syscall_init() {
    handlers[SYSCALL_WRITE] = sys_write;
    handlers[SYSCALL_READ] = sys_read;
    handlers[SYSCALL_BRK] = sys_brk;
    // handlers[SYSCALL_SBRK] = sys_sbrk;
    handlers[SYSCALL_SLEEP] = sys_sleep;
    register_handler(0x80, syscall_handler);
}

// static int sys_write(const char *str) {
//     tty_puts(get_current_thread()->tty_no, str, CONS_BLACK, CONS_GRAY);
//     return 0;
// }

static void *sys_brk(uintptr_t __addr) {
    vmm_t *vmm = get_current_thread()->vmm;
    // all user processes/threads should all have vmm struct
    ASSERT(vmm != NULL);

    console_kprintf(KPL_DEBUG, "__addr = 0x%X\n", __addr);

    if (__addr == NULL) {
        return vmm->heap_top;
    }

    // heap [USER_HEAP_BOTTOM, USER_HEAP_BOTTOM) -> [USER_HEAP_BOTTOM, USER_HEAP_BOTTOM + USER_HEAP_LIMIT)
    if (__addr < USER_HEAP_BOTTOM || __addr > USER_HEAP_BOTTOM + USER_HEAP_LIMIT) {
        console_kprintf(KPL_DEBUG, "bad __addr, should be [0x%X, 0x%X]\n", USER_HEAP_BOTTOM, USER_HEAP_BOTTOM + USER_HEAP_LIMIT);
        return NULL;
    }

    uint32_t new_real_top = ROUND_UP_DIV(__addr, PAGE_SIZE) * PAGE_SIZE;

    console_kprintf(KPL_DEBUG, "new_real_top = 0x%X\n", new_real_top);

    mutex_lock(vmm->vmm_mutex);

    if (new_real_top < vmm->real_heap_top) {
        // shrink heap
        for (
            uint32_t brk = new_real_top;
            brk < vmm->real_heap_top;
            brk += PAGE_SIZE
        ) {
            page_unmap(vmm->pgdir, brk);
        }
    } else if (new_real_top > vmm->real_heap_top) {
        // expand heap
        // ppage_t *zp = get_zpage();
        for (
            uint32_t brk = vmm->real_heap_top;
            brk < new_real_top;
            brk += PAGE_SIZE
        ) {
            ppage_t *zp = pages_alloc(1, GFP_ZERO);
            // assume enough memory for now
            ASSERT(zp != NULL);
            // if (zp == NULL) {
            //     new_top = brk;
            //     break;
            // }
            page_map(vmm->pgdir, brk, zp, PTE_USER | PTE_WRITABLE);
        }
    }
    vmm->real_heap_top = new_real_top;
    vmm->heap_top = __addr;
    void *retval = vmm->heap_top;
    mutex_unlock(vmm->vmm_mutex);
    console_kprintf(
        KPL_DEBUG, "vmm->real_heap_top = 0x%X, vmm->heap_top = 0x%X, retval = 0x%X\n",
        vmm->real_heap_top, vmm->heap_top, retval
    );
    return retval;
}


static void *sys_sleep(uint32_t ms) {
    thread_msleep(ms);
    return NULL;
}
