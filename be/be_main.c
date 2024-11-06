#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../common/IR.h"

#define DEBUG

#define TARGET_x86_64_stack 0           // for stack machine emulator
#define TARGET_x86_64_register 1        // for register machine

void stackmachine_emulator_x86_64(Funcs* func_head, FILE *dstfile);
void stackmachine_emulator_x86_64_blocks(Block *block, FILE *dstfile);
void stackmachine_emulator_x86_64_block(Block *block, FILE *dstfile);
void op_generator_x86_64(Quadruple *qr, FILE *dstfile);

/**
 *              0: rax, 1: rcx, 2: rdx, 3:rbx
 *  dim 1 ->    0: empty, num: temporary variable entry
 */
int register_x86_64[4];
void init_registers_x86_64();

/**
 * @brief ENTRY POINT of gompiler backend
 */
void be_main(Funcs *func_head, FILE *dstfile)
{
    printf("*** GOMPILER BACK END ***\n");

    if(TARGET_x86_64_stack) {
        printf("- target stack machine emulator on x86_64\n");
        stackmachine_emulator_x86_64(func_head, dstfile);
    }

    if(TARGET_x86_64_register) {
        printf("- target register machine on x86_64\n");
        codegen_x86_64(func_head, dstfile);
    }
}

/* for x86_64 target register machine */
void codegen_x86_64(Funcs* func_head, FILE *dstfile)
{
    init_registers_x86_64();
    fprintf(dstfile, ".intel_syntax noprefix\n");
    fprintf(dstfile, ".globl main\n");
    fprintf(dstfile, "main:\n");
    for(Funcs *func_iter = func_head; func_iter != NULL; func_iter = func_iter->next){
        codegen_x86_64_blocks(func_iter->block_head, dstfile);
    }
    fprintf(dstfile, "\tpop rax\n");
    fprintf(dstfile, "ret\n");
}

void codegen_x86_64_blocks(Block *block, FILE *dstfile)
{
    for(Block *b = block; b != NULL; b = b->next){
        if(b->type == B_FOR || b->type == B_WHILE || b->type == B_IF) {
            codegen_x86_64_block(b, dstfile);
            codegen_x86_64_blocks(b->inner, dstfile);
        }else{
            codegen_x86_64_block(b, dstfile);
        }
    }
}

void codegen_x86_64_block(Block *block, FILE *dstfile)
{
    for(Quadruple_List *qr_list_iter = block->qr_head; qr_list_iter != NULL; qr_list_iter = qr_list_iter->next){
        Quadruple *qr_head = qr_list_iter->data;
        op_generator_x86_64(qr_head, dstfile);
    }
}

void op_generator_x86_64(Quadruple *qr, FILE *dstfile)
{
    int popcount = 0;
    if(qr->src_op1->type == OP_NUM) {
        fprintf(dstfile, "\tpush %d\n", qr->src_op1->num);
        popcount++;
    }
    if(qr->src_op2 != NULL && qr->src_op2->type == OP_NUM) {
        fprintf(dstfile, "\tpush %d\n", qr->src_op2->num);
        popcount++;
    }
    switch(popcount) {
        case 1:
            fprintf(dstfile, "\tpop %rcx\n");
            break;
        case 2:
            fprintf(dstfile, "\tpop %rcx\n");
            fprintf(dstfile, "\tpop %rdx\n");
            break;
        default:
            break;
    }
    switch(qr->type) {
        case OPE_ADD:
            fprintf(dstfile, "\tadd %rcx %rdx\n");
            break;
        case OPE_SUB:
            fprintf(dstfile, "\tadd %rcx %rdx\n");
            break;
        case OPE_MUL:
            fprintf(dstfile, "\tadd %rcx %rdx\n");
            break;
        case OPE_DIV:
            fprintf(dstfile, "\tadd %rcx %rdx\n");
            break;
        case OPE_ASSIGN:
            fprintf(dstfile, "\tmov %rax %rcx\n");
            break;
        default:
            break;
    }
}

void init_registers_x86_64()
{
    for(int i = 0; i < 4; i++){
        register_x86_64[i] = 0;
    }
}

/* for x86_64 target emulating a stach machine */
void stackmachine_emulator_x86_64(Funcs* func_head, FILE *dstfile)
{
    fprintf(dstfile, ".intel_syntax noprefix\n");
    fprintf(dstfile, ".globl main\n");
    fprintf(dstfile, "main:\n");
    for(Funcs *func_iter = func_head; func_iter != NULL; func_iter = func_iter->next){
        stackmachine_emulator_x86_64_blocks(func_iter->block_head, dstfile);
    }
    fprintf(dstfile, "    pop rax\n");
    fprintf(dstfile, "    ret\n");
}

void stackmachine_emulator_x86_64_blocks(Block *block, FILE *dstfile)
{
    for(Block *b = block; b != NULL; b = b->next){
        if(b->type == B_FOR || b->type == B_WHILE || b->type == B_IF) {
            stackmachine_emulator_x86_64_block(b, dstfile);
            stackmachine_emulator_x86_64_blocks(b->inner, dstfile);
        }else{
            stackmachine_emulator_x86_64_block(b, dstfile);
        }
    }
}

void stackmachine_emulator_x86_64_block(Block *block, FILE *dstfile)
{
    for(AST_Node_List *ast_iter = block->ast_head; ast_iter != NULL; ast_iter = ast_iter->next){
        AST_Node *node_head = ast_iter->data->right;    // assignのexprを抽出
        code_generator_x86_64_stack(node_head, dstfile);
    }
}

void code_generator_x86_64_stack(AST_Node *node, FILE *dstfile)
{
    if(node->kind == AST_NUM) {
        fprintf(dstfile, "    push %d\n", node->value);
        return;
    }

    code_generator_x86_64_stack(node->left, dstfile);
    code_generator_x86_64_stack(node->right, dstfile);

            fprintf(dstfile, "    pop rdi\n");
            fprintf(dstfile, "    pop rax\n");

    switch(node->kind) {
        case AST_ADD:
            fprintf(dstfile, "    add rax, rdi\n");
            break;
        case AST_SUB:
            fprintf(dstfile, "    sub rax, rdi\n");
            break;
        case AST_MUL:
            fprintf(dstfile, "    imul rax, rdi\n");
            break;
        case AST_DIV:
            fprintf(dstfile, "    cqo\n");
            fprintf(dstfile, "    idiv rdi\n");
            break;
    }

    fprintf(dstfile, "    push rax\n");
}

