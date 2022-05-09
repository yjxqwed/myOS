#include <mm/vmm.h>
#include <mm/pmem.h>
#include <mm/kvmm.h>
#include <common/types.h>
#include <myos.h>

/**
 * @brief Create a new page directory
 * 
 * @return pde_t* Pointer to the created PD
 */
static pde_t *create_pd() {
    pde_t *pd = k_get_free_pages(1, GFP_ZERO);
    if (pd == NULL) {
        return NULL;
    }
    page_dir_init(pd);
    return pd;
}

static void destroy_pd(pde_t *pd) {
    if (pd != NULL) {
        k_free_pages(pd, 1);;
    }
    return;
}


bool_t init_vmm_struct(vmm_t *vmm) {
    ASSERT(vmm != NULL);

    // init mutex
    vmm->vmm_mutex = (mutex_t *)kmalloc(sizeof(mutex_t));
    if (vmm->vmm_mutex == NULL) {
        return False;
    }
    mutex_init(vmm->vmm_mutex);

    // init pd
    vmm->pgdir = create_pd();
    if (vmm->pgdir == NULL) {
        kfree(vmm->vmm_mutex);
        return False;
    }

    // alloc one physical page for user stack
    ppage_t *p = pages_alloc(1, GFP_ZERO);
    if (p == NULL) {
        kfree(vmm->vmm_mutex);
        destroy_pd(vmm->pgdir);
        return False;
    }
    int ret = page_map(
        vmm->pgdir, USER_STACK_BOTTOM - 1 * PAGE_SIZE, p, PTE_USER | PTE_WRITABLE
    );

    vmm->heap_bot = USER_HEAP_BOTTOM;
    vmm->heap_top = USER_HEAP_BOTTOM;
    vmm->stack_bot = USER_STACK_BOTTOM;
    vmm->stack_top = USER_STACK_BOTTOM + PAGE_SIZE;
    return True;
}

void destroy_vmm_struct(vmm_t *vmm) {
    if (vmm == NULL) {
        return;
    }
    kfree(vmm->vmm_mutex);
    k_free_pages(vmm->pgdir, 1);
}
