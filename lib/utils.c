#include "utils.h"


char* itos(int32_t number, char out[INT32LEN]) {
    if (number == 0) {
        out[0] = '0';
        out[1] = '\0';
        return out;
    } else if (number == -2147483648) {
        // strcpy(out, "-2147483648")
        return out;
    }

    int idx = 0;
    if (number < 0) {
        number = -number;
        out[idx++] = '-';
    }
    while (number > 0) {
        out[idx++] = '0' + (number % 10);
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
