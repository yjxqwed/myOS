// #include "tss.h"
#include <sys/tss.h>

tss_entry_t *tss_entry_0 = (tss_entry_t*)0x01281000;

void setTssEntry0() {
    tss_entry_0->ss0 = 0x18;
    tss_entry_0->esp0 = 0x01280800;
    // tss_entry_0->ds = 0x18;
    // tss_entry_0->es = 0x18;
    // tss_entry_0->fs = 0x18;
    // tss_entry_0->gs = 0x18;
    // tss_entry_0->esp0 = 0x01287654;
}
