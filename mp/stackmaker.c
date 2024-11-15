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
void stack_analysis_all_blocks(Funcs *func_head);
void stack_analysis_blocks(Block *block);
void stack_analysis_block(Block *block);
void append_stack(struct Stack_List *stack_head, struct stack *new_stack_region);
void set_offsets(struct Stack_List *stack_head);

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
        func_iter->stack_head = (struct Stack_List*)malloc(sizeof(struct Stack_List));
        func_iter->stack_head->data = (struct stack*)malloc(sizeof(struct stack));
        func_iter->stack_head->data->byte = 0;
        stack_analysis_blocks(func_iter->block_head);
        func_iter->has_bytestack = stacksize;
        set_offsets(func_iter->stack_head);
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
                struct stack *new_stack_region = (struct stack*)malloc(sizeof(struct stack));
                strcpy(new_stack_region->symbol_name, titer->next->value);
                new_stack_region->byte = 4;
                append_stack(block->func->stack_head, new_stack_region);
                stacksize += 4;
            }else if(strcmp(declaretion_type, "char") == 0) {
                struct stack *new_stack_region = (struct stack*)malloc(sizeof(struct stack));
                strcpy(new_stack_region->symbol_name, titer->next->value);
                new_stack_region->byte = 1;
                append_stack(block->func->stack_head, new_stack_region);
                stacksize += 1;
            }else if(strcmp(declaretion_type, "double") == 0) {
                struct stack *new_stack_region = (struct stack*)malloc(sizeof(struct stack));
                strcpy(new_stack_region->symbol_name, titer->next->value);
                new_stack_region->byte = 8;
                append_stack(block->func->stack_head, new_stack_region);
                stacksize += 8;
            }else if(strcmp(declaretion_type, "half") == 0) {
                struct stack *new_stack_region = (struct stack*)malloc(sizeof(struct stack));
                strcpy(new_stack_region->symbol_name, titer->next->value);
                new_stack_region->byte = 2;
                append_stack(block->func->stack_head, new_stack_region);
                stacksize += 2;
            }
        }
    }
}

void append_stack(struct Stack_List *stack_head, struct stack *new_stack_region)
{
    if(stack_head->data->byte == 0) {
        stack_head->data = new_stack_region;
        stack_head->next = NULL;
    }else{
        for(struct Stack_List *siter = stack_head; siter != NULL; siter = siter->next){
            if(siter->next == NULL) {
                siter->next = (struct Stack_List*)malloc(sizeof(struct Stack_List));
                siter->next->data = new_stack_region;
                siter->next->prev = siter;
                break;
            }
        }
    }
}

void set_offsets(struct Stack_List *stack_head)
{
    int offset_byte = 0;
    for(struct Stack_List *siter = stack_head; siter != NULL; siter = siter->next){
        siter->data->offset = offset_byte;
        offset_byte -= siter->data->byte;
    }
}

