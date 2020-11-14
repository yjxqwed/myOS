#include <driver/kb.h>
#include <arch/x86.h>
#include <driver/screen.h>
#include <common/utils.h>
#include <string.h>
#include <kprintf.h>

// keyboard data port: 0x60
//          control port: 0x64
void kb_handler(isrp_t *p) {
    uint8_t scan_code = inportb(0x60);

    char s[INT32LEN];
    if (scan_code & 0x80) {  // bit 7 set => released
        kprintf(KPL_NOTICE, "{%x released!}", scan_code);
    } else {  // else pressed
        // kprintf(uitosh((int32_t)scan_code, s));
        // kprintf(" pressed! ");
        kprintf(KPL_DEBUG, "{%x pressed!}", scan_code);
    }
}
