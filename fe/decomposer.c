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
void append_func(Funcs *func_head, Funcs *new_func);
void set_func_type(Funcs *func, char *type);
void print_token_type(int type);

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

    // 切り出したトークンのタイプを設定する．ついでにエントリを登録
    int entry_no = 0;
    for(TOKENS *iter = root; iter != NULL; iter = iter->next){
        set_token_type(iter, iter->value);
        iter->entry = entry_no;
        entry_no++;
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
        printf("token: <%s>, \ttype: <", iter->value);
        print_token_type(iter->type);
        printf(">\n");
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
/* ---トークンの解析系 --- */
    TOKENS  *token_iter;
    Funcs   *func_head = (Funcs*)malloc(sizeof(Funcs));
    func_head->type = F_ROOT;
    /**
     * @brief 全てのトークンを解析する
     *
     * 例えば，以下関数のみデコード済の様子
     * <int> <main> <(> <void> <)> <{> <a> <=> <1> <;>
     *
     *  1      1     1    1     1   1   0   0   0   0
     */
    while(remains_to_be_analyzed(root)) { // 全てのトークンが解析済でなければ解析
        for(token_iter = root; token_iter != NULL; token_iter = token_iter->next){
            if(token_iter->mark_as_decoded == 1) continue; // 解析済のトークンはスキップ
    /* --- 以下，未解析のトークンに対する処理 --- */
            /* 解析情報 */
            int nest_level = 0;
            int current_token_type = token_iter->type;
            int next_token_type = (token_iter->next == NULL) ? -1 : token_iter->next->type;   // LL(1)

        /* --- 関数内の全てのトークンに対し，解析を行う --- */
            /**
             * @brief 関数宣言を検知した場合の処理
             * funcノードを作成し，各メンバを作りこんでいくと同時に，解析フラグの付与に責任を負う．
             *
             * "<TYPE> <FUNC> <(> <arguments...> <)> <{>"から，"<}>"までトークンを読んでいく
             *    ^
             *    |
             * func_token_iter
             */
            if(token_iter->type == TYPE && token_iter->next->type == FUNC) {
                Funcs *new_func = (Funcs*)malloc(sizeof(Funcs));
                set_func_type(new_func, token_iter->value);
                strcpy(new_func->name, token_iter->next->value);
                Block *root_block = (Block*)malloc(sizeof(Block));
                new_func->block_head = root_block;

                printf(">> In function <%s> ...\n", new_func->name);

                int last_level = 0;
                int level_changed = 0;
                for(TOKENS *func_token_iter = token_iter; func_token_iter != NULL; func_token_iter = func_token_iter->next) {
                    func_token_iter->mark_as_decoded = 1;
                    func_token_iter->level = nest_level;
#ifdef DEBUG
                    printf("[%d]: %s\n", func_token_iter->level, func_token_iter->value);
#endif
                    /* 以下，関数の<}>を検知したらbreakする処理 */
                    /**
                     * type func (args) {       <- level 0 (nest_level == 0)
                     *      for() {                 <- level 1
                     *                                  ...
                     *      }                       <- level 1
                     * }                        <- level 0
                     */
                    if(func_token_iter->type == LBRACE) {
                        nest_level++;
                        continue;
                    }
                    if(func_token_iter->type == RBRACE) {
                        if(nest_level == 1){
                            printf(">> detect the last part of function\n");
                            func_token_iter->mark_as_decoded = 1;  /* level 0な'}'を見つけた */
                            break;
                        }else if(nest_level > 1){
                            nest_level--;
                        }else{  // レベルがずれている
                            printf("the number of brace is illegal\n");
                            assert(1);
                        }
                    }

                    /* 引数対応処理 */
                    if(nest_level == 0 && (func_token_iter->type == LPAREN && func_token_iter->next->type != RPAREN)) {; // 初めての<(>の時で，"()"を除く
                        strcpy(new_func->arg, func_token_iter->next->value); // ひとまず(void)のみ対応
                    }

                    /* 関数内トークンの解析 */
                    if(nest_level >= 1) {    // 関数内のステートメントを表すトークンの解析
                        if(func_token_iter->level == last_level) {      // like 1 -> 1
                            level_changed = 0;
                        }else if(func_token_iter->level > last_level){  // like 1 -> 2
                            level_changed = 1;
                        }else{                                          // like 2 -> 1
                            level_changed = -1;
                        }
                        // set_all_tokens_to_block(root_block, func_token_iter);
                        // int func_stm = func_token_iter->type;
                        // /* 1文ずつstatementを構築し，解析フラグを付与する責任を負う */
                        // switch(func_stm) {
                        //     case TYPE:  // 宣言のみ
                        //         break;
                        //     case VAR:   // assign
                        //         break;
                        //     case ARRAY: // assign
                        //         break;
                        //     case IF:    // if
                        //         break;
                        //     case FOR:   // for
                        //         break;
                        //     case WHILE: // while
                        //         break;
                        //     case RET:   // return
                        //         break;
                        //     case FUNC:  // 関数の利用
                        //         break;
                        //     default:
                        //         // printf("[!] function %s contains illegal statement\n", new_func->name);
                        //         // assert(1);
                        //         break;
                        // }

                    }
                    last_level = func_token_iter->level;
                }
                append_func(func_head, new_func);
            }

/* --- TODO: 関数外に存在するトークンに対する解析(グローバル変数やマクロなど) --- */
        }
    }

}

/**
 * @brief デバッグ用．トークンタイプをデコードする．
 */
void print_token_type(int type)
{
    char decoded_string[MAX_TOKENNAME_SIZE];
    strcpy(decoded_string, "undec_type");

    switch(type) {
        case LPAREN:
            strcpy(decoded_string, "LPAREN");
            break;
        case RPAREN:
            strcpy(decoded_string, "RPAREN");
            break;
        case LBRACKET:
            strcpy(decoded_string, "LBRACKET");
            break;
        case RBRACKET:
            strcpy(decoded_string, "RBRACKET");
            break;
        case LBRACE:
            strcpy(decoded_string, "LBRACE");
            break;
        case RBRACE:
            strcpy(decoded_string, "RBRACE");
            break;
        case COLON:
            strcpy(decoded_string, "COLON");
            break;
        case SEMICOLON:
            strcpy(decoded_string, "SEMICOLON");
            break;
        case RET:
            strcpy(decoded_string, "RET");
            break;
        case TYPE:
            strcpy(decoded_string, "TYPE");
            break;
        case OP:
            strcpy(decoded_string, "OP");
            break;
        case ASSIGN:
            strcpy(decoded_string, "ASSIGN");
            break;
        case NUM:
            strcpy(decoded_string, "NUM");
            break;
        case FOR:
            strcpy(decoded_string, "FOR");
            break;
        case WHILE:
            strcpy(decoded_string, "WHILE");
            break;
        case IF:
            strcpy(decoded_string, "IF");
            break;
        case ELSE:
            strcpy(decoded_string, "ELSE");
            break;
        case LATER:
            strcpy(decoded_string, "LATER");
            break;
        case ARRAY:
            strcpy(decoded_string, "ARRAY");
            break;
        case FUNC:
            strcpy(decoded_string, "FUNC");
            break;
        case VAR:
            strcpy(decoded_string, "VAR");
            break;
        case LT:
            strcpy(decoded_string, "LT");
            break;
        case GT:
            strcpy(decoded_string, "GT");
            break;
        case UNDEC:
            strcpy(decoded_string, "UNDEC");
            break;
    }

    printf("%s", decoded_string);
}

void set_func_type(Funcs *func, char *type)
{
    if(strcmp(type, "void") == 0) {
        func->type = F_VOID;
    }else if(strcmp(type, "int") == 0){
        func->type = F_INT;
    }else if(strcmp(type, "float") == 0){
        func->type = F_FLOAT;
    }else if(strcmp(type, "double") == 0){
        func->type = F_DOUBLE;
    }else if(strcmp(type, "half") == 0){
        func->type = F_HALF;
    }else{
        func->type = F_OTHERS;
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
        case '<':
            ret = 1;
            break;
        case '>':
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
 * @brief funcs構造体をheadにappend
 */
void append_func(Funcs *func_head, Funcs *new_func)
{
    Funcs *iter = func_head;
    while(iter->next != NULL){
        iter = iter->next;
    }

    iter->next = new_func;
    new_func->prev = iter;
}

/**
 * @brief ここに型の予約語を記載
 */
int is_type_token(char *token)
{
    if( (strcmp(token, "int") == 0)     ||
        (strcmp(token, "float") == 0)   ||
        (strcmp(token, "double") == 0)  ||
        (strcmp(token, "void") == 0)    ||
        (strcmp(token, "half") == 0)
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
    if(strcmp(token, "else") == 0) {
        token_node->type = ELSE;
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
            token_node->type = RPAREN;
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
        case '<':
            token_node->type = LT;
            break;
        case '>':
            token_node->type = GT;
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
