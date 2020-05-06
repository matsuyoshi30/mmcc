#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdarg.h>
#include <string.h>

extern char *user_input;

void error(char *fmt, ...);
void error_at(char *loc, char *fmt, ...);

// Tokenizer

typedef enum {
    TK_RESERVED,
    TK_IDENT,
    TK_TYPE,
    TK_SIZEOF,
    TK_NUM,
    TK_EOF,
} Tokenkind;

typedef struct Token Token;

struct Token {
    Tokenkind kind;
    Token *next;
    int val;
    char *str;
    int len;
};

extern Token *token;

typedef enum {
    TY_INT,
    TY_PTR,
    TY_ARR,
} Typekind;

typedef struct Type Type;

struct Type {
    Typekind kind;
    int size;
    struct Type *ptr_to;
    size_t size_array;
};

bool consume(char *op);
bool peek(char *op);
bool consume_tk(Tokenkind tk);
Token *consume_ident();
Type *consume_type();
void expect(char *op);
int expect_number();
bool expect_type();
char *expect_ident();
bool at_eof();

void tokenize();

// Parser

typedef enum {
    ND_ADD,
    ND_SUB,
    ND_MUL,
    ND_DIV,
    ND_EQ, // ==
    ND_NE, // !=
    ND_MT, // >
    ND_LT, // <
    ND_OM, // >=
    ND_OL, // <=
    ND_AS, // =
    ND_LV, // local variable
    ND_GV, // global variable
    ND_ADDR,  // &
    ND_DEREF, // *
    ND_IF,
    ND_WHILE,
    ND_FOR,
    ND_BLOCK, // {...}
    ND_FUNC,
    ND_EXPR_STMT,
    ND_RET,
    ND_NUM,
} Nodekind;

typedef struct Var Var;

struct Var {
    Type *type;
    Var *next;
    char *name;
    int offset;

    bool is_local;
};

typedef struct Node Node;

struct Node {
    Nodekind kind;
    Type *type;
    Node *next;

    Node *lhs;
    Node *rhs;

    int val;    // for ND_NUM
    Var *var;

    Node *cond;
    Node *then;
    Node *els;
    Node *preop;
    Node *postop;

    Node *blocks; // for ND_BLOCK

    char *funcname;
    Node *args;
};

typedef struct Function Function;

struct Function {
    Function *next;
    Type *type;
    char *name;
    Var *params;
    Var *locals;
    Node *body;
};

extern Var *locals;
extern Var *globals;
extern Function *code;

void program();

// Code generator

void codegen();
