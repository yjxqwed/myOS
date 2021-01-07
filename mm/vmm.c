#include <mm/vmm.h>
#include <mm/pmem.h>
#include <mm/kvmm.h>
#include <common/types.h>
#include <myos.h>

static pde_t *create_pde() {
    pde_t *pd = k_get_free_pages(1, GFP_ZERO);
    if (pd == NULL) {
        return NULL;
    }
    page_dir_init(pd);
    return pd;
}


bool_t init_vmm_struct(vmm_t *vmm) {
    vmm->vmm_mutex = (mutex_t *)kmalloc(sizeof(mutex_t));
    if (vmm->vmm_mutex == NULL) {
        return False;
    }
    mutex_init(vmm->vmm_mutex);
    vmm->pgdir = create_pde();
    if (vmm->pgdir == NULL) {
        kfree(vmm->vmm_mutex);
        return False;
    }
    vmm->heap_bot = USER_HEAP_BOTTOM;
    vmm->heap_top = USER_HEAP_BOTTOM;
}

void destroy_vmm_struct(vmm_t *vmm) {
    if (vmm == NULL) {
        return;
    }
    kfree(vmm->vmm_mutex);
    k_free_pages(vmm->pgdir, 1);
}
