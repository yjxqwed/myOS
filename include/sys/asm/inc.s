; some constants

%macro MAGICBP 0
xchg bx, bx
%endmacro


[section .constant]
kernel_space_base_addr: equ 0x80000000

kernel_stk_top:  equ 0x01ffffff
usr_stk_top:     equ 0x0c800000

kernel_code_sel: equ 0x10
kernel_data_sel: equ 0x18
tss_sel:         equ 0x20
usr_code_sel:    equ 0x2B
usr_data_sel:    equ 0x33
