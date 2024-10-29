#ifndef IR_H
#define IR_H

char code[256][256];
#define MAX_TOKENNAME_SIZE 256    // MAX_TOKEN_SIZE

enum stm_type {
    S_NORMAL,
    S_FOR,
    S_WHILE,
    S_IF,
    S_COMMENT
};

enum func_type {
    F_ROOT,
    F_VOID,
    F_INT,
    F_FLOAT,
    F_DOUBLE,
    F_HALF,
    F_OTHERS
};

typedef struct statement {
    enum stm_type type;
    struct tokens *token_head;
    struct statement *prev;
    struct statement *next;
    struct statement *inner;
    struct statement *outer;
};

typedef struct func {
    enum func_type type;            // root以外は関数の型
    char name[MAX_TOKENNAME_SIZE];
    struct block *block_head;
    char arg[MAX_TOKENNAME_SIZE];   // ゆくゆく複数引数対応
    struct func *prev;
    struct func *next;
} FUNCS;

typedef struct block {
    struct statement *stm_head;
    struct block *prev;
    struct block *next;
};

#endif