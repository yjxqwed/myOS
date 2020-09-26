#include "kb.h"
#include "system.h"


// keyboard data port: 0x60
//          control port: 0x64
void kb_handler(isrp_t *p) {
    uint8_t scan_code = inportb(0x60);

    if (scan_code & 0x80) {  // bit 7 set => released
        printf(" released! ");
    } else {  // else pressed
        printf(" pressed! ");
    }
}