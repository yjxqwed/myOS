ENTRY(loader)
OUTPUT_FORMAT(elf32-i386)
OUTPUT_ARCH(i386:i386)

SECTIONS {
    . = 0xc0100000;

    .text : ALIGN(8) {
        KEEP(*(.multiboot));
        *(.text*)
        *(.rodata)
        *(.constant)
    }

    .data : {
        KEEP(*( .init_array ));
        KEEP(*(SORT_BY_INIT_PRIORITY( .init_array.* )));
        *(.data)
    }

    .bss : {
        *(.bss)
    }

    /DISCARD/ : {
        *(.fini_array*)
        *(.comment)
    }
}
