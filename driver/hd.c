#include <driver/hd.h>
#include <kprintf.h>
#include <common/types.h>
#include <arch/x86.h>

// ATA0
#define DEVICE_REG_PRIMARY_PORT 0x1F6
// ATA1
#define DEVICE_REG_SECONDARY_PORT 0x176

void print_hd_info() {
    int num_hd = (int)(*(uint8_t*)(0x475));
    kprintf(KPL_DUMP, "Number of hd(s): %d\n", num_hd);
    uint32_t dev = (uint32_t)inportb(DEVICE_REG_PRIMARY_PORT);
    kprintf(KPL_DUMP, "ATA0 device = %x\n", dev);
    dev = (uint32_t)inportb(DEVICE_REG_SECONDARY_PORT);
    kprintf(KPL_DUMP, "ATA1 device = %x\n", dev);
}
