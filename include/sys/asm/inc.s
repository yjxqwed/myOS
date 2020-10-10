; some constants

%macro magic_bp 0
xchg bx, bx
%endmacro


[section .constant]
kernel_stk_top: equ 0x01280800
usr_stk_top: equ 0x0c800000
kernel_code_sel: equ 0x10
kernel_data_sel: equ 0x18
usr_code_sel: equ 0x23
usr_data_sel: equ 0x2B
tss_sel: equ 0x30