#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "decomposer.h"
#include "IR.h"

#define DEBUG
#define SIZE 256

void stackmachine_emulator_x86_64(Funcs* func_head, FILE *dstfile);
void stackmachine_emulator_x86_64_blocks(Block *block, FILE *dstfile);
void stackmachine_emulator_x86_64_block(Block *block, FILE *dstfile);
void code_generator_x86_64(AST_Node *node, FILE *dstfile);

/**
 * @brief ENTRY POINT of gompiler frontend
 *
 * @param argv[1]: input srcfile
 * @param argv[2]: output dstfile
 */
int main(int argc, char *argv[])
{
    printf("*** GOMPILER FRONT END ***\n");
/* --- INITIALIZE FILES --- */
    FILE *srcfile, *dstfile;
    char srcline[SIZE], dstline[SIZE];
    srcline[0] = '\0';
    dstline[0] = '\0';
    srcfile = fopen(argv[1], "r");
    dstfile = fopen(argv[2], "w");

/* --- OBTAIN CODE INFO --- */
    int codeline = 0;
    /**
     * @brief code配列にソースコードをコピーしていく
     */
    while(fgets(srcline, SIZE, srcfile) != NULL) {
        strcpy(code[codeline], srcline);
        codeline++;
    }

/* --- TOKENIZER --- */
    Funcs   *func_head = (Funcs*)malloc(sizeof(Funcs));
    func_head->type = F_ROOT;
    decomposer(code, codeline, func_head);

/* --- PRINT MODIFIED CODE --- */
    stackmachine_emulator_x86_64(func_head, dstfile);
    /**
     * TODO: 後々，codeにはソースと1対1な中間表現が格納され，ミドルパスに渡される予定
     */
    // for(int dstlineiter = 0; dstlineiter < codeline; dstlineiter++){
    //     fprintf(dstfile, code[dstlineiter]);
    // }

#ifdef DEBUG
    //  printf("> IR's been generated\n");
#endif

    fclose(srcfile);
    fclose(dstfile);

    return 0;
}

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
        code_generator_x86_64(node_head, dstfile);
    }
}

void code_generator_x86_64(AST_Node *node, FILE *dstfile)
{
    if(node->kind == AST_NUM) {
        fprintf(dstfile, "    push %d\n", node->value);
        return;
    }

    code_generator_x86_64(node->left, dstfile);
    code_generator_x86_64(node->right, dstfile);

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





