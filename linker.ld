ENTRY(entry)
OUTPUT_FORMAT(elf32-i386)
OUTPUT_ARCH(i386:i386)

SECTIONS {
    . = 0x80100000;

    .text ALIGN(8) : AT(ADDR(.text) - 0x80000000) {
        KEEP(*(.multiboot));
        *(.text*)
    }

    .rodata ALIGN(8) : AT(ADDR(.rodata) - 0x80000000) {
        *(.rodata)
        *(.constant)
    }

    .data ALIGN(8) : AT(ADDR(.data) - 0x80000000) {
        KEEP(*( .init_array ));
        KEEP(*(SORT_BY_INIT_PRIORITY( .init_array.* )));
        *(.data)
    }

    .bss ALIGN(8) : AT(ADDR(.bss) - 0x80000000) {
        *(COMMON)
        *(.bss)
    }

    kernel_image_end = .;
}
