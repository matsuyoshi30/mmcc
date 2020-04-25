#include "mmcc.h"

// Tokenizer
Token *token;

// consume the current token if it matches 'op'
bool consume(char *op) {
    if (token->kind != TK_RESERVED || token->len != strlen(op) || memcmp(token->str, op, token->len))
        return false;
    token = token->next;
    return true;
}

// consume the current token if it is identifier
Token *consume_ident() {
    if (token->kind != TK_IDENT)
        return NULL;
    token = token->next;
    return token;
}

// check whether the current token matches 'op'
void expect(char *op) {
    if (token->kind != TK_RESERVED || token->len != strlen(op) || memcmp(token->str, op, token->len))
        error_at(token->str, "expected '%s' but got '%s'\n", op, token->str);
    token = token->next;
}

// check whether the current token is number
int expect_number() {
    if (token->kind != TK_NUM)
        error_at(token->str, "expected number but got '%s'\n", token->str);
    int val = token->val;
    token = token->next;
    return val;
}

// check whether the current token is EOF
bool at_eof() {
    return token->kind == TK_EOF;
}

Token *new_token(Tokenkind kind, Token *cur, char *str, int len) {
    Token *tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->str = str;
    tok->len = len;
    cur->next = tok;
    return tok;
}

int num_of_digits(int n) {
    int cnt = 0;
    while (n) {
        n /= 10;
        cnt++;
    }
    return cnt;
}

Token *tokenize(char *p) {
    Token head;
    head.next = NULL;
    Token *cur = &head;

    while (*p) {
        if (isspace(*p)) {
            p++;
            continue;
        }

        if (*p == '>' || *p == '<') {
            char *q = p;
            p++;
            if (*p == '=') {
                cur = new_token(TK_RESERVED, cur, q, 2);
                p++;
                continue;
            } else {
                cur = new_token(TK_RESERVED, cur, q, 1);
                continue;
            }
        }

        if (*p == '=' || *p == '!') {
            char *q = p;
            p++;
            if (*p == '=') {
                cur = new_token(TK_RESERVED, cur, q, 2);
                p++;
                continue;
            } else {
                if (*q == '!') {
                    error_at(p, "unable tokenize\n");
                } else {
                    cur = new_token(TK_RESERVED, cur, q, 1);
                    continue;
                }
            }
        }

        if (*p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '(' || *p == ')') {
            cur = new_token(TK_RESERVED, cur, p++, 1);
            continue;
        }

        if ('a' <= *p && *p <= 'z') {
            int len = 0;
            while (isalpha(*p)) {
                p++;
                len++;
            }
            cur = new_token(TK_IDENT, cur, p, len);
            continue;
        }

        if (isdigit(*p)) {
            cur = new_token(TK_NUM, cur, p, 0);
            cur->val = strtol(p, &p, 10);
            cur->len = num_of_digits(cur->val);
            continue;
        }

        if (*p == ';') {
            cur = new_token(TK_RESERVED, cur, p++, 1);
            continue;
        }

        error_at(p, "unable tokenize\n");
    }

    new_token(TK_EOF, cur, p, 1);
    return head.next;
}
