#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../common/IR.h"

#define DEBUG

#define TARGET_x86_64_stack 1           // for stack machine emulator
#define TARGET_x86_64_register 0        // for register machine

void stackmachine_emulator_x86_64(Funcs* func_head, FILE *dstfile);
void stackmachine_emulator_x86_64_blocks(Block *block, FILE *dstfile);
void stackmachine_emulator_x86_64_block(Block *block, FILE *dstfile);
void op_generator_x86_64(Quadruple *qr, FILE *dstfile);
int find_offset(Funcs *func, char *symbol_name);

char ret_var[MAX_TOKENNAME_SIZE];

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

/* for x86_64 target emulating a stach machine */
void stackmachine_emulator_x86_64(Funcs* func_head, FILE *dstfile)
{
    fprintf(dstfile, ".intel_syntax noprefix\n");
    fprintf(dstfile, ".globl main\n");
    fprintf(dstfile, "main:\n");
    for(Funcs *func_iter = func_head; func_iter != NULL; func_iter = func_iter->next){
        /* 関数のスタック操作: プロローグ */
        fprintf(dstfile, "# function stack epilogue\n");
        fprintf(dstfile, "\tpush rbp\n");
        fprintf(dstfile, "\tmov rbp, rsp\n");
        fprintf(dstfile, "\tsub rsp, %d\n", func_iter->has_bytestack);
        
        fprintf(dstfile, "# function body\n");
        stackmachine_emulator_x86_64_blocks(func_iter->block_head, dstfile);
        
        fprintf(dstfile, "# function stack prologue\n");
        /* 関数のスタック操作: エピローグ */
        fprintf(dstfile, "\tmov rax, %d[rbp]\n", find_offset(func_iter, ret_var));
        fprintf(dstfile, "\tmov rsp, rbp\n");
        fprintf(dstfile, "\tpop rbp\n");
        fprintf(dstfile, "\tret\n");
    }
    // fprintf(dstfile, "\tret\n");
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
        if(ast_iter->data->kind == AST_RET) {
            for(int c = 0; c < MAX_TOKENNAME_SIZE; c++){
                ret_var[c] = '\0';
            }
            strcpy(ret_var, ast_iter->data->var);
            return;
        }
        AST_Node *node_head = ast_iter->data->right;        // assignのexprを抽出
        fprintf(dstfile, "## right value evaluation\n");
        code_generator_x86_64_stack(node_head, dstfile, block);    // 右辺の評価結果はraxにpushされる
        
        fprintf(dstfile, "## assign the value to left var\n");
        /* ここに，左辺値の評価結果であるraxを，右辺のメモリ領域に書き込む処理を追加 */
        fprintf(dstfile, "\tpop rax\n");
        fprintf(dstfile, "\tmov %d[rbp], rax\n", find_offset(block->func, ast_iter->data->left->var));     // TODO: オフセットは変数によって変える
    }
}

/* 末尾の引数blockはfunc->stack_headを辿るため */
void code_generator_x86_64_stack(AST_Node *node, FILE *dstfile, Block *block)
{
    if(node->kind == AST_NUM) {
        fprintf(dstfile, "\tpush %d\n", node->value);
        return;
    }
    if(node->kind == AST_VAR) {
        // fprintf(dstfile, "\tmov rcx, %d[rbp]\n", find_offset(block->func, node->var));
        // VARの時に，メモリからレジスタへpopする処理は後続に任せる
        return;
    }
    // TODO: 変数が2つの場合はrdxを用いる

    code_generator_x86_64_stack(node->left, dstfile, block);
    code_generator_x86_64_stack(node->right, dstfile, block);

    // TODO: 変数を使う場合の処理を追加
    switch(node->kind) {
        case AST_ADD:
            if(node->left->kind == AST_VAR && node->right->kind == AST_VAR) {   // ソースオペランドが2つともレジスタ
                fprintf(dstfile, "\tmov rax, %d[rbp]\n", find_offset(block->func, node->left->var));
                fprintf(dstfile, "\tmov rcx, %d[rbp]\n", find_offset(block->func, node->right->var));
                fprintf(dstfile, "\tadd rax, rcx\n");
            }else if(node->left->kind == AST_VAR){   // ソースオペランドが1つだけレジスタ
                fprintf(dstfile, "\tmov rax, %d[rbp]\n", find_offset(block->func, node->left->var));
                fprintf(dstfile, "\tpop rdi\n");
                fprintf(dstfile, "\tadd rax, rdi\n");
            }else if(node->right->kind == AST_VAR){   // ソースオペランドが1つだけレジスタ
                fprintf(dstfile, "\tpop rax\n");
                fprintf(dstfile, "\tmov rcx, %d[rbp]\n", find_offset(block->func, node->right->var));
                fprintf(dstfile, "\tadd rax, rdi\n");
            }else{  // 数値同士の演算
                fprintf(dstfile, "\tpop rdi\n");
                fprintf(dstfile, "\tpop rax\n");
                fprintf(dstfile, "\tadd rax, rdi\n");
            }
            break;
        case AST_SUB:
            if(node->left->kind == AST_VAR && node->right->kind == AST_VAR) {   // ソースオペランドが2つともレジスタ
                fprintf(dstfile, "\tmov rax, %d[rbp]\n", find_offset(block->func, node->left->var));
                fprintf(dstfile, "\tmov rcx, %d[rbp]\n", find_offset(block->func, node->right->var));
                fprintf(dstfile, "\tsub rax, rcx\n");
            }else if(node->left->kind == AST_VAR){   // ソースオペランドが1つだけレジスタ
                fprintf(dstfile, "\tmov rax, %d[rbp]\n", find_offset(block->func, node->left->var));
                fprintf(dstfile, "\tpop rdi\n");
                fprintf(dstfile, "\tsub rax, rdi\n");
            }else if(node->right->kind == AST_VAR){   // ソースオペランドが1つだけレジスタ
                fprintf(dstfile, "\tpop rax\n");
                fprintf(dstfile, "\tmov rcx, %d[rbp]\n", find_offset(block->func, node->right->var));
                fprintf(dstfile, "\tsub rax, rdi\n");
            }else{  // 数値同士の演算
                fprintf(dstfile, "\tpop rdi\n");
                fprintf(dstfile, "\tpop rax\n");
                fprintf(dstfile, "\tsub rax, rdi\n");
            }
            break;
        case AST_MUL:
            if(node->left->kind == AST_VAR && node->right->kind == AST_VAR) {   // ソースオペランドが2つともレジスタ
                fprintf(dstfile, "\tmov rax, %d[rbp]\n", find_offset(block->func, node->left->var));
                fprintf(dstfile, "\tmov rcx, %d[rbp]\n", find_offset(block->func, node->right->var));
                fprintf(dstfile, "\timul rax, rcx\n");
            }else if(node->left->kind == AST_VAR){   // ソースオペランドが1つだけレジスタ
                fprintf(dstfile, "\tmov rax, %d[rbp]\n", find_offset(block->func, node->left->var));
                fprintf(dstfile, "\tpop rdi\n");
                fprintf(dstfile, "\timul rax, rdi\n");
            }else if(node->right->kind == AST_VAR){   // ソースオペランドが1つだけレジスタ
                fprintf(dstfile, "\tpop rax\n");
                fprintf(dstfile, "\tmov rcx, %d[rbp]\n", find_offset(block->func, node->right->var));
                fprintf(dstfile, "\timul rax, rdi\n");
            }else{  // 数値同士の演算
                fprintf(dstfile, "\tpop rdi\n");
                fprintf(dstfile, "\tpop rax\n");
                fprintf(dstfile, "\timul rax, rdi\n");
            };
            break;
        case AST_DIV:
            // TODO: レジスタがソースの時のdiv処理追加
            fprintf(dstfile, "\tcqo\n");
            fprintf(dstfile, "\tidiv rdi\n");
            break;
    }

    fprintf(dstfile, "\tpush rax\n");
}

int find_offset(Funcs *func, char *symbol_name)
{
    for(struct Stack_List *siter = func->stack_head; siter != NULL; siter = siter->next){
        if(strcmp(siter->data->symbol_name, symbol_name) == 0) return siter->data->offset;
    }
    return -1;
}

/* ------------------------------- */
// /**
//  *              0: rax, 1: rcx, 2: rdx, 3:rbx
//  *  dim 1 ->    0: empty, num: temporary variable entry, minus num: for pop usage
//  */
// int register_x86_64[4];
// void init_registers_x86_64();
// int search_empty_reg();
// void get_empty_reg_name(int reg_num, char *dst_reg_name, int whoisuser);

// /* for x86_64 target register machine */
// void codegen_x86_64(Funcs* func_head, FILE *dstfile)
// {
//     init_registers_x86_64();
//     fprintf(dstfile, ".intel_syntax noprefix\n");
//     fprintf(dstfile, ".globl main\n");
//     fprintf(dstfile, "main:\n");
//     for(Funcs *func_iter = func_head; func_iter != NULL; func_iter = func_iter->next){
//         codegen_x86_64_blocks(func_iter->block_head, dstfile);
//     }
//     fprintf(dstfile, "\tpop rax\n");
//     fprintf(dstfile, "ret\n");
// }

// void codegen_x86_64_blocks(Block *block, FILE *dstfile)
// {
//     for(Block *b = block; b != NULL; b = b->next){
//         if(b->type == B_FOR || b->type == B_WHILE || b->type == B_IF) {
//             codegen_x86_64_block(b, dstfile);
//             codegen_x86_64_blocks(b->inner, dstfile);
//         }else{
//             codegen_x86_64_block(b, dstfile);
//         }
//     }
// }

// void codegen_x86_64_block(Block *block, FILE *dstfile)
// {
//     for(Quadruple_List *qr_list_iter = block->qr_head; qr_list_iter != NULL; qr_list_iter = qr_list_iter->next){
//         Quadruple *qr_head = qr_list_iter->data;
//         op_generator_x86_64(qr_head, dstfile);
//     }
// }

// void op_generator_x86_64(Quadruple *qr, FILE *dstfile)
// {
//     int popcount = 0;
//     int whoistemp = 0;
//     if(qr->src_op1->type == OP_NUM) {
//         fprintf(dstfile, "\tpush %d\n", qr->src_op1->num);
//         popcount++;
//     }else if(qr->src_op1->type == OP_VAR){
//         // スタックから取り出す
//     }else if(qr->src_op1->type == OP_TEMP){
//         whoistemp = 1;
//     }
//     if(qr->src_op2 != NULL && qr->src_op2->type == OP_NUM) {
//         fprintf(dstfile, "\tpush %d\n", qr->src_op2->num);
//         popcount++;
//     }else if(qr->src_op2->type == OP_VAR){
//         // スタックから取り出す
//     }else if(qr->src_op2->type == OP_TEMP){
//         whoistemp = 2;
//     }
//     char empty_reg_name[5];
//     switch(popcount) {
//         case 1:
//             fprintf(dstfile, "\tpop ");
//             get_empty_reg_name(search_empty_reg(), empty_reg_name, -1);
//             fprintf(dstfile, "%s\n", empty_reg_name);
//             break;
//         case 2:
//             fprintf(dstfile, "\tpop ");
//             get_empty_reg_name(search_empty_reg(), empty_reg_name, -1);
//             fprintf(dstfile, "%s\n", empty_reg_name);
//             fprintf(dstfile, "\tpop ");
//             get_empty_reg_name(search_empty_reg(), empty_reg_name, -2);
//             fprintf(dstfile, "%s\n", empty_reg_name);
//             break;
//         default:
//             break;
//     }
//     switch(qr->type) {
//         case OPE_ADD:
//             fprintf(dstfile, "\tadd");
//             break;
//         case OPE_SUB:
//             fprintf(dstfile, "\tsub");
//             break;
//         case OPE_MUL:
//             fprintf(dstfile, "\tmul");
//             break;
//         case OPE_DIV:
//             fprintf(dstfile, "\tdiv");
//             break;
//         case OPE_ASSIGN:
//             fprintf(dstfile, "\tmov");
//             break;
//         default:
//             break;
//     }
//     if(popcount == 0){
//         get_used_reg(qr->src_op1->temp_entry);
//         printf(", ");
//         get_used_reg(qr->src_op2->temp_entry);
//         char dummy[5];
//         get_empty_reg_name(get_used_reg_num(qr->src_op1->temp_entry), dummy, qr->src_op1->temp_entry);
//     }else if(popcount == 1){
//         if(whoistemp == 1) {
//             get_used_reg(qr->src_op1->temp_entry);
//             printf(", ");
//             get_used_reg(-1);
//         }else if(whoistemp == 2){
//             get_used_reg(-1);
//             printf(", ");
//             get_used_reg(qr->src_op2->temp_entry);
//         }
//     }else if(popcount == 2){
//         get_used_reg(-1);
//         printf(", ");
//         get_used_reg(-2);
//     }
//     clear_regs_for_pop();   // -1や-2なregsを0(unused)な状態にする
// }

// // 未使用にセット
// void init_registers_x86_64()
// {
//     for(int i = 0; i < 4; i++){
//         register_x86_64[i] = 0;
//     }
// }

// void clear_regs_for_pop()
// {
//     for(int i = 0; i < 4; i++){
//         if(register_x86_64[i] == -1 || register_x86_64[i] == -2) register_x86_64[i] = 0;
//     }
// }

// // 0: rax, 1: rcx, 2: rdx, 3:rbx
// void get_used_reg(int entry)
// {
//     for(int i = 0; i < 4; i++){
//         if(register_x86_64[i] == entry) {
//             switch(i){
//                 case 0:
//                     printf("%rax");
//                     break;
//                 case 1:
//                     printf("%rcx");
//                     break;
//                 case 2:
//                     printf("%rdx");
//                     break;
//                 case 3:
//                     printf("%rbx");
//                     break;
//                 default:
//                     break;
//             }
//         }
//     }
// }

// int get_used_reg_num(int entry)
// {
//     for(int i = 0; i < 4; i++){
//         if(register_x86_64[i] == entry) {
//             return i;
//         }
//     }
//     return -1;
// }

// int search_empty_reg()
// {
//     int is_empty = -1;
//     for(int i = 0; i < 4; i++){
//         if(register_x86_64[i] == 0) {
//             is_empty = i;
//         }
//     }
//     if(is_empty == -1) printf("[!] there're no regs available\n");
//     return is_empty;
// }


// // 0: rax, 1: rcx, 2: rdx, 3:rbx
// void get_empty_reg_name(int reg_num, char *dst_reg_name, int whoisuser)
// {
//     for(int i = 0; i < 5; i++){
//         dst_reg_name[i] = '\0';
//     }
//     switch(reg_num) {
//         case -1:
//             break;
//         case 0:
//             register_x86_64[0] = whoisuser;
//             strcpy(dst_reg_name, "%rax");
//             break;
//         case 1:
//             register_x86_64[1] = whoisuser;
//             strcpy(dst_reg_name, "%rcx");
//             break;
//         case 2:
//             register_x86_64[2] = whoisuser;
//             strcpy(dst_reg_name, "%rdx");
//             break;
//         case 3:
//             register_x86_64[3] = whoisuser;
//             strcpy(dst_reg_name, "%rbx");
//             break;
//         default:
//             break;
//     }
// }
