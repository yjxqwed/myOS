/*   myOS kernel main code   */

#include <kprintf.h>

void kernelMain() {
    kprintf(KPL_DUMP, "Hello Wolrd! --- This is myOS by Justing Yang\n");
    kprintf(KPL_DUMP, "%d is the minimum int32\n", (int32_t)0x80000000);
    kprintf(KPL_DUMP, "%d is the maximum int32\n", (int32_t)0x7FFFFFFF);
    kprintf(KPL_DUMP, "%d is zero in decimal\n", 0);
    kprintf(KPL_DUMP, "%x is zero in hexadecimal\n", 0);
    kprintf(KPL_DUMP, "%s is a test string\n", "#abcdefg$");
    kprintf(KPL_DUMP, "%c and %% are test characters\n", '@');
    while (1);
}
