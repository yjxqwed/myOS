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

extern kernelMain
extern usr_test
extern test_magic_number
extern mm_init
extern video_mem_enable_paging
extern setGlobalDescriptorTable
[section .text]
global loader
loader:
    mov esp, kernel_stk_top
    mov ebp, esp
    push eax
    call test_magic_number
    push ebx  ; for mm_init
    call setGlobalDescriptorTable
    call mm_init

    add esp, kernel_space_base_addr
    add ebp, kernel_space_base_addr

    ; mov ax, tss_sel
    ; ltr ax

    ; ; push usr_data_sel  ; usr ss
    ; ; push usr_stk_top   ; usr esp
    ; ; push usr_code_sel  ; usr code
    ; ; push usr_test      ; usr func
    ; ; retf

    call video_mem_enable_paging

    mov eax, kernelMain
    add eax, kernel_space_base_addr
    jmp eax

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
    ; magic_bp
    lidt [_ip]
    ret

global flushPD
extern _pd  ; page directory base address
extern gdt_enable_paging
flushPD:
    ; magic_bp

    ; sgdt [_gp]
    add dword [_gp + 2], kernel_space_base_addr

    ; call gdt_enable_paging

    mov eax, [_pd]
    mov cr3, eax

    ; enable paging
    mov eax, cr0
    ; bit 31 of cr0 is to enable paging
    or eax, 0x80000000
    mov cr0, eax

    lgdt [_gp]
    ret


[section .data]
one: db 0x01
