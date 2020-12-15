#ifndef __BITMAP_H__
#define __BITMAP_H__

#include <common/types.h>

typedef struct BitMap {
    int first_zero_bit;
    uint32_t num_zero;   // number of zeros in this map
    uint32_t byte_num_;  // number of bytes of this map
    uint8_t *bits_;      // bits
} btmp_t;

// always call this function when init a bitmap
void bitmap_init(btmp_t *btmp, uint32_t byte_len);

// return true if bit <bit_idx> is set
int bitmap_bit_test(btmp_t *btmp, uint32_t bit_idx);

// return the index of the first region continuous bits of length <len>
// -1 if no region
int bitmap_scan(btmp_t *btmp, uint32_t len);

// set bit <bit_idx> in the bitmap
void bitmap_set(btmp_t *btmp, uint32_t bit_idx, int value);

void print_btmp(btmp_t *btmp);

#endif
