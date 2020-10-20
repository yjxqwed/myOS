#include <sys/tss.h>
#include <sys/global.h>

tss_entry_t *tss = (tss_entry_t*)TSS0_BASE_ADDR;

void setTssEntry0() {

}

void init_tss() {
    tss[0].ss0 = 0x18;
    tss[0].esp0 = 0x01280800;
}