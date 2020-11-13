#include <sys/tss.h>
#include <string.h>
#include <common/types.h>
#include <sys/gdt.h>

// tss_entry_t *tss = (tss_entry_t*)TSS_BASE_ADDR;
tss_entry_t tss;

// void setTssEntry0() {

// }

void init_tss(tss_entry_t *tss_pointer) {
    memset(tss_pointer, 0, sizeof(tss_entry_t));
    // tss.ss0 = SELECTOR_KDATA;
    // tss.esp0 = K_STACK_TOP;
}