#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdarg.h>
#include <string.h>

extern char *user_input;

void error_at(char *loc, char *fmt, ...);

// Tokenizer

typedef enum {
    TK_RESERVED,
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
    ND_NUM,
} Nodekind;

typedef struct Node Node;

struct Node {
    Nodekind kind;
    Node *lhs;
    Node *rhs;
    int val;
};

Node *expr();

// Code generator

void gen(Node *node);
