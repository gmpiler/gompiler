.intel_syntax noprefix
.globl main
main:
# function stack epilogue
	push rbp
	mov rbp, rsp
	sub rsp, 4
# function body
## right value evaluation
	push 1
	push 2
	pop rdi
	pop rax
	add rax, rdi
	push rax
	push 3
	push 4
	pop rdi
	pop rax
	add rax, rdi
	push rax
	pop rdi
	pop rax
	imul rax, rdi
	push rax
## assign the value to left var
	pop rax
	mov -4[rbp], rax
# function stack prologue
	mov rax, -4[rbp]
	mov rsp, rbp
	pop rbp
	ret
