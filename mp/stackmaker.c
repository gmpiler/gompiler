#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>
#include "stackmaker.h"
#include "../common/token.h"

#define SIZE 256            // codeの1行あたりのMAX文字数

#if 1
#define DEBUG
#endif

int stacksize;

/**
 *  @brief ENTRY POINT
 */
void stackmaker(Funcs *func_head)
{
    printf("--- stack maker ---\n");

    stack_analysis_all_blocks(func_head);
}

void stack_analysis_all_blocks(Funcs *func_head)
{
    for(Funcs *func_iter = func_head; func_iter != NULL; func_iter = func_iter->next){
        /**
         * @brief ひとまず関数ごとにスタック確保するが，ゆくゆくブロック内のローカル変数は区別したい
         */
        func_iter->has_bytestack = 0;
        stacksize = 0;
        stack_analysis_blocks(func_iter->block_head);
        func_iter->has_bytestack = stacksize;
        printf(">> allocated %d byte(s) stack to function %s\n", func_iter->has_bytestack, func_iter->name);
    }
}

void stack_analysis_blocks(Block *block)
{
    for(Block *b = block; b != NULL; b = b->next){
        if(b->type == B_FOR || b->type == B_WHILE || b->type == B_IF) {
            stack_analysis_block(b);
            stack_analysis_blocks(b->inner);
        }else{
            stack_analysis_block(b);
        }
    }
}

void stack_analysis_block(Block *block)
{
    for(TOKENS *titer = block->token_head; titer != NULL; titer = titer->next){
        if(titer->next == NULL) continue;
        if(titer->next->next == NULL) continue;
        if(titer->type == TYPE && titer->next->type == VAR && titer->next->next->type == SEMICOLON) {
            char *declaretion_type[MAX_TOKENNAME_SIZE];
            strcpy(declaretion_type, titer->value);
            if(strcmp(declaretion_type, "int") == 0 || strcmp(declaretion_type, "float") == 0) {
                stacksize += 4;
            }else if(strcmp(declaretion_type, "char") == 0) {
                stacksize += 1;
            }else if(strcmp(declaretion_type, "double") == 0) {
                stacksize += 8;
            }else if(strcmp(declaretion_type, "half") == 0) {
                stacksize += 2;
            }
        }
    }
}