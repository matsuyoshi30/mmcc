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

// consume the current token it it matches 'tk'
bool consume_tk(Tokenkind tk) {
    if (token->kind != tk)
        return false;
    token = token->next;
    return true;
}

// consume the current token if it is identifier
Token *consume_ident() {
    if (token->kind != TK_IDENT)
        return NULL;
    Token *tok = token;
    token = token->next;
    return tok;
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

bool is_alpha(char c) {
    return isalpha(c) || (c == '_');
}

bool is_alnum(char c) {
    return is_alpha(c) || isdigit(c);
}

char *is_reserved(char *c) {
    char *kw[] = {"return", "if", "else", "while", "for"};
    for (int i=0; i<sizeof(kw)/sizeof(*kw); i++) {
        int len = strlen(kw[i]);
        if (strncmp(c, kw[i], len) == 0 && !is_alnum(c[len]))
            return kw[i];
    }

    char *compOp[] = {"==", "!=", ">=", "<="};
    for (int i=0; i<sizeof(compOp)/sizeof(*compOp); i++) {
        int len = strlen(compOp[i]);
        if (strncmp(c, compOp[i], len) == 0)
            return compOp[i];
    }

    return NULL;
}

int num_of_digits(int n) {
    int cnt = 0;
    while (n) {
        n /= 10;
        cnt++;
    }
    return cnt;
}

void tokenize() {
    Token head;
    head.next = NULL;
    Token *cur = &head;

    char *p = user_input;
    while (*p) {
        if (isspace(*p)) {
            p++;
            continue;
        }

        char *reserved = is_reserved(p);
        if (reserved) {
            if (strcmp(reserved, "return") == 0)
                cur = new_token(TK_RETURN, cur, p, 6);
            else if (strcmp(reserved, "if") == 0)
                cur = new_token(TK_IF, cur, p, 2);
            else if (strcmp(reserved, "else") == 0)
                cur = new_token(TK_ELSE, cur, p, 4);
            else if (strcmp(reserved, "while") == 0)
                cur = new_token(TK_WHILE, cur, p, 5);
            else if (strcmp(reserved, "for") == 0)
                cur = new_token(TK_FOR, cur, p, 3);
            else
                cur = new_token(TK_RESERVED, cur, p, strlen(reserved));

            p += strlen(reserved);
            continue;
        }

        if (strchr("+-*/(){}><=;", *p)) {
            cur = new_token(TK_RESERVED, cur, p++, 1);
            continue;
        }

        if (is_alpha(*p)) {
            char *q = p;
            while (is_alnum(*p))
                p++;
            cur = new_token(TK_IDENT, cur, q, p-q);
            continue;
        }

        if (isdigit(*p)) {
            cur = new_token(TK_NUM, cur, p, 0);
            cur->val = strtol(p, &p, 10);
            cur->len = num_of_digits(cur->val);
            continue;
        }

        error_at(p, "unable tokenize\n");
    }

    new_token(TK_EOF, cur, p, 1);
    token = head.next;
}
