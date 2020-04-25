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
    TK_NUM,
    TK_IF,
    TK_ELSE,
    TK_WHILE,
    TK_FOR,
    TK_RETURN,
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

bool consume(char *op);
bool consume_return();
bool consume_if();
bool consume_else();
bool consume_while();
bool consume_for();
Token *consume_ident();
void expect(char *op);
int expect_number();
bool at_eof();

Token *tokenize(char *p);

// Parser

typedef enum {
    ND_ADD,
    ND_SUB,
    ND_MUL,
    ND_DIV,
    ND_EQ,
    ND_NE,
    ND_MT, // more than >
    ND_LT, // less than <
    ND_OM, // or more >=
    ND_OL, // or less <=
    ND_AS, // assgin
    ND_LV, // local value
    ND_IF,
    ND_WHILE,
    ND_FOR,
    ND_RET,
    ND_NUM,
} Nodekind;

typedef struct Node Node;

struct Node {
    Nodekind kind;
    Node *lhs;
    Node *rhs;
    int val;    // use if kind == ND_NUM
    int offset; // use if kind == ND_LV
    Node *cond; // use if kind == ND_IF, ND_WHILE or ND_FOR
    Node *then;
    Node *els;
    Node *preop;  // use if kind == ND_FOR
    Node *postop; // use if kind == ND_FOR
};

typedef struct LVar LVar;

struct LVar {
    LVar *next;
    char *name;
    int len;
    int offset;
};

extern LVar *locals;
extern Node *code[100];

void program();

// Code generator

void gen(Node *node);
