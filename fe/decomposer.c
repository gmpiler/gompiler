#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "decomposer.h"

#define SIZE 256
#define MAX_LEX_SIZE 256

#if 1
#define DEBUG
#endif

size_t split(char* s, const char* separator, char** result, size_t result_size);

/**
 *  @brief ENTRY POINT
 *  modify codes here
 *  @return codeに中間言語を格納する
 */
void decomposer(char code[][SIZE], int codenum)
{
    char *token;
    char token_for_split[] = " ;\n";  /* ' ' + ';' + '\n' */

    printf("--- decomposer ---\n");
    // codeを1行ごと読み込む
    for(int codeline = 0; codeline < codenum; codeline++){
        // 字句ごとに分ける
        token = strtok(code[codeline], token_for_split);
        while(token != NULL){
            printf("%s\n", token);
            // TODO: ここでtokenを適当なデータ構造に保存
            token = strtok(NULL, token_for_split);
        }
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
