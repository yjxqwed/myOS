#include <common/debug.h>
#include <driver/screen.h>
#include <string.h>
#include <kprintf.h>
#include <arch/x86.h>

void printISRParam(const isrp_t* p) {
    kprintf(KPL_PANIC, " {eip=%x; errco=%x}", p->eip, p->err_code);
}

void panic_spin(
    const char* filename, int line,
    const char* funcname, const char* condition
) {
    disable_int();
    kprintf(KPL_DUMP, "\n");
    kprintf(KPL_PANIC, "!!!!! error !!!!!\n");
    kprintf(KPL_PANIC, "file: %s\n", filename);
    kprintf(KPL_PANIC, "line: %d\n", line);
    kprintf(KPL_PANIC, "function: %s\n", funcname);
    kprintf(KPL_PANIC, "condition: %s\n", condition);
    while (1);
}
