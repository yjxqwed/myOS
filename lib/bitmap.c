#include <bitmap.h>
#include <string.h>
#include <common/debug.h>
#include <kprintf.h>

#define BITMASK 1

#define CHECK_BITIDX(btmp, bit_idx) ASSERT(bit_idx < 8 * btmp->byte_num_)

static int bitmap_find_first_zero_bit(btmp_t *btmp, uint32_t start_bit_idx);

void bitmap_init(btmp_t *btmp, uint32_t byte_len) {
    btmp->first_zero_bit = 0;
    btmp->byte_num_ = byte_len;
    btmp->num_zero = byte_len * 8;
    memset(btmp->bits_, 0, btmp->byte_num_);
}

static uint32_t count_bit0_in_byte(uint8_t byte) {
    if (byte == 0) {
        return 8;
    }
    if (byte == 0xff) {
        return 0;
    }
    uint32_t num = 0;
    for (int i = 0; i < 8; i++) {
        if ((byte & 0x01) == 0) {
            num++;
        }
        byte >>= 1;
    }
    return num;
}

void bitmap_reinit(btmp_t *btmp, uint32_t byte_len) {
    btmp->byte_num_ = byte_len;
    btmp->first_zero_bit = bitmap_find_first_zero_bit(btmp, 0);
    for (uint32_t i = btmp->first_zero_bit / 8; i < byte_len; i++) {
        btmp->num_zero += count_bit0_in_byte(btmp->bits_[i]);
    }
}

int bitmap_bit_test(btmp_t *btmp, uint32_t bit_idx) {
    CHECK_BITIDX(btmp, bit_idx);
    return (btmp->bits_[bit_idx / 8] & (BITMASK << (bit_idx % 8))) ? 1 : 0;
}

// return the idx of the first zero bit of byte
static inline int byte_find_first_zero_bit(uint8_t byte) {
    if (byte == (uint8_t)0xff) {
        return -1;
    }
    int idx = 0;
    while (byte % 2) {
        byte /= 2;
        idx++;
    }
    return idx;
}

// return the first zero bit from start_bit_idx, -1 if no such bit
static int bitmap_find_first_zero_bit(btmp_t *btmp, uint32_t start_bit_idx) {
    // if start_bit_idx overflows, return -1
    if (start_bit_idx >= btmp->byte_num_ * 8) {
        return -1;
    }

    uint32_t byte_idx = start_bit_idx / 8;
    int bit_offset = start_bit_idx % 8;
    // if not a whole byte, search the first byte specifically
    if (bit_offset != 0) {
        for (; bit_offset < 8; bit_offset++) {
            if ((btmp->bits_[byte_idx] & (1 << bit_offset)) == 0) {
                return byte_idx * 8 + bit_offset;
            }
        }
        byte_idx++;
    }

    for (
        ;
        (byte_idx < btmp->byte_num_) && (btmp->bits_[byte_idx] == (uint8_t)0xff);
        byte_idx++
    );
    if (byte_idx == btmp->byte_num_) {
        return -1;
    } else {
        return byte_idx * 8 + byte_find_first_zero_bit(btmp->bits_[byte_idx]);
    }
}

int bitmap_scan(btmp_t *btmp, uint32_t len) {
    if (len > btmp->num_zero) {
        return -1;
    } else if (len == 1) {
        return btmp->first_zero_bit;
    }

    int start_bit_idx = btmp->first_zero_bit;
    while (start_bit_idx != -1) {
        uint32_t cnt = 1;
        int bit_idx = start_bit_idx + 1;
        while (
            bit_idx < btmp->byte_num_ * 8 &&
            !bitmap_bit_test(btmp, bit_idx) &&
            cnt < len
        ) {
            cnt++;
            bit_idx++;
        }
        if (cnt == len) {
            return start_bit_idx;
        } else {
            start_bit_idx = bitmap_find_first_zero_bit(btmp, bit_idx + 1);
        }
    }

    return -1;
}

void bitmap_set(btmp_t *btmp, uint32_t bit_idx, int value) {
    CHECK_BITIDX(btmp, bit_idx);
    uint8_t *byte = &(btmp->bits_[bit_idx / 8]);
    uint8_t mask = BITMASK << (bit_idx % 8);
    if (value) {
        // btmp->bits_[bit_idx / 8] |= (BITMASK << (bit_idx % 8));
        ASSERT((uint8_t)(*byte & mask) == (uint8_t)0);
        ASSERT(bit_idx >= btmp->first_zero_bit);
        *byte |= mask;
        btmp->num_zero--;
        if (bit_idx == btmp->first_zero_bit) {
            // find next first_zero_bit
            btmp->first_zero_bit = bitmap_find_first_zero_bit(btmp, bit_idx + 1);
        }
    } else {
        // btmp->bits_[bit_idx / 8] &= ~(BITMASK << (bit_idx % 8));
        ASSERT((uint8_t)(*byte | ~mask) == (uint8_t)0xff);
        ASSERT(bit_idx != btmp->first_zero_bit);
        *byte &= ~mask;
        btmp->num_zero++;
        if (bit_idx < btmp->first_zero_bit) {
            btmp->first_zero_bit = bit_idx;
        }
    }
}

void print_btmp(btmp_t *btmp) {
    kprintf(
        KPL_DEBUG,
        "(0x%X){first_zero_bit=%d, num_zero=%d, byte_num=%d, bits=0x%X}",
        btmp, btmp->first_zero_bit, btmp->num_zero, btmp->byte_num_, btmp->bits_
    );
}
