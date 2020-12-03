#include <driver/kb.h>
#include <arch/x86.h>
#include <common/utils.h>
#include <string.h>
#include <kprintf.h>
#include <sys/isr.h>

// keyboard data port
#define KB_DATA_PORT 0x60
// keyboard control port
#define KB_CTRL_PORT 0x64

#define KB_BREAK_MASK 0x80

static void kb_handler(isrp_t *p) {
    uint8_t scan_code = inportb(KB_DATA_PORT);

    char s[INT32LEN];
    if (scan_code & KB_BREAK_MASK) { 
        // bit 7 set => released
        kprintf(KPL_NOTICE, "{%x released!}", scan_code);
    } else {
        // else pressed
        // kprintf(uitosh((int32_t)scan_code, s));
        // kprintf(" pressed! ");
        kprintf(KPL_DEBUG, "{%x pressed!}", scan_code);
    }
}

void kb_init() {
    register_handler(INT_KB, kb_handler);
}
