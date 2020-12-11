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

static char buf[1024];
static int idx = 0;

static char hex_char[16] = "0123456789ABCDEF";

static void parse_dec(int32_t number) {
    if (number == 0) {
        buf[idx++] = '0';
        return;
    } else if (number == -2147483648) {
        strncpy("-2147483648", &(buf[idx]), 11);
        idx += 11;
        return;
    }
    int l = idx;
    if (number < 0) {
        buf[idx++] = '-';
        number = -number;
    }
    while (number > 0) {
        buf[idx++] = hex_char[number % 10];
        number /= 10;
    }
    int i = l;
    if (buf[l] == '-') {
        i++;
    }
    int j = idx - 1;
    while (i < j) {
        char t = buf[i];
        buf[i] = buf[j];
        buf[j] = t;
        i++;
        j--;
    }
}

static void parse_hex(uint32_t number) {
    if (number == 0) {
        buf[idx++] = '0';
        return;
    }
    int l = idx;
    while (number > 0) {
        buf[idx++] = hex_char[number % 16];
        number /= 16;
    }
    int i = l;
    int j = idx - 1;
        while (i < j) {
        char t = buf[i];
        buf[i] = buf[j];
        buf[j] = t;
        i++;
        j--;
    }
}

static void parse_heX(uint32_t number) {
    memset(&(buf[idx]), '0', 8);
    idx += 8;
    for (int d = 1; number > 0; d++) {
        buf[(idx - d)] = hex_char[number % 16];
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

void kprintf(KP_LEVEL kpl, const char *fmt, ...) {
    args_list args;
    args_start(args, fmt);
    idx = 0;
    for (int i = 0; fmt[i] != '\0'; i++) {
        if (fmt[i] != '%' && fmt[i] != '\\') {  // normal characters
            buf[idx++] = fmt[i];
        } else if (fmt[i] == '\\') {  // escape characters
            /* \a \b \t \n \v \f \r \\ */
            switch (fmt[++i]) {
                case 'a':  buf[idx++] = '\a'; break;
                case 'b':  buf[idx++] = '\b'; break;
                case 't':  buf[idx++] = '\t'; break;
                case 'n':  buf[idx++] = '\n'; break;
                case 'v':  buf[idx++] = '\v'; break;
                case 'f':  buf[idx++] = '\f'; break;
                case 'r':  buf[idx++] = '\r'; break;
                case '\\': buf[idx++] = '\\'; break;
            }
        } else {
            switch (fmt[++i]) {
                case 's': {
                    char *s = (char *)args_next(args, char *);
                    while (*s) {
                        buf[idx++] = *s++;
                    }
                    break;
                } case 'c': {
                    buf[idx++] = (char)args_next(args, int32_t);
                    break;
                } case 'x': {
                    parse_hex((uint32_t)args_next(args, uint32_t));
                    break;
                } case 'X': {
                    parse_heX((uint32_t)args_next(args, uint32_t));
                    break;
                } case 'd': {
                    parse_dec((int32_t)args_next(args, int32_t));
                    break;
                } case '%': {
                    buf[idx++] = '%';
                    break;
                } default: {
                    buf[idx++] = fmt[i];
                    break;
                }
            }
        }
    }
    buf[idx] = '\0';

    // for (int i = 0; i < idx; i++) {
    //     putc(buf[i], KPL[kpl].bg, KPL[kpl].fg);
    // }
    scrn_puts(buf, KPL[kpl].bg, KPL[kpl].fg);
    args_end(args);
}
