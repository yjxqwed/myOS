OUTPUT_FORMAT(elf32-i386)
OUTPUT_ARCH(i386:i386)

SECTIONS {
    . = 0x00100000;

    .text ALIGN(8) : AT(ADDR(.text)) {
        KEEP(*(.multiboot));
        *(.text*)
    }

    .rodata ALIGN(8) : AT(ADDR(.rodata)) {
        *(.rodata)
        *(.constant)
    }

    .data ALIGN(8) : AT(ADDR(.data)) {
        KEEP(*( .init_array ));
        KEEP(*(SORT_BY_INIT_PRIORITY( .init_array.* )));
        *(.data)
    }

    .bss ALIGN(8) : AT(ADDR(.bss)) {
        *(COMMON)
        *(.bss)
    }

}
