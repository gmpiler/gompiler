#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>
#include "decomposer.h"
#include "token.h"
#include "IR.h"

#define SIZE 256            // codeの1行あたりのMAX文字数

#if 1
#define DEBUG
#endif

void init_codes(char code[][SIZE], int codenum);
void set_token(TOKENS *root_token, char* token, int key);
void set_token_type(TOKENS *node, char *token);
void append_token(TOKENS *root_token, TOKENS *new_node);
void tokenize_others(TOKENS *root_token);
void analyze_tokens(TOKENS *root);
void init_analyze_token(TOKENS *root);
int remains_to_be_analyzed(TOKENS *root);
int check_split(char c);
int is_type_token(char *token);
int is_num_token(char *token);

/**
 *  @brief ENTRY POINT
 *  tokenize original codes here
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
    // 2回目
    for(TOKENS *iter = root; iter != NULL; iter = iter->next){
        /* --- 2回目，あとを読んで決定 --- */
        if(iter->type == LATER) {
            if(iter->next->type == LPAREN) {
                iter->type = FUNC;
            }else if(iter->next->type == LBRACKET) {
                iter->type = ARRAY;
            }else{
                iter->type = VAR;
            }
        }
    }


#ifdef DEBUG
    for(TOKENS *iter = root; iter != NULL; iter = iter->next){
        printf("token: <%s>, type: <%d>\n", iter->value, iter->type);
    }
    printf("\n");
#endif

/* --- TOKENIZER(PARSER) --- */
    analyze_tokens(root);

}

/**
 * @brief entry point of parser
 */
void analyze_tokens(TOKENS *root)
{
    init_analyze_token(root);
    printf("--- tokens_analyzer ---\n");

#ifdef DEBUG
    for(TOKENS *iter = root; iter != NULL; iter = iter->next){
        printf(" '%s' ", iter->value);
        if(strcmp(iter->value, ";") == 0 || strcmp(iter->value, "{") == 0 || strcmp(iter->value, "}") == 0) printf("\n");
    }
    printf("\n");
#endif

/* --- main関数単体を解析済にする仮の処理 --- */
    /* TODO: 暫定的にmain関数のみかつ関数内に関数がないと仮定 */
    // struct func *mainfunc = (struct func*)malloc(sizeof(struct func));
    // mainfunc->arg_head = (struct statement*)malloc(sizeof(struct statement));
    int search_func_paren = 0;
    for(TOKENS *iter = root; iter != NULL; iter = iter->next){
        printf(">> @Token %s(%d), ...\n", iter->value, iter->mark_as_decoded);
        if(iter->mark_as_decoded == 1) continue;
        int ty = iter->type;
        int nextty = (iter->next == NULL) ? -1 : iter->next->type;
        if(ty == TYPE && nextty == FUNC) {
            printf("func detected\n");
            iter->mark_as_decoded = 1;                  // int
            iter->next->mark_as_decoded = 1;            // main
            // strcpy(mainfunc->name, iter->next->value);
            /* 関数の場合，'{'まで各情報を解析．"int main(void) {"など */
            for(TOKENS *funciter = iter->next->next; funciter != NULL; funciter = funciter->next){
                printf("funciter %s\n", funciter->value);
                funciter->mark_as_decoded = 1;
                if(funciter->type == LBRACE) break;

                // if(funciter->type == TYPE) strcpy(mainfunc->arg_head->token_head->value, funciter); // (void)
            }
        }
        if(iter->type == LBRACE) {
            search_func_paren++;
            printf("detect { , %d\n", search_func_paren);
        }
        if(iter->type == RBRACE) {
            printf("sfp: %d\n", search_func_paren);
            if(search_func_paren == 0){
                iter->mark_as_decoded = 1;  /* int main(void){ '}'を見つけた */
                printf("detect final }, %d\n", search_func_paren);
                break;
            }else{
                search_func_paren--;
                printf("detect } , %d\n", search_func_paren);
            }
        }
    }
    printf("main decoded\n");

/* ---本来の解析系 --- */
    TOKENS *token_iter;
    while(remains_to_be_analyzed(root)) { // 全てのトークンが解析済でなければ解析
        for(token_iter = root; token_iter != NULL; token_iter = token_iter->next){
            if(token_iter->mark_as_decoded == 1) continue; // 解析済のトークンはスキップ
            int current_token_type = token_iter->type;
            int next_token_type = token_iter->next->type;   // LL(1)

/* --- 関数内の全てのトークンに対し，解析を行う --- */
            // TODO: 本当は関数ごとに解析を行いたい．関数xを見つけたら，関数内が全てmark_as_decodedになるまで解析し，x内のデータ構造に格納していく
            // TODO: 暫定的にmain関数のみと仮定し，関数内の要素を解析していく
            switch(current_token_type){
                case TYPE:
                    // if(next_token_type == FUNC)
                    if(next_token_type == VAR)
                    if(next_token_type == ARRAY)
                    break;
                case FOR:   // for文
                    break;
                case WHILE: // while文
                    break;
                case IF:    // if文
                    break;
                case RET:   // return文
                    break;
                default:    // normal文(assign, declare等)
                    break;
            }
        }
    }

}

/**
 * @brief mark_as_decodedが0のものが残って入れば1
 */
int remains_to_be_analyzed(TOKENS *root)
{
    int remain = 0; // mark_as_decodedが0であるトークンの数
    for(TOKENS *token_iter = root; token_iter != NULL; token_iter = token_iter->next){
        if(token_iter->mark_as_decoded != 1) remain++;
    }

    return (remain == 0) ? 0 : 1;
}

/**
 * @brief 各トークンを未解析状態に設定
 */
void init_analyze_token(TOKENS *root)
{
    for(TOKENS *token_iter = root; token_iter != NULL; token_iter = token_iter->next){
        token_iter->mark_as_decoded = 0;
    }
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
    if(strcmp(token, "for") == 0) {
        token_node->type = FOR;
        return;
    }
    if(strcmp(token, "if") == 0) {
        token_node->type = IF;
        return;
    }
    if(strcmp(token, "while") == 0) {
        token_node->type = WHILE;
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
            token_node->type = LATER;
            // 後でfunc, array, varに割り当て
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
