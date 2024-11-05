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

enum block_type {
    B_ROOT,
    B_BASIC,
    B_IF,
    B_FOR,
    B_WHILE,
    B_OTHERS
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
} Funcs;

typedef struct block {
    enum   block_type type;
    int    level;                   // 階層情報
    struct statement *stm_head;
    struct tokens *token_head;
    struct tokens *expr_head;
    struct AST_Node_List *ast_head;
    struct block *prev;
    struct block *next;
    struct block *inner;
    struct block *outer;
    struct func  *func_head;
} Block;

/* --- for parser --- */
typedef enum {
    AST_ADD,
    AST_SUB,
    AST_MUL,
    AST_DIV,
    AST_NUM,
    AST_VAR,
    AST_ASSIGN
} AST_Node_Kind;

typedef struct AST_Node AST_Node;

struct AST_Node {
    AST_Node_Kind kind;
    AST_Node *left;
    AST_Node *right;
    int value;
    char var[MAX_TOKENNAME_SIZE];
};

typedef struct AST_Node_List AST_Node_List;

struct AST_Node_List {
    struct AST_Node *data;
    struct AST_Node_List *prev;
    struct AST_Node_List *next;
};


#endif