; file loader.s
[bits 32]

[section .multiboot]
    MAGIC_NUM equ 0x1badb002
    FLAG      equ (1 << 0 | 1 << 1)
    CHECK_SUM equ -(MAGIC_NUM + FLAG)   

    dd MAGIC_NUM
    dd FLAG
    dd CHECK_SUM

extern kernelMain
[section .text]
global loader
kernel_stk equ 0x01280800
loader:
    ; xchg bx, bx
    mov esp, kernel_stk
    mov ebp, esp
    push eax
    push ebx
    call kernelMain

    loop:
        jmp loop

global flushGDT
extern _gp, _dss, _css  ; _gp is the gdt pointer
                        ; _dss is the kernel data seg selector
                        ; _css is the kernel code seg selector
flushGDT:
    ; xchg bx, bx  ; magic bp
    lgdt [_gp]
    mov ax, [_dss]
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    jmp 0x10:flush2
flush2:
    ret

global flushIDT
extern _ip  ; the idt pointer
flushIDT:
    lidt [_ip]
    ret

[section .data]
one: db 0x01
