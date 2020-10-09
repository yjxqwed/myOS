#include "tss.h"

tss_entry_t *tss_entry_0 = (tss_entry_t*)0x01281000;

void setTssEntry0() {
    tss_entry_0->ss0 = 0x18;
    tss_entry_0->esp0 = 0x01280800;
}
