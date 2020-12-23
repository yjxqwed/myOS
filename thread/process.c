#include <thread/process.h>
#include <arch/x86.h>
#include <mm/pmem.h>
#include <mm/kvmm.h>
#include <kprintf.h>
#include <common/debug.h>
#include <sys/gdt.h>


static pde_t *create_pde() {
    pde_t *pd = k_get_free_pages(1, GFP_ZERO);
    if (pd == NULL) {
        return NULL;
    }
    page_dir_init(pd);
    return pd;
}

static void start_process(void *filename) {
    void *func = filename;
    uint32_t ss = SELECTOR_USTK;

    uint32_t esp = 0;
    ppage_t *p = pages_alloc(1, GFP_ZERO);
    ASSERT(p != NULL);
    task_t *curr = get_current_thread();
    int ret = page_map(
        curr->pg_dir, KERNEL_BASE - 2 * PAGE_SIZE, p, PTE_USER | PTE_WRITABLE
    );
    ASSERT(ret == ERR_NO_ERR);
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

task_t *process_execute(char *filename, char *name) {
    task_t *t = task_create(name, 31, start_process, filename);
    if (t == NULL) {
        return NULL;
    }
    // t->pg_dir = create_pde();
    // if (t->pg_dir == NULL) {
    //     k_free_pages(t, 1);
    //     return NULL;
    // }
    t->vmm = kmalloc(sizeof(vmm_t));
    if (t->vmm == NULL) {
        k_free_pages(t, 1);
        return NULL;
    }
    INT_STATUS old_status = disable_int();
    ASSERT(t->status == TASK_READY);
    task_push_back_ready(t);
    task_push_back_all(t);
    set_int_status(old_status);
}
