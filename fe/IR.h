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

typedef struct statement {
    enum stm_type type;
    struct tokens *token_head;
    struct statement *prev;
    struct statement *next;
};

typedef struct block {
    struct statement *stm_head;
    struct block *prev;
    struct block *next;
    struct block *inner;
    struct block *outer;
};

typedef struct func {
    char name[MAX_TOKENNAME_SIZE];
    struct block *block_head;
    struct statement *arg_head;
};

typedef struct sourcecode {
    struct func *func_head;
};

#endif