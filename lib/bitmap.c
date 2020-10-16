#include <bitmap.h>
#include <string.h>
#include <common/debug.h>

#define BITMASK 1

#define CHECK_BITIDX(btmp, bit_idx) ASSERT(bit_idx <= 8 * btmp->byte_num_)

void bitmap_init(btmp_t *btmp, uint32_t byte_len) {
    btmp->byte_num_ = byte_len;
    memset(btmp->bits_, 0, btmp->byte_num_);
}

bool bitmap_bit_test(btmp_t *btmp, uint32_t bit_idx) {
    CHECK_BITIDX(btmp, bit_idx);
    return (
        btmp->bits_[bit_idx / 8] & (BITMASK << (bit_idx % 8))
    ) ? true : false;
}

int bitmap_scan(btmp_t *btmp, uint32_t len) {
    
}

void bitmap_set(btmp_t *btmp, uint32_t bit_idx, int value) {
    CHECK_BITIDX(btmp, bit_idx);
    if (value) {
        btmp->bits_[bit_idx / 8] |= (BITMASK << (bit_idx % 8));
    } else {
        btmp->bits_[bit_idx / 8] &= ~(BITMASK << (bit_idx % 8));
    }
}