#include "string.h"


static char hex_char[16] = {
    '0', '1', '2', '3', '4', '5', '6', '7',
    '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
};

char* itos(int32_t number, char out[INT32LEN]) {
    if (number == 0) {
        out[0] = '0';
        out[1] = '\0';
        return out;
    } else if (number == -2147483648) {
        strcpy(out, "-2147483648");
        return out;
    }

    int idx = 0;
    if (number < 0) {
        number = -number;
        out[idx++] = '-';
    }
    while (number > 0) {
        out[idx++] = hex_char[number % 10];
        number /= 10;
    }
    out[idx] = '\0';
    int i = 0;
    if (out[0] == '-') {
        i = 1;
    }
    int j = idx - 1;
    while (i < j) {
        char t = out[i];
        out[i] = out[j];
        out[j] = t;
        i++;
        j--;
    }
    return out;
}

char* uitosh(uint32_t number, char out[UINT32LEN]) {
    if (number == 0) {
        return strcpy("0x0", out);
    }
    strncpy("0x", out, 2);
    int idx = 2;
    while (number > 0) {
        out[idx++] = hex_char[number % 16];
        number /= 16;
    }
    out[idx] = '\0';
    int i = 2;
    int j = idx - 1;
    while (i < j) {
        char t = out[i];
        out[i] = out[j];
        out[j] = t;
        i++;
        j--;
    }
    return out;
}

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


uint32_t strlen(const char* str) {
    uint32_t len = 0;
    while (str[len] != '\0') {
        len++;
    }
    return len;
}



char* strcpy(const char* src, char* dest) {
    if (src == dest) {
        return dest;
    }
    uint32_t len = strlen(src);
    if (src < dest) {
        for (int i = 0; i < len; i++) {
            dest[i] = src[i];
        }
    } else {
        for (int i = 0; i < len; i++) {
            dest[len - 1 - i] = src[len - 1 - i];
        }
    }
    dest[len] = '\0';
    return dest;
}


char* strncpy(const char* src, char* dest, uint32_t n) {
    if (src == dest) {
        return dest;
    }
    if (src < dest) {
        for (int i = 0; i < n; i++) {
            dest[i] = src[i];
        }
    } else {
        for (int i = 0; i < n; i++) {
            dest[n - 1 - i] = src[n - 1 - i];
        }
    }
    return dest;
}