#include <device/tty.h>
#include <device/console.h>
#include <device/kb.h>

struct TTY {
    // the console associated to this tty
    console_t *my_console;
};
