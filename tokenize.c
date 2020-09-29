#include "mmcc.h"

static char *cur_filename;
static char *cur_input;

void error(char *fmt, ...) {
    va_list arg;
    va_start(arg, fmt);

    vfprintf(stderr, fmt, arg);
    fprintf(stderr, "\n");
    exit(1);
}

// report an error message in the following format and exit
void verror_at(int line_num, char *loc, char *fmt, ...) {
    va_list arg;
    va_start(arg, fmt);

    char *line = loc;
    while (cur_input < line && line[-1] != '\n')
        line--;

    char *end = loc;
    while (*end != '\n')
        end++;

    int indent = fprintf(stderr, "%s:%d: ", cur_filename, line_num);
    fprintf(stderr, "%.*s\n", (int)(end - line), line);

    int pos = loc - line + indent;
    fprintf(stderr, "%*s", pos, " ");
    fprintf(stderr, "^ ");
    vfprintf(stderr, fmt, arg);
    fprintf(stderr, "\n");
    exit(1);
}

void error_at(char *loc, char *fmt, ...) {
    int line_no = 1;
    for (char *p=cur_input; p<loc; p++)
        if (*p == '\n')
            line_no++;

    va_list arg;
    va_start(arg, fmt);
    verror_at(line_no, loc, fmt, arg);
}

void error_tok(Token *tok, char *fmt, ...) {
    va_list arg;
    va_start(arg, fmt);
    verror_at(tok->line_no, tok->loc, fmt, arg);
}

char *read_file(char *path) {
    FILE *fp;

    if (strcmp(path, "-") == 0) {
        fp = stdin;
    } else {
        fp = fopen(path, "r");
        if (!fp)
            error("cannot open %s: %s\n", path, strerror(errno));
    }

    int buflen = 4096;
    int nread = 0;
    char *buf = malloc(buflen);

    // Read the entire file.
    for (;;) {
        int end = buflen - 2; // extra 2 bytes for the trailing "\n\0"
        int n = fread(buf + nread, 1, end - nread, fp);
        if (n == 0)
            break;
        nread += n;
        if (nread == end) {
            buflen *= 2;
            buf = realloc(buf, buflen);
        }
    }

    if (fp != stdin)
        fclose(fp);

    if (nread == 0 || buf[nread-1] != '\n')
        buf[nread++] = '\n';
    buf[nread] = '\0';

    return buf;
}

/// Tokenizer

Token *token;

// consume the current token if it matches 'op'
Token *consume(char *op) {
    if (token->kind != TK_RESERVED || token->len != strlen(op) || memcmp(token->str, op, token->len))
        return NULL;
    Token *tok = token;
    token = token->next;
    return tok;
}

// check the current token if it matches 'op'
bool peek(char *op) {
    if (token->len != strlen(op) || memcmp(token->str, op, token->len))
        return false;
    return true;
}

// check the current token if it matches list end
bool peek_end() {
    Token *tok = token;
    bool ret = consume("}") || (consume(",") && consume("}"));
    token = tok;
    return ret;
}

// consume the current token if it is identifier
Token *consume_ident() {
    if (token->kind != TK_IDENT)
        return NULL;
    Token *tok = token;
    token = token->next;
    return tok;
}

// consume the current token if it is string literal
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
        error_tok(token, "expected '%s'\n", op);
    token = token->next;
}

// check whether the current token is number
long expect_number() {
    if (token->kind != TK_NUM)
        error_tok(token, "expected number\n");
    long val = token->val;
    token = token->next;
    return val;
}

// check whether the current token is EOF
bool at_eof() {
    return token->kind == TK_EOF;
}

Token *new_token(Tokenkind kind, Token *cur, char *loc, char *str, int len) {
    Token *tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->loc = loc;
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
    char *kw[] = {"return", "if", "else", "while", "for", "do",
                  "switch", "case", "default", "break", "continue", "goto",
                  "sizeof", "typedef", "static", "extern", "..."};
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

    char *logicOp[] = {"&&", "||"};
    for (int i=0; i<sizeof(logicOp)/sizeof(*logicOp); i++) {
        int len = strlen(logicOp[i]);
        if (strncmp(c, logicOp[i], len) == 0)
            return logicOp[i];
    }

    return NULL;
}

char *is_type(char *c) {
    char *tw[] = {"int", "char", "short", "long", "_Bool", "struct", "void", "enum"};
    for (int i=0; i<sizeof(tw)/sizeof(*tw); i++) {
        int len = strlen(tw[i]);
        if (strncmp(c, tw[i], len) == 0 && !is_alnum(c[len]))
            return tw[i];
    }

    return NULL;
}

char *is_assign(char *c) {
    char *ops[] = {"+=", "-=", "*=", "/=", "%="};

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
    case 'n':
        return '\n';
    case 'r':
        return '\r';
    case 't':
        return '\t';
    case 'v':
        return '\v';
    default:
        return *c;
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

// initialize line number information for all tokens
void add_line_numbers(Token *tok) {
    char *p = cur_input;
    int n = 1;

    do {
        if (p == tok->loc) {
            tok->line_no = n;
            tok = tok->next;
        }

        if (*p == '\n')
            n++;
    } while (*p++);
}

void tokenize(char *filename, char *input) {
    cur_filename = filename;
    cur_input = input;

    Token head;
    head.next = NULL;
    Token *cur = &head;

    char *p = input;
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
            cur = new_token(TK_RESERVED, cur, p, p, strlen(reserved));
            p += strlen(reserved);
            continue;
        }

        char *typeword = is_type(p);
        if (typeword) {
            cur = new_token(TK_RESERVED, cur, p, p, strlen(typeword));
            p += strlen(typeword);
            continue;
        }

        char *assignment = is_assign(p);
        if (assignment) {
            cur = new_token(TK_RESERVED, cur, p, p, strlen(assignment));
            p += strlen(assignment);
            continue;
        }

        if (strncmp(p, "++", 2) == 0 || strncmp(p, "--", 2) == 0) {
            cur = new_token(TK_RESERVED, cur, p, p, 2);
            p += 2;
            continue;
        }

        if (strncmp(p, "->", 2) == 0) {
            cur = new_token(TK_RESERVED, cur, p, p, 2);
            p += 2;
            continue;
        }

        if (strchr("+-*/%(){}[]><=!?,.:;&*", *p)) {
            cur = new_token(TK_RESERVED, cur, p, p, 1);
            p++;
            continue;
        }

        // read character
        if (*p == '\'') {
            char *start = p + 1;

            char c;
            if (*start == '\\') {
                c = is_escape(start+1);
                start += 2;
            } else {
                c = *start++;
            }

            if (*start != '\'')
                error_at(p, "unclosed char");

            cur = new_token(TK_NUM, cur, p, p, 0);
            cur->val = c;
            cur->len = start - p;
            p += cur->len+1;
            continue;
        }

        // read string literal
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
                    buf[len++] = is_escape(start+1);
                    start += 2;
                } else {
                    buf[len++] = *start++;
                }
            }
            buf[len++] = '\0';

            cur = new_token(TK_STR, cur, p, buf, end-p);
            cur->strlen = len;
            p += end-p+1;
            continue;
        }

        if (is_alpha(*p)) {
            char *q = p;
            while (is_alnum(*p))
                p++;
            cur = new_token(TK_IDENT, cur, q, q, p-q);
            continue;
        }

        if (isdigit(*p)) {
            cur = new_token(TK_NUM, cur, p, p, 0);
            cur->val = strtol(p, &p, 10);
            cur->len = num_of_digits(cur->val);
            continue;
        }

        error_at(p, "unable tokenize\n");
    }

    new_token(TK_EOF, cur, p, p, 1);
    add_line_numbers(head.next);
    token = head.next;
}
