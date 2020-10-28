#include <bitmap.h>
#include <string.h>
#include <common/debug.h>
#include <common/utils.h>

#define BITMASK 1

#define CHECK_BITIDX(btmp, bit_idx) ASSERT(bit_idx < 8 * btmp->byte_num_)

void bitmap_init(btmp_t *btmp, uint32_t byte_len) {
    btmp->first_zero_bit_idx = 0;
    btmp->byte_num_ = byte_len;
    memset(btmp->bits_, 0, btmp->byte_num_);
}

int bitmap_bit_test(btmp_t *btmp, uint32_t bit_idx) {
    CHECK_BITIDX(btmp, bit_idx);
    return (btmp->bits_[bit_idx / 8] & (BITMASK << (bit_idx % 8))) ? 1 : 0;
}

// return the idx of the first zero bit of byte
static inline int first_zero_bit(uint8_t byte) {
    int idx = 0;
    while (byte % 2) {
        byte /= 2;
        idx++;
    }
    return idx < 8 ? idx : -1;
}

int bitmap_scan(btmp_t *btmp, uint32_t len) {
    if (len > btmp->byte_num_ * 8) {
        return -1;
    }
    uint32_t byte_idx = 0;
    while (byte_idx < btmp->byte_num_ && 0xff == btmp->bits_[byte_idx]) {
        byte_idx++;
    }
    if (byte_idx == btmp->byte_num_) {
        return -1;
    }

    uint32_t bit_idx = 8 * byte_idx + first_zero_bit(btmp->bits_[byte_idx]);
    if (len == 1) {
        return bit_idx;
    }

}

void bitmap_set(btmp_t *btmp, uint32_t bit_idx, int value) {
    CHECK_BITIDX(btmp, bit_idx);
    if (value) {
        btmp->bits_[bit_idx / 8] |= (BITMASK << (bit_idx % 8));
    } else {
        btmp->bits_[bit_idx / 8] &= ~(BITMASK << (bit_idx % 8));
    }
}