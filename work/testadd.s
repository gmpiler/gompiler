.intel_syntax noprefix
.globl main
main:
	push 1
	push 2
	pop %rbx
	pop %rdx
	add %rcx %rdx
	push 3
	push 4
	pop %rcx
	pop %rax
	add %rcx %rdx
	add %rcx %rdx
	mov %rax %rcx
	pop rax
ret
