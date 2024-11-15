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
    int has_bytestack;              // ゆくゆく各ブロックに設置
    struct Stack_List *stack_head;
} Funcs;

typedef struct block {
    enum   block_type type;
    int    level;                   // 階層情報
    struct statement *stm_head;
    struct tokens *token_head;
    struct tokens *expr_head;
    struct AST_Node_List *ast_head;
    struct Quadruple_List *qr_head;
    struct block *prev;
    struct block *next;
    struct block *inner;
    struct block *outer;
    struct func  *func_head;
    struct func  *func;
} Block;

struct stack {
    char symbol_name[MAX_TOKENNAME_SIZE];
    int byte;
    int offset;
};

struct Stack_List {
    struct stack *data;
    struct Stack_List *prev;
    struct Stack_List *next;
};

/* --- for parser --- */
typedef enum {
    AST_ADD,
    AST_SUB,
    AST_MUL,
    AST_DIV,
    AST_NUM,
    AST_VAR,
    AST_ASSIGN,
    AST_RET
} AST_Node_Kind;

typedef struct AST_Node AST_Node;

struct AST_Node {
    AST_Node_Kind kind;
    AST_Node *left;
    AST_Node *right;
    int value;
    char var[MAX_TOKENNAME_SIZE];
    int temp_entry; // あるassignの中で，kindが非終端記号かつtemp_entryが同一ならば，同じレジスタ
};

typedef struct AST_Node_List AST_Node_List;

struct AST_Node_List {
    struct AST_Node *data;
    struct AST_Node_List *prev;
    struct AST_Node_List *next;
};

struct Quadruple_List {
    struct Quadruple *data;
    struct Quadruple_List *prev;
    struct Quadruple_List *next;
};

typedef struct Quadruple_List Quadruple_List;

typedef enum {
    OPE_ADD,
    OPE_SUB,
    OPE_MUL,
    OPE_DIV,
    OPE_ASSIGN
} Operator_Kind;

struct Quadruple {
    Operator_Kind type;
    struct Operand *dst_op;
    struct Operand *src_op1;
    struct Operand *src_op2;
};

typedef struct Quadruple Quadruple;

enum Operand_Kind {
    OP_TEMP,
    OP_VAR,
    OP_NUM,
    OP_ARRAY
};

struct Operand {
    enum Operand_Kind type;
    int num;
    char value[MAX_TOKENNAME_SIZE];
    int temp_entry;
};

typedef struct Operand Operand;

#endif