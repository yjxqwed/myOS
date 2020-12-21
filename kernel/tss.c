#include <sys/tss.h>
#include <string.h>
#include <common/types.h>
#include <sys/gdt.h>

tss_t tss;

void init_tss(tss_t *tss_pointer) {
    uint32_t tss_size = sizeof(tss_t);
    memset(tss_pointer, 0, tss_size);
    // no io map
    tss_pointer->ss0 = SELECTOR_KSTK;
    tss_pointer->iomap_base = tss_size;
}


void tss_update_esp0(uint32_t esp0) {
    tss.esp0 = esp0;
}
