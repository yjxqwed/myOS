[bits 32]

[section .text]
global usr_test

usr_test:
    mov ebp, 0x0c800000
    jmp $
