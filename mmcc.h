#define _GNU_SOURCE
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdarg.h>
#include <string.h>

extern char *user_input;

void error(char *fmt, ...);
void error_at(char *loc, char *fmt, ...);

void nop();

// Tokenizer

typedef enum {
    TK_RESERVED,
    TK_IDENT,
    TK_STR,
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
    int strlen;
};

extern Token *token;

typedef enum {
    TY_VOID,
    TY_CHAR,
    TY_SHORT,
    TY_INT,
    TY_LONG,
    TY_PTR,
    TY_ARR,
    TY_STRUCT,
    TY_ENUM,
} Typekind;

typedef struct Type Type;
typedef struct Member Member;

struct Type {
    Typekind kind;
    int size;
    struct Type *ptr_to;
    size_t size_array;

    char *name;

    Member *members; // for access struct members
};

bool consume(char *op);
bool peek(char *op);
Token *consume_ident();
Token *consume_str();
void expect(char *op);
int expect_number();
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
    ND_ADDR,  // &
    ND_DEREF, // *
    ND_NOT,   // !
    ND_LOGOR,  // ||
    ND_LOGAND, // &&
    ND_IF,
    ND_FOR,
    ND_BREAK,
    ND_CONT,
    ND_GOTO,
    ND_LABEL,
    ND_SWITCH,
    ND_CASE,
    ND_CAST,
    ND_BLOCK, // {...}
    ND_FUNC,
    ND_EXPR_STMT,
    ND_STMT_EXPR,
    ND_COMMA,
    ND_MEMBER,
    ND_RET,
    ND_STR,
    ND_NUM,
} Nodekind;

typedef struct Var Var;

struct Var {
    Type *type;
    Var *next;
    char *name;
    int offset;

    bool is_local;

    char *str;
    int lc;

    int enum_val; // for enum
};

typedef struct VarScope VarScope;

struct VarScope {
    VarScope *next;
    int depth;
    Var *var;
    char *name;
    Type *def_type;
};

typedef struct Tag Tag;

struct Tag {
    Tag *next;
    char *name;
    Type *type;
};

typedef struct TagScope TagScope;

struct TagScope {
    TagScope *next;
    int depth;
    Tag *tag;
};

struct Member {
    Member *next;
    Type *type;
    char *name;
    int offset;
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

    Node *blocks; // for ND_BLOCK or ND_STMT_EXPR

    Member *member; // for ND_MEMBER

    char *labelname;

    Node *next_case;
    Node *default_case;
    int case_label;

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
extern Var *strs;
extern Tag *tags;
extern Function *code;

void program();

// Code generator

void codegen();
