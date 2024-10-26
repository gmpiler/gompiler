#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "decomposer.h"
#include "token.h"

#define SIZE 256
#define MAX_LEX_SIZE 256

#if 1
#define DEBUG
#endif

void delete_crlf(char *code);
size_t split(char* s, const char* separator, char** result, size_t result_size);
void set_token(TOKENS *root_token, char* token);
void set_token_type(TOKENS *node, char *token);
void append_token(TOKENS *root_token, TOKENS *new_node);

/**
 *  @brief ENTRY POINT
 *  modify codes here
 *  @return codeに中間言語を格納する
 */
void decomposer(char code[][SIZE], int codenum)
{
    char *token; // codeから切り出す語句
    char token_for_split[] = " \n\t";  /* ' ' + ';' + '\n' */

    printf("--- decomposer ---\n");
    // codeを1行ごと読み込む
    for(int codeline = 0; codeline < codenum; codeline++){
#ifdef DEBUG
        printf("<%s>\n", code[codeline]);
#endif
        // 字句ごとに分ける
        token = strtok(code[codeline], token_for_split);
        TOKENS *root = malloc(sizeof(TOKENS));
        while(token != NULL){
            // TODO: ここでtokenを適当なデータ構造に保存
            printf("\ttoken is %s\n", token);
            set_token(root, token);
            token = strtok(NULL, token_for_split);
        }
#ifdef DEBUG
        for(TOKENS *iter = root; iter != NULL; iter = iter->next){
            printf("\t|%s| -> type: %d\n", iter->value, iter->type);
        }
#endif
    }
}

/**
 * @brief codelineの2個目以降のtokenに関して，dataとtypeを設定する．
 * また，root_tokenから辿り末尾にtokenをくっつける
 */
void set_token(TOKENS *root_token, char *token)
{
    if(root_token->value != NULL) { // root以外(codeの2番目以降のtoken)の時
        TOKENS *new_node = malloc(sizeof(TOKENS));
        strcpy(new_node->value, token);
        set_token_type(new_node, token);
        append_token(root_token, new_node);
        printf("[NODE] val: %s, type: %d\n", new_node->value, new_node->type);
    }else{ // root(codeの1番目のtoken)の時
        strcpy(root_token->value, token);
        set_token_type(root_token, token);
        root_token->next = NULL;
        root_token->prev = NULL;
        printf("[ROOT] val: %s, type: %d\n", root_token->value, root_token->type);
    }
}

/**
 * @brief root_tokenから辿り，末尾にnew_nodeをappendする
 */
void append_token(TOKENS *root_token, TOKENS *new_node)
{
    TOKENS *iter = root_token;
    while(iter->next != NULL){
        iter = iter->next;
    }

    iter->next = new_node;
    new_node->prev = iter;
}

/**
 * @brief tokenに応じて，nodeのtypeを設定することに責任を負う
 */
void set_token_type(TOKENS *token_node, char *token)
{
    char temp = token[0];
    switch(temp) {
        case '0':
            token_node->type = CONST;
            break;
        default:
            break;
    }
}

/**
 * @attention 現在は未使用．特定文字で分けたい時に使うかも．
 * @brief 1行を語句に分ける関数．外部関数．
 * @return 語句数
 */
size_t split(char* s, const char* separator, char** result, size_t result_size)
{
    assert(s != NULL);
    assert(separator != NULL);
    assert(result != NULL);
    assert(result_size > 0);

    size_t i = 0;

    char* p = strtok(s, separator);
    while (p != NULL) {
        assert(i < result_size);
        result[i] = p;
        ++i;

        p = strtok(NULL, separator);
    }

    return i;
}

/**
 * @brief 文字列の末尾にある改行を取り除く
 */
void delete_crlf(char *code)
{
    int c = 0;
    while(c < SIZE){
        if(code[c] == '\n') {
            code[c] = '\0';
            break;
        }
        c++;
    }
}
