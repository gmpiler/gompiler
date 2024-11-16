.intel_syntax noprefix
.globl main
main:
# function stack epilogue
	push rbp
	mov rbp, rsp
	sub rsp, 0
# function body
# function stack prologue
	mov rax, -1[rbp]
	mov rsp, rbp
	pop rbp
	ret
