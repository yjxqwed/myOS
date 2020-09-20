[bits 16]  ; real mode 16-bit

boot_seg: equ 0x07c0
init_set: equ 0x9000
kernel_seg: equ 0x1000

[section .text]
jmp boot_seg:boot_start
boot_start:
    call print_str
    call load_kernel
    loop:
        jmp loop

; usage: str in dx and len in cx
print_str:
    ; read cursor position
    mov ah, 0x03
    int 0x10

    ; print str
    mov ax, cs
    mov es, ax
    mov ax, 0x1301
    mov bx, 0x000f
    ; mov dx, 0x0000
    mov cx, msg_len
    mov bp, hello_msg
    int 0x10
    ret

load_kernel:
    mov dx, 0x0000
    mov cx, 0x0002
    mov ax, kernel_seg
    mov es, ax
    mov bx, 0x0000
    mov ah, 0x02
    mov al, 0x10
    int 0x13
    jnc kernel_load_ok
    jmp load_kernel

kernel_load_ok:
    ; jump to the kernel
    mov ax, kernel_seg
    mov ds, ax
    jmp kernel_seg:0x0000

    

hello_msg: db "hello world bootloader! Justin :)", 0xa, 0xd
msg_len: equ $ - hello_msg
times 510 - ($ - $$) db 0
db 0x55, 0xaa  ; magic number
