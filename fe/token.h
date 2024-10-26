#define MAX_TOKEN_LENGTH 256

typedef enum token_type {
    VAR,
    CONST,
    ARRAY,
    FUNC,
    RETURN,
    INT
} TOKEN_TYPE;

typedef struct tokens {
    TOKEN_TYPE type;
    char value[MAX_TOKEN_LENGTH]; // 変数名はひとまず長さ256まで
    struct tokens *prev;
    struct tokens *next;
} TOKENS;