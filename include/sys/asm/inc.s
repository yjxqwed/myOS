; some constants

%macro MAGICBP 0
xchg bx, bx
%endmacro


[section .constant]
KERNEL_BASE: equ 0x80000000

kernel_code_sel: equ 0x10
kernel_data_sel: equ 0x18
tss_sel:         equ 0x20
usr_code_sel:    equ 0x2B
usr_data_sel:    equ 0x33
