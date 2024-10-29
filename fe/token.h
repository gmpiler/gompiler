#define MAX_TOKEN_LENGTH 256    // 1トークンあたりの最大文字数

typedef enum token_type {
/* --- 1発で確定できるもの --- */
    LPAREN,
    RPAREN,
    LBRACKET,
    RBRACKET,
    LBRACE,
    RBRACE,
    COLON,
    SEMICOLON,
    RET,            // return
    TYPE,           // int, float, double, void等
    OP,             // +, -, *, /等
    ASSIGN,         // =
    NUM,            // numbers
    FOR,
    WHILE,
    IF,

/* --- 後を読んで決定するもの --- */
    LATER,
    ARRAY,
    FUNC,
    VAR,

/* --- その他 --- */
    UNDEC
} TOKEN_TYPE;

typedef struct tokens {
    TOKEN_TYPE type;
    char value[MAX_TOKEN_LENGTH]; // 変数名はひとまず長さ256まで
    struct tokens *prev;
    struct tokens *next;
    int mark_as_decoded;
} TOKENS;