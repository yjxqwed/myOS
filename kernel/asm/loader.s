; file loader.s
[bits 32]

%include "sys/asm/inc.s"

[section .multiboot]
    MAGIC_NUM equ 0x1badb002
    FLAG      equ (1 << 0 | 1 << 1)
    CHECK_SUM equ -(MAGIC_NUM + FLAG)

    dd MAGIC_NUM
    dd FLAG
    dd CHECK_SUM

extern ksetup_before_paging
extern ksetup_after_paging

[section .text]
global loader
loader:
    mov esp, kernel_stk_top
    mov ebp, esp

    push eax
    push ebx
    call ksetup_before_paging
    add esp, 8
    ; jmp to the higher half
    lea eax, [after_paging]
    jmp eax
after_paging:
    add esp, kernel_space_base_addr
    add ebp, kernel_space_base_addr
    call ksetup_after_paging

    ; setup done, jmp to the kernel
    jmp kernel_entry


global flushGDT
extern _gp  ; _gp is the gdt pointer
flushGDT:
    lgdt [_gp]
    mov ax, kernel_data_sel
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    jmp kernel_code_sel:flush2
flush2:
    ret

global flushIDT
extern _ip  ; the idt pointer
flushIDT:
    MAGICBP
    lidt [_ip]
    ret

global flushPD
extern _pd  ; page directory base address
flushPD:
    ; _pd is vaddr, we need paddr here
    mov eax, [_pd - kernel_space_base_addr]
    mov cr3, eax

    mov eax, cr0  ; enable paging
    or eax, 0x80000000  ; bit 31 of cr0 is to enable paging
    mov cr0, eax

    ret

extern kernelMain
; start of the kernel
kernel_entry:
    call kernelMain
    jmp $

[section .data]
one: db 0x01
