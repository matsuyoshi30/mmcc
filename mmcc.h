#define _GNU_SOURCE
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdarg.h>
#include <string.h>

void nop();

void error(char *fmt, ...);
void error_at(char *loc, char *fmt, ...);

char *read_file(char *path);

// Tokenizer

typedef enum {
    TK_RESERVED,
    TK_IDENT, // identifier
    TK_STR,   // string literal
    TK_NUM,   // integer literal
    TK_EOF,   // end-of-file
} Tokenkind;

typedef struct Token Token;
struct Token {
    Tokenkind kind; // token kind
    Token *next;    // next token
    long val;        // value (TK_NUM)
    char *str;      // token string
    int len;        // token length
    int strlen;     // string literal length
};

extern Token *token;

bool consume(char *op);
bool peek(char *op);
bool peek_end();
Token *consume_ident();
Token *consume_str();
void expect(char *op);
long expect_number();
bool at_eof();

void tokenize(char *filename, char *input);

// type

typedef enum {
    TY_VOID,
    TY_CHAR,
    TY_SHORT,
    TY_INT,
    TY_LONG,
    TY_BOOL,
    TY_PTR,
    TY_ARR,
    TY_STRUCT,
    TY_ENUM,
} Typekind;

typedef struct Type Type;
typedef struct Member Member;

struct Type {
    Typekind kind;       // type kind
    int size;            // type size
    struct Type *ptr_to; // pointer or array
    int size_array;      // array
    bool is_incomplete;  // incomplete type or not

    // declaration
    char *name;

    // access struct members
    Member *members;
};

// Parser

typedef struct Initializer Initializer;
struct Initializer {
    Initializer *next;

    // constant expression
    int size;
    long val;

    // reference to another global variable
    char *label;
};

typedef struct Var Var;
struct Var {
    Type *type;   // variable type
    Var *next;    // next variable
    char *name;   // variable name
    bool is_local;
    bool is_static;

    // local variable
    int offset;

    // global variable
    Initializer *initializer;

    // string literal
    char *str;
    int lc;

    // enum
    int enum_val;
};

typedef enum {
    ND_ADD,       // +
    ND_SUB,       // -
    ND_MUL,       // *
    ND_DIV,       // /
    ND_MOD,       // %
    ND_EQ,        // ==
    ND_NE,        // !=
    ND_MT,        // >
    ND_LT,        // <
    ND_OM,        // >=
    ND_OL,        // <=
    ND_AS,        // =
    ND_LV,        // local variable
    ND_ADDR,      // &
    ND_DEREF,     // *
    ND_NOT,       // !
    ND_LOGOR,     // ||
    ND_LOGAND,    // &&
    ND_IF,        // "if"
    ND_FOR,       // "for"
    ND_BREAK,     // "break"
    ND_CONT,      // "continue"
    ND_GOTO,      // "goto"
    ND_LABEL,     // labelded statement
    ND_SWITCH,    // "switch"
    ND_CASE,      // "case"
    ND_CAST,      // type cast
    ND_COND,      // ?:
    ND_BLOCK,     // {...}
    ND_FUNC,      // function
    ND_EXPR_STMT, // expression statement
    ND_STMT_EXPR, // statement expression
    ND_NULL_EXPR, // empty statement
    ND_COMMA,     // ,
    ND_MEMBER,    // struct member
    ND_RET,       // "return"
    ND_STR,       // string
    ND_NUM,       // number
} Nodekind;

typedef struct VarScope VarScope;
struct VarScope {
    VarScope *next; // next variable scope
    int depth;      // variable scope depth
    Var *var;
    char *name;
    Type *def_type;
};

typedef struct Tag Tag;
struct Tag {
    Tag *next;  // next tag
    char *name; // tag name
    Type *type; // tag type
};

typedef struct TagScope TagScope;
struct TagScope {
    TagScope *next; // next tag scope
    int depth;      // tag scope depth
    Tag *tag;
};

struct Member {
    Member *next; // next member
    Type *type;   // member type
    char *name;   // member name
    int offset;   // member offset
};

typedef struct Node Node;
struct Node {
    Nodekind kind; // node kind
    Type *type;    // node type
    Node *next;    // next node

    Node *lhs;     // left-hand side node
    Node *rhs;     // right-hand side node

    // integer
    long val;

    // variable
    Var *var;

    // "if", "for" or "while" statement
    Node *cond;
    Node *then;
    Node *els;
    Node *preop;
    Node *postop;

    // { ... } or statement expression
    Node *blocks;

    // struct member
    Member *member;

    // "goto" or labeled statement
    char *labelname;

    // switch-case
    Node *next_case;
    Node *default_case;
    int case_label;

    // function
    char *funcname;
    Node *args;
};

typedef struct Function Function;
struct Function {
    Function *next;
    Type *type;
    bool is_static;
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
