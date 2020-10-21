#include <sys/tss.h>
#include <sys/global.h>
#include <string.h>
#include <common/types.h>
#include <sys/gdt.h>

tss_entry_t *tss = (tss_entry_t*)TSS_BASE_ADDR;

// void setTssEntry0() {

// }

void init_tss() {
    memset((uint8_t *)tss, 0, sizeof(tss_entry_t));
    tss[0].ss0 = SELECTOR_KDATA;
    tss[0].esp0 = K_STACK_TOP;
}