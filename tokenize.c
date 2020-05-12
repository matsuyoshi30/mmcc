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

// check the current token if it matches 'op'
bool peek(char *op) {
    if (token->len != strlen(op) || memcmp(token->str, op, token->len))
        return false;
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

// consume the current token if it is string
Token *consume_str() {
    if (token->kind != TK_STR)
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

// expect the current token if it is type word
char *expect_type() {
    if (token->kind != TK_TYPE)
        error_at(token->str, "expected type word but got '%s'\n", token->str);
    char *typeword = strndup(token->str, token->len);
    token = token->next;
    return typeword;
}

// check whether the current token is ident
char *expect_ident() {
    if (token->kind != TK_IDENT)
        error_at(token->str, "expected identifier but got '%s'\n", token->str);
    char *name = strndup(token->str, token->len);
    token = token->next;
    return name;
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
    char *kw[] = {"return", "if", "else", "while", "for", "sizeof"};
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

char *is_type(char *c) {
    char *tw[] = {"int", "char"};
    for (int i=0; i<sizeof(tw)/sizeof(*tw); i++) {
        int len = strlen(tw[i]);
        if (strncmp(c, tw[i], len) == 0 && !is_alnum(c[len]))
            return tw[i];
    }

    return NULL;
}

char *is_assign(char *c) {
    char *ops[] = {"+=", "-=", "*=", "/="};

    for (int i=0; i<sizeof(ops)/sizeof(*ops); i++) {
        int len = strlen(ops[i]);
        if (strncmp(c, ops[i], len) == 0)
            return ops[i];
    }

    return NULL;
}

char is_escape(char *c) {
    switch (*c) {
    case 'a':
        return '\a';
    case 'b':
        return '\b';
    case 'f':
        return '\f';
    case 'r':
        return '\r';
    case 't':
        return '\t';
    case 'v':
        return '\v';
    }

    return 0;
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

        // skip line comment
        if (strncmp(p, "//", 2) == 0) {
            p += 2;
            while (*p != '\n')
                p++;
            continue;
        }

        // skip block comment
        if (strncmp(p, "/*", 2) == 0) {
            char *q = strstr(p+2, "*/");
            if (!q)
                error_at(p, "comments are not closed");
            p = q + 2;
            continue;
        }

        char *reserved = is_reserved(p);
        if (reserved) {
            cur = new_token(TK_RESERVED, cur, p, strlen(reserved));
            p += strlen(reserved);
            continue;
        }

        char *typeword = is_type(p);
        if (typeword) {
            cur = new_token(TK_TYPE, cur, p, strlen(typeword));
            p += strlen(typeword);
            continue;
        }

        char *assignment = is_assign(p);
        if (assignment) {
            cur = new_token(TK_RESERVED, cur, p, strlen(assignment));
            p += strlen(assignment);
            continue;
        }

        if (strchr("+-*/(){}[]><=,;&*", *p)) {
            if (strncmp(p,  "++", 2) == 0 || strncmp(p, "--", 2) == 0) {
                cur = new_token(TK_RESERVED, cur, p, 2);
                p += 2;
            } else {
                cur = new_token(TK_RESERVED, cur, p++, 1);
            }
            continue;
        }

        if (*p == '"') {
            char *start = p + 1;
            char *end = start;

            while (*end && *end != '"') {
                if (*end == '\\')
                    end++;
                end++;
            }

            char *buf = malloc(end-start+1);
            int len = 0;

            while (*start != '"') {
                if (*start == '\\') {
                    start++;
                    char es = is_escape(start);
                    if (es) {
                        buf[len++] = es;
                        start++;
                    } else {
                        buf[len++] = '\\';
                        buf[len++] = *start++;
                    }
                } else {
                    buf[len++] = *start++;
                }
            }
            buf[len++] = '\0';

            cur = new_token(TK_STR, cur, buf, end-p);
            cur->strlen = len;
            p += end-p+1;
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
