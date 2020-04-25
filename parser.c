#include "mmcc.h"

// Parser

Node *new_node(Nodekind kind, Node *lhs, Node *rhs) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = kind;
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

Node *new_node_num(int val) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_NUM;
    node->val = val;
    return node;
}

LVar *locals;

LVar *find_lvar(Token *tok) {
    for (LVar *lvar = locals; lvar; lvar=lvar->next)
        if (lvar->len == tok->len && !strncmp(tok->str, lvar->name, lvar->len))
            return lvar;
    return NULL;
}

Node *code[100];

void program();
Node *stmt();
Node *cond();
Node *expr();
Node *assign();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *primary();

// program = stmt*
void program() {
    locals = calloc(1, sizeof(LVar));
    locals->offset = 0;

    int i = 0;
    while (!at_eof()) {
        code[i++] = stmt();
    }
    code[i] = NULL;
}

// stmt = "return" expr ";"
//        | "{" stmt* "}"
//        | "if" "(" cond ")" stmt ( "else" stmt )?
//        | "while" "(" cond ")" stmt
//        | "for" "(" expr? ";" expr? ";" expr? ")" stmt
//        | expr ";"
Node *stmt() {
    Node *node;

    if (consume("{")) {
        node = calloc(1, sizeof(Node));
        node->kind = ND_BLOCK;
        int i = 0;
        while (!consume("}")) {
            node->blocks[i++] = stmt();
        }
        node->blocks[i] = NULL;
        return node;
    }

    if (consume_return()) {
        node = calloc(1, sizeof(Node));
        node->kind = ND_RET;
        node->lhs = expr();
    } else if (consume_if()) {
        node = calloc(1, sizeof(Node));
        node->kind = ND_IF;
        expect("(");
        node->cond = cond();
        expect(")");
        node->then = stmt();
        if (consume_else())
            node->els = stmt();
        return node;
    } else if (consume_while()) {
        node = calloc(1, sizeof(Node));
        node->kind = ND_WHILE;
        expect("(");
        node->cond = cond();
        expect(")");
        node->then = stmt();
        return node;
    } else if (consume_for()) {
        node = calloc(1, sizeof(Node));
        node->kind = ND_FOR;
        expect("(");

        if (consume(";")) {
            node->preop = NULL;
        } else {
            node->preop = expr();
            expect(";");
        }

        if (consume(";")) {
            node->cond = NULL;
        } else {
            node->cond = expr();
            expect(";");
        }

        if (consume(")")) {
            node->postop = NULL;
        } else {
            node->postop = expr();
            expect(")");
        }

        node->then = stmt();
        return node;
    } else {
        node = expr();
    }

    expect(";");

    return node;
}

// cond = expr
Node *cond() {
    return expr();
}

// expr = assign
Node *expr() {
    return assign();
}

// assign = equality ( "=" assign )?
Node *assign() {
    Node *node = equality();

    if (consume("="))
        node = new_node(ND_AS, node, assign());

    return node;
}

// equality = relational ( "==" relational | "!=" relational )*
Node *equality() {
    Node *node = relational();

    for (;;) {
        if (consume("=="))
            node = new_node(ND_EQ, node, relational());
        else if (consume("!="))
            node = new_node(ND_NE, node, relational());
        else
            return node;
    }
}

// relational = add ( ">" add | "<" add | ">=" add | "<=" add )*
Node *relational() {
    Node *node = add();

    for (;;) {
        if (consume(">="))
            node = new_node(ND_OM, node, add());
        else if (consume("<="))
            node = new_node(ND_OL, node, add());
        else if (consume(">"))
            node = new_node(ND_MT, node, add());
        else if (consume("<"))
            node = new_node(ND_LT, node, add());
        else
            return node;
    }
}

// add = mul ( "+" mul | "-" mul )*
Node *add() {
    Node *node = mul();

    for (;;) {
        if (consume("+"))
            node = new_node(ND_ADD, node, mul());
        else if (consume("-"))
            node = new_node(ND_SUB, node, mul());
        else
            return node;
    }
}

// mul = unary ( '*' unary | '/' unary )*
Node *mul() {
    Node *node = unary();

    for (;;) {
        if (consume("*"))
            node = new_node(ND_MUL, node, unary());
        else if (consume("/"))
            node = new_node(ND_DIV, node, unary());
        else
            return node;
    }
}

// unary = ( '+' | '-' )? primary
Node *unary() {
    if (consume("+"))
        return unary();
    if (consume("-"))
        return new_node(ND_SUB, new_node_num(0), unary());

    return primary();
}

// primary = '(' expr ')' | ident | num
Node *primary() {
    if (consume("(")) {
        Node *node = expr();
        expect(")");
        return node;
    }

    Token *tok = consume_ident();
    if (tok) {
        Node *node = calloc(1, sizeof(Node));
        node->kind = ND_LV;

        LVar *lvar = find_lvar(tok);
        if (lvar) {
            node->offset = lvar->offset;
        } else {
            lvar = calloc(1, sizeof(LVar));
            lvar->next = locals;
            lvar->name = tok->str;
            lvar->len = tok->len;
            lvar->offset = locals->offset + 8;
            node->offset = lvar->offset;
            locals = lvar;
        }

        return node;
    }

    return new_node_num(expect_number());
}
