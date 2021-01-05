#include <thread/process.h>
#include <arch/x86.h>
#include <mm/pmem.h>
#include <mm/kvmm.h>
#include <kprintf.h>
#include <common/debug.h>
#include <sys/gdt.h>


static void start_process(void *filename) {
    void *func = filename;
    uint32_t ss = SELECTOR_USTK;

    uint32_t esp = 0;
    ppage_t *p = pages_alloc(1, GFP_ZERO);
    ASSERT(p != NULL);
    task_t *curr = get_current_thread();
    int ret = page_map(
        curr->vmm->pgdir, KERNEL_BASE - 2 * PAGE_SIZE,
        p, PTE_USER | PTE_WRITABLE
    );
    esp = KERNEL_BASE - PAGE_SIZE;
    uint32_t eflags = EFLAGS_MSB(1) | EFLAGS_IF(1);
    uint32_t cs = SELECTOR_UCODE;
    uint32_t eip = (uint32_t)func;
    __asm_volatile(
        "push %0\n\t"
        "push %1\n\t"
        "push %2\n\t"
        "push %3\n\t"
        "push %4\n\t"
        "mov ds, %5\n\t"
        "mov es, %5\n\t"
        "mov fs, %5\n\t"
        "mov gs, %5\n\t"
        "iretd"
        :
        : "r"(ss),
          "r"(esp),
          "r"(eflags),
          "r"(cs),
          "r"(eip),
          "r"(SELECTOR_UDATA)
        :
    );
}

task_t *process_execute(char *filename, char *name, int tty_no) {
    task_t *t = task_create(name, 31, start_process, filename);
    if (t == NULL) {
        return NULL;
    }
    t->vmm = kmalloc(sizeof(vmm_t));
    if (t->vmm == NULL || !init_vmm_struct(t->vmm)) {
        k_free_pages(t, 1);
        return NULL;
    }
    init_vmm_struct(t->vmm);
    INT_STATUS old_status = disable_int();
    ASSERT(t->status == TASK_READY);
    task_push_back_ready(t);
    task_push_back_all(t);
    task_assign_tty(t, tty_no);
    set_int_status(old_status);
    return t;
}
