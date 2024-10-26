#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>
#include "decomposer.h"
#include "token.h"

#define SIZE 256            // codeの1行あたりのMAX文字数

#if 1
#define DEBUG
#endif

void init_codes(char code[][SIZE], int codenum);
void set_token(TOKENS *root_token, char* token, int key);
void set_token_type(TOKENS *node, char *token);
void append_token(TOKENS *root_token, TOKENS *new_node);
void tokenize_others(TOKENS *root_token);
int check_split(char c);
int is_type_token(char *token);
int is_num_token(char *token);

/**
 *  @brief ENTRY POINT
 *  modify codes here
 *  @return codeに中間言語を格納する
 */
void decomposer(char code[][SIZE], int codenum)
{
    init_codes(code, codenum);
    printf("--- decomposer ---\n");

    char spaces[] = " \t";  /* ' ' or '\t'*/

/* --- TOKENIZER(LEXER) --- */
    // spacesでトークンを切りだす
    int key = 0;
    TOKENS *root = (TOKENS*)malloc(sizeof(TOKENS));         // tokens双方向リストの先頭．ここに全てのトークンを格納
    for(int codeline = 0; codeline < codenum; codeline++){  // codeを1行ごと読み込む
        // code中の1番初めのtokenをrootとする
        char *token = strtok(code[codeline], spaces);

        // code中の2番目以降のtokenをrootにappendしていく
        while(token != NULL) {
            set_token(root, token, key);
            token = strtok(NULL, spaces);
            key = 1;
        }
    }

    // カッコやカンマ対応
    tokenize_others(root);

    // 切り出したトークンのタイプを設定する
    for(TOKENS *iter = root; iter != NULL; iter = iter->next){
        set_token_type(iter, iter->value);
    }

#ifdef DEBUG
    for(TOKENS *iter = root; iter != NULL; iter = iter->next){
        printf("token: <%s>, type: <%d>\n", iter->value, iter->type);
    }
#endif

/* --- TOKENIZER(PARSER) --- */

}

/**
 * @brief 文字cが分割対象文字かどうか
 * TODO: ここに，トークンを切り出したい文字を追加していけばよい
 */
int check_split(char c)
{
    int ret = 0;
    switch(c) {
        case '(':
            ret = 1;
            break;
        case ')':
            ret = 1;
            break;
        case ';':
            ret = 1;
            break;
        case '[':
            ret = 1;
            break;
        case ']':
            ret = 1;
            break;
        case '=':
            ret = 1;
            break;
        case '+':
            ret = 1;
            break;
        case '-':
            ret = 1;
            break;
        case '*':
            ret = 1;
            break;
        case '/':
            ret = 1;
            break;

/* --- ELSE --- */
        default:
            break;
    }
    return ret;
}

/**
 * @brief spaces以外でもトークンを分割
 */
void tokenize_others(TOKENS *root_token)
{
    for(TOKENS *iter = root_token; iter != NULL; iter = iter->next){
        for(int c = 0; c < MAX_TOKEN_LENGTH; c++){
            if(check_split(iter->value[c])) {   // もしiter->valueがsplit対象ならば(e.g. ';'を含むなど)
                TOKENS *first_token = (TOKENS*)malloc(sizeof(TOKENS));
                TOKENS *latter_token = (TOKENS*)malloc(sizeof(TOKENS));
                TOKENS *middle_token = (TOKENS*)malloc(sizeof(TOKENS));
                middle_token->value[0] = iter->value[c];

                char *first[MAX_TOKEN_LENGTH], latter[MAX_TOKEN_LENGTH];
                for(int init = 0; init < MAX_TOKEN_LENGTH; init++){
                    first[init] = latter[init] ='\0';
                }
                memcpy(first_token->value, iter->value, c);
                memcpy(latter_token->value, &iter->value[c + 1], MAX_TOKEN_LENGTH - c);

                if(first_token->value[0] == 0x0){       // firstがヌル文字の場合．"0;();;;"など区切り文字が連続する場合に生じるケースに対応
                    iter->prev->next = middle_token;
                    middle_token->prev = iter->prev;
                }else{
                    iter->prev->next = first_token;
                    first_token->prev = iter->prev;
                    first_token->next = middle_token;
                    middle_token->prev = first_token;
                }

                /**
                 * 例えば，トークンがmain(void)の場合．
                 *
                 * (i)
                 * value[c]が'('の場合，firstにはmain, middleには(, latterにはvoid)が入り，iter->prev => first => middle => latter => iter->nextとなる．
                 * latterにはまだ分割対象文字である')'が残っているから，最外側ループのiterでlatterから走査を始めるように調整する．(iter = latter_tolen->prev．prevなのは最外側ループでnextされるから)
                 *
                 * (ii)
                 * value[c]が今度は')'の場合，firstにはvoid, middleには), latterには''(== (char)0x0)が入り，iter->preb => first => middle => iter->nextとなる．
                 * latterには)以降のヌル文字が入る為，トークン「main(void)」の走査を終え，iter = iter->next(middle_token->next)から次のトークンの走査を行う．
                 */
                if(latter_token->value[0] == 0x0){      // (ii)のケース
                    middle_token->next = iter->next;
                    iter->next->prev = middle_token;

                    iter = middle_token;
                }else{                                  // (i)のケース
                    middle_token->next = latter_token;
                    latter_token->prev = middle_token;
                    latter_token->next = iter->next;
                    iter->next->prev = latter_token;

                    iter = latter_token->prev;
                }
                break;
            }
        }
    }
}

/**
 * @brief codelineの2個目以降のtokenに関して，dataとtypeを設定する．
 * また，root_tokenから辿り末尾にtokenをくっつける
 */
void set_token(TOKENS *root_token, char *token, int key)
{
    TOKENS *new_node = (TOKENS*)malloc(sizeof(TOKENS));
    strcpy(new_node->value, token);

    if(key == 0) { // rootの場合
        strcpy(root_token->value, new_node->value);
        root_token->next = NULL;
        root_token->prev = NULL;
    }else{
        append_token(root_token, new_node);
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
 * @brief ここに型の予約語を記載
 */
int is_type_token(char *token)
{
    if( (strcmp(token, "int") == 0) ||
        (strcmp(token, "float") == 0) ||
        (strcmp(token, "double") == 0) ||
        (strcmp(token, "void") == 0)
    ) {
        return 1;
    }else{
        return 0;
    }
}

int is_num_token(char *token)
{
    int violate = 0;

    for(int i = 0; i < MAX_TOKEN_LENGTH; i++){
        if(token[i] == 0x0) break;  // NULL character
        if(isdigit(token[i]) == 0) {
            violate++;
        }
    }
    return (violate == 0) ? 1 : 0;
}

/**
 * @brief tokenに応じて，nodeのtypeを設定することに責任を負う
 */
void set_token_type(TOKENS *token_node, char *token)
{
    if(is_type_token(token)) {
        token_node->type = TYPE;
        return;
    }
    if(is_num_token(token)) {
        token_node->type = NUM;
        return;
    }
    if(strcmp(token, "return") == 0) {
        token_node->type = RET;
        return;
    }

/* --- one character --- */
    char onechar = token[0];
    switch(onechar) {
        case '(':
            token_node->type = LPAREN;
            break;
        case ')':
            token_node->type = RPALEN;
            break;
        case '[':
            token_node->type = LBRACKET;
            break;
        case ']':
            token_node->type = RBRACKET;
            break;
        case '{':
            token_node->type = LBRACE;
            break;
        case '}':
            token_node->type = RBRACE;
            break;
        case ':':
            token_node->type = COLON;
            break;
        case ';':
            token_node->type = SEMICOLON;
            break;
        case '+':
            token_node->type = OP;
            break;
        case '-':
            token_node->type = OP;
            break;
        case '*':
            token_node->type = OP;
            break;
        case '/':
            token_node->type = OP;
            break;
        case '=':
            token_node->type = ASSIGN;
            break;
        default:
            // それ以外をfunc, array, varに割り当て
            if(token_node->next->type == LPAREN) {
                token_node->type = FUNC;
            }else if(token_node->next->type == LBRACKET) {
                token_node->type = ARRAY;
            }else{
                token_node->type = VAR;
            }
            break;
    }
}

/**
 * @brief codeから改行を削除する
 */
void init_codes(char code[][SIZE], int codenum)
{
    for(int codeline = 0; codeline < codenum; codeline++){
        for(int s = 0; s < SIZE; s++){
            if(code[codeline][s] == '\n' || code[codeline][s] == '\r' || code[codeline][s] == '\r\n') {
                code[codeline][s] = '\0';
            }
        }
    }
}
