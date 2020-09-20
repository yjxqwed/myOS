	.file	"kernel.cpp"
	.text
	.globl	_Z6printfPKc
	.type	_Z6printfPKc, @function
_Z6printfPKc:
.LFB0:
	.cfi_startproc
	endbr32
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	movl	%esp, %ebp
	.cfi_def_cfa_register 5
	subl	$16, %esp
	call	__x86.get_pc_thunk.ax
	addl	$_GLOBAL_OFFSET_TABLE_, %eax
	movl	$753664, -8(%ebp)
	movl	$0, -4(%ebp)
.L3:
	movl	-4(%ebp), %edx
	movl	8(%ebp), %eax
	addl	%edx, %eax
	movzbl	(%eax), %eax
	testb	%al, %al
	je	.L4
	movl	-4(%ebp), %eax
	leal	(%eax,%eax), %edx
	movl	-8(%ebp), %eax
	addl	%edx, %eax
	movzwl	(%eax), %eax
	movb	$0, %al
	movl	%eax, %ecx
	movl	-4(%ebp), %edx
	movl	8(%ebp), %eax
	addl	%edx, %eax
	movzbl	(%eax), %eax
	cbtw
	orl	%eax, %ecx
	movl	%ecx, %edx
	movl	-4(%ebp), %eax
	leal	(%eax,%eax), %ecx
	movl	-8(%ebp), %eax
	addl	%ecx, %eax
	movw	%dx, (%eax)
	addl	$1, -4(%ebp)
	jmp	.L3
.L4:
	nop
	leave
	.cfi_restore 5
	.cfi_def_cfa 4, 4
	ret
	.cfi_endproc
.LFE0:
	.size	_Z6printfPKc, .-_Z6printfPKc
	.section	.rodata
	.align 4
.LC0:
	.string	"Hello Wolrd! --- This is myOS by Justing Yang"
	.text
	.globl	kernelMain
	.type	kernelMain, @function
kernelMain:
.LFB1:
	.cfi_startproc
	endbr32
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	movl	%esp, %ebp
	.cfi_def_cfa_register 5
	call	__x86.get_pc_thunk.ax
	addl	$_GLOBAL_OFFSET_TABLE_, %eax
	leal	.LC0@GOTOFF(%eax), %eax
	pushl	%eax
	call	_Z6printfPKc
	addl	$4, %esp
.L6:
	jmp	.L6
	.cfi_endproc
.LFE1:
	.size	kernelMain, .-kernelMain
	.section	.text.__x86.get_pc_thunk.ax,"axG",@progbits,__x86.get_pc_thunk.ax,comdat
	.globl	__x86.get_pc_thunk.ax
	.hidden	__x86.get_pc_thunk.ax
	.type	__x86.get_pc_thunk.ax, @function
__x86.get_pc_thunk.ax:
.LFB2:
	.cfi_startproc
	movl	(%esp), %eax
	ret
	.cfi_endproc
.LFE2:
	.ident	"GCC: (Ubuntu 9.3.0-10ubuntu2) 9.3.0"
	.section	.note.GNU-stack,"",@progbits
	.section	.note.gnu.property,"a"
	.align 4
	.long	 1f - 0f
	.long	 4f - 1f
	.long	 5
0:
	.string	 "GNU"
1:
	.align 4
	.long	 0xc0000002
	.long	 3f - 2f
2:
	.long	 0x3
3:
	.align 4
4:
