.intel_syntax noprefix
.globl main
main:
# function stack epilogue
	push rbp
	mov rbp, rsp
	sub rsp, 8
# function body
## right value evaluation
	push 0
## assign the value to left var
	pop rax
	mov -4[rbp], rax
	push 0
	pop rcx
	mov 0[rbp], rcx
.L0:
	cmp rcx, 9
	jns .L0
## right value evaluation
	mov rax, -4[rbp]
	mov rcx, 0[rbp]
	add rax, rcx
	push rax
## assign the value to left var
	pop rax
	mov -4[rbp], rax
## right value evaluation
	push 1
	mov rax, 0[rbp]
	pop rdi
	sub rax, rdi
	push rax
## assign the value to left var
	pop rax
	mov 0[rbp], rax
	jmp .L0
.L1:
# function stack prologue
	mov rax, -4[rbp]
	mov rsp, rbp
	pop rbp
	ret
