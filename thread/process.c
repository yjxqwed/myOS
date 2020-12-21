#include <thread/process.h>
#include <arch/x86.h>
#include <mm/pmem.h>
#include <mm/kvmm.h>


pde_t *create_pde() {
    ppage_t *p = pages_alloc(1, GFP_ZERO);
    if (p == NULL) {
        return NULL;
    }
    
}
