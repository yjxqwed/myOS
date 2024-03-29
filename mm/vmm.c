#include <mm/vmm.h>
#include <mm/pmem.h>
#include <mm/kvmm.h>
#include <common/types.h>
#include <common/utils.h>
#include <myos.h>

#include <lib/string.h>
#include <lib/kprintf.h>

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
        k_free_pages(pd, 1);
    }
}

int vmm_map_pages(pde_t *pd, uint32_t va, size_t np, uint32_t perm) {
    int ret = 0;

    // va must be page aligned
    ASSERT(__page_aligned(va));

    for (int i = 0; i < np; i++) {
        ppage_t *p = pages_alloc(1, GFP_ZERO);
        // assume enough memory
        ASSERT(p != NULL);
        int ok = page_map(pd, va, p, perm);
        // assume successful map
        ASSERT(ok == 0);
        va += PAGE_SIZE;
    }

    return ret;
}


int init_vmm_struct(vmm_t *vmm, int argc, char * const argv[]) {
    // ASSERT(vmm != NULL);

    int ret = -1;
    if (vmm == NULL) {
        // bad arg
    } else {
        // mutex_init(&(vmm->vmm_mutex));
        ppage_t *p = NULL;
        if ((vmm->pgdir = create_pd()) == NULL) {
            // no mem to create the page directory
        } else if (
            (p = pages_alloc(1, GFP_ZERO)) == NULL ||
            (ret = page_map(vmm->pgdir, USER_STACK_BOTTOM - 1 * PAGE_SIZE, p, PTE_USER | PTE_WRITABLE)) != 0
        ) {
            // failed to alloc the user stack
        }
        else if (
            (p = pages_alloc(1, GFP_ZERO)) == NULL ||
            (ret = page_map(vmm->pgdir, USER_ARGS, p, PTE_USER)) != 0
        ) {
            // failed to alloc the cmd args
        } else {
            cmd_args_t *cmd_args = (cmd_args_t *)page2kva(p);
            cmd_args->argc = argc;
            for (int i = 0; i < argc; i++) {
                strcpy(argv[i], cmd_args->argv_str[i]);
                cmd_args->argv[i] = cmd_args->argv_str[i];
                // console_kprintf(KPL_DEBUG, "%d: %s\n", i, cmd_args->argv_str[i]);
            }
            ret = 0;
        }
    }

    if (ret == 0) {
        vmm->heap_bot = USER_HEAP_BOTTOM;
        vmm->heap_top = USER_HEAP_BOTTOM;
        vmm->real_heap_top = USER_HEAP_BOTTOM;

        vmm->stack_bot = USER_STACK_BOTTOM;
        vmm->stack_top = USER_STACK_BOTTOM + PAGE_SIZE;

        vmm->args_start = USER_ARGS;
        vmm->args_end = vmm->args_start + PAGE_SIZE;
    }
    // else {
    //     page_unmap(vmm->pgdir, USER_STACK_BOTTOM - 1 * PAGE_SIZE);
    //     page_unmap(vmm->pgdir, USER_ARGS);
    //     destroy_pd(vmm->pgdir);
    // }

    return ret;
}

void destroy_vmm_struct(vmm_t *vmm) {
    if (vmm == NULL) {
        return;
    }
    // TODO: delete the address space
    for (int i = __pde_idx(0); i < __pde_idx(KERNEL_BASE); i++) {
        pde_t pde = vmm->pgdir[i];
        if (pde & PDE_PRESENT) {
            pte_t *pt = (pte_t *)__va(__pg_start_addr(pde));
            for (int j = 0; j < NRPTE; j++) {
                pte_t pte = pt[j];
                if (pte & PTE_PRESENT) {
                    uintptr_t page_pa = pte & PG_START_ADDRESS_MASK;
                    ppage_t *p = pa2page(page_pa);
                    page_decref(p);
                }
            }
            ppage_t *ptp = kva2page(pt);
            page_decref(ptp);
        }
    }
    destroy_pd(vmm->pgdir);
}

void *sys_brk(uintptr_t __addr) {
    vmm_t *vmm = get_current_thread()->vmm;
    // all user processes/threads should all have vmm struct
    ASSERT(vmm != NULL);

    if (__addr == NULL) {
        return vmm->heap_top;
    }

    // heap [USER_HEAP_BOTTOM, USER_HEAP_BOTTOM) -> [USER_HEAP_BOTTOM, USER_HEAP_BOTTOM + USER_HEAP_LIMIT)
    if (__addr < USER_HEAP_BOTTOM || __addr > USER_HEAP_BOTTOM + USER_HEAP_LIMIT) {
        return NULL;
    }

    uint32_t new_real_top = ROUND_UP_DIV(__addr, PAGE_SIZE) * PAGE_SIZE;

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

    return retval;
}
