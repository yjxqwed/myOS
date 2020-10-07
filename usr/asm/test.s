[bits 32]

[section .text]
global usr_test
; extern clear_screen
usr_test:
    ; call 0x10:clear_screen
    jmp $
