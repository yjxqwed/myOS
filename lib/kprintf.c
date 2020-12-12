#include <kprintf.h>
#include <device/screen.h>  // for color and video mem operation
#include <common/types.h>
#include <string.h>

#define args_list char *

#define _arg_stack_size(type) ( \
    ((sizeof(type) - 1) / sizeof(int32_t) + 1) * sizeof(int32_t) \
)

#define args_start(ap, fmt) do { \
    ap = (char *)((unsigned int)(&fmt) + _arg_stack_size(&fmt)); \
} while (0);

#define args_end(ap)

#define args_next(ap, type) ( \
    ((type *)(ap += _arg_stack_size(type)))[-1] \
)


// static int idx = 0;

static char hex_char[16] = "0123456789ABCDEF";

static void parse_dec(char *out, int *idx, int32_t number) {
    if (number == 0) {
        out[(*idx)++] = '0';
        return;
    } else if (number == -2147483648) {
        strncpy("-2147483648", &(out[(*idx)]), 11);
        (*idx) += 11;
        return;
    }
    int l = (*idx);
    if (number < 0) {
        out[(*idx)++] = '-';
        number = -number;
    }
    while (number > 0) {
        out[(*idx)++] = hex_char[number % 10];
        number /= 10;
    }
    int i = l;
    if (out[l] == '-') {
        i++;
    }
    int j = (*idx) - 1;
    while (i < j) {
        char t = out[i];
        out[i] = out[j];
        out[j] = t;
        i++;
        j--;
    }
}

static void parse_hex(char *out, int *idx, uint32_t number) {
    if (number == 0) {
        out[(*idx)++] = '0';
        return;
    }
    int l = (*idx);
    while (number > 0) {
        out[(*idx)++] = hex_char[number % 16];
        number /= 16;
    }
    int i = l;
    int j = (*idx) - 1;
        while (i < j) {
        char t = out[i];
        out[i] = out[j];
        out[j] = t;
        i++;
        j--;
    }
}

static void parse_heX(char *out, int *idx, uint32_t number) {
    memset(&(out[(*idx)]), '0', 8);
    (*idx) += 8;
    for (int d = 1; number > 0; d++) {
        out[((*idx) - d)] = hex_char[number % 16];
        number /= 16;
    }
}

// To be implemented
static void parse_double(double number) {
}

// To be implemented
static void parse_float(float number) {
}

static struct KPC_STRUCT {
    COLOR bg;
    COLOR fg;
} KPL[KPL_SIZE] = {
    {BLACK, GRAY},
    {BLUE, LIGHT_GREEN},
    {CYAN, WHITE},
    {RED, YELLOW},
};

static char *__ksprintf(char *out, const char *fmt, args_list args) {
    if (out == NULL || fmt == NULL) {
        return NULL;
    }
    int idx = 0;
    for (int i = 0; fmt[i] != '\0'; i++) {
        if (fmt[i] != '%' && fmt[i] != '\\') {  // normal characters
            out[idx++] = fmt[i];
        } else if (fmt[i] == '\\') {  // escape characters
            /* \a \b \t \n \v \f \r \\ */
            switch (fmt[++i]) {
                case 'a':  out[idx++] = '\a'; break;
                case 'b':  out[idx++] = '\b'; break;
                case 't':  out[idx++] = '\t'; break;
                case 'n':  out[idx++] = '\n'; break;
                case 'v':  out[idx++] = '\v'; break;
                case 'f':  out[idx++] = '\f'; break;
                case 'r':  out[idx++] = '\r'; break;
                case '\\': out[idx++] = '\\'; break;
            }
        } else {
            switch (fmt[++i]) {
                case 's': {
                    char *s = (char *)args_next(args, char *);
                    while (*s) {
                        out[idx++] = *s++;
                    }
                    break;
                } case 'c': {
                    out[idx++] = (char)args_next(args, int32_t);
                    break;
                } case 'x': {
                    parse_hex(out, &idx, (uint32_t)args_next(args, uint32_t));
                    break;
                } case 'X': {
                    parse_heX(out, &idx, (uint32_t)args_next(args, uint32_t));
                    break;
                } case 'd': {
                    parse_dec(out, &idx, (int32_t)args_next(args, int32_t));
                    break;
                } case '%': {
                    out[idx++] = '%';
                    break;
                } default: {
                    out[idx++] = fmt[i];
                    break;
                }
            }
        }
    }
    out[idx] = '\0';
    return out;
}

char *ksprintf(char *out, const char *fmt, ...) {
    args_list args;
    args_start(args, fmt);
    char *ret = __ksprintf(out, fmt, args);
    args_end(args);
    return ret;
}

void kprintf(KP_LEVEL kpl, const char *fmt, ...) {
    static char buf[1024];
    args_list args;
    args_start(args, fmt);
    __ksprintf(buf, fmt, args);
    scrn_puts(buf, KPL[kpl].bg, KPL[kpl].fg);
    args_end(args);
}
