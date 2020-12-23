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
    vmm->pgdir = create_pde();
    if (vmm->pgdir == NULL) {
        return False;
    }
    vmm->heap_bot = USER_HEAP_BOTTOM;
    vmm->heap_top = USER_HEAP_BOTTOM;
}