#include "system.h"

uint8_t* memset(uint8_t* mem, uint8_t val, uint32_t size) {
    if (mem == NULL) {
        return NULL;
    }
    // TODO: what if mem + size > 4GiB
    for (int i = 0; i < size; i++) {
        mem[i] = val;
    }
    return mem;
}