.intel_syntax noprefix
.globl main
main:
# function stack epilogue
	push rbp
	mov rbp, rsp
	sub rsp, 12
# function body
## right value evaluation
	push 0
## assign the value to left var
	pop rax
	mov -8[rbp], rax
	push 0
	pop rcx
	mov 0[rbp], rcx
.L0:
	mov rcx, 0[rbp]
	cmp rcx, 2
	jns .L1
	push 0
	pop rcx
	mov -4[rbp], rcx
.L2:
	mov rcx, -4[rbp]
	cmp rcx, 2
	jns .L3
## right value evaluation
	mov rax, -8[rbp]
	mov rcx, 0[rbp]
	add rax, rcx
	push rax
## assign the value to left var
	pop rax
	mov -8[rbp], rax
## right value evaluation
	push 1
	mov rax, -4[rbp]
	pop rdi
	add rax, rdi
	push rax
## assign the value to left var
	pop rax
	mov -4[rbp], rax
	jmp .L2
.L3:
## right value evaluation
	push 1
	mov rax, 0[rbp]
	pop rdi
	add rax, rdi
	push rax
## assign the value to left var
	pop rax
	mov 0[rbp], rax
	jmp .L0
.L1:
# function stack prologue
	mov rax, -8[rbp]
	mov rsp, rbp
	pop rbp
	ret
