.intel_syntax noprefix
.globl main
main:
    push 1
    push 2
    push 3
    push 4
    pop rdi
    pop rax
    sub rax, rdi
    push rax
    pop rdi
    pop rax
    imul rax, rdi
    push rax
    pop rdi
    pop rax
    add rax, rdi
    push rax
    pop rax
    ret
