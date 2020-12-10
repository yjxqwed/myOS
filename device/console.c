#include <device/console.h>
#include <thread/sync.h>

static mutex_t m;

void console_init() {
    mutex_init(&m);
}

void console_puts(const char *str) {
    mutex_lock(&m);
    kprintf(KPL_DUMP, str);
    mutex_unlock(&m);
}