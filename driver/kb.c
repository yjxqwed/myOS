#include "kb.h"
#include "system.h"
#include "screen.h"
#include "utils.h"
#include "string.h"

// keyboard data port: 0x60
//          control port: 0x64
void kb_handler(isrp_t *p) {
    uint8_t scan_code = inportb(0x60);

    char s[INT32LEN];
    if (scan_code & 0x80) {  // bit 7 set => released
        putstr(" released! ");
    } else {  // else pressed
        putstr(itos((int32_t)scan_code, s));
        putstr(" pressed! ");
    }
}
